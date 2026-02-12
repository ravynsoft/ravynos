/*
 * Copyright (c) 2009 Apple Inc. All rights reserved.
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
 */

/*!
 * @header DirServicesUtils
 */

#include "DirServicesConst.h"
#include "DirServicesUtils.h"
#include "DirServicesUtilsPriv.h"
#include "DirServicesTypesPriv.h"
#include "DirServicesPriv.h"
#include "DirServices.h"

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#define kDSUserAuthAuthorityMarker		"{*AuthenticationAuthority*}"
#define DSCFRelease(a)					do { if (a != NULL) { CFRelease(a); a = NULL; } } while(0)
#define DSFree(a)						do { free(a); a = NULL; } while(0)

// -- Static ---------------------------------------------------------------------------------------

static tDataNodePtr	dsGetThisNodePriv(tDataNode *inFirsNode, const UInt32 inIndex);
static tDataNodePtr	dsGetLastNodePriv(tDataNode *inFirsNode);
static tDataNodePtr	dsAllocListNodeFromStringPriv(const char *inString);
static tDataNodePtr	dsAllocListNodeFromBuffPriv(const void *inData, const UInt32 inSize);
static tDirStatus	dsVerifyDataListPriv(const tDataList *inDataList);
static tDirStatus	dsAppendAuthBufferWithAuthority(const char *inUserName, tDataBufferPtr inAuthAuthorityBuffer,
                                                    tDataBufferPtr inOutAuthBuffer);

tDataBufferPtr
dsDataBufferAllocate(tDirReference inDirRef, UInt32 inBufferSize)
{
	UInt32				size		= 0;
	tDataBufferPtr		outBuff		= NULL;

	size = sizeof(tDataBufferPriv) + inBufferSize;
	outBuff = (tDataBuffer *)calloc(size + 1, sizeof(char));		// +1 for null term
	if (outBuff != NULL) {
		outBuff->fBufferSize	= inBufferSize;
		outBuff->fBufferLength	= 0;
	}

	return outBuff;
}

tDirStatus
dsDataBufferDeAllocate(tDirReference inDirRef, tDataBuffer *inDataBufferPtr)
{
	tDirStatus		tResult	= eDSNoErr;

	if (inDataBufferPtr != NULL) {
		free(inDataBufferPtr);
		inDataBufferPtr = NULL;
	} else {
		tResult = eDSNullDataBuff;
	}

	return tResult;
}

tDataNodePtr
dsDataNodeAllocateBlock(tDirReference inDirRef, UInt32 inDataNodeSize, UInt32 inDataNodeLength, tBuffer inDataNodeBuffer)
{
	tDataNode		   *pOutBuff	= NULL;

	if (inDataNodeBuffer != NULL) {
		if ((inDataNodeSize >= inDataNodeLength) && (inDataNodeSize != 0) && (inDataNodeLength != 0)) {
			pOutBuff = dsDataBufferAllocate(inDirRef, inDataNodeSize);
			if (pOutBuff != NULL) {
				memcpy(pOutBuff->fBufferData, inDataNodeBuffer, inDataNodeLength);
				dsDataNodeSetLength(pOutBuff, inDataNodeLength);
			}
		}
	}

	return pOutBuff;

}

tDataNodePtr
dsDataNodeAllocateString(tDirReference inDirRef, const char *inCString)
{
	tDataNode		   *pOutBuff	= NULL;
	UInt32				len			= 0;

	if (inCString != NULL) {
		len = strlen(inCString);
	}

	pOutBuff = dsDataBufferAllocate(inDirRef, len);
	if ((pOutBuff != NULL) && (inCString != NULL)) {
		strcpy(pOutBuff->fBufferData, inCString);
		dsDataNodeSetLength(pOutBuff, len);
	}

	return pOutBuff;
}

tDirStatus
dsDataNodeDeAllocate(tDirReference inDirRef, tDataNode *inDataNodePtr)
{
	tDirStatus		tResult	= eDSNoErr;

	if (inDataNodePtr != NULL) {
		free(inDataNodePtr);
		inDataNodePtr = NULL;
	} else {
		tResult = eDSNullParameter;
	}

	return tResult;
}

tDirStatus
dsDataNodeSetLength(tDataNode *inDataNodePtr, UInt32 inDataNodeLength)
{
	tDirStatus		tResult	= eDSNoErr;

	if (inDataNodePtr != NULL) {
		inDataNodePtr->fBufferLength = inDataNodeLength;
	} else {
		tResult = eDSNullParameter;
	}

	return tResult;
}

UInt32
dsDataNodeGetLength(tDataNode *inDataNodePtr)
{
	UInt32	uiResult	= 0;

	if (inDataNodePtr != NULL) {
		uiResult = inDataNodePtr->fBufferLength;
	}

	return uiResult;
}

UInt32
dsDataNodeGetSize(tDataNode *inDataNodePtr)
{
	UInt32	uiResult	= 0;

	if (inDataNodePtr != NULL) {
		uiResult = inDataNodePtr->fBufferSize;
	}

	return uiResult;
}

tDataListPtr
dsDataListAllocate(tDirReference inDirRef)
{
	return (tDataList *) calloc(1, sizeof(tDataList));
}

tDirStatus
dsDataListDeAllocate(tDirReference inDirRef, tDataList *inDataList, dsBool inDeAllocateNodesFlag)
{
	return dsDataListDeallocate(inDirRef, inDataList);
}

tDirStatus
dsDataListDeallocate(tDirReference inDirRef, tDataList *inDataList)
{
	tDirStatus			tResult		= eDSNoErr;
	tDataBufferPriv	   *pPrivData	= NULL;
	tDataBuffer		   *pTmpBuff	= NULL;
	tDataBuffer		   *pDataBuff	= NULL;

	if (inDataList == NULL) {
		return(eDSNullDataList);
	}

	if (inDataList->fDataListHead != NULL) {
		pDataBuff = inDataList->fDataListHead;

		inDataList->fDataListHead = NULL;
		inDataList->fDataNodeCount = 0;

		pPrivData = (tDataBufferPriv *)pDataBuff;
		while (pDataBuff != NULL) {
			pTmpBuff = pDataBuff;

			if (pPrivData != NULL) {
				pDataBuff = pPrivData->fNextPtr;
				if (pDataBuff != NULL) {
					pPrivData = (tDataBufferPriv *)pDataBuff;
				}
			} else {
				pDataBuff = NULL;
			}

			free(pTmpBuff);
			pTmpBuff = NULL;
		}
	}

	return tResult; //by above code this never fails
}

tDataListPtr
dsBuildListFromStrings(tDirReference inDirRef, const char *in1stCString, ...)
{
	tDataList		   *pOutList	= NULL;
	va_list				args;

	pOutList = dsDataListAllocate(inDirRef);
	if (pOutList == NULL) {
		return NULL;
	}

	va_start(args, in1stCString);

	dsBuildListFromStringsAllocV(inDirRef, pOutList, in1stCString, args);

	return pOutList;

}

char *
dsGetPathFromList(tDirReference	inDirRef, const tDataList *inDataList, const char *inDelimiter)
{
	char			   *outStr			= NULL;
	UInt32				uiSafetyCntr	= 0;
	UInt32				uiStrLen		= 0;
	tDataNode		   *pCurrNode		= NULL;
	tDataBufferPriv	   *pPrivData		= NULL;
	char			   *prevStr			= NULL;
	UInt32				cStrLen			= 256;
	char			   *nextStr			= NULL;

	if ((inDataList == NULL) || (inDelimiter == NULL)) {
		return NULL;
	}

	if ((inDataList->fDataNodeCount == 0) || (inDataList->fDataListHead == NULL)) {
		return NULL;
	}

	prevStr = (char *)calloc(cStrLen, sizeof(char));
	pCurrNode = inDataList->fDataListHead;
	while (pCurrNode != NULL) {
		// Append the delimiter
		strncat(prevStr, inDelimiter, strlen(inDelimiter));

		//check if there is more char buffer length required
		// Append the string
		pPrivData = (tDataBufferPriv *)pCurrNode;
		//check if there is more char buffer length required ie. look at the next string plus
		//the delimiter plus termination null plus pad of 4
		while (cStrLen < (1 + strlen(prevStr) + pPrivData->fBufferLength + 4)) {
			nextStr = (char *)calloc(strlen(prevStr) + 1, sizeof(char));
			strcpy(nextStr, prevStr);
			free(prevStr);
			cStrLen *= 2;
			prevStr = (char *)calloc(cStrLen, sizeof(char));
			strcpy(prevStr, nextStr);
			free(nextStr);
		}
		strncat(prevStr, (const char *)pPrivData->fBufferData, pPrivData->fBufferLength);

		pCurrNode = pPrivData->fNextPtr;

		uiSafetyCntr++;

		if (uiSafetyCntr == inDataList->fDataNodeCount) {
			// Yes, we are done
			pCurrNode = NULL;
		}
	}

	uiStrLen = strlen(prevStr);
	if (uiStrLen != 0) {
		outStr = (char *)calloc(uiStrLen + 1, sizeof(char));
		strcpy(outStr, prevStr);
	}

	free(prevStr);

	return outStr;
}

char **
dsAllocStringsFromList(tDirReference inDirRef, const tDataList *inDataList)
{
	tDataNode		   *pCurrNode		= NULL;
	tDataBufferPriv	   *pPrivData		= NULL;
	char			  **listOfStrings   = NULL;

	if (inDataList == NULL) {
		return NULL;
	}

	if ((inDataList->fDataNodeCount == 0) || (inDataList->fDataListHead == NULL)) {
		return NULL;
	}

	listOfStrings = (char **)calloc(inDataList->fDataNodeCount + 1, sizeof(char *));
	pCurrNode = inDataList->fDataListHead;
	UInt32 strCount = 0;
	while (pCurrNode != NULL) {
		pPrivData = (tDataBufferPriv *)pCurrNode;

		listOfStrings[strCount] = (char *)calloc(1, strlen((const char *)pPrivData->fBufferData) + 1);
		strcpy(listOfStrings[strCount], (const char *)pPrivData->fBufferData);

		pCurrNode = pPrivData->fNextPtr;

		strCount++;

		if (strCount == inDataList->fDataNodeCount) {
			// Yes, we are done
			pCurrNode = NULL;
		}
	}

	return listOfStrings;

}

