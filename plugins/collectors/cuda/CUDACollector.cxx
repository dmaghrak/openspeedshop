////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014 Argo Navis Technologies. All Rights Reserved.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the Free
// Software Foundation; either version 2.1 of the License, or (at your option)
// any later version.
//
// This library is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
// details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
////////////////////////////////////////////////////////////////////////////////

/** @file Definition of the CUDACollector class. */

#include <boost/algorithm/string/predicate.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/ref.hpp>
#include <cstring>
#include <utility>
 
#include "AddressRange.hxx"
#include "Path.hxx"
#include "StackTrace.hxx"

#include "CUDACollector.hxx"
#include "CUDAExecDetail.hxx"
#include "CUDAXferDetail.hxx"

using namespace boost;
using namespace OpenSpeedShop::Framework;
using namespace std;



/** Anonymous namespace hiding implementation details. */
namespace {

    /** Type returned for the CUDA kernel execution detail metrics. */
    typedef map<StackTrace, vector<CUDAExecDetail> > ExecDetails;

    /** Type returned for the CUDA data transfer detail metrics. */
    typedef map<StackTrace, vector<CUDAXferDetail> > XferDetails;

    /** Type of function invoked when visiting individual addresses. */
    typedef function<void (uint64_t)> AddressVisitor;

    /** Type of function invoked to process a completed kernel execution. */
    typedef function<
        void (const CUDACollector::Request&, const CUDA_ExecutedKernel&)
        > ExecutedKernelVisitor;

    /** Type of function invoked to process a completed memory copy. */
    typedef function<
        void (const CUDACollector::Request&, const CUDA_CopiedMemory&)
        > MemoryCopyVisitor;

    /** Type of function invoked when visiting individual CUDA messages. */
    typedef function<
        void (const CBTF_cuda_data&, const CBTF_cuda_message&)
        > MessageVisitor;
    
    /** TEMPORARY DEBUGGING FUNCTION */
    void debug(const StackTrace& trace)
    {
        std::cout << endl;
        for (int i = 0; i < trace.size(); ++i)
        {
            pair<bool, Function> function = trace.getFunctionAt(i);
            pair<bool, LinkedObject> linked_object = trace.getLinkedObjectAt(i);
            
            std::cout << "[" << i << "] "
                      << (function.first ? function.second.getName() : "?")
                      << " ("
                      << (linked_object.first ? 
                          linked_object.second.getPath().getBaseName() : "?")
                      << ")" << std::endl;
        }
    }
    
    /** Convert a CUDA request call site into a stack trace. */
    StackTrace toStackTrace(const Thread& thread, const Time& time,
                            const CUDACollector::Request& request)
    {
        StackTrace trace(thread, time);

        // Initially include ALL frames for the call site of this CUDA request
        for (int i = 0; i < request.call_site.size(); ++i)
        {
            trace.push_back(request.call_site[i]);
        }
        
        // TEMPORARY DEBUGGING CODE
        std::cout << std::endl << "---" << std::endl;
        debug(trace);
        
        //
        // Trim all of the frames that preceeded the actual entry into main(),
        // assuming, of course, that main() can actually be found.
        //
        
        for (int i = 0; i < trace.size(); ++i)
        {
            pair<bool, Function> function = trace.getFunctionAt(i);
            if (function.first && (function.second.getName() == "main"))
            {
                trace.erase(trace.begin() + i + 1, trace.end());
                break;
            }            
        }

        // TEMPORARY DEBUGGING CODE
        debug(trace);

        //
        // Now iterate over the frames from main() towards the final call site,
        // looking for the first frame that is part of the CUDA implementation
        // or our collector. Trim the stack trace from that point all the way
        // to the final call site.
        //
        
        for (int i = trace.size(); i > 0; --i)
        {
            pair<bool, LinkedObject> linked_object = trace.getLinkedObjectAt(i);
            if (linked_object.first)
            {
                Path base_name = linked_object.second.getPath().getBaseName();
                if (starts_with(base_name, "cuda-collector") ||
                    starts_with(base_name, "libcu"))
                {
                    trace.erase(trace.begin(), trace.begin() + i + 1);
                    break;
                }
            }
            
            pair<bool, Function> function = trace.getFunctionAt(i);
            if (function.first &&
                starts_with(function.second.getName(), "__device_stub"))
            {
                trace.erase(trace.begin(), trace.begin() + i + 1);
                break;
            }
        }
        
        // TEMPORARY DEBUGGING CODE
        debug(trace);

        return trace;
    }
    
