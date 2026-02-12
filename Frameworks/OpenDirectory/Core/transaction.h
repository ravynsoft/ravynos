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

typedef struct od_transaction_s *od_transaction_t;

typedef void (*transaction_callback_t)(CFDictionaryRef response, uint32_t code, bool complete, void *context);
typedef void (^transaction_response_handler_t)(void);

od_transaction_t transaction_create(dispatch_queue_t queue, ODSessionRef session, ODNodeRef node, CFDictionaryRef dict, void *context, transaction_callback_t function);
void transaction_cancel(od_transaction_t transaction);
void transaction_release(od_transaction_t transaction);

void transaction_send(od_transaction_t transaction);

CFArrayRef transaction_simple(uint32_t *code, ODSessionRef session, ODNodeRef node, CFStringRef func, CFIndex param_count, ...);
void transaction_simple_response(CFArrayRef response, uint32_t code, CFIndex error_index, CFErrorRef *error, transaction_response_handler_t handler);