tDataListPtr
dsBuildFromPath(tDirReference inDirRef, const char *inPathCString, const char *inPathSeparatorCString)
{
	const char		   *inStr		= NULL;
	char			   *ptr			= NULL;
	const char		   *inDelim		= NULL;
	SInt32				delimLen	= 0;
	dsBool				done		= false;
	SInt32				len			= 0;
	tDataList		   *outDataList	= NULL;
	char			   *cStr		= NULL;

	if ((inPathCString == NULL) || (inPathSeparatorCString == NULL)) {
		return NULL;
	}

	inStr = inPathCString;

	inDelim = inPathSeparatorCString;
	delimLen = strlen(inDelim);

	// Does the string == the delimiter
	if (strcmp(inStr, inDelim) == 0) {
		return NULL;
	}

	outDataList = dsDataListAllocate(inDirRef);
	if (outDataList == NULL) {
		return NULL;
	}

	ptr = strstr(inStr, inDelim);

	// Does the first char(s) == the delimiter
	if ((ptr != NULL) && (ptr == inStr)) {
		inStr += delimLen;
	}

	while (!done && (*inStr != '\0')) {
		ptr = strstr(inStr, inDelim);
		if (ptr == NULL) {
			len = strlen(inStr);

			cStr = (char *)calloc(len + 1, sizeof(char));
			strncpy(cStr, inStr, len);

			dsAppendStringToListAlloc(0, outDataList, cStr);
			free(cStr);

			done = true;
		} else {
			len = ptr - inStr;

			cStr = (char *)calloc(len + 1, sizeof(char));
			strncpy(cStr, inStr, len);

			dsAppendStringToListAlloc(0, outDataList, cStr);
			free(cStr);

			inStr += len + delimLen;
		}
	}

	return outDataList;
}

tDirStatus
dsBuildListFromPathAlloc(tDirReference inDirRef, tDataList *inDataList, const char *inPathCString, const char *inPathSeparatorCString)
{
	tDirStatus			tResult		= eDSNoErr;
	const char		   *inStr		= NULL;
	char			   *ptr			= NULL;
	const char		   *inDelim		= NULL;
	SInt32				delimLen	= 0;
	dsBool				done		= false;
	SInt32				len			= 0;
	char			   *cStr		= NULL;

	if (inDataList == NULL) {
		return(eDSNullDataList);
	}

	if ((inPathCString == NULL) || (inPathSeparatorCString == NULL)) {
		return(eDSNullParameter);
	}

	// Does the string == the delimiter
	if (strcmp(inPathCString, inPathSeparatorCString) == 0) {
		return(eDSEmptyParameter);
	}

	inStr = inPathCString;

	inDelim = inPathSeparatorCString;
	delimLen = strlen(inDelim);

	// This could leak
	inDataList->fDataNodeCount = 0;
	inDataList->fDataListHead  = NULL;

	ptr = strstr(inStr, inDelim);

	// Does the first char(s) == the delimiter
	if ((ptr != NULL) && (ptr == inStr)) {
		inStr += delimLen;
	}

	while (!done && (tResult == eDSNoErr) && (*inStr != '\0')) {
		ptr = strstr(inStr, inDelim);
		if (ptr == NULL) {
			len = strlen(inStr);
			done = true;
		} else {
			len = ptr - inStr;
		}

		cStr = (char *)calloc(len + 1, sizeof(char));
		strncpy(cStr, inStr, len);
		tResult = dsAppendStringToListAlloc(0, inDataList, cStr);
		free(cStr);

		inStr += len + delimLen;
	}

	if (tResult != eDSNoErr) {
		dsDataListDeallocate(inDirRef, inDataList);
	}

	return tResult;
}

tDataListPtr
dsBuildListFromNodes(tDirReference inDirRef, tDataNode *in1stDataNodePtr, ...)
{
	return NULL;
}

tDirStatus
dsBuildListFromStringsAlloc(tDirReference inDirRef, tDataList *inDataList, const char *inStr, ...)
{
	va_list		args;
	tDirStatus	tResult	= eDSNoErr;

	va_start(args, inStr);
	tResult = dsBuildListFromStringsAllocV(inDirRef, inDataList, inStr, args);
	va_end(args);

	return tResult;
}

tDirStatus
dsBuildListFromStringsAllocV(tDirReference inDirRef, tDataList *inDataList, const char *inStr, va_list args)
{
	tDirStatus			tResult		= eDSNoErr;
	const char		   *pStr		= NULL;
	tDataNode		   *pCurrNode	= NULL;
	tDataNode		   *pPrevNode	= NULL;
	tDataBufferPriv    *pPrivData	= NULL;

	if (inDataList == NULL) {
		return(eDSNullDataList);
	}

	// This could leak
	inDataList->fDataNodeCount = 0;
	inDataList->fDataListHead  = NULL;

	pStr = inStr;

	while ((pStr != NULL) && (tResult == eDSNoErr)) {
		// Node size is: struct size + string length + null term byte
		pCurrNode = dsAllocListNodeFromStringPriv(pStr);
		if (pCurrNode != NULL) {
			if (inDataList->fDataNodeCount == 0) {
				inDataList->fDataListHead = pCurrNode;
				pPrivData = (tDataBufferPriv *)pCurrNode;
				pPrivData->fPrevPtr		= NULL;
				pPrivData->fNextPtr		= NULL;
				pPrivData->fScriptCode	= kASCIICodeScript;

				inDataList->fDataNodeCount++;
			} else if (pPrevNode != NULL) {
				// Get the previous node's header and point it to the next
				pPrivData = (tDataBufferPriv *)pPrevNode;
				pPrivData->fNextPtr = pCurrNode;

				// Get the current node's header and point it to the prevous
				pPrivData = (tDataBufferPriv *)pCurrNode;
				pPrivData->fPrevPtr = pPrevNode;

				// Set the script code to ASCII
				pPrivData->fScriptCode = kASCIICodeScript;

				inDataList->fDataNodeCount++;
			} else {
				tResult = eMemoryAllocError;
			}
		} else {
			tResult = eMemoryAllocError;
		}

		pStr = va_arg(args, char *);

		pPrevNode = pCurrNode;
		pCurrNode = NULL;
	}

	va_end(args);

	return tResult;
}

tDirStatus
dsAppendStringToList(tDataList *inDataList, const char *inCString)
{
	return eNoLongerSupported;

}

tDirStatus
dsAppendStringToListAlloc(tDirReference	inDirRef, tDataList *inOutDataList, const char *inCString)
{
	tDirStatus			tResult			= eDSNoErr;
	const char		   *pInString		= inCString;
	tDataNode		   *pNewNode		= NULL;
	tDataNode		   *pCurrNode		= NULL;
	tDataBufferPriv    *pNewNodePriv	= NULL;
	tDataBufferPriv    *pCurNodePriv	= NULL;

	if (inOutDataList == NULL) {
		return(eDSNullDataList);
	}

	if (inCString == NULL) {
		return(eDSNullParameter);
	}

	if ((inOutDataList->fDataNodeCount == 0) || (inOutDataList->fDataListHead == NULL)) {
		// This could leak
		inOutDataList->fDataNodeCount	= 0;
		inOutDataList->fDataListHead	= NULL;
	}

	pNewNode = dsAllocListNodeFromStringPriv(pInString);
	if (pNewNode == NULL) {
		return(eMemoryAllocError);
	}

	if (inOutDataList->fDataNodeCount == 0) {
		inOutDataList->fDataListHead = pNewNode;
		pNewNodePriv = (tDataBufferPriv *)pNewNode;
		pNewNodePriv->fPrevPtr		= NULL;
		pNewNodePriv->fNextPtr		= NULL;
		pNewNodePriv->fScriptCode	= kASCIICodeScript;

		inOutDataList->fDataNodeCount++;
	} else {
		pCurrNode = dsGetLastNodePriv(inOutDataList->fDataListHead);
		if (pCurrNode != NULL) {
			// Get the current node's header and point it to the new
			pCurNodePriv = (tDataBufferPriv *)pCurrNode;
			pCurNodePriv->fNextPtr = pNewNode;

			// Get the new node's header and point it to the prevous end
			pNewNodePriv = (tDataBufferPriv *)pNewNode;
			pNewNodePriv->fPrevPtr	= pCurrNode;
			pNewNodePriv->fNextPtr	= NULL;

			// Set the script code to ASCII
			pNewNodePriv->fScriptCode = kASCIICodeScript;

			inOutDataList->fDataNodeCount++;
		} else {
			tResult = eDSInvalidIndex;
		}
	}

	return tResult;
}

tDirStatus
dsBuildListFromNodesAlloc(tDirReference	inDirRef, tDataList *inDataList, tDataNode *in1stDataNodePtr, ...)
{
	tDirStatus			tResult		= eDSNoErr;
	tDataNode		   *pNodePtr	= in1stDataNodePtr;;
	tDataNode		   *pCurrNode	= NULL;
	tDataNode		   *pPrevNode	= NULL;
	tDataBufferPriv    *pPrivData	= NULL;
	va_list				args;

	if (inDataList == NULL) {
		return(eDSNullDataList);
	}

	// This could leak
	inDataList->fDataNodeCount	= 0;
	inDataList->fDataListHead	= NULL;

	va_start(args, in1stDataNodePtr);

	while ((pNodePtr != NULL) && (tResult == eDSNoErr)) {
		pCurrNode = dsAllocListNodeFromBuffPriv(pNodePtr->fBufferData, pNodePtr->fBufferLength);
		if (pCurrNode != NULL) {
			// Is it the first node in the list
			if (inDataList->fDataNodeCount == 0) {
				inDataList->fDataListHead = pCurrNode;
				pPrivData = (tDataBufferPriv *)pCurrNode;
				pPrivData->fPrevPtr		= NULL;
				pPrivData->fNextPtr		= NULL;
				pPrivData->fScriptCode	= kASCIICodeScript;

				inDataList->fDataNodeCount++;
			} else if (pPrevNode != NULL) {
				// Get the previous node's header and point it to the next
				pPrivData = (tDataBufferPriv *)pPrevNode;
				pPrivData->fNextPtr = pCurrNode;

				// Get the current node's header and point it to the prevous
				pPrivData = (tDataBufferPriv *)pCurrNode;
				pPrivData->fPrevPtr = pPrevNode;

				// Set the script code to ASCII
				pPrivData->fScriptCode = kASCIICodeScript;

				inDataList->fDataNodeCount++;
			} else {
				tResult = eMemoryAllocError;
			}
		} else {
			tResult = eMemoryAllocError;
		}

		pNodePtr = va_arg(args, tDataNode *);

		pPrevNode = pCurrNode;
		pCurrNode = NULL;
	}

	return tResult;
}