    /** Visitor used to compute the "exec_time" or "xfer_time" metric. */
    template <typename T>
    void computeTime(const CUDACollector::Request& request,
                     const T& completion,
                     const Thread& thread,
                     const ExtentGroup& subextents,
                     vector<double>& data)
    {
        TimeInterval interval(Time(completion.time_begin),
                              Time(completion.time_end));
        
        StackTrace trace = toStackTrace(thread, interval.getBegin(), request);
        
        set<ExtentGroup::size_type> intersection = 
            subextents.getIntersectionWith(
                Extent(interval, AddressRange(trace[0]))
                );
        
        for (set<ExtentGroup::size_type>::const_iterator
                 i = intersection.begin(); i != intersection.end(); ++i)
        {
            data[*i] += static_cast<double>(
                (interval & subextents[*i].getTimeInterval()).getWidth()
                ) / 1000000000.0;
        }
    }

    /**
     * Visitor extracting the unique addresses referenced by a CUDA message.
     */
    void extractUniqueAddresses(const CBTF_cuda_data& data,
                                const CBTF_cuda_message& message,
                                const AddressVisitor& visitor)
    {
        switch (message.type)
        {
            
        case EnqueueRequest:
            {
                const CUDA_EnqueueRequest& msg = 
                    message.CBTF_cuda_message_u.enqueue_request;
                
                for (uint32_t i = msg.call_site;
                     (i < data.stack_traces.stack_traces_len) &&
                         (data.stack_traces.stack_traces_val[i] != 0);
                     ++i)
                {
                    visitor(data.stack_traces.stack_traces_val[i]);
                }
            }
            break;
            
        case OverflowSamples:
            {
                const CUDA_OverflowSamples& msg =
                    message.CBTF_cuda_message_u.overflow_samples;
                
                for (uint32_t i = 0; i < msg.pcs.pcs_len; ++i)
                {
                    visitor(msg.pcs.pcs_val[i]);
                }
            }
            break;
            
        default:
            break;
        }
    }

    /**
     * Visitor handling an asynchronously executed CUDA request described by a
     * CUDA message. Manages pushing individual requests onto, and popping them
     * off of, the given thread-specific data's table of pending requests.
     */
    void handleRequests(const CBTF_cuda_data& data,
                        const CBTF_cuda_message& message,
                        CUDACollector::ThreadSpecificData& tsd,
                        ExecutedKernelVisitor& executed_kernel_visitor,
                        MemoryCopyVisitor& memory_copy_visitor)
    {
        switch (message.type)
        {
            
        case CopiedMemory:
            {
                const CUDA_CopiedMemory& msg =
                    message.CBTF_cuda_message_u.copied_memory;

                if (memory_copy_visitor)
                {
                    for (list<CUDACollector::Request>::iterator 
                             i = tsd.requests.begin();
                         i != tsd.requests.end();
                         ++i)
                    {
                        if ((i->message.type == MemoryCopy) &&
                            (i->message.context == msg.context) &&
                            (i->message.stream == msg.stream))
                        {
                            memory_copy_visitor(*i, msg);
                            tsd.requests.erase(i);
                            break;
                        }
                    }
                }
            }
            break;
            
        case EnqueueRequest:
            {
                const CUDA_EnqueueRequest& msg = 
                    message.CBTF_cuda_message_u.enqueue_request;

                CUDACollector::Request request;
                request.message = msg;
                
                for (uint32_t i = msg.call_site;
                     (i < data.stack_traces.stack_traces_len) &&
                         (data.stack_traces.stack_traces_val[i] != 0);
                     ++i)
                {
                    request.call_site.push_back(
                        data.stack_traces.stack_traces_val[i]
                        );
                }
                
                tsd.requests.push_back(request);
            }
            break;
            
        case ExecutedKernel:
            {
                const CUDA_ExecutedKernel& msg =
                    message.CBTF_cuda_message_u.executed_kernel;

                if (executed_kernel_visitor)
                {
                    for (list<CUDACollector::Request>::iterator 
                             i = tsd.requests.begin();
                         i != tsd.requests.end();
                         ++i)
                    {
                        if ((i->message.type == LaunchKernel) &&
                            (i->message.context == msg.context) &&
                            (i->message.stream == msg.stream))
                        {
                            executed_kernel_visitor(*i, msg);
                            tsd.requests.erase(i);
                            break;
                        }
                    }
                }
            }
            break;
            
        default:
            break;
        }
    }
    
