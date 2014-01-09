////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013,2014 The Krell Institute. All Rights Reserved.
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

/** @file
 *
 * Definition of the DyninstSymbols namespace.
 *
 */

#include "Address.hxx"
#include "AddressRange.hxx"
#include "DyninstSymbols.hxx"
#include "LinkedObject.hxx"

#include <BPatch.h>
#include <BPatch_addressSpace.h>
#include <BPatch_basicBlock.h>
#include <BPatch_basicBlockLoop.h>
#include <BPatch_flowGraph.h>
#include <BPatch_function.h>
#include <BPatch_image.h>
#include <BPatch_module.h>
#include <BPatch_Vector.h>

using namespace OpenSpeedShop::Framework;



#ifndef NDEBUG
/** Flag indicating if debugging for loop information is enabled. */
bool is_debug_enabled = (getenv("OPENSS_DEBUG_DYNINSTAPI_LOOPS") != NULL);
#endif



/** Find the loop containing the specified address. */
bool findLoop(const Address& address,
              BPatch_image& image,
              Address& loop_head,
              std::vector<AddressRange>& loop_ranges)
{
    // Iterate over each module within the specified image
    
    BPatch_Vector<BPatch_module*>* modules = image.getModules();
    
    if (modules == NULL)
    {
        return false;
    }
    
    for (unsigned int m = 0; m < modules->size(); ++m)
    {
        BPatch_module* module = (*modules)[m];
        
        if (module == NULL)
        {
            continue;
        }
        
        Address module_base = (uint64_t)module->getBaseAddr();

        // Find the function(s) containing the specified address

        BPatch_Vector<BPatch_function*> functions;
        module->findFunctionByAddress(
            (void*)((module_base + address).getValue()), functions, false, false
            );
        
        for (unsigned int f = 0; f < functions.size(); ++f)
        {
            BPatch_function* function = functions[f];
            
            if (function == NULL)
            {
                continue;
            }

            // Find the loop containing the specified address
            
            BPatch_flowGraph* cfg = function->getCFG();
            
            if (cfg == NULL)
            {
                continue;
            }
            
            BPatch_Vector<BPatch_basicBlockLoop*> loops;
            cfg->getLoops(loops);
            
            for (unsigned int l = 0; l < loops.size(); ++l)
            {
                BPatch_basicBlockLoop* loop = loops[l];
                
                if ((loop == NULL) || 
                    !loop->containsAddress((module_base + address).getValue()))
                {
                    continue;
                }
                
                // The loop containing this address has been found! Rejoice!
                // And, of course, obtain the loop's head address and basic
                // block address ranges...
                
                BPatch_basicBlock* head = loop->getLoopHead();
                
                if (head == NULL)
                {
                    continue;
                }
                
                loop_head = Address(head->getStartAddress()) - module_base;

                BPatch_Vector<BPatch_basicBlock*> blocks;
                loop->getLoopBasicBlocks(blocks);
                
                for (unsigned int i = 0; i < blocks.size(); ++i)
                {
                    BPatch_basicBlock* block = blocks[i];

                    if (block != NULL)
                    {
                        loop_ranges.push_back(
                            AddressRange(
                                Address(block->getStartAddress()) - module_base,
                                Address(block->getEndAddress()) - module_base
                                )
                            );
                    }
                }
                
                return true;

            } // l
        } // f
    } // m

    return false;
}



/**
 * Get loops for a linked object.
 *
 * Get loop information for the specified linked object and add it to the given
 * symbol tables. Only obtain information about those loops which contain one or
 * more of the specified unique addresses.
 *
 * @param unique_addresses    Restrict the collected information to loops
 *                            that contain one or more of these addresses.
 * @param linked_object       Linked object for which to get loops.
 * @param symbol_tables       Symbol tables to contain the loop information.
 */