UInt32
dsDataListGetNodeCount(const tDataList *inDataList)
{
	tDirStatus			tResult		= eDSNoErr;
	UInt32				outCount	= 0;

	if (inDataList != NULL) {
		// Verify node count first
		tResult = dsVerifyDataListPriv(inDataList);
		if (tResult == eDSNoErr) {
			outCount = inDataList->fDataNodeCount;
		}
	}

	return outCount;

}

UInt32
dsGetDataLength(const tDataList *inDataList)
{
	UInt32				outStrLen	= 0;
	tDataNode		   *pCurrNode	= NULL;
	tDataBufferPriv    *pPrivData	= NULL;

	if (inDataList != NULL) {
		pCurrNode = inDataList->fDataListHead;

		// Get the list total length
		while (pCurrNode != NULL) {
			// Get this node's length
			outStrLen += pCurrNode->fBufferLength;

			// Get the private header
			pPrivData = (tDataBufferPriv *)pCurrNode;

			// Assign the current node to the next one
			pCurrNode = pPrivData->fNextPtr;
		}
	}

	return outStrLen;
}

tDirStatus
dsDataListInsertNode(tDataList *inDataList, tDataNode *inAfterDataNode, tDataNode *inInsertDataNode)
{
	return eNoLongerSupported;
}

tDirStatus
dsDataListInsertAfter(tDirReference inDirRef, tDataList *inDataList, tDataNode *inDataNode, const UInt32 inIndex)
{
	tDirStatus			tResult			= eDSNoErr;
	tDataNode		   *pNewNode		= NULL;
	tDataNode		   *pCurrNode		= NULL;
	tDataNode		   *pNextNode		= NULL;
	tDataBufferPriv    *pNewNodePriv	= NULL;
	tDataBufferPriv    *pCurNodePriv	= NULL;
	tDataBufferPriv    *pNextNodeHdr	= NULL;

	if (inDataList == NULL) {
		return(eDSNullDataList);
	}

	if (inDataNode == NULL) {
		return(eDSNullParameter);
	}

	if (inDataNode->fBufferLength > inDataNode->fBufferSize) {
		// Length is bigger than its size
		return(eDSBadDataNodeLength);
	}

	if (((inDataList->fDataNodeCount != 0) && (inDataList->fDataListHead == NULL)) ||
	        ((inDataList->fDataNodeCount == 0) && (inDataList->fDataListHead != NULL))) {
		// Can't trust this node list
		return(eDSBadDataNodeFormat);
	}

	if (inIndex > inDataList->fDataNodeCount) {
		return(eDSIndexOutOfRange);
	}

	pNewNode = dsAllocListNodeFromBuffPriv(inDataNode->fBufferData, inDataNode->fBufferLength);
	if (pNewNode == NULL) {
		return(eMemoryAllocError);
	}

	if (inIndex == 0) {
		pNextNode = inDataList->fDataListHead;
		inDataList->fDataListHead = pNewNode;

		if (pNextNode != NULL) {
			pNextNodeHdr = (tDataBufferPriv *)pNextNode;

			// Set the next node's back pointer
			pNextNodeHdr->fPrevPtr = pNewNode;
		}

		pNewNodePriv = (tDataBufferPriv *)pNewNode;
		pNewNodePriv->fPrevPtr		= NULL;
		pNewNodePriv->fNextPtr		= pNextNode;
		pNewNodePriv->fScriptCode	= kASCIICodeScript;

		inDataList->fDataNodeCount++;
	} else {
		pCurrNode = dsGetThisNodePriv(inDataList->fDataListHead, inIndex);
		if (pCurrNode != NULL) {
			// Get the current node's header and point it to the new
			pCurNodePriv = (tDataBufferPriv *)pCurrNode;

			// Get the new node's header and point it to the prevous end
			pNewNodePriv = (tDataBufferPriv *)pNewNode;

			pNextNode = pCurNodePriv->fNextPtr;
			if (pNextNode != NULL) {
				pNextNodeHdr = (tDataBufferPriv *)pNextNode;

				// Set the next node's back pointer
				pNextNodeHdr->fPrevPtr = pNewNode;
			}

			// Set the current node's front pointer
			pCurNodePriv->fNextPtr = pNewNode;

			// Set the new nodes front and back pointer
			pNewNodePriv->fPrevPtr	= pCurrNode;
			pNewNodePriv->fNextPtr	= pNextNode;

			// Set the script code to ASCII
			pNewNodePriv->fScriptCode = kASCIICodeScript;

			inDataList->fDataNodeCount++;
		} else {
			free(pNewNode);
			pNewNode = NULL;

			tResult = eDSInvalDataList;
		}
	}

	return tResult;
}

tDirStatus
dsDataListMergeList(tDataListPtr inDataList, tDataNode *inAfterDataNode, tDataListPtr inMergeDataList)
{
	return eNoLongerSupported;
}

tDirStatus
dsDataListMergeListAfter(tDataList *inTargetList, tDataList *inSourceList, const UInt32 inIndex)
{
	tDirStatus			tResult			= eDSNoErr;
	tDataNode		   *pCurrNode		= NULL;
	tDataNode		   *pNextNode		= NULL;
	tDataNode		   *pFirstNode		= NULL;
	tDataNode		   *pLastNode		= NULL;
	tDataBufferPriv    *pCurrPrivData	= NULL;
	tDataBufferPriv    *pNextPrivData	= NULL;
	tDataBufferPriv    *pFirstPrivData	= NULL;
	tDataBufferPriv    *pLastPrivData	= NULL;

	// Do a null check
	if ((inTargetList == NULL) || (inSourceList == NULL)) {
		return(eDSNullDataList);
	}

	// Make sure we have a valid data node list
	tResult = dsVerifyDataListPriv(inTargetList);
	if (tResult != eDSNoErr) {
		return tResult;
	}
	tResult = dsVerifyDataListPriv(inSourceList);
	if (tResult != eDSNoErr) {
		return tResult;
	}

	if (inIndex > inTargetList->fDataNodeCount) {
		return(eDSIndexOutOfRange);
	}

	if (inSourceList->fDataNodeCount == 0) {
		return tResult;
	}

	// Get the first node from the source list
	pFirstNode = dsGetThisNodePriv(inSourceList->fDataListHead, 1);

	if (inSourceList->fDataNodeCount == 1) {
		// First and last node are the same
		pLastNode = pFirstNode;
	} else {
		// Get the last node from the source list
		pLastNode = dsGetLastNodePriv(inSourceList->fDataListHead);
	}

	// xxxx deal with 0 --- head of list ii

	// Get the merg point node
	pCurrNode = dsGetThisNodePriv(inTargetList->fDataListHead, inIndex);

	if (inIndex < inTargetList->fDataNodeCount) {
		// Get the node after the merg point node
		pNextNode = dsGetThisNodePriv(inTargetList->fDataListHead, inIndex + 1);
	}

	if ((pFirstNode == NULL) || (pLastNode == NULL) || (pCurrNode == NULL)) {
		return(eDSInvalDataList);
	}

	// Link the first two nodes
	pCurrPrivData	= (tDataBufferPriv *)pCurrNode;
	pFirstPrivData	= (tDataBufferPriv *)pFirstNode;

	pCurrPrivData->fNextPtr		= pFirstNode;
	pFirstPrivData->fPrevPtr	= pCurrNode;

	if (pNextNode != NULL) {
		// Link the last two nodes
		pLastPrivData = (tDataBufferPriv *)pLastNode;
		pNextPrivData = (tDataBufferPriv *)pNextNode;

		pLastPrivData->fNextPtr	= pNextNode;
		pNextPrivData->fPrevPtr	= pLastNode;
	} else {
		pCurrPrivData->fNextPtr = NULL;
	}

	return tResult;
}

tDataListPtr
dsDataListCopyList(tDirReference inDirRef, const tDataList *inSourceList)
{
	tDirStatus			tResult			= eDSNoErr;
	tDataList		   *pOutList		= NULL;
	UInt32				count			= 0;
	UInt32				uiSize			= 0;
	UInt32				uiLength		= 0;
	tDataNode		   *pCurrNode		= NULL;
	tDataNode		   *pNewNode		= NULL;
	tDataNode		   *pPrevNode		= NULL;
	tDataBufferPriv	   *pPrevPrivData	= NULL;
	tDataBufferPriv	   *pCurrPrivData	= NULL;
	tDataBufferPriv	   *pNewPrivData	= NULL;

	// Make sure we have a valid data node list
	tResult = dsVerifyDataListPriv(inSourceList);
	if (tResult != eDSNoErr) {
		return NULL;
	}

	pOutList = dsDataListAllocate(inDirRef);
	if (pOutList == NULL) {
		return NULL;
	}

	for (count = 1; count <= inSourceList->fDataNodeCount; count++) {
		pCurrNode = dsGetThisNodePriv(inSourceList->fDataListHead, count);
		if (pCurrNode != NULL) {
			// Duplicate the data into a new node
			pCurrPrivData = (tDataBufferPriv *)pCurrNode;
			uiSize = pCurrPrivData->fBufferSize;
			uiLength = pCurrPrivData->fBufferLength;

			pNewNode = dsDataBufferAllocate(inDirRef, uiSize);
			if (pNewNode != NULL) {
				pNewNode->fBufferSize	= uiSize;
				pNewNode->fBufferLength	= uiLength;

				pNewPrivData = (tDataBufferPriv *)pNewNode;
				memcpy(pNewPrivData->fBufferData, pCurrPrivData->fBufferData, uiLength);

				// Set the script code
				pNewPrivData->fScriptCode = pCurrPrivData->fScriptCode;

				// Link the new node in the list
				if (pPrevNode == NULL) {
					pPrevNode = pNewNode;
					pOutList->fDataListHead = pNewNode;

					pNewPrivData->fPrevPtr	= NULL;
					pNewPrivData->fNextPtr	= NULL;

					pNewNode = NULL;
				} else {
					pPrevPrivData = (tDataBufferPriv *)pPrevNode;

					pPrevPrivData->fNextPtr	= pNewNode;

					pNewPrivData->fPrevPtr	= pPrevNode;
					pNewPrivData->fNextPtr	= NULL;

					pPrevNode = pNewNode;
					pNewNode = NULL;
				}
				pOutList->fDataNodeCount++;
			}
		}
	}

	return pOutList;
}