    /** Visitor used to insert an address into a buffer of unique addresses. */
    void insertIntoAddressBuffer(uint64_t address, PCBuffer& buffer)
    {
        UpdatePCBuffer(address, &buffer);
    }

    /** Visitor used to insert an address into a set of unique addresses. */
    void insertIntoAddressSet(uint64_t address, set<Address>& addresses)
    {
        addresses.insert(address);
    }
    
    /**
     * Visit the individual CUDA messages packed within a performance data blob.
     */
    void visit(const Blob& blob, const MessageVisitor& visitor)
    {
        CBTF_cuda_data data;
        memset(&data, 0, sizeof(data));

        blob.getXDRDecoding(
            reinterpret_cast<xdrproc_t>(xdr_CBTF_cuda_data), &data
            );
        
        for (u_int i = 0; i < data.messages.messages_len; ++i)
        {
            visitor(data, data.messages.messages_val[i]);
        }
        
        xdr_free(reinterpret_cast<xdrproc_t>(xdr_CBTF_cuda_data),
                 reinterpret_cast<char*>(&data));        
    }
        
} // namespace <anonymous>



/**
 * Collector's factory method.
 *
 * Factory method for instantiating a collector implementation. This is the
 * only function that is externally visible from outside the collector plugin.
 *
 * @return    New instance of this collector's implementation.
 */
extern "C" CollectorImpl* cuda_LTX_CollectorFactory()
{
    return new CUDACollector();
}



/**
 * Default constructor.
 *
 * Constructs a new CUDA collector with the proper metadata.
 */
CUDACollector::CUDACollector() :
    CollectorImpl("cuda", "CUDA",
                  "Intercepts all calls to CUDA memory copy/set and kernel "
                  "executions and records, for each call, the current stack "
                  "trace and start/end time, as well as additional relevant "
                  "information depending on the operation. In addition, has "
                  "the ability to periodically sample hardware event counts "
                  "via PAPI for both the CPU and GPU."),
    dm_threads()
{
    // Declare our metrics
    declareMetric(Metadata("exec_time",
                           "CUDA Kernel Execution Time",
                           "CUDA kernel execution time in seconds.",
                           typeid(double)));
    declareMetric(Metadata("exec_inclusive_details",
                           "Inclusive CUDA Kernel Execution Details",
                           "Inclusive CUDA kernel execution details.",
                           typeid(ExecDetails)));
    declareMetric(Metadata("exec_exclusive_details",
                           "Exclusive CUDA Kernel Execution Details",
                           "Exclusive CUDA kernel execution details.",
                           typeid(ExecDetails)));

    declareMetric(Metadata("xfer_time",
                           "CUDA Data Transfer Time",
                           "CUDA data transfer time in seconds.",
                           typeid(double)));
    declareMetric(Metadata("xfer_inclusive_details",
                           "Inclusive CUDA Data Transfer Details",
                           "Inclusive CUDA data transfer details.",
                           typeid(XferDetails)));
    declareMetric(Metadata("xfer_exclusive_details",
                           "Exclusive CUDA Data Transfer Details",
                           "Exclusive CUDA data transfer details.",
                           typeid(XferDetails)));

    // TODO: Add additional metrics for hardware counter sampling data...
}



