/*
 * Copyright 1991-1998 by Open Software Foundation, Inc. 
 *              All Rights Reserved 
 *  
 * Permission to use, copy, modify, and distribute this software and 
 * its documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation. 
 *  
 * OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE. 
 *  
 * IN NO EVENT SHALL OSF BE LIABLE FOR ANY SPECIAL, INDIRECT, OR 
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT, 
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 */
/*
 * MkLinux
 */
/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */

#include <sys/cdefs.h>
#include <sys/types.h>

#include <stdlib.h>
#include <sys/queue.h>

#include <mach/mach.h>
#include <mach/boolean.h>
#include <mach/kern_return.h>
#include <mach/message.h>
#include <mach/mig_errors.h>
#include <mach/mach_port.h>
#include <mach/mach_vm.h>

#define LIBMACH_OPTIONS	(MACH_SEND_INTERRUPT|MACH_RCV_INTERRUPT)

mach_msg_return_t
mach_msg(msg, option, send_size, rcv_size, rcv_name, timeout, notify)
	mach_msg_header_t *msg;
	mach_msg_option_t option;
	mach_msg_size_t send_size;
	mach_msg_size_t rcv_size;
	mach_port_t rcv_name;
	mach_msg_timeout_t timeout;
	mach_port_t notify;
{
	mach_msg_return_t mr;

	/*
	 * Consider the following cases:
	 *	1) Errors in pseudo-receive (eg, MACH_SEND_INTERRUPTED
	 *	plus special bits).
	 *	2) Use of MACH_SEND_INTERRUPT/MACH_RCV_INTERRUPT options.
	 *	3) RPC calls with interruptions in one/both halves.
	 *
	 * We refrain from passing the option bits that we implement
	 * to the kernel.  This prevents their presence from inhibiting
	 * the kernel's fast paths (when it checks the option value).
	 */

	mr = mach_msg_overwrite_trap(msg, option &~ LIBMACH_OPTIONS,
			   send_size, rcv_size, rcv_name,
			   timeout, notify, MACH_MSG_NULL, 0);
	if (mr == MACH_MSG_SUCCESS)
		return MACH_MSG_SUCCESS;

	if ((option & MACH_SEND_INTERRUPT) == 0)
		while (mr == MACH_SEND_INTERRUPTED)
			mr = mach_msg_overwrite_trap(msg,
				option &~ LIBMACH_OPTIONS,
				send_size, rcv_size, rcv_name,
				timeout, notify, MACH_MSG_NULL, 0);

	if ((option & MACH_RCV_INTERRUPT) == 0)
		while (mr == MACH_RCV_INTERRUPTED)
			mr = mach_msg_overwrite_trap(msg,
				option &~ (LIBMACH_OPTIONS|MACH_SEND_MSG),
				0, rcv_size, rcv_name,
				timeout, notify, MACH_MSG_NULL, 0);

	return mr;
}

mach_msg_return_t
mach_msg_overwrite(msg, option, send_size, rcv_limit, rcv_name, timeout, 
		   notify, rcv_msg, rcv_msg_size)
	mach_msg_header_t *msg;
	mach_msg_option_t option;
	mach_msg_size_t send_size;
	mach_msg_size_t rcv_limit;
	mach_port_t rcv_name;
	mach_msg_timeout_t timeout;
	mach_port_t notify;
	mach_msg_header_t *rcv_msg;
	mach_msg_size_t rcv_msg_size;
{
	mach_msg_return_t mr;

	/*
	 * Consider the following cases:
	 *	1) Errors in pseudo-receive (eg, MACH_SEND_INTERRUPTED
	 *	plus special bits).
	 *	2) Use of MACH_SEND_INTERRUPT/MACH_RCV_INTERRUPT options.
	 *	3) RPC calls with interruptions in one/both halves.
	 *
	 * We refrain from passing the option bits that we implement
	 * to the kernel.  This prevents their presence from inhibiting
	 * the kernel's fast paths (when it checks the option value).
	 */

	mr = mach_msg_overwrite_trap(msg, option &~ LIBMACH_OPTIONS,
			   send_size, rcv_limit, rcv_name,
			   timeout, notify, rcv_msg, rcv_msg_size);
	if (mr == MACH_MSG_SUCCESS)
		return MACH_MSG_SUCCESS;

	if ((option & MACH_SEND_INTERRUPT) == 0)
		while (mr == MACH_SEND_INTERRUPTED)
			mr = mach_msg_overwrite_trap(msg,
				option &~ LIBMACH_OPTIONS,
				send_size, rcv_limit, rcv_name,
				timeout, notify, rcv_msg, rcv_msg_size);

	if ((option & MACH_RCV_INTERRUPT) == 0)
		while (mr == MACH_RCV_INTERRUPTED)
			mr = mach_msg_overwrite_trap(msg,
				option &~ (LIBMACH_OPTIONS|MACH_SEND_MSG),
				0, rcv_limit, rcv_name,
				timeout, notify, rcv_msg, rcv_msg_size);

	return mr;
}