tDirStatus
dsDataListRemoveNodes(tDataListPtr inDataList, tDataNode *in1stDataNode, UInt32 inDeleteCount)
{
	return eNoLongerSupported;
}

tDirStatus
dsDataListRemoveThisNode(tDataListPtr inDataList, UInt32 inNodeIndex, UInt32 inDeleteCount)
{
	return eNoLongerSupported;
}

tDirStatus dsDataListDeleteThisNode(tDirReference inDirRef, tDataList *inDataList, UInt32 inIndex)
{
	tDirStatus			tResult		= eDSNoErr;
	tDataNode		   *pCurrNode	= NULL;
	tDataBufferPriv	   *pCurrPriv	= NULL;
	tDataBufferPriv	   *pPrevPriv	= NULL;
	tDataBufferPriv	   *pNextPriv	= NULL;

	// Make sure we have a valid data node list
	tResult = dsVerifyDataListPriv(inDataList);
	if (tResult != eDSNoErr) {
		return tResult;
	}

	if ((inIndex > inDataList->fDataNodeCount) && (inIndex != 0)) {
		return(eDSIndexOutOfRange);
	}

	// Get the node we are looking for
	pCurrNode = dsGetThisNodePriv(inDataList->fDataListHead, inIndex);
	if (pCurrNode != NULL) {
		pCurrPriv = (tDataBufferPriv *)pCurrNode;
		pPrevPriv = (tDataBufferPriv *)pCurrPriv->fPrevPtr;
		pNextPriv = (tDataBufferPriv *)pCurrPriv->fNextPtr;

		if (inIndex == 1) {
			// Delete the head of the list
			inDataList->fDataListHead = (tDataNode *)pNextPriv;
			if (pNextPriv != NULL) {
				pNextPriv->fPrevPtr = NULL;
			}
		} else if (inIndex == inDataList->fDataNodeCount) {
			// Delete the last node from the list
			pPrevPriv->fNextPtr = NULL;
		} else {
			// Delete from the middle
			pPrevPriv->fNextPtr = (tDataNode *)pNextPriv;
			pNextPriv->fPrevPtr = (tDataNode *)pPrevPriv;
		}

		dsDataBufferDeAllocate(inDirRef, pCurrNode);
		pCurrNode = NULL;

		inDataList->fDataNodeCount--;
	}

	return tResult;
}

tDirStatus
dsDataListGetNode(tDataListPtr inDataList, UInt32 inIndex, tDataNode **outDataNode)
{
	return eNoLongerSupported;
}

tDirStatus dsDataListGetNodeAlloc(tDirReference inDirRef, const tDataList *inDataList, const UInt32 inIndex,
                                  tDataNode **outDataNode)
{
	tDirStatus			tResult			= eDSNoErr;
	UInt32				uiLength		= 0;
	tDataBuffer		   *pOutDataNode	= NULL;
	tDataNode		   *pCurrNode		= NULL;
	tDataBufferPriv	   *pPrivData		= NULL;

	// NULL check in data list
	if (inDataList == NULL) {
		return(eDSNullDataList);
	}

	// NULL check in data list head pointer
	if (inDataList->fDataListHead == NULL) {
		return(eDSEmptyDataList);
	}

	pCurrNode = dsGetThisNodePriv(inDataList->fDataListHead, inIndex);
	if (pCurrNode == NULL) {
		return(eDSIndexOutOfRange);
	}

	if (outDataNode == NULL) {
		return(eDSNullTargetArgument);
	}

	pPrivData = (tDataBufferPriv *)pCurrNode;
	uiLength = pPrivData->fBufferLength;

	pOutDataNode = dsDataBufferAllocate(inDirRef, uiLength);
	if (pOutDataNode != NULL) {
		memcpy(pOutDataNode->fBufferData, pPrivData->fBufferData, uiLength);
		pOutDataNode->fBufferSize = uiLength + 1;
		pOutDataNode->fBufferLength = uiLength;
		*outDataNode = pOutDataNode;
	} else {
		tResult = eMemoryAllocError;
	}

	return tResult;
}

tAttributeValueEntryPtr
dsAllocAttributeValueEntry(tDirReference inDirRef, UInt32 inAttrValueID, void *inAttrValueData, UInt32 inAttrValueDataLen)
{
	UInt32						uiDataSize	= 0;
	tAttributeValueEntryPtr		outEntryPtr	= NULL;

	uiDataSize = sizeof(tAttributeValueEntry) + inAttrValueDataLen + 1;
	outEntryPtr = (tAttributeValueEntry *)calloc(1, uiDataSize);
	if (outEntryPtr != NULL) {
		outEntryPtr->fAttributeValueID = inAttrValueID;
		memcpy(outEntryPtr->fAttributeValueData.fBufferData, inAttrValueData, inAttrValueDataLen);
		outEntryPtr->fAttributeValueData.fBufferSize	= inAttrValueDataLen;
		outEntryPtr->fAttributeValueData.fBufferLength	= inAttrValueDataLen;
	}

	return outEntryPtr;
}

tDirStatus
dsDeallocAttributeValueEntry(tDirReference inDirRef, tAttributeValueEntryPtr inAttrValueEntry)
{
	tDirStatus			tResult	= eDSNoErr;

	if (inAttrValueEntry != NULL) {
		free(inAttrValueEntry); //sufficient since calloc above in dsAllocAttributeValueEntry done on all including the tDataNode
		inAttrValueEntry = NULL;
	}

	return tResult;
}

tDirStatus
dsDeallocAttributeEntry(tDirReference inDirRef, tAttributeEntryPtr inAttrEntry)
{
	tDirStatus			tResult	= eDSNoErr;

	if (inAttrEntry != NULL) {
		free(inAttrEntry);	//sufficient since Add_tAttrEntry_ToMsg calloc done on all including the tDataNode
		//and Get_tRecordEntry_FromMsg retrieves it all
		inAttrEntry = NULL;
	}

	return tResult;
}

tDirStatus
dsDeallocRecordEntry(tDirReference inDirRef, tRecordEntryPtr inRecEntry)
{
	tDirStatus			tResult	= eDSNoErr;

	if (inRecEntry != NULL) {
		free(inRecEntry);		//sufficient since all calloc's done on all data including the tDataNode
		//and Get_tAttrEntry_FromMsg retrieves it all as well
		inRecEntry = NULL;
	}

	return tResult;
}

tDirStatus
dsGetRecordNameFromEntry(tRecordEntryPtr inRecEntryPtr, char **outRecName)
{
	tDirStatus		tResult		= eDSNoErr;
	UInt32			uiOffset	= 2;
	UInt32			uiBuffSize	= 0;
	UInt16			usLength	= 0;
	tDataNodePtr	dataNode 	= NULL;
	char		   *pData	 	= NULL;
	char		   *pOutData 	= NULL;

	if (outRecName == NULL) {
		tResult = eDSNullParameter;
	} else if (inRecEntryPtr != NULL) {
		dataNode = &inRecEntryPtr->fRecordNameAndType;
		if ((dataNode->fBufferSize != 0) &&
		        (dataNode->fBufferLength != 0) &&
		        (dataNode->fBufferLength <= dataNode->fBufferSize)) {
			pData = dataNode->fBufferData;
			uiBuffSize = dataNode->fBufferSize;

			memcpy(&usLength, pData, 2);
			if ((usLength == 0) || (usLength > (uiBuffSize - uiOffset))) {
				tResult = eDSCorruptRecEntryData;
			} else {
				if (outRecName != NULL) {
					pOutData = (char *)calloc(usLength + 1, sizeof(char));
					if (pOutData != NULL) {
						memcpy(pOutData, pData + 2, usLength);
						*outRecName = pOutData;
					}
				}
			}
		} else {
			tResult = eDSCorruptBuffer;
		}
	} else {
		tResult = eDSNullRecEntryPtr;
	}

	return tResult;
}

tDirStatus
dsGetRecordTypeFromEntry(tRecordEntryPtr inRecEntryPtr, char **outRecType)
{
	tDirStatus		tResult		= eDSNoErr;
	UInt16			usLength	= 0;
	UInt32			uiOffset	= 2;
	UInt32			uiBuffSize	= 0;
	tDataNodePtr	dataNode 	= NULL;
	char		   *pData	 	= NULL;
	char		   *pOutData	= NULL;

	if (outRecType == NULL) {
		tResult = eDSNullParameter;
	} else if (inRecEntryPtr != NULL) {
		dataNode = &inRecEntryPtr->fRecordNameAndType;
		if ((dataNode->fBufferSize != 0) &&
		        (dataNode->fBufferLength != 0) &&
		        (dataNode->fBufferLength <= dataNode->fBufferSize)) {
			pData = dataNode->fBufferData;
			uiBuffSize = dataNode->fBufferSize;

			memcpy(&usLength, pData, 2);
			if ((usLength == 0) || (usLength > (uiBuffSize - uiOffset))) {
				tResult = eDSCorruptRecEntryData;
			} else {
				uiOffset += 2 + usLength;
				pData += 2 + usLength;

				memcpy(&usLength, pData, 2);
				if ((usLength == 0) || (usLength > (uiBuffSize - uiOffset))) {
					tResult = eDSCorruptRecEntryData;
				} else {
					if (outRecType != NULL) {
						pOutData = (char *)calloc(usLength + 1, sizeof(char));
						if (pOutData != NULL) {
							memcpy(pOutData, pData + 2, usLength);

							*outRecType = pOutData;
						}
					}
				}
			}
		} else {
			tResult = eDSCorruptBuffer;
		}
	} else {
		tResult = eDSNullRecEntryPtr;
	}

	return tResult;
}