/**
 * Get the default parameter values.
 *
 * Implement getting our default parameter values.
 *
 * @return    Blob containing the default parameter values.
 */
Blob CUDACollector::getDefaultParameterValues() const
{
    // This method's implementation is intentionally empty.
    //
    // The CUDA collector is only available for the CBTF-based data collection
    // model. And that data collection model doesn't use this method.

    return Blob();
}



/**
 * Get a parameter value.
 *
 * Implement getting one of our parameter values.
 *
 * @param parameter    Unique identifier of the parameter.
 * @param data         Blob containing the parameter values.
 * @retval ptr         Untyped pointer to the parameter value.
 */
void CUDACollector::getParameterValue(const string& parameter,
                                      const Blob& data, void* ptr) const
{
    // This method's implementation is intentionally empty.
    //
    // The CUDA collector is only available for the CBTF-based data collection
    // model. And that data collection model doesn't use this method.
}



/**
 * Set a parameter value.
 *
 * Implements setting one of our parameter values.
 *
 * @param parameter    Unique identifier of the parameter.
 * @param ptr          Untyped pointer to the parameter value.
 * @retval data        Blob containing the parameter values.
 */
void CUDACollector::setParameterValue(const string& parameter,
                                      const void* ptr, Blob& data) const
{
    // This method's implementation is intentionally empty.
    //
    // The CUDA collector is only available for the CBTF-based data collection
    // model. And that data collection model doesn't use this method.
}



/**
 * Start data collection.
 *
 * Implement starting data collection for the specified threads.
 *
 * @param collector    Collector starting data collection.
 * @param threads      Threads for which to start collecting data.
 */
void CUDACollector::startCollecting(const Collector& collector,
                                    const ThreadGroup& threads) const
{
    // This method's implementation is intentionally empty.
    //
    // The CUDA collector is only available for the CBTF-based data collection
    // model. And that data collection model doesn't use this method.
}



/**
 * Stops data collection.
 *
 * Implement stopping data collection for the specified threads.
 *
 * @param collector    Collector stopping data collection.
 * @param threads      Threads for which to stop collecting data.
 */
void CUDACollector::stopCollecting(const Collector& collector,
                                   const ThreadGroup& threads) const
{
    // This method's implementation is intentionally empty.
    //
    // The CUDA collector is only available for the CBTF-based data collection
    // model. And that data collection model doesn't use this method.
}



/**
 * Get metric values.
 *
 * Implements getting one of this collector's metric values over all subextents
 * of the specified extent for a particular thread, for one of the collected
 * performance data blobs.
 *
 * @param metric        Unique identifier of the metric.
 * @param collector     Collector for which to get values.
 * @param thread        Thread for which to get values.
 * @param extent        Extent of the performance data blob.
 * @param blob          Blob containing the performance data.
 * @param subextents    Subextents for which to get values.
 * @retval ptr          Untyped pointer to the values of the metric.
 */