void DyninstSymbols::getLoops(const std::set<Address>& unique_addresses,
                              const LinkedObject& linked_object,
                              SymbolTableMap& symbol_tables)
{
    // Find the specified linked object's possible address ranges
    std::set<AddressRange> ranges = linked_object.getAddressRange();

    // Open the specified linked object with Dyninst
    
    BPatch bpatch;
    
    BPatch_addressSpace* space = bpatch.openBinary(
        linked_object.getPath().c_str(), false
        );
    
    if (space == NULL)
    {
        std::cout << "WARNING: Dyninst failed to open the linked object \"" 
                  << linked_object.getPath() << "\"!" << std::endl;
        return;
    }
    
    BPatch_image* image = space->getImage();
    
    if (image == NULL)
    {
        std::cout << "WARNING: Dyninst failed to open the linked object \"" 
                  << linked_object.getPath() << "\"!" << std::endl;
        return;
    }

    // Make a copy of the specified set of unique addresses and use the copy
    // to track which addresses have been resolved to their containing loop.
    // Keep iterating until the copy is empty.
    
    std::set<Address> unique_addresses_copy(unique_addresses);
    
    while (!unique_addresses_copy.empty())
    {
#ifndef NDEBUG
        if (is_debug_enabled)
        {
            std::cout << std::endl;
            for (std::set<Address>::const_iterator
                     i = unique_addresses_copy.begin();
                 i != unique_addresses_copy.end();
                 ++i)
            {
                std::cout << "    Unique: " << *i << std::endl;
            }
        }
#endif

        Address next = *unique_addresses_copy.begin();

        // Find which locations (if any) of the specified linked object
        // contain this address. Since the linked object might be found
        // at multiple different base addresses, more than one location
        // may be found.
        
        for (std::set<AddressRange>::const_iterator
                 i = ranges.begin(); i != ranges.end(); ++i)
        {
            if (!i->doesContain(next))
            {
                continue;
            }

            // Find which symbol table (if any) correponds to this address
            // range. It is unclear to me why this is necessary, but Don's
            // code in SymtabAPISymbols does this, so when in Rome...
                
            if (symbol_tables.find(*i) == symbol_tables.end())
            {
                continue;
            }
            
            SymbolTable& symbol_table = symbol_tables.find(*i)->second.first;

            // Find the loop (if any) that contains this address. Note
            // that findLoop() expects an address which is relative to
            // the beginning of the linked object.
            
            Address loop_head;
            std::vector<AddressRange> loop_ranges;
            
            if (findLoop(next - i->getBegin(), *image, loop_head, loop_ranges))
            {
#ifndef NDEBUG
                if (is_debug_enabled)
                {
                    std::cout << std::endl;
                }
#endif

                // Add this loop's address ranges to the symbol table. Note
                // that findLoop() returns addresses which are relative to
                // the beginning of the linked object.
                
                Address head(i->getBegin() + loop_head);
                
                for (std::vector<AddressRange>::const_iterator 
                         j = loop_ranges.begin(); j != loop_ranges.end(); ++j)
                {
                    Address begin(i->getBegin() + j->getBegin());
                    Address end(i->getBegin() + j->getEnd());

#ifndef NDEBUG
                    if (is_debug_enabled)
                    {
                        std::cout << "    Loop: " << head << ", " 
                                  << AddressRange(begin, end) << std::endl;
                    }
#endif
                    
                    symbol_table.addLoop(begin, end, head);

                    // Now erase from our copy of the unique addresses all
                    // such addresses which are covered by this loop. There
                    // is no need to look them up again since they have now
                    // been resolved.
                    
                    unique_addresses_copy.erase(
                        unique_addresses_copy.lower_bound(begin),
                        unique_addresses_copy.upper_bound(end)
                        );

                } // j
            }
        } // i

        // If this address couldn't be resolved it will still be found in
        // our copy of unique addresses. And if isn't erased, we'll simply
        // loop endlessly trying to resolve that same address.
        
        unique_addresses_copy.erase(next);
        
    } // while (!unique_addresses_copy.empty())

#ifndef NDEBUG
    if (is_debug_enabled && !unique_addresses.empty())
    {
        std::cout << std::endl;
    }
#endif
}