tDataNodePtr
dsGetThisNodePriv(tDataNode *inFirsNode, const UInt32 inIndex)
{
	UInt32				i			= 1;
	tDataNode		   *pCurrNode	= NULL;
	tDataBufferPriv    *pPrivData	= NULL;

	pCurrNode = inFirsNode;
	while (pCurrNode != NULL) {
		if (i == inIndex) {
			break;
		} else {
			pPrivData = (tDataBufferPriv *)pCurrNode;
			pCurrNode = pPrivData->fNextPtr;
		}
		i++;
	}

	return pCurrNode;
}

tDataNodePtr
dsGetLastNodePriv(tDataNode *inFirsNode)
{
	tDataNode		   *pCurrNode	= NULL;
	tDataBufferPriv    *pPrivData	= NULL;

	pCurrNode = inFirsNode;
	pPrivData = (tDataBufferPriv *)pCurrNode;

	while (pPrivData->fNextPtr != NULL) {
		pCurrNode = pPrivData->fNextPtr;
		pPrivData = (tDataBufferPriv *)pCurrNode;
	}

	return pCurrNode;
}

tDataNodePtr
dsAllocListNodeFromStringPriv(const char *inString)
{
	UInt32				nodeSize	= 0;
	UInt32				strLen		= 0;
	tDataNode		   *pOutNode	= NULL;
	tDataBufferPriv	   *pPrivData	= NULL;

	if (inString != NULL) {
		strLen = strlen(inString);
		nodeSize = sizeof(tDataBufferPriv) + strLen + 1;
		pOutNode = (tDataNode *)calloc(nodeSize, sizeof(char));
		if (pOutNode != NULL) {
			pOutNode->fBufferSize = nodeSize;
			pOutNode->fBufferLength = nodeSize;

			pPrivData = (tDataBufferPriv *)pOutNode;
			pPrivData->fBufferSize = strLen;
			pPrivData->fBufferLength = strLen;

			strcpy(pPrivData->fBufferData, inString);
		}
	}

	return pOutNode;
}

tDataNodePtr
dsAllocListNodeFromBuffPriv(const void *inData, const UInt32 inSize)
{
	UInt32				nodeSize	= 0;
	tDataNode		   *pOutNode	= NULL;
	tDataBufferPriv	   *pPrivData	= NULL;

	if (inData != NULL) {
		nodeSize = sizeof(tDataBufferPriv) + inSize + 1;
		pOutNode = (tDataNode *)calloc(nodeSize, sizeof(char));
		if (pOutNode != NULL) {
			pOutNode->fBufferSize = nodeSize;
			pOutNode->fBufferLength = nodeSize;

			pPrivData = (tDataBufferPriv *)pOutNode;
			pPrivData->fBufferSize = inSize;
			pPrivData->fBufferLength = inSize;

			memcpy(pPrivData->fBufferData, inData, inSize);
		}
	}

	return pOutNode;
}

tDirStatus
dsVerifyDataListPriv(const tDataList *inDataList)
{
	UInt32				count		= 0;
	tDataNode		   *pCurrNode	= NULL;
	tDataBufferPriv    *pPrivData	= NULL;

	if (inDataList == NULL) {
		return eDSNullDataList;
	}

	pCurrNode = inDataList->fDataListHead;

	while (pCurrNode != NULL) {
		// Bump the count and limit loop lengths for bad or corrupted data.
		if (++count > inDataList->fDataNodeCount) {
			break;
		}

		pPrivData = (tDataBufferPriv *)pCurrNode;

		pCurrNode = pPrivData->fNextPtr;
	}

	if (inDataList->fDataNodeCount == count) {
		return eDSNoErr;
	}

	// Probably should have a custom error for this condition.
	return eDSInvalidBuffFormat;
}

tDirStatus
dsParseAuthAuthority(const char *inAuthAuthority, char **outVersion, char **outAuthTag, char **outAuthData)
{
	char *authAuthority = NULL;
	char *current = NULL;
	char *tempPtr = NULL;
	tDirStatus result = eDSAuthFailed;

	if (inAuthAuthority == NULL || outVersion == NULL
	        || outAuthTag == NULL || outAuthData == NULL) {
		return eDSAuthFailed;
	}
	authAuthority = strdup(inAuthAuthority);
	if (authAuthority == NULL) {
		return eDSAuthFailed;
	}
	current = authAuthority;
	do {
		tempPtr = strsep(&current, ";");
		if (tempPtr == NULL) {
			break;
		}
		*outVersion = strdup(tempPtr);

		tempPtr = strsep(&current, ";");
		if (tempPtr == NULL) {
			break;
		}
		*outAuthTag = strdup(tempPtr);

		tempPtr = strsep(&current, ";");
		if (tempPtr == NULL) {
			break;
		}
		*outAuthData = strdup(tempPtr);

		result = eDSNoErr;
	} while (false);

	free(authAuthority);
	authAuthority = NULL;

	if (result != eDSNoErr) {
		if (*outVersion != NULL) {
			free(*outVersion);
			*outVersion = NULL;
		}
		if (*outAuthTag != NULL) {
			free(*outAuthTag);
			*outAuthTag = NULL;
		}
		if (*outAuthData != NULL) {
			free(*outAuthData);
			*outAuthData = NULL;
		}
	}

	return result;
}

tDirStatus
dsParseAuthAuthorityExtended(const char *inAuthAuthority, char **outVersion, char **outAuthTag, char ***outAuthData)
{
	tDirStatus status = eDSNoErr;
	CFStringRef aaString = NULL;
	CFArrayRef aaArray = NULL;
	CFIndex arrayCount = 0;
	int idx = 0;
	char *curPtr = NULL;
	CFIndex curPtrLen = 0;

	if (inAuthAuthority == NULL || outVersion == NULL || outAuthTag == NULL || outAuthData == NULL) {
		return eDSAuthFailed;
	}

	*outVersion = NULL;
	*outAuthTag = NULL;
	*outAuthData = NULL;

	aaString = CFStringCreateWithCString(kCFAllocatorDefault, inAuthAuthority, kCFStringEncodingUTF8);
	if (aaString == NULL) {
		return eDSAuthFailed;
	}

	aaArray = CFStringCreateArrayBySeparatingStrings(kCFAllocatorDefault, aaString, CFSTR(";"));
	DSCFRelease(aaString);

	if (aaArray == NULL) {
		return eDSAuthFailed;
	}

	arrayCount = CFArrayGetCount(aaArray);
	if (arrayCount < 2) {
		CFRelease(aaArray);
		return eDSAuthFailed;
	}

	if (arrayCount >= 3) {
		(*outAuthData) = (char **) calloc(arrayCount - 1, sizeof(char *));
		assert((*outAuthData) != NULL);
	}

	for (idx = 0; idx < arrayCount; idx++) {
		aaString = (CFStringRef) CFArrayGetValueAtIndex(aaArray, idx);
		curPtrLen = CFStringGetMaximumSizeForEncoding(CFStringGetLength(aaString), kCFStringEncodingUTF8) + 1;
		curPtr = (char *) malloc(curPtrLen);
		assert(curPtr != NULL);

		if (CFStringGetCString(aaString, curPtr, curPtrLen, kCFStringEncodingUTF8) == FALSE) {
			status = eDSAuthFailed;
		}

		switch (idx) {
			case 0:
				*outVersion = curPtr;
				break;

			case 1:
				*outAuthTag = curPtr;
				break;

			default:
				(*outAuthData)[idx - 2] = curPtr;
				break;
		}
	}

	if (status != eDSNoErr) {
		DSFree(*outVersion);
		DSFree(*outAuthTag);

		if (*outAuthData != NULL) {
			for (idx = 0; (*outAuthData)[idx] != NULL && idx < arrayCount; idx++) {
				DSFree((*outAuthData)[idx]);
			}

			DSFree(*outAuthData);
		}
	}

	DSCFRelease(aaArray);

	return status;
}

