/*
 * Copyright (c) 2019 Apple Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "dnssd_xpc.h"

#if 0
//======================================================================================================================
#pragma mark - XPC Dictionary Helper Declarations
#endif

static int32_t
_dnssd_xpc_dictionary_get_int32(xpc_object_t dict, const char *key, bool *out_valid);

static int64_t
_dnssd_xpc_dictionary_get_int64(xpc_object_t dict, const char *key, bool *out_valid);

static int64_t
_dnssd_xpc_dictionary_get_int64_limited(xpc_object_t dict, const char *key, int64_t min, int64_t max, bool *out_valid);

static uint16_t
_dnssd_xpc_dictionary_get_uint16(xpc_object_t dict, const char *key, bool *out_valid);

static uint32_t
_dnssd_xpc_dictionary_get_uint32(xpc_object_t dict, const char *key, bool *out_valid);

static uint64_t
_dnssd_xpc_dictionary_get_uint64(xpc_object_t dict, const char *key, bool *out_valid);

static uint64_t
_dnssd_xpc_dictionary_get_uint64_limited(xpc_object_t dict, const char *key, uint64_t min, uint64_t max,
	bool *out_valid);

static xpc_object_t
_dnssd_xpc_dictionary_get_value(xpc_object_t dict, const char *key, xpc_type_t type);

#if 0
//======================================================================================================================
#pragma mark - Top-Level Message Dictionaries
#endif

#define DNSSD_XPC_MESSAGE_KEY_COMMAND	"command"
#define DNSSD_XPC_MESSAGE_KEY_ERROR		"error"
#define DNSSD_XPC_MESSAGE_KEY_ID		"id"
#define DNSSD_XPC_MESSAGE_KEY_PARAMS	"params"
#define DNSSD_XPC_MESSAGE_KEY_RESULTS	"results"

//======================================================================================================================

const char *
dnssd_xpc_message_get_command(xpc_object_t msg)
{
	return xpc_dictionary_get_string(msg, DNSSD_XPC_MESSAGE_KEY_COMMAND);
}

//======================================================================================================================

void
dnssd_xpc_message_set_command(xpc_object_t msg, const char *command)
{
	xpc_dictionary_set_string(msg, DNSSD_XPC_MESSAGE_KEY_COMMAND, command);
}

//======================================================================================================================

DNSServiceErrorType
dnssd_xpc_message_get_error(xpc_object_t msg, bool *out_valid)
{
	return _dnssd_xpc_dictionary_get_int32(msg, DNSSD_XPC_MESSAGE_KEY_ERROR, out_valid);
}

//======================================================================================================================

void
dnssd_xpc_message_set_error(xpc_object_t msg, DNSServiceErrorType error)
{
	xpc_dictionary_set_int64(msg, DNSSD_XPC_MESSAGE_KEY_ERROR, error);
}

//======================================================================================================================

uint64_t
dnssd_xpc_message_get_id(xpc_object_t msg, bool *out_valid)
{
	return _dnssd_xpc_dictionary_get_uint64(msg, DNSSD_XPC_MESSAGE_KEY_ID, out_valid);
}

//======================================================================================================================

void
dnssd_xpc_message_set_id(xpc_object_t msg, uint64_t ident)
{
	xpc_dictionary_set_uint64(msg, DNSSD_XPC_MESSAGE_KEY_ID, ident);
}

//======================================================================================================================

xpc_object_t
dnssd_xpc_message_get_parameters(xpc_object_t msg)
{
	return xpc_dictionary_get_dictionary(msg, DNSSD_XPC_MESSAGE_KEY_PARAMS);
}

//======================================================================================================================

void
dnssd_xpc_message_set_parameters(xpc_object_t msg, xpc_object_t params)
{
	xpc_dictionary_set_value(msg, DNSSD_XPC_MESSAGE_KEY_PARAMS, params);
}

//======================================================================================================================

xpc_object_t
dnssd_xpc_message_get_results(xpc_object_t msg)
{
	return xpc_dictionary_get_array(msg, DNSSD_XPC_MESSAGE_KEY_RESULTS);
}

//======================================================================================================================

void
dnssd_xpc_message_set_results(xpc_object_t msg, xpc_object_t results)
{
	xpc_dictionary_set_value(msg, DNSSD_XPC_MESSAGE_KEY_RESULTS, results);
}

#if 0
//======================================================================================================================
#pragma mark - Parameter Dictionaries
#endif

#define DNSSD_XPC_PARAMETERS_KEY_DELEGATE_ID		"delegate_id"
#define DNSSD_XPC_PARAMETERS_KEY_FLAGS				"flags"
#define DNSSD_XPC_PARAMETERS_KEY_HOSTNAME			"hostname"
#define DNSSD_XPC_PARAMETERS_KEY_INTERFACE_INDEX	"interface_index"
#define DNSSD_XPC_PARAMETERS_KEY_NEED_AUTH_TAGS		"need_auth_tags"
#define DNSSD_XPC_PARAMETERS_KEY_PROTOCOLS			"protocols"

//======================================================================================================================

pid_t
dnssd_xpc_parameters_get_delegate_pid(xpc_object_t params, bool *out_valid)
{
	return (pid_t)_dnssd_xpc_dictionary_get_int64(params, DNSSD_XPC_PARAMETERS_KEY_DELEGATE_ID, out_valid);
}

//======================================================================================================================

void
dnssd_xpc_parameters_set_delegate_pid(xpc_object_t params, pid_t pid)
{
	xpc_dictionary_set_int64(params, DNSSD_XPC_PARAMETERS_KEY_DELEGATE_ID, pid);
}

//======================================================================================================================

const uint8_t *
dnssd_xpc_parameters_get_delegate_uuid(xpc_object_t params)
{
	return xpc_dictionary_get_uuid(params, DNSSD_XPC_PARAMETERS_KEY_DELEGATE_ID);
}

//======================================================================================================================

void
dnssd_xpc_parameters_set_delegate_uuid(xpc_object_t params, uuid_t uuid)
{
	xpc_dictionary_set_uuid(params, DNSSD_XPC_PARAMETERS_KEY_DELEGATE_ID, uuid);
}

//======================================================================================================================

DNSServiceFlags
dnssd_xpc_parameters_get_flags(xpc_object_t params, bool *out_valid)
{
	return _dnssd_xpc_dictionary_get_uint32(params, DNSSD_XPC_PARAMETERS_KEY_FLAGS, out_valid);
}

//======================================================================================================================

void
dnssd_xpc_parameters_set_flags(xpc_object_t params, DNSServiceFlags flags)
{
	xpc_dictionary_set_uint64(params, DNSSD_XPC_PARAMETERS_KEY_FLAGS, flags);
}

//======================================================================================================================

xpc_object_t
dnssd_xpc_parameters_get_hostname_object(xpc_object_t params)
{
	return _dnssd_xpc_dictionary_get_value(params, DNSSD_XPC_PARAMETERS_KEY_HOSTNAME, XPC_TYPE_STRING);
}

//======================================================================================================================

void
dnssd_xpc_parameters_set_hostname(xpc_object_t params, const char *hostname)
{
	xpc_dictionary_set_string(params, DNSSD_XPC_PARAMETERS_KEY_HOSTNAME, hostname);
}

//======================================================================================================================

uint32_t
dnssd_xpc_parameters_get_interface_index(xpc_object_t params, bool *out_valid)
{
	return _dnssd_xpc_dictionary_get_uint32(params, DNSSD_XPC_PARAMETERS_KEY_INTERFACE_INDEX, out_valid);
}

//======================================================================================================================

void
dnssd_xpc_parameters_set_interface_index(xpc_object_t params, uint32_t interface_index)
{
	xpc_dictionary_set_uint64(params, DNSSD_XPC_PARAMETERS_KEY_INTERFACE_INDEX, interface_index);
}

//======================================================================================================================

bool
dnssd_xpc_parameters_get_need_authentication_tags(xpc_object_t params)
{
	return xpc_dictionary_get_bool(params, DNSSD_XPC_PARAMETERS_KEY_NEED_AUTH_TAGS);
}

//======================================================================================================================

void
dnssd_xpc_parameters_set_need_authentication_tags(xpc_object_t params, bool need_auth_tags)
{
	xpc_dictionary_set_bool(params, DNSSD_XPC_PARAMETERS_KEY_NEED_AUTH_TAGS, need_auth_tags);
}

//======================================================================================================================

DNSServiceProtocol
dnssd_xpc_parameters_get_protocols(xpc_object_t params, bool *out_valid)
{
	return _dnssd_xpc_dictionary_get_uint32(params, DNSSD_XPC_PARAMETERS_KEY_PROTOCOLS, out_valid);
}

//======================================================================================================================

void
dnssd_xpc_parameters_set_protocols(xpc_object_t params, DNSServiceProtocol protocols)
{
	xpc_dictionary_set_uint64(params, DNSSD_XPC_PARAMETERS_KEY_PROTOCOLS, protocols);
}

#if 0
//======================================================================================================================
#pragma mark - Result Dictionaries
#endif

#define DNSSD_XPC_RESULT_KEY_AUTH_TAG			"auth_tag"
#define DNSSD_XPC_RESULT_KEY_ERROR				"error"
#define DNSSD_XPC_RESULT_KEY_FLAGS				"flags"
#define DNSSD_XPC_RESULT_KEY_INTERFACE_INDEX	"interface_index"
#define DNSSD_XPC_RESULT_KEY_RECORD_CLASS		"rclass"
#define DNSSD_XPC_RESULT_KEY_RECORD_DATA		"rdata"
#define DNSSD_XPC_RESULT_KEY_RECORD_NAME		"rname"
#define DNSSD_XPC_RESULT_KEY_RECORD_TYPE		"rtype"

//======================================================================================================================

xpc_object_t
dnssd_xpc_result_get_authentication_tag_object(xpc_object_t result)
{
	return _dnssd_xpc_dictionary_get_value(result, DNSSD_XPC_RESULT_KEY_AUTH_TAG, XPC_TYPE_DATA);
}

//======================================================================================================================

void
dnssd_xpc_result_set_authentication_tag(xpc_object_t result, const void *auth_tag_ptr, size_t auth_tag_len)
{
	xpc_dictionary_set_data(result, DNSSD_XPC_RESULT_KEY_AUTH_TAG, auth_tag_ptr, auth_tag_len);
}

//======================================================================================================================

DNSServiceErrorType
dnssd_xpc_result_get_error(xpc_object_t result, bool *out_valid)
{
	return _dnssd_xpc_dictionary_get_int32(result, DNSSD_XPC_RESULT_KEY_ERROR, out_valid);
}

//======================================================================================================================

void
dnssd_xpc_result_set_error(xpc_object_t result, DNSServiceErrorType error)
{
	xpc_dictionary_set_int64(result, DNSSD_XPC_RESULT_KEY_ERROR, error);
}

//======================================================================================================================

DNSServiceFlags
dnssd_xpc_result_get_flags(xpc_object_t result, bool *out_valid)
{
	return _dnssd_xpc_dictionary_get_uint32(result, DNSSD_XPC_RESULT_KEY_FLAGS, out_valid);
}

//======================================================================================================================

void
dnssd_xpc_result_set_flags(xpc_object_t result, DNSServiceFlags flags)
{
	xpc_dictionary_set_uint64(result, DNSSD_XPC_RESULT_KEY_FLAGS, flags);
}

//======================================================================================================================

uint32_t
dnssd_xpc_result_get_interface_index(xpc_object_t result, bool *out_valid)
{
	return _dnssd_xpc_dictionary_get_uint32(result, DNSSD_XPC_RESULT_KEY_INTERFACE_INDEX, out_valid);
}

//======================================================================================================================

void
dnssd_xpc_result_set_interface_index(xpc_object_t result, uint32_t interface_index)
{
	xpc_dictionary_set_uint64(result, DNSSD_XPC_RESULT_KEY_INTERFACE_INDEX, interface_index);
}

//======================================================================================================================

uint16_t
dnssd_xpc_result_get_record_class(xpc_object_t result, bool *out_valid)
{
	return _dnssd_xpc_dictionary_get_uint16(result, DNSSD_XPC_RESULT_KEY_RECORD_CLASS, out_valid);
}

//======================================================================================================================

void
dnssd_xpc_result_set_record_class(xpc_object_t result, uint16_t class)
{
	xpc_dictionary_set_uint64(result, DNSSD_XPC_RESULT_KEY_RECORD_CLASS, class);
}

//======================================================================================================================

xpc_object_t
dnssd_xpc_result_get_record_data_object(xpc_object_t result)
{
	return _dnssd_xpc_dictionary_get_value(result, DNSSD_XPC_RESULT_KEY_RECORD_DATA, XPC_TYPE_DATA);
}

//======================================================================================================================

void
dnssd_xpc_result_set_record_data(xpc_object_t result, const void *data_ptr, size_t data_len)
{
	xpc_dictionary_set_data(result, DNSSD_XPC_RESULT_KEY_RECORD_DATA, data_ptr, data_len);
}

//======================================================================================================================

xpc_object_t
dnssd_xpc_result_get_record_name_object(xpc_object_t result)
{
	return _dnssd_xpc_dictionary_get_value(result, DNSSD_XPC_RESULT_KEY_RECORD_NAME, XPC_TYPE_STRING);
}

//======================================================================================================================

void
dnssd_xpc_result_set_record_name(xpc_object_t result, const char *name)
{
	xpc_dictionary_set_string(result, DNSSD_XPC_RESULT_KEY_RECORD_NAME, name);
}

//======================================================================================================================

uint16_t
dnssd_xpc_result_get_record_type(xpc_object_t result, bool *out_valid)
{
	return _dnssd_xpc_dictionary_get_uint16(result, DNSSD_XPC_RESULT_KEY_RECORD_TYPE, out_valid);
}

//======================================================================================================================

void
dnssd_xpc_result_set_record_type(xpc_object_t result, uint16_t type)
{
	xpc_dictionary_set_uint64(result, DNSSD_XPC_RESULT_KEY_RECORD_TYPE, type);
}

#if 0
//======================================================================================================================
#pragma mark - XPC Dictionary Helpers
#endif

static int32_t
_dnssd_xpc_dictionary_get_int32(xpc_object_t dict, const char *key, bool *out_valid)
{
	return (int32_t)_dnssd_xpc_dictionary_get_int64_limited(dict, key, INT32_MIN, INT32_MAX, out_valid);
}

//======================================================================================================================

static int64_t
_dnssd_xpc_dictionary_get_int64(xpc_object_t dict, const char *key, bool *out_valid)
{
	return _dnssd_xpc_dictionary_get_int64_limited(dict, key, INT64_MIN, INT64_MAX, out_valid);
}

//======================================================================================================================

static int64_t
_dnssd_xpc_dictionary_get_int64_limited(xpc_object_t dict, const char *key, int64_t min, int64_t max, bool *out_valid)
{
	int64_t	i64;
	bool	valid;

	xpc_object_t const value = _dnssd_xpc_dictionary_get_value(dict, key, XPC_TYPE_INT64);
	if (value) {
		i64 = xpc_int64_get_value(value);
		if ((i64 >= min) && (i64 <= max)) {
			valid	= true;
		} else {
			i64		= 0;
			valid	= false;
		}
	} else {
		i64		= 0;
		valid	= false;
	}
	if (out_valid) {
		*out_valid = valid;
	}
	return i64;
}

//======================================================================================================================

static uint16_t
_dnssd_xpc_dictionary_get_uint16(xpc_object_t dict, const char *key, bool *out_valid)
{
	return (uint16_t)_dnssd_xpc_dictionary_get_uint64_limited(dict, key, 0, UINT16_MAX, out_valid);
}

//======================================================================================================================

static uint32_t
_dnssd_xpc_dictionary_get_uint32(xpc_object_t dict, const char *key, bool *out_valid)
{
	return (uint32_t)_dnssd_xpc_dictionary_get_uint64_limited(dict, key, 0, UINT32_MAX, out_valid);
}

//======================================================================================================================

static uint64_t
_dnssd_xpc_dictionary_get_uint64(xpc_object_t dict, const char *key, bool *out_valid)
{
	return _dnssd_xpc_dictionary_get_uint64_limited(dict, key, 0, UINT64_MAX, out_valid);
}

//======================================================================================================================

static uint64_t
_dnssd_xpc_dictionary_get_uint64_limited(xpc_object_t dict, const char *key, uint64_t min, uint64_t max,
	bool *out_valid)
{
	uint64_t	u64;
	bool		valid;

	xpc_object_t const value = _dnssd_xpc_dictionary_get_value(dict, key, XPC_TYPE_UINT64);
	if (value) {
		u64 = xpc_uint64_get_value(value);
		if ((u64 >= min) && (u64 <= max)) {
			valid	= true;
		} else {
			u64		= 0;
			valid	= false;
		}
	} else {
		u64		= 0;
		valid	= false;
	}
	if (out_valid) {
		*out_valid = valid;
	}
	return u64;
}

//======================================================================================================================

static xpc_object_t
_dnssd_xpc_dictionary_get_value(xpc_object_t dict, const char *key, xpc_type_t type)
{
	xpc_object_t value = xpc_dictionary_get_value(dict, key);
	return (value && (xpc_get_type(value) == type)) ? value : NULL;
}