void CUDACollector::getMetricValues(const string& metric,
                                    const Collector& collector,
                                    const Thread& thread,
                                    const Extent& extent,
                                    const Blob& blob,
                                    const ExtentGroup& subextents,
                                    void* ptr) const
{
    //
    // Locate the thread-specific data belonging to the thread for which this
    // performance data blob was gathered, creating new thread-specific data
    // when necessary.
    //

    ThreadTable::iterator t = dm_threads.find(thread);
    
    if (t == dm_threads.end())
    {
        t = dm_threads.insert(make_pair(thread, ThreadSpecificData())).first;
    }

    //
    // CUDA allows requests to be executed asynchronously. Thus the issue and
    // completion of a given request can be found within 2 separate blobs. As
    // a consequence, it is necessary to have a stateful collector in order to
    // correctly compute certain metrics. But the collector API isn't designed
    // for that. I.e. there is no direct indication of whether a given call to
    // getMetricValues() is part of a previous or new metric computation. The
    // fix employed here is to assume:
    //
    //     * Each blob is seen once per metric computation.
    //     * A blob can be unique identified by its extent.
    //     * Blobs are visited in time order.
    //
    // And so when a given blob's extent is seen a 2nd time, it can be assumed
    // that a new metric computation has begun and the thread-specific data for
    // the thread should be cleared to reset the state.
    //

    if (t->second.extents.find(extent) != t->second.extents.end())
    {
        t->second = ThreadSpecificData();
    }

    t->second.extents.insert(extent);

    //
    // Compute the requested metric by processing the individual CUDA messages
    // in the specified performance data blob. This involves constructing a set
    // of appropriate visitors, chaining them together, and then applying them.
    //

    if (metric == "exec_time")
    {
        ExecutedKernelVisitor executed_kernel_visitor = bind(
            computeTime<CUDA_ExecutedKernel>, _1, _2,
            ref(thread), ref(subextents),
            ref(*reinterpret_cast<vector<double>*>(ptr))
            );
        
        MemoryCopyVisitor memory_copy_visitor;
        
        MessageVisitor visitor = bind(
            handleRequests, _1, _2, t->second, 
            ref(executed_kernel_visitor), ref(memory_copy_visitor)
            );
        
        visit(blob, visitor);
    }
    else if (metric == "exec_inclusive_details")
    {
        // ...
    }
    else if (metric == "exec_exclusive_details")
    {
        // ...
    }

    else if (metric == "xfer_time")
    {
        ExecutedKernelVisitor executed_kernel_visitor;
        
        MemoryCopyVisitor memory_copy_visitor = bind(
            computeTime<CUDA_CopiedMemory>, _1, _2,
            ref(thread), ref(subextents),
            ref(*reinterpret_cast<vector<double>*>(ptr))
            );
        
        MessageVisitor visitor = bind(
            handleRequests, _1, _2, t->second, 
            ref(executed_kernel_visitor), ref(memory_copy_visitor)
            );
        
        visit(blob, visitor);
    }
    else if (metric == "xfer_inclusive_details")
    {
        // ...
    }
    else if (metric == "xfer_exclusive_details")
    {
        // ...
    }
}



/**
 * Get unique PC values.
 *
 * Implements getting all of the unique program counter (PC) values encountered
 * for a particular thread, for one of the collected performance data blobs.
 *
 * @param thread     Thread for which to get unique PC values.
 * @param blob       Blob containing the performance data.
 * @retval buffer    Buffer of the unique PC values.
 */
void CUDACollector::getUniquePCValues(const Thread& thread,
                                      const Blob& blob,
                                      PCBuffer* buffer) const
{
    AddressVisitor inserter = bind(insertIntoAddressBuffer, _1, ref(*buffer));
    MessageVisitor visitor = bind(extractUniqueAddresses, _1, _2, inserter);
    visit(blob, visitor);
}



/**
 * Get unique PC values.
 *
 * Implements getting all of the unique program counter (PC) values encountered
 * for a particular thread, for one of the collected performance data blobs.
 *
 * @param thread        Thread for which to get unique PC values.
 * @param blob          Blob containing the performance data.
 * @retval addresses    Set of unique PC values.
 */
void CUDACollector::getUniquePCValues(const Thread& thread,
                                      const Blob& blob,
                                      set<Address>& addresses) const
{
    AddressVisitor inserter = bind(insertIntoAddressSet, _1, ref(addresses));
    MessageVisitor visitor = bind(extractUniqueAddresses, _1, _2, inserter);
    visit(blob, visitor);
}