char *
dsCopyDirStatusName(SInt32 inDirStatus)
{
	char	*outString   = NULL;
	UInt32	caseIndex   = (-1 * inDirStatus) - 14000;

	if (inDirStatus == 0) {
		outString = strdup("eDSNoErr");
	} else if ((caseIndex < 1000) && (caseIndex > 0)) {
		switch (caseIndex) {
			case 0:
				outString = strdup("eDSOpenFailed");
				break;
			case 1:
				outString = strdup("eDSCloseFailed");
				break;
			case 2:
				outString = strdup("eDSOpenNodeFailed");
				break;
			case 3:
				outString = strdup("eDSBadDirRefences");
				break;
			case 4:
				outString = strdup("eDSNullRecordReference");
				break;
			case 5:
				outString = strdup("eDSMaxSessionsOpen");
				break;
			case 6:
				outString = strdup("eDSCannotAccessSession");
				break;
			case 7:
				outString = strdup("eDSDirSrvcNotOpened");
				break;
			case 8:
				outString = strdup("eDSNodeNotFound");
				break;
			case 9:
				outString = strdup("eDSUnknownNodeName");
				break;
			case 10:
				outString = strdup("eDSRegisterCustomFailed");
				break;
			case 11:
				outString = strdup("eDSGetCustomFailed");
				break;
			case 12:
				outString = strdup("eDSUnRegisterFailed");
				break;
			case 15:
				outString = strdup("eDSLocalDSDaemonInUse");
				break;
			case 16:
				outString = strdup("eDSNormalDSDaemonInUse");
				break;
			case 50:
				outString = strdup("eDSAllocationFailed");
				break;
			case 51:
				outString = strdup("eDSDeAllocateFailed");
				break;
			case 52:
				outString = strdup("eDSCustomBlockFailed");
				break;
			case 53:
				outString = strdup("eDSCustomUnblockFailed");
				break;
			case 54:
				outString = strdup("eDSCustomYieldFailed");
				break;
			case 60:
				outString = strdup("eDSCorruptBuffer");
				break;
			case 61:
				outString = strdup("eDSInvalidIndex");
				break;
			case 62:
				outString = strdup("eDSIndexOutOfRange");
				break;
			case 63:
				outString = strdup("eDSIndexNotFound");
				break;
			case 65:
				outString = strdup("eDSCorruptRecEntryData");
				break;
			case 69:
				outString = strdup("eDSRefSpaceFull");
				break;
			case 70:
				outString = strdup("eDSRefTableAllocError");
				break;
			case 71:
				outString = strdup("eDSInvalidReference");
				break;
			case 72:
				outString = strdup("eDSInvalidRefType");
				break;
			case 73:
				outString = strdup("eDSInvalidDirRef");
				break;
			case 74:
				outString = strdup("eDSInvalidNodeRef");
				break;
			case 75:
				outString = strdup("eDSInvalidRecordRef");
				break;
			case 76:
				outString = strdup("eDSInvalidAttrListRef");
				break;
			case 77:
				outString = strdup("eDSInvalidAttrValueRef");
				break;
			case 78:
				outString = strdup("eDSInvalidContinueData");
				break;
			case 79:
				outString = strdup("eDSInvalidBuffFormat");
				break;
			case 80:
				outString = strdup("eDSInvalidPatternMatchType");
				break;
			case 81:
				outString = strdup("eDSRefTableError");
				break;
			case 82:
				outString = strdup("eDSRefTableNilError");
				break;
			case 83:
				outString = strdup("eDSRefTableIndexOutOfBoundsError");
				break;
			case 84:
				outString = strdup("eDSRefTableEntryNilError");
				break;
			case 85:
				outString = strdup("eDSRefTableCSBPAllocError");
				break;
			case 86:
				outString = strdup("eDSRefTableFWAllocError");
				break;
			case 90:
				outString = strdup("eDSAuthFailed");
				break;
			case 91:
				outString = strdup("eDSAuthMethodNotSupported");
				break;
			case 92:
				outString = strdup("eDSAuthResponseBufTooSmall");
				break;
			case 93:
				outString = strdup("eDSAuthParameterError");
				break;
			case 94:
				outString = strdup("eDSAuthInBuffFormatError");
				break;
			case 95:
				outString = strdup("eDSAuthNoSuchEntity");
				break;
			case 96:
				outString = strdup("eDSAuthBadPassword");
				break;
			case 97:
				outString = strdup("eDSAuthContinueDataBad");
				break;
			case 98:
				outString = strdup("eDSAuthUnknownUser");
				break;
			case 99:
				outString = strdup("eDSAuthInvalidUserName");
				break;
			case 100:
				outString = strdup("eDSAuthCannotRecoverPasswd");
				break;
			case 101:
				outString = strdup("eDSAuthFailedClearTextOnly");
				break;
			case 102:
				outString = strdup("eDSAuthNoAuthServerFound");
				break;
			case 103:
				outString = strdup("eDSAuthServerError");
				break;
			case 104:
				outString = strdup("eDSInvalidContext");
				break;
			case 105:
				outString = strdup("eDSBadContextData");
				break;
			case 120:
				outString = strdup("eDSPermissionError");
				break;
			case 121:
				outString = strdup("eDSReadOnly");
				break;
			case 122:
				outString = strdup("eDSInvalidDomain");
				break;
			case 123:
				outString = strdup("eNetInfoError");
				break;
			case 130:
				outString = strdup("eDSInvalidRecordType");
				break;
			case 131:
				outString = strdup("eDSInvalidAttributeType");
				break;
			case 133:
				outString = strdup("eDSInvalidRecordName");
				break;
			case 134:
				outString = strdup("eDSAttributeNotFound");
				break;
			case 135:
				outString = strdup("eDSRecordAlreadyExists");
				break;
			case 136:
				outString = strdup("eDSRecordNotFound");
				break;
			case 137:
				outString = strdup("eDSAttributeDoesNotExist");
				break;
			case 138:
				outString = strdup("eDSRecordTypeDisabled");
				break;
			case 140:
				outString = strdup("eDSNoStdMappingAvailable");
				break;
			case 141:
				outString = strdup("eDSInvalidNativeMapping");
				break;
			case 142:
				outString = strdup("eDSSchemaError");
				break;
			case 143:
				outString = strdup("eDSAttributeValueNotFound");
				break;
			case 149:
				outString = strdup("eDSVersionMismatch");
				break;
			case 150:
				outString = strdup("eDSPlugInConfigFileError");
				break;
			case 151:
				outString = strdup("eDSInvalidPlugInConfigData");
				break;
			case 161:
				outString = strdup("eDSAuthNewPasswordRequired");
				break;
			case 162:
				outString = strdup("eDSAuthPasswordExpired");
				break;
			case 165:
				outString = strdup("eDSAuthPasswordQualityCheckFailed");
				break;
			case 167:
				outString = strdup("eDSAuthAccountDisabled");
				break;
			case 168:
				outString = strdup("eDSAuthAccountExpired");
				break;
			case 169:
				outString = strdup("eDSAuthAccountInactive");
				break;
			case 170:
				outString = strdup("eDSAuthPasswordTooShort");
				break;
			case 171:
				outString = strdup("eDSAuthPasswordTooLong");
				break;
			case 172:
				outString = strdup("eDSAuthPasswordNeedsLetter");
				break;
			case 173:
				outString = strdup("eDSAuthPasswordNeedsDigit");
				break;
			case 174:
				outString = strdup("eDSAuthPasswordChangeTooSoon");
				break;
			case 175:
				outString = strdup("eDSAuthInvalidLogonHours");
				break;
			case 176:
				outString = strdup("eDSAuthInvalidComputer");
				break;
			case 177:
				outString = strdup("eDSAuthMasterUnreachable");
				break;
			case 200:
				outString = strdup("eDSNullParameter");
				break;
			case 201:
				outString = strdup("eDSNullDataBuff");
				break;
			case 202:
				outString = strdup("eDSNullNodeName");
				break;
			case 203:
				outString = strdup("eDSNullRecEntryPtr");
				break;
			case 204:
				outString = strdup("eDSNullRecName");
				break;
			case 205:
				outString = strdup("eDSNullRecNameList");
				break;
			case 206:
				outString = strdup("eDSNullRecType");
				break;
			case 207:
				outString = strdup("eDSNullRecTypeList");
				break;
			case 208:
				outString = strdup("eDSNullAttribute");
				break;
			case 209:
				outString = strdup("eDSNullAttributeAccess");
				break;
			case 210:
				outString = strdup("eDSNullAttributeValue");
				break;
			case 211:
				outString = strdup("eDSNullAttributeType");
				break;
			case 212:
				outString = strdup("eDSNullAttributeTypeList");
				break;
			case 213:
				outString = strdup("eDSNullAttributeControlPtr");
				break;
			case 214:
				outString = strdup("eDSNullAttributeRequestList");
				break;
			case 215:
				outString = strdup("eDSNullDataList");
				break;
			case 216:
				outString = strdup("eDSNullDirNodeTypeList");
				break;
			case 217:
				outString = strdup("eDSNullAutMethod");
				break;
			case 218:
				outString = strdup("eDSNullAuthStepData");
				break;
			case 219:
				outString = strdup("eDSNullAuthStepDataResp");
				break;
			case 220:
				outString = strdup("eDSNullNodeInfoTypeList");
				break;
			case 221:
				outString = strdup("eDSNullPatternMatch");
				break;
			case 222:
				outString = strdup("eDSNullNodeNamePattern");
				break;
			case 223:
				outString = strdup("eDSNullTargetArgument");
				break;
			case 230:
				outString = strdup("eDSEmptyParameter");
				break;
			case 231:
				outString = strdup("eDSEmptyBuffer");
				break;
			case 232:
				outString = strdup("eDSEmptyNodeName");
				break;
			case 233:
				outString = strdup("eDSEmptyRecordName");
				break;
			case 234:
				outString = strdup("eDSEmptyRecordNameList");
				break;
			case 235:
				outString = strdup("eDSEmptyRecordType");
				break;
			case 236:
				outString = strdup("eDSEmptyRecordTypeList");
				break;
			case 237:
				outString = strdup("eDSEmptyRecordEntry");
				break;
			case 238:
				outString = strdup("eDSEmptyPatternMatch");
				break;
			case 239:
				outString = strdup("eDSEmptyNodeNamePattern");
				break;
			case 240:
				outString = strdup("eDSEmptyAttribute");
				break;
			case 241:
				outString = strdup("eDSEmptyAttributeType");
				break;
			case 242:
				outString = strdup("eDSEmptyAttributeTypeList");
				break;
			case 243:
				outString = strdup("eDSEmptyAttributeValue");
				break;
			case 244:
				outString = strdup("eDSEmptyAttributeRequestList");
				break;
			case 245:
				outString = strdup("eDSEmptyDataList");
				break;
			case 246:
				outString = strdup("eDSEmptyNodeInfoTypeList");
				break;
			case 247:
				outString = strdup("eDSEmptyAuthMethod");
				break;
			case 248:
				outString = strdup("eDSEmptyAuthStepData");
				break;
			case 249:
				outString = strdup("eDSEmptyAuthStepDataResp");
				break;
			case 250:
				outString = strdup("eDSEmptyPattern2Match");
				break;
			case 255:
				outString = strdup("eDSBadDataNodeLength");
				break;
			case 256:
				outString = strdup("eDSBadDataNodeFormat");
				break;
			case 257:
				outString = strdup("eDSBadSourceDataNode");
				break;
			case 258:
				outString = strdup("eDSBadTargetDataNode");
				break;
			case 260:
				outString = strdup("eDSBufferTooSmall");
				break;
			case 261:
				outString = strdup("eDSUnknownMatchType");
				break;
			case 262:
				outString = strdup("eDSUnSupportedMatchType");
				break;
			case 263:
				outString = strdup("eDSInvalDataList");
				break;
			case 264:
				outString = strdup("eDSAttrListError");
				break;
			case 270:
				outString = strdup("eServerNotRunning");
				break;
			case 271:
				outString = strdup("eUnknownAPICall");
				break;
			case 272:
				outString = strdup("eUnknownServerError");
				break;
			case 273:
				outString = strdup("eUnknownPlugIn");
				break;
			case 274:
				outString = strdup("ePlugInDataError");
				break;
			case 275:
				outString = strdup("ePlugInNotFound");
				break;
			case 276:
				outString = strdup("ePlugInError");
				break;
			case 277:
				outString = strdup("ePlugInInitError");
				break;
			case 278:
				outString = strdup("ePlugInNotActive");
				break;
			case 279:
				outString = strdup("ePlugInFailedToInitialize");
				break;
			case 280:
				outString = strdup("ePlugInCallTimedOut");
				break;
			case 290:
				outString = strdup("eNoSearchNodesFound");
				break;
			case 291:
				outString = strdup("eSearchPathNotDefined");
				break;
			case 292:
				outString = strdup("eNotHandledByThisNode");
				break;
			case 330:
				outString = strdup("eIPCSendError");
				break;
			case 331:
				outString = strdup("eIPCReceiveError");
				break;
			case 332:
				outString = strdup("eServerReplyError");
				break;
			case 350:
				outString = strdup("eDSTCPSendError");
				break;
			case 351:
				outString = strdup("eDSTCPReceiveError");
				break;
			case 352:
				outString = strdup("eDSTCPVersionMismatch");
				break;
			case 353:
				outString = strdup("eDSIPUnreachable");
				break;
			case 354:
				outString = strdup("eDSUnknownHost");
				break;
			case 400:
				outString = strdup("ePluginHandlerNotLoaded");
				break;
			case 402:
				outString = strdup("eNoPluginsLoaded");
				break;
			case 404:
				outString = strdup("ePluginAlreadyLoaded");
				break;
			case 406:
				outString = strdup("ePluginVersionNotFound");
				break;
			case 408:
				outString = strdup("ePluginNameNotFound");
				break;
			case 410:
				outString = strdup("eNoPluginFactoriesFound");
				break;
			case 412:
				outString = strdup("ePluginConfigAvailNotFound");
				break;
			case 414:
				outString = strdup("ePluginConfigFileNotFound");
				break;
			case 450:
				outString = strdup("eCFMGetFileSysRepErr");
				break;
			case 452:
				outString = strdup("eCFPlugInGetBundleErr");
				break;
			case 454:
				outString = strdup("eCFBndleGetInfoDictErr");
				break;
			case 456:
				outString = strdup("eCFDictGetValueErr");
				break;
			case 470:
				outString = strdup("eDSServerTimeout");
				break;
			case 471:
				outString = strdup("eDSContinue");
				break;
			case 472:
				outString = strdup("eDSInvalidHandle");
				break;
			case 473:
				outString = strdup("eDSSendFailed");
				break;
			case 474:
				outString = strdup("eDSReceiveFailed");
				break;
			case 475:
				outString = strdup("eDSBadPacket");
				break;
			case 476:
				outString = strdup("eDSInvalidTag");
				break;
			case 477:
				outString = strdup("eDSInvalidSession");
				break;
			case 478:
				outString = strdup("eDSInvalidName");
				break;
			case 479:
				outString = strdup("eDSUserUnknown");
				break;
			case 480:
				outString = strdup("eDSUnrecoverablePassword");
				break;
			case 481:
				outString = strdup("eDSAuthenticationFailed");
				break;
			case 482:
				outString = strdup("eDSBogusServer");
				break;
			case 483:
				outString = strdup("eDSOperationFailed");
				break;
			case 484:
				outString = strdup("eDSNotAuthorized");
				break;
			case 485:
				outString = strdup("eDSNetInfoError");
				break;
			case 486:
				outString = strdup("eDSContactMaster");
				break;
			case 487:
				outString = strdup("eDSServiceUnavailable");
				break;
			case 488:
				outString = strdup("eDSInvalidFilePath");
				break;
			case 501:
				outString = strdup("eFWGetDirNodeNameErr1");
				break;
			case 502:
				outString = strdup("eFWGetDirNodeNameErr2");
				break;
			case 503:
				outString = strdup("eFWGetDirNodeNameErr3");
				break;
			case 504:
				outString = strdup("eFWGetDirNodeNameErr4");
				break;
			case 700:
				outString = strdup("eParameterSendError");
				break;
			case 720:
				outString = strdup("eParameterReceiveError");
				break;
			case 740:
				outString = strdup("eServerSendError");
				break;
			case 760:
				outString = strdup("eServerReceiveError");
				break;
			case 900:
				outString = strdup("eMemoryError");
				break;
			case 901:
				outString = strdup("eMemoryAllocError");
				break;
			case 910:
				outString = strdup("eServerError");
				break;
			case 915:
				outString = strdup("eParameterError");
				break;
			case 950:
				outString = strdup("eDataReceiveErr_NoDirRef");
				break;
			case 951:
				outString = strdup("eDataReceiveErr_NoRecRef");
				break;
			case 952:
				outString = strdup("eDataReceiveErr_NoAttrListRef");
				break;
			case 953:
				outString = strdup("eDataReceiveErr_NoAttrValueListRef");
				break;
			case 954:
				outString = strdup("eDataReceiveErr_NoAttrEntry");
				break;
			case 955:
				outString = strdup("eDataReceiveErr_NoAttrValueEntry");
				break;
			case 956:
				outString = strdup("eDataReceiveErr_NoNodeCount");
				break;
			case 957:
				outString = strdup("eDataReceiveErr_NoAttrCount");
				break;
			case 958:
				outString = strdup("eDataReceiveErr_NoRecEntry");
				break;
			case 959:
				outString = strdup("eDataReceiveErr_NoRecEntryCount");
				break;
			case 960:
				outString = strdup("eDataReceiveErr_NoRecMatchCount");
				break;
			case 961:
				outString = strdup("eDataReceiveErr_NoDataBuff");
				break;
			case 962:
				outString = strdup("eDataReceiveErr_NoContinueData");
				break;
			case 963:
				outString = strdup("eDataReceiveErr_NoNodeChangeToken");
				break;
			case 986:
				outString = strdup("eNoLongerSupported");
				break;
			case 987:
				outString = strdup("eUndefinedError");
				break;
			case 988:
				outString = strdup("eNotYetImplemented");
				break;
			case 999:
				outString = strdup("eDSLastValue");
				break;
			default:
				outString = strdup("Not a known DirStatus");
				break;
		}
	} else {
		outString = strdup("Not a known DirStatus");
	}

	return outString;
}