mach_msg_return_t
mach_msg_send(mach_msg_header_t *msg)
{
	return mach_msg(msg, MACH_SEND_MSG,
			msg->msgh_size, 0, MACH_PORT_NULL,
			MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
}

mach_msg_return_t
mach_msg_receive(mach_msg_header_t *msg)
{
	return mach_msg(msg, MACH_RCV_MSG,
			0, msg->msgh_size, msg->msgh_local_port,
			MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
}

static void
mach_msg_destroy_port(mach_port_t port, mach_msg_type_name_t type)
{
    if (MACH_PORT_VALID(port)) switch (type) {
      case MACH_MSG_TYPE_MOVE_SEND:
      case MACH_MSG_TYPE_MOVE_SEND_ONCE:
	/* destroy the send/send-once right */
	(void) mach_port_deallocate(mach_task_self(), port);
	break;

      case MACH_MSG_TYPE_MOVE_RECEIVE:
	/* destroy the receive right */
	(void) mach_port_mod_refs(mach_task_self(), port,
				  MACH_PORT_RIGHT_RECEIVE, -1);
	break;

      case MACH_MSG_TYPE_MAKE_SEND:
	/* create a send right and then destroy it */
	(void) mach_port_insert_right(mach_task_self(), port,
				      port, MACH_MSG_TYPE_MAKE_SEND);
	(void) mach_port_deallocate(mach_task_self(), port);
	break;

      case MACH_MSG_TYPE_MAKE_SEND_ONCE:
	/* create a send-once right and then destroy it */
	(void) mach_port_extract_right(mach_task_self(), port,
				       MACH_MSG_TYPE_MAKE_SEND_ONCE,
				       &port, &type);
	(void) mach_port_deallocate(mach_task_self(), port);
	break;
    }
}

static void
mach_msg_destroy_memory(vm_offset_t addr, vm_size_t size)
{
    if (size != 0)
	(void) mach_vm_deallocate(mach_task_self(), addr, size);
}

/*
 *	Routine:	mach_msg_destroy
 *	Purpose:
 *		mach_msg_destroy is useful in two contexts.
 *
 *		First, it can deallocate all port rights and
 *		out-of-line memory in a received message.
 *		When a server receives a request it doesn't want,
 *		it needs this functionality.
 *
 *		Second, it can mimic the side-effects of a msg-send
 *		operation.  The effect is as if the message were sent
 *		and then destroyed inside the kernel.  When a server
 *		can't send a reply (because the client died),
 *		it needs this functionality.
 */
void
mach_msg_destroy(mach_msg_header_t *msg)
{
    mach_msg_bits_t mbits = msg->msgh_bits;

    /*
     *	The msgh_local_port field doesn't hold a port right.
     *	The receive operation consumes the destination port right.
     */

    mach_msg_destroy_port(msg->msgh_remote_port, MACH_MSGH_BITS_REMOTE(mbits));

    if (mbits & MACH_MSGH_BITS_COMPLEX) {
		mach_msg_base_t base __aligned(8);
		mach_msg_base_t *basep;
		mach_msg_body_t		*body;
		mach_msg_descriptor_t	*saddr, *eaddr;

		basep = &base;
		memcpy(basep, msg, sizeof(base));
    	body = (mach_msg_body_t *) (msg + 1);
		saddr = (mach_msg_descriptor_t *) (uintptr_t)(basep + 1);
    	eaddr =  saddr + body->msgh_descriptor_count;

	for  ( ; saddr < eaddr; saddr++) {
	    switch (saddr->type.type) {
	    
	        case MACH_MSG_PORT_DESCRIPTOR: {
		    mach_msg_port_descriptor_t *dsc;

		    /* 
		     * Destroy port rights carried in the message 
		     */
		    dsc = &saddr->port;
		    mach_msg_destroy_port(dsc->name, dsc->disposition);		
		    break;
	        }

	        case MACH_MSG_OOL_DESCRIPTOR : {
		    mach_msg_ool_descriptor_t *dsc;

		    /* 
		     * Destroy memory carried in the message 
		     */
		    dsc = &saddr->out_of_line;
		    if (dsc->deallocate) {
		        mach_msg_destroy_memory((vm_offset_t)dsc->address,
						dsc->size);
		    }
		    break;
	        }

	        case MACH_MSG_OOL_PORTS_DESCRIPTOR : {
		    mach_port_t             		*ports;
		    mach_msg_ool_ports_descriptor_t	*dsc;
		    mach_msg_type_number_t   		j;

		    /*
		     * Destroy port rights carried in the message 
		     */
		    dsc = &saddr->ool_ports;
		    ports = (mach_port_t *) dsc->address;
		    for (j = 0; j < dsc->count; j++, ports++)  {
		        mach_msg_destroy_port(*ports, dsc->disposition);
		    }

		    /* 
		     * Destroy memory carried in the message 
		     */
		    if (dsc->deallocate) {
		        mach_msg_destroy_memory((vm_offset_t)dsc->address, 
					dsc->count * sizeof(mach_port_t));
		    }
		    break;
	        }
	    }
	}
    }
}

/*
 *	Routine:	mach_msg_server_once
 *	Purpose:
 *		A simple generic server function.  It allows more flexibility
 *		than mach_msg_server by processing only one message request
 *		and then returning to the user.  Note that more in the way
 * 		of error codes are returned to the user; specifically, any
 * 		failing error from mach_msg_overwrite_trap will be returned
 *		(though errors from the demux routine or the routine it calls
 *		will not be).
 */
mach_msg_return_t
mach_msg_server_once(
    boolean_t (*demux)(mach_msg_header_t *, mach_msg_header_t *),
    mach_msg_size_t max_size,
    mach_port_t rcv_name,
    mach_msg_options_t options)
{
    mig_reply_error_t *bufRequest = 0, *bufReply = 0, *bufTemp;
    boolean_t allocatedRequest = FALSE, allocatedReply = FALSE, allocatedTemp;
    register mach_msg_return_t mr;
    register kern_return_t kr;

    if ((kr = mach_vm_allocate(mach_task_self(),
		     (vm_address_t *)&bufRequest,
		     max_size + MAX_TRAILER_SIZE,
		     TRUE)) != KERN_SUCCESS)
      goto cleanup;
    allocatedRequest = TRUE;

    if ((kr = mach_vm_allocate(mach_task_self(),
		     (vm_address_t *)&bufReply,
		     max_size + MAX_TRAILER_SIZE,
		     TRUE)) != KERN_SUCCESS)
      goto cleanup;
    allocatedReply = TRUE;

    mr = mach_msg_overwrite_trap(&bufRequest->Head, MACH_RCV_MSG|options,
				 0, max_size, rcv_name,
				 MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL,
				 (mach_msg_header_t *) 0, 0);
    if (mr == MACH_MSG_SUCCESS) {
	/* we have a request message */

	(void) (*demux)(&bufRequest->Head, &bufReply->Head);

	if (!(bufReply->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) &&
	    bufReply->RetCode != KERN_SUCCESS) {
	    if (bufReply->RetCode == MIG_NO_REPLY) {
		/*
		 * This return code is a little tricky--
		 * it appears that the demux routine found an
		 * error of some sort, but since that error
		 * would not normally get returned either to
		 * the local user or the remote one, we pretend it's
		 * ok.
		 */
	
                kr = KERN_SUCCESS;
		goto cleanup;
            }

	    /* don't destroy the reply port right,
	       so we can send an error message */
	    bufRequest->Head.msgh_remote_port = MACH_PORT_NULL;
	    mach_msg_destroy(&bufRequest->Head);
	}

	if (bufReply->Head.msgh_remote_port == MACH_PORT_NULL) {
	    /* no reply port, so destroy the reply */
	    if (bufReply->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX)
		mach_msg_destroy(&bufReply->Head);

            kr = KERN_SUCCESS;
            goto cleanup;
	}

	/* send reply.  */

	bufTemp = bufRequest;
	bufRequest = bufReply;
	bufReply = bufTemp;

        allocatedTemp = allocatedRequest;
        allocatedRequest = allocatedReply;
        allocatedReply = allocatedRequest;
        
	/*
	 *	We don't want to block indefinitely because the client
	 *	isn't receiving messages from the reply port.
	 *	If we have a send-once right for the reply port, then
	 *	this isn't a concern because the send won't block.
	 *	If we have a send right, we need to use MACH_SEND_TIMEOUT.
	 *	To avoid falling off the kernel's fast RPC path unnecessarily,
	 *	we only supply MACH_SEND_TIMEOUT when absolutely necessary.
	 */

	mr = mach_msg_overwrite_trap(&bufRequest->Head,
			 (MACH_MSGH_BITS_REMOTE(bufRequest->Head.msgh_bits) ==
			  MACH_MSG_TYPE_MOVE_SEND_ONCE) ?
			 MACH_SEND_MSG|options :
			 MACH_SEND_MSG|MACH_SEND_TIMEOUT|options,
			 bufRequest->Head.msgh_size, 0, MACH_PORT_NULL,
			 0, MACH_PORT_NULL, (mach_msg_header_t *) 0, 0);
    }
    /* Has a message error occurred? */

    switch (mr) {
      case MACH_SEND_INVALID_DEST:
      case MACH_SEND_TIMED_OUT:
	/* the reply can't be delivered, so destroy it */
	mach_msg_destroy(&bufRequest->Head);
        kr = KERN_SUCCESS;      /* Matches error hiding behavior in
                                   mach_msg_server.  */
        goto cleanup;
				  

      case MACH_RCV_TOO_LARGE:
	kr = KERN_SUCCESS;	/* Matches error hiding behavior in
				   mach_msg_server.  */
        goto cleanup;

      default:
	/* Includes success case.  */
        kr = mr;
        goto cleanup;
    }

cleanup:
        if ( allocatedRequest == TRUE )  
	    (void)mach_vm_deallocate(mach_task_self(),
			    (vm_address_t) bufRequest,
			    max_size + MAX_TRAILER_SIZE);
        if ( allocatedReply == TRUE ) 
	    (void)mach_vm_deallocate(mach_task_self(),
			    (vm_address_t) bufReply,
			    max_size + MAX_TRAILER_SIZE);
        return kr;

}

/*
 *	Routine:	mach_msg_server
 *	Purpose:
 *		A simple generic server function.  Note that changes here
 * 		should be considered for duplication above.
 */
mach_msg_return_t
mach_msg_server(
    boolean_t (*demux)(mach_msg_header_t *, mach_msg_header_t *),
    mach_msg_size_t max_size,
    mach_port_t rcv_name,
    mach_msg_options_t options)
{
    mig_reply_error_t *bufRequest = 0, *bufReply = 0, *bufTemp;
    boolean_t allocatedRequest = FALSE, allocatedReply = FALSE, allocatedTemp;
    register mach_msg_return_t mr;
    register kern_return_t kr;

    if ((kr = mach_vm_allocate(mach_task_self(),
		     (vm_address_t *)&bufRequest,
		     max_size + MAX_TRAILER_SIZE,
		     TRUE)) != KERN_SUCCESS)
      goto cleanup;
    allocatedRequest = TRUE;

    if ((kr = mach_vm_allocate(mach_task_self(),
		     (vm_address_t *)&bufReply,
		     max_size + MAX_TRAILER_SIZE,
		     TRUE)) != KERN_SUCCESS)
      goto cleanup;
    allocatedReply = TRUE;

    for (;;) {
      get_request:
	mr = mach_msg_overwrite_trap(&bufRequest->Head, MACH_RCV_MSG|options,
		      0, max_size, rcv_name,
		      MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL,
		      (mach_msg_header_t *) 0, 0);
	while (mr == MACH_MSG_SUCCESS) {
	    /* we have a request message */

	    (void) (*demux)(&bufRequest->Head, &bufReply->Head);

	    if (!(bufReply->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) &&
		bufReply->RetCode != KERN_SUCCESS) {
		    if (bufReply->RetCode == MIG_NO_REPLY)
			goto get_request;

		    /* don't destroy the reply port right,
			so we can send an error message */
		    bufRequest->Head.msgh_remote_port = MACH_PORT_NULL;
		    mach_msg_destroy(&bufRequest->Head);
	    }

	    if (bufReply->Head.msgh_remote_port == MACH_PORT_NULL) {
		/* no reply port, so destroy the reply */
		if (bufReply->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX)
		    mach_msg_destroy(&bufReply->Head);

		goto get_request;
	    }

	    /* send reply and get next request */

	    bufTemp = bufRequest;
	    bufRequest = bufReply;
	    bufReply = bufTemp;

            allocatedTemp = allocatedRequest;
            allocatedRequest = allocatedReply;
            allocatedReply = allocatedTemp;

	    /*
	     *	We don't want to block indefinitely because the client
	     *	isn't receiving messages from the reply port.
	     *	If we have a send-once right for the reply port, then
	     *	this isn't a concern because the send won't block.
	     *	If we have a send right, we need to use MACH_SEND_TIMEOUT.
	     *	To avoid falling off the kernel's fast RPC path unnecessarily,
	     *	we only supply MACH_SEND_TIMEOUT when absolutely necessary.
	     */

	    mr = mach_msg_overwrite_trap(&bufRequest->Head,
			  (MACH_MSGH_BITS_REMOTE(bufRequest->Head.msgh_bits) ==
						MACH_MSG_TYPE_MOVE_SEND_ONCE) ?
			  MACH_SEND_MSG|MACH_RCV_MSG|options :
			  MACH_SEND_MSG|MACH_SEND_TIMEOUT|MACH_RCV_MSG|options,
			  bufRequest->Head.msgh_size, max_size, rcv_name,
			  0, MACH_PORT_NULL, (mach_msg_header_t *) 0, 0);
	}

	/* a message error occurred */

	switch (mr) {
	  case MACH_SEND_INVALID_DEST:
	  case MACH_SEND_TIMED_OUT:
	    /* the reply can't be delivered, so destroy it */
	    mach_msg_destroy(&bufRequest->Head);
            goto cleanup;

	  case MACH_RCV_TOO_LARGE:
	    /* the kernel destroyed the request */
            goto cleanup;

	  default:
	    /* should only happen if the server is buggy */
            kr = mr;
            goto cleanup;
	}

cleanup:
        if ( allocatedRequest == TRUE )
	    (void)mach_vm_deallocate(mach_task_self(),
				(vm_address_t) bufRequest,
				max_size + MAX_TRAILER_SIZE);
        if ( allocatedReply == TRUE )
	    (void)mach_vm_deallocate(mach_task_self(),
				(vm_address_t) bufReply,
				max_size + MAX_TRAILER_SIZE);
    }
}
