////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 Silicon Graphics, Inc. All Rights Reserved.
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
 * Definition of the MainLoop namespace.
 *
 */

#include "Assert.hxx"
#include "MainLoop.hxx"

#include <dpcl.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

using namespace OpenSpeedShop::Framework;



namespace {

    /** Identifier of the monitor thread. */
    pthread_t monitor_tid;
    
    /** Lock used for insuring exclusive access to DPCL client library. */
    pthread_mutex_t exclusive_access_lock = PTHREAD_MUTEX_INITIALIZER;
    
    /** Pipe used to asynchronously interrupt the DPCL main loop. */
    int pipe_fds[2] = { -1, -1 };
    
    /** Access-controlled flag indicating if monitor thread is suspended. */
    struct {
	bool flag;             /**< Actual flag value. */
	pthread_mutex_t lock;  /**< Mutual exclusion lock for this flag. */
	pthread_cond_t cv;     /**< Condition variable for this flag. */
    } is_suspended = {
	false, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER
    };

    /** Flag indicating monitor should exit upon next resume. */
    bool do_exit_upon_resume = false;
    

    
    /**
     * Interrupt pipe handler.
     *
     * Called by the DPCL main loop when incoming data is available on the
     * pipe used for asynchronously interrupting that loop. The incoming message
     * is first read and ignored. Then we ask the DPCL main loop to terminate at
     * its earliest convenience.
     *
     * @param fd    File descriptor with incoming data.
     * @return      Number of bytes read from the file descriptor.
     */
    int pipeHandler(int fd)
    {
	// Read one-byte message from the passed file descriptor
	char message;	
	while(1) {
	    int retval = read(fd, &message, 1);
	    Assert((retval >= 0) || ((retval == -1) && (errno == EINTR)));
	    if(retval > 0)
		break;
	}
	
	// Request that the DPCL main loop end
	Ais_end_main_loop();
	
	// Return the number of bytes read to the caller
	return 1;
    }
    


    /**       
     * Monitor thread function.
     *
     * Monitor thread for executing the DPCL main loop. By executing the loop
     * in a separate thread, it is insured that incoming DPCL messages will be
     * handled in a timely manner. The monitor simply sits in Ais_main_loop()
     * until it is asynchronously interrupted. When such an interrupt occurs,
     * we inform the interrupter that the DPCL main loop is suspended and then
     * wait for the go-ahead to resume the loop again.
     */
    void* monitorThread(void*)
    {
	// Run the DPCL main loop indefinitely
	while(true) {
	    
	    // Call the DPCL main loop
	    Ais_main_loop();
	    
	    // Indicate that we've suspended
	    Assert(pthread_mutex_lock(&is_suspended.lock) == 0);
	    is_suspended.flag = true;
	    Assert(pthread_mutex_unlock(&is_suspended.lock) == 0);
	    Assert(pthread_cond_signal(&is_suspended.cv) == 0);
	    
	    // Wait for go-ahead to resume the DPCL main loop
	    Assert(pthread_mutex_lock(&is_suspended.lock) == 0);
	    while(is_suspended.flag == true)
		Assert(pthread_cond_wait(&is_suspended.cv,
					 &is_suspended.lock) == 0);
	    Assert(pthread_mutex_unlock(&is_suspended.lock) == 0);
	    
	    // Exit monitor thread if instructed to do so
	    if(do_exit_upon_resume)
		break;
	    
	}
	
	// Empty, unused, return value from this thread
	return NULL;
    }
    
    
    
}



/**
 * Start the DPCL main loop.
 *
 * Initializes the DPCL client library and initiates a monitor thread for
 * executing the DPCL main loop. A pipe is established to the monitor thread
 * that allows main threads to temporarily suspend the DPCL main loop when
 * necessary.
 */
void MainLoop::start()
{
    // Initialize the DPCL client library
    Ais_initialize();
    char host[256];
    Assert(gethostname(host, 256) == 0);    
    Ais_blog_on(host, LGL_detail, LGD_daemon, NULL, 0);
    
    // Setup pipe used for asynchronously interrupting the DPCL main loop
    Assert(pipe(pipe_fds) == 0);
    
    // Add a DPCL handler for the read end of the above pipe
    AisStatus retval = Ais_add_fd(pipe_fds[0], pipeHandler);
    Assert(retval.status() == ASC_success);
    
    // Create the monitor thread
    Assert(pthread_create(&monitor_tid, NULL, monitorThread, NULL) == 0);
}



/**
 * Suspend the DPCL main loop.
 *
 * Acquire exclusive access to the DPCL client library. That library is
 * predominantly thread-unsafe, so most of its functions cannot be called
 * by multiple threads concurrently. The first action taken is to acquire
 * the exclusive-access lock. The monitor thread must then be temporarily
 * suspended as it might be accessing the DPCL client library too.
 */
void MainLoop::suspend()
{	
    // Acquire the exclusive-access lock
    Assert(pthread_mutex_lock(&exclusive_access_lock) == 0);
    
    // Send one-byte message down the pipe to interrupt the DPCL main loop
    char message = 0;
    while(1) {
	int retval = write(pipe_fds[1], &message, 1);
	Assert((retval >= 0) || ((retval == -1) && (errno == EINTR)));
	if(retval > 0)
	    break;
    }
    
    // Wait until the monitor thread has suspended
    Assert(pthread_mutex_lock(&is_suspended.lock) == 0);
    while(is_suspended.flag == false)
	Assert(pthread_cond_wait(&is_suspended.cv,
				 &is_suspended.lock) == 0);
    Assert(pthread_mutex_unlock(&is_suspended.lock) == 0);
}


    
/**
 * Resume the DPCL main loop.
 *
 * Releases exclusive access to the DPCL client library. The monitor thread
 * is given the go-ahead to resume execution of the DPCL main loop and the
 * exclusive-access lock is then released.
 */
void MainLoop::resume()
{
    // Tell the monitor thread to resume the DPCL main loop
    Assert(pthread_mutex_lock(&is_suspended.lock) == 0);
    is_suspended.flag = false;
    Assert(pthread_mutex_unlock(&is_suspended.lock) == 0);
    Assert(pthread_cond_signal(&is_suspended.cv) == 0);
    
    // Release the exclusive-access lock
    Assert(pthread_mutex_unlock(&exclusive_access_lock) == 0);
}



/**
 * Stop the DPCL main loop.
 *
 * Finalize the DPCL client library by exiting the DPCL main loop, instructing
 * the monitor thread to exit, and then waiting for it to do so. Also closes the
 * pipe previously established for suspending the DPCL main loop.
 *
 * @note    A little bit of confusing trickery is utilized here in that we call
 *          suspend() and resume() even though we aren't really using the DPCL
 *          client library. We are using these here because they provided an
 *          effective synchronization point at which to instruct the monitor
 *          thread that it should exit.
 */
void MainLoop::stop()
{
    // Obtain exclusive access to DPCL client library
    suspend();

    // Remove DPCL handler for the read end of the async interupt pipe
    AisStatus retval = Ais_remove_fd(pipe_fds[0]);
    Assert(retval.status() == ASC_success);

    // Close pipe used for asynchronously interrupting the DPCL main loop
    Assert(close(pipe_fds[0]) == 0);
    Assert(close(pipe_fds[1]) == 0);    
    
    // Instruct monitor thread to exit upon resume
    do_exit_upon_resume = true;

    // Release exclusive access to DPCL client library
    resume();
 
    // Wait for monitor thread to actually exit
    Assert(pthread_join(monitor_tid, NULL) == 0);
    
    // Cleanup exit-upon-resume flag
    do_exit_upon_resume = false;
}