tDirStatus
dsFillAuthBuffer(tDataBufferPtr inOutAuthBuffer, UInt32 inCount, UInt32 inLen, const void *inData, ...)
{
	UInt32				curr		= 0;
	UInt32				buffSize	= 0;
	UInt32				count		= inCount;
	UInt32				len			= inLen;
	const void			*data		= inData;
	bool				firstPass	= true;
	char				*p			= NULL;
	va_list				args;

	if (inOutAuthBuffer == NULL || inOutAuthBuffer->fBufferData == NULL || inCount == 0) {
		return eParameterError;
	}

	// Make sure we have data to copy
	if ((inLen != 0) && (inData == NULL)) {
		return eParameterError;
	}

	// Get buffer info
	p		 = inOutAuthBuffer->fBufferData;
	buffSize = inOutAuthBuffer->fBufferSize;

	// Set up the arg list
	va_start(args, inData);

	while (count-- > 0) {
		if (!firstPass) {
			len = va_arg(args, UInt32);
			data = va_arg(args, void *);
		}

		if ((curr + len) > buffSize) {
			// Out of buffer space, bail
			return eDSBufferTooSmall;
		}

		memcpy(&(p[curr]), &len, sizeof(SInt32));
		curr += sizeof(SInt32);

		if (len > 0 && data != NULL) {
			memcpy(&(p[curr]), data, len);
			curr += len;
		}
		firstPass = false;
	}

	inOutAuthBuffer->fBufferLength = curr;

	return eDSNoErr;
}

tDirStatus
dsAppendAuthBuffer(tDataBufferPtr inOutAuthBuffer, UInt32 inCount, UInt32 inLen, const void *inData, ...)
{
	UInt32				curr		= 0;
	UInt32				buffSize	= 0;
	UInt32				count		= inCount;
	UInt32				len			= inLen;
	const void			*data		= inData;
	bool				firstPass	= true;
	char				*p			= NULL;
	va_list				args;

	if (inOutAuthBuffer == NULL || inOutAuthBuffer->fBufferData == NULL || inCount == 0) {
		return eParameterError;
	}

	// Make sure we have data to copy
	if ((inLen != 0) && (inData == NULL)) {
		return eParameterError;
	}

	// Get buffer info
	p		 = inOutAuthBuffer->fBufferData + inOutAuthBuffer->fBufferLength;
	buffSize = inOutAuthBuffer->fBufferSize - inOutAuthBuffer->fBufferLength;

	// Set up the arg list
	va_start(args, inData);

	while (count-- > 0) {
		if (!firstPass) {
			len = va_arg(args, UInt32);
			data = va_arg(args, void *);
		}

		if ((curr + len) > buffSize) {
			// Out of buffer space, bail
			return eDSBufferTooSmall;
		}

		memcpy(&(p[curr]), &len, sizeof(SInt32));
		curr += sizeof(SInt32);

		if (len > 0 && data != NULL) {
			memcpy(&(p[curr]), data, len);
			curr += len;
		}
		firstPass = false;
	}

	inOutAuthBuffer->fBufferLength += curr;

	return eDSNoErr;
}

tDirStatus
dsAppendAuthBufferWithAuthorityAttribute(tDirNodeReference inNodeRef, tDataBufferPtr inRecordListBuffPtr,
                                         tAttributeEntryPtr inAttributePtr, tAttributeValueListRef inValueRef,
                                         const char *inUserName, tDataBufferPtr inOutAuthBuffer)
{
	tDirStatus					status				= eDSNoErr;
	tDataBufferPtr				innerDataBuff		= NULL;
	tAttributeValueEntry	   *attrValue			= NULL;
	UInt32						attrValIndex		= 0;
	UInt32						attrValCount		= 0;

	if (inOutAuthBuffer == NULL) {
		return eParameterError;
	}

	attrValCount = inAttributePtr->fAttributeValueCount;
	innerDataBuff = dsDataBufferAllocate(0, inAttributePtr->fAttributeDataSize + sizeof(UInt32) * attrValCount);
	if (innerDataBuff != NULL) {
		// run through the values create a tDataBuffer with the list of authentication authorities.
		for (attrValIndex = 1; attrValIndex <= attrValCount && status == eDSNoErr; attrValIndex++) {
			status = dsGetAttributeValue(inNodeRef, inRecordListBuffPtr, attrValIndex, inValueRef, &attrValue);
			if (status == eDSNoErr) {
				status = dsAppendAuthBuffer(innerDataBuff, 1, attrValue->fAttributeValueData.fBufferLength,
				                            attrValue->fAttributeValueData.fBufferData);
				dsDeallocAttributeValueEntry(0, attrValue);
			}
		}

		status = dsAppendAuthBufferWithAuthority(inUserName, innerDataBuff, inOutAuthBuffer);

		dsDataBufferDeAllocate(0, innerDataBuff);
	}

	return status;
}

