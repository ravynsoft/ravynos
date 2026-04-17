/*
 * Copyright (c) 1996-2002 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 *
 *	@(#)BTreeScanner.h
 */

#ifndef	_BTREESCANNER_H_
#define _BTREESCANNER_H_

#include "BTreePrivate.h"

// btree node scanner buffer size.  Joe Sokol suggests 128K as a max (2002 WWDC)
enum { kCatScanBufferSize = (128 * 1024) };


/*
	BTScanState - This structure is used to keep track of the current state
	of a BTree scan.  It contains both the dynamic state information (like
	the current node number and record number) and information that is static
	for the duration of a scan (such as buffer pointers).
	
	NOTE: recordNum may equal or exceed the number of records in the node
	number nodeNum.  If so, then the next attempt to get a record will move
	to a new node number.
*/
struct BTScanState 
{
	//	The following fields are set up once at initialization time.
	//	They are not changed during a scan.
	u_int32_t			bufferSize;
	void *				bufferPtr;
	BTreeControlBlock *	btcb;
	
	//	The following fields are the dynamic state of the current scan.
	u_int32_t			nodeNum;			// zero is first node
	u_int32_t			recordNum;			// zero is first record
	BTNodeDescriptor *	currentNodePtr;		// points to current node within buffer
	int32_t				nodesLeftInBuffer;	// number of valid nodes still in the buffer
	int64_t				recordsFound;		// number of leaf records seen so far
};
typedef struct BTScanState BTScanState;


/* *********************** PROTOTYPES *********************** */

int	BTScanInitialize(	const SFCB *	btreeFile,
							BTScanState	*	scanState     );
							
int BTScanNextRecord(	BTScanState *	scanState,
						void * *		key,
						void * *		data,
						u_int32_t *		dataSize  );

int	BTScanTerminate(	BTScanState *	scanState	);

#endif /* !_BTREESCANNER_H_ */
