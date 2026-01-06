/*
 * Copyright (c) 2006 Apple Computer, Inc. All Rights Reserved.
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

#include <unistd.h>
#ifdef __linux__
#include <string.h>
#endif

//
// blob - generic extensible binary blob frame
//
#include "blob.h"

namespace Security {


//
// Content access and validation calls
//
char *BlobCore::stringAt(Offset offset)
{
	char *s = at<char>(offset);
	if (offset < this->length() && memchr(s, 0, this->length() - offset))
		return s;
	else
		return NULL;
}

const char *BlobCore::stringAt(Offset offset) const
{
	const char *s = at<const char>(offset);
	if (offset < this->length() && memchr(s, 0, this->length() - offset))
		return s;
	else
		return NULL;
}


//
// Read a blob from a standard file stream.
// Reads in one pass, so it's suitable for transmission over pipes and networks.
// The blob is allocated with malloc(3).
// On error, sets errno and returns NULL; in which case the input stream may
// be partially consumed.
//
BlobCore *BlobCore::readBlob(int fd, size_t offset, uint32_t magic, size_t minSize, size_t maxSize)
{
	BlobCore header;
	if (::pread(fd, &header, sizeof(header), offset) == sizeof(header))
		if (header.validateBlob(magic, minSize, maxSize))
			if (BlobCore *blob = (BlobCore *)malloc(header.length())) {
				memcpy(blob, &header, sizeof(header));
				size_t remainder = header.length() - sizeof(header);
				if (::pread(fd, blob+1, remainder, offset + sizeof(header)) == ssize_t(remainder))
					return blob;
				free(blob);
				errno = EINVAL;
			}
	return NULL;
}

BlobCore *BlobCore::readBlob(int fd, uint32_t magic, size_t minSize, size_t maxSize)
{
	BlobCore header;
	if (::read(fd, &header, sizeof(header)) == sizeof(header))
		if (header.validateBlob(magic, minSize, maxSize))
			if (BlobCore *blob = (BlobCore *)malloc(header.length())) {
				memcpy(blob, &header, sizeof(header));
				size_t remainder = header.length() - sizeof(header);
				if (::read(fd, blob+1, remainder) == ssize_t(remainder))
					return blob;
				free(blob);
				errno = EINVAL;
			}
	return NULL;
}

BlobCore *BlobCore::readBlob(std::FILE *file, uint32_t magic, size_t minSize, size_t maxSize)
{
	BlobCore header;
	if (::fread(&header, sizeof(header), 1, file) == 1)
		if (header.validateBlob(magic, minSize, maxSize))
			if (BlobCore *blob = (BlobCore *)malloc(header.length())) {
				memcpy(blob, &header, sizeof(header));
				if (::fread(blob+1, header.length() - sizeof(header), 1, file) == 1)
					return blob;
				free(blob);
				errno = EINVAL;
			}
	return NULL;
}


//
// BlobWrappers
//
BlobWrapper *BlobWrapper::alloc(size_t length, Magic magic /* = _magic */)
{
	size_t wrapLength = length + sizeof(BlobCore);
	BlobWrapper *w = (BlobWrapper *)malloc(wrapLength);
	w->BlobCore::initialize(magic, wrapLength);
	return w;
}

BlobWrapper *BlobWrapper::alloc(const void *data, size_t length, Magic magic /* = _magic */)
{
	BlobWrapper *w = alloc(length, magic);
	memcpy(w->data(), data, w->length());
	return w;
}


}	// Security