tDirStatus
dsAppendAuthBufferWithAuthorityStrings(const char *inUserName, const char *inAuthAuthority[], tDataBufferPtr inOutAuthBuffer)
{
	tDirStatus					status				= eDSNoErr;
	tDataBufferPtr				innerDataBuff		= NULL;
	const char					*tptr				= NULL;
	UInt32						attrValIndex		= 0;
	UInt32						attrValCount		= 0;
	UInt32						neededSize			= sizeof(tDataBuffer);

	if (inOutAuthBuffer == NULL) {
		return eParameterError;
	}

	for (tptr = inAuthAuthority[0]; tptr != NULL;) {
		neededSize += sizeof(uint32_t) + strlen(tptr);
		tptr = inAuthAuthority[++attrValCount];
	}

	innerDataBuff = dsDataBufferAllocate(0, neededSize);
	if (innerDataBuff != NULL) {
		// run through the values create a tDataBuffer with the list of authentication authorities.
		for (attrValIndex = 0; attrValIndex < attrValCount && status == eDSNoErr; attrValIndex++) {
			status = dsAppendAuthBuffer(innerDataBuff, 1, strlen(inAuthAuthority[attrValIndex]),
			                            inAuthAuthority[attrValIndex]);
		}

		if (status == eDSNoErr) {
			status = dsAppendAuthBufferWithAuthority(inUserName, innerDataBuff, inOutAuthBuffer);
		}

		dsDataBufferDeAllocate(0, innerDataBuff);
	}

	return status;
}

tDirStatus
dsAppendAuthBufferWithAuthority(const char *inUserName, tDataBufferPtr inAuthAuthorityBuffer, tDataBufferPtr inOutAuthBuffer)
{
	tDirStatus				status				= eDSNoErr;
	int						userNameLen			= 0;
	int						userBufferLen		= 0;
	char					*userBuffer			= NULL;

	if (inUserName == NULL || inAuthAuthorityBuffer == NULL || inOutAuthBuffer == NULL) {
		return eParameterError;
	}

	userNameLen = strlen(inUserName);
	userBufferLen = userNameLen + sizeof(kDSUserAuthAuthorityMarker) + sizeof(tDataBuffer) +
	                inAuthAuthorityBuffer->fBufferLength;

	userBuffer = (char *) malloc(userBufferLen);
	if (userBuffer == NULL) {
		return eMemoryError;
	}

	strcpy(userBuffer, inUserName);
	strcpy(userBuffer + userNameLen + 1, kDSUserAuthAuthorityMarker);
	memcpy(userBuffer + userNameLen + sizeof(kDSUserAuthAuthorityMarker), inAuthAuthorityBuffer,
	       sizeof(tDataBuffer) + inAuthAuthorityBuffer->fBufferLength);
	status = dsAppendAuthBuffer(inOutAuthBuffer, 1, userBufferLen, userBuffer);
	free(userBuffer);

	return status;
}

tDirStatus
dsServiceInformationAllocate(CFDictionaryRef inServiceInfo, UInt32 inBufferSize, tDataBufferPtr *outPackedServiceInfo)
{
	tDirStatus				status			= eDSNoErr;
	tDataBufferPtr			outBuff			= NULL;
	CFDataRef				xmlData			= NULL;
	const UInt8				*sourcePtr		= NULL;
	long					length			= 0;
	UInt32					bufferSize		= inBufferSize;

	if (outPackedServiceInfo == NULL) {
		return eParameterError;
	}
	*outPackedServiceInfo = NULL;

	if (inServiceInfo == NULL || CFGetTypeID(inServiceInfo) != CFDictionaryGetTypeID()) {
		return eParameterError;
	}

	xmlData = CFPropertyListCreateXMLData(kCFAllocatorDefault, (CFPropertyListRef)inServiceInfo);
	if (xmlData == NULL) {
		return eMemoryError;
	}

	sourcePtr = CFDataGetBytePtr(xmlData);
	length = CFDataGetLength(xmlData);
	if (sourcePtr == NULL || length <= 0) {
		status = eDSEmptyAuthStepData;
		goto fail;
	}

	// maximize the buffer allocation
	if (bufferSize < (UInt32)length) {
		bufferSize = length + 1;
	}

	// allocate the buffer
	outBuff = dsDataBufferAllocate(0, bufferSize);
	if (outBuff == NULL) {
		status = eMemoryError;
		goto fail;
	}

	status = dsFillAuthBuffer(outBuff, 1, length, sourcePtr);

fail:
	if (status == eDSNoErr) {
		*outPackedServiceInfo = outBuff;
	} else if (outBuff != NULL) {
		dsDataBufferDeAllocate(0, outBuff);
	}

	DSCFRelease(xmlData);
	return status;
}

CFMutableDictionaryRef
dsConvertAuthAuthorityToCFDict(const char *inAuthAuthorityStr)
{
	CFMutableDictionaryRef theDict = NULL;
	CFMutableArrayRef aaDataArray = NULL;
	CFStringRef aString = NULL;
	CFNumberRef aNumber = NULL;
	char *aaVers = NULL;
	char *aaTag = NULL;
	char **aaData = NULL;
	int aaVersion = 0;
	int ii = 0;

	if (dsParseAuthAuthorityExtended(inAuthAuthorityStr, &aaVers, &aaTag, &aaData) == eDSNoErr) {
		theDict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		if (theDict == NULL) {
			goto bail;
		}

		sscanf(aaVers, "%d", &aaVersion);
		if (aaVersion == 0) {
			aaVersion = 1;
		}

		aNumber = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &aaVersion);
		if (aNumber == NULL) {
			goto bail;
		}

		CFDictionaryAddValue(theDict, CFSTR("version"), aNumber);
		DSCFRelease(aNumber);

		aString = CFStringCreateWithCString(kCFAllocatorDefault, aaTag, kCFStringEncodingUTF8);
		if (aString == NULL) {
			goto bail;
		}

		CFDictionaryAddValue(theDict, CFSTR("tag"), aString);
		DSCFRelease(aString);

		aaDataArray = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
		if (aaDataArray == NULL) {
			goto bail;
		}

		while (aaData[ii]) {
			aString = CFStringCreateWithCString(kCFAllocatorDefault, aaData[ii++], kCFStringEncodingUTF8);
			if (aString == NULL) {
				goto bail;
			}

			CFArrayAppendValue(aaDataArray, aString);
			DSCFRelease(aString);
		}

		CFDictionaryAddValue(theDict, CFSTR("data"), aaDataArray);
		DSCFRelease(aaDataArray);
	}

bail:
	DSCFRelease(theDict);
	DSCFRelease(aaDataArray);

	// clean up
	DSFree(aaVers);
	DSFree(aaTag);
	for (ii = 0; aaData[ii] != NULL; ii++) {
		DSFree(aaData[ii]);
	}
	DSFree(aaData);

	return theDict;
}

char *
dsConvertCFDictToAuthAuthority(CFDictionaryRef inAuthAuthorityDict)
{
	char *theAA = NULL;
	size_t needSize = 0;
	CFStringRef aaDataString = NULL;
	int idx = 0;
	int arrayCount = 0;
	int aaVersion = 1;
	char *aaTag = NULL;
	char *aaData = NULL;

	if (inAuthAuthorityDict == NULL) {
		return NULL;
	}

	CFNumberRef aaVersNumber = (CFNumberRef) CFDictionaryGetValue(inAuthAuthorityDict, CFSTR("version"));
	CFStringRef aaTagString = (CFStringRef) CFDictionaryGetValue(inAuthAuthorityDict, CFSTR("tag"));
	CFArrayRef aaDataArray = (CFArrayRef) CFDictionaryGetValue(inAuthAuthorityDict, CFSTR("data"));

	// must have a tag
	if (aaTagString == NULL) {
		return NULL;
	}

	// SInt32 max is -2100000000 (11 chars)
	needSize = (aaVersNumber != NULL) ? 12 : 1;

	// tag is always 7-bit ASCII
	needSize += (aaTagString != NULL) ? CFStringGetLength(aaTagString) : 1;

	// data can be any UTF8
	if (aaDataArray != NULL) {
		arrayCount = CFArrayGetCount(aaDataArray);
		for (idx = 0; idx < arrayCount; idx++) {
			aaDataString = (CFStringRef) CFArrayGetValueAtIndex(aaDataArray, idx);
			needSize += (aaDataString != NULL) ? CFStringGetMaximumSizeForEncoding(CFStringGetLength(aaDataString), kCFStringEncodingUTF8) : 1;
		}
	}

	// add room for a zero-terminator
	needSize++;

	if (aaVersNumber != NULL) {
		CFNumberGetValue(aaVersNumber, kCFNumberSInt32Type, &aaVersion);
		if (aaVersion == 0) {
			aaVersion = 1;
		}
	}

	aaTag = (char *) malloc(CFStringGetLength(aaTagString) + 1);
	if (aaTag == NULL) {
		return NULL;
	}

	aaData = (char *) malloc(needSize);
	if (aaData == NULL) {
		free(aaTag);
		return NULL;
	}

	if (CFStringGetCString(aaTagString, aaTag, CFStringGetLength(aaTagString) + 1, kCFStringEncodingUTF8)) {
		theAA = (char *) malloc(needSize);
		if (theAA != NULL) {
			if (aaVersion != 1) {
				snprintf(theAA, needSize, "%d;%s;", aaVersion, aaTag);
			} else {
				snprintf(theAA, needSize, ";%s;", aaTag);
			}

			if (aaDataArray != NULL) {
				arrayCount = CFArrayGetCount(aaDataArray);
				for (idx = 0; idx < arrayCount; idx++) {
					aaDataString = (CFStringRef) CFArrayGetValueAtIndex(aaDataArray, idx);
					if (aaDataString != NULL) {
						if (CFStringGetCString(aaDataString, aaData, needSize, kCFStringEncodingUTF8)) {
							strlcat(theAA, aaData, needSize);
							if (idx < arrayCount - 1) {
								strlcat(theAA, ";", needSize);
							}
						}
					}
				}
			}
		}
	}

	DSFree(aaTag);
	DSFree(aaData);

	return theAA;
}
