/*
 * Copyright (c) 2004-2015 Apple Inc. All rights reserved.
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

#ifndef __ASL_H__
#define __ASL_H__

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <sys/cdefs.h>
#include <Availability.h>

/* Version number encodes the date YYYYMMDD */
#define ASL_API_VERSION 20150225

typedef struct __asl_object_s *asl_object_t;
typedef asl_object_t aslclient;
typedef asl_object_t aslmsg;
typedef asl_object_t aslresponse;

/*! @header
 * These routines are deprecated and replaced by os_log(3).  On OS versions
 * with os_log(3), the ASL routines for emitting log messages are shimmed into
 * the equivalent os_log functionality.  Functions whose deprecation messages
 * indicate they are unsupported will have no effect on OS versions with
 * os_log(3) support.
 *
 * These routines provide an interface to the Apple System Log facility.
 * The API allows client applications to create flexible, structured messages
 * and send them to the syslogd server.  Messages received by the server are
 * saved in a data store, subject to input filtering constraints.
 * This API also permits clients to create queries and search the message
 * data store for matching messages.
 */

/*
 * NOTE FOR HeaderDoc
 *
 * These are added to allow headerdoc2html to process 
 * the prototypes of asl_log and asl_vlog correctly.
 * The "-p" option to headerdoc2html is required.
 */
#ifndef __printflike
/*! @parseOnly */
#define __printflike(a,b)
#endif

#pragma mark -

/*! @defineblock Log Message Priority Levels
 * Log levels of the message.
 */
#define ASL_LEVEL_EMERG   0
#define ASL_LEVEL_ALERT   1
#define ASL_LEVEL_CRIT    2
#define ASL_LEVEL_ERR     3
#define ASL_LEVEL_WARNING 4
#define ASL_LEVEL_NOTICE  5
#define ASL_LEVEL_INFO    6
#define ASL_LEVEL_DEBUG   7
/*! @/defineblock */

#pragma mark -

/*! @defineblock Log Message Priority Level Strings
 * Strings corresponding to log levels.
 */
#define ASL_STRING_EMERG	"Emergency"
#define ASL_STRING_ALERT	"Alert"
#define ASL_STRING_CRIT		"Critical"
#define ASL_STRING_ERR		"Error"
#define ASL_STRING_WARNING  "Warning"
#define ASL_STRING_NOTICE   "Notice"
#define ASL_STRING_INFO		"Info"
#define ASL_STRING_DEBUG	"Debug"
/*! @/defineblock */

#pragma mark -

/*! @defineblock Attribute Matching
 * Attribute value comparison operations.
 */
#define ASL_QUERY_OP_CASEFOLD      0x0010
#define ASL_QUERY_OP_PREFIX		   0x0020
#define ASL_QUERY_OP_SUFFIX		   0x0040
#define ASL_QUERY_OP_SUBSTRING     0x0060
#define ASL_QUERY_OP_NUMERIC       0x0080
#define ASL_QUERY_OP_REGEX         0x0100

#define ASL_QUERY_OP_EQUAL         0x0001
#define ASL_QUERY_OP_GREATER       0x0002
#define ASL_QUERY_OP_GREATER_EQUAL 0x0003
#define ASL_QUERY_OP_LESS          0x0004
#define ASL_QUERY_OP_LESS_EQUAL    0x0005
#define ASL_QUERY_OP_NOT_EQUAL     0x0006
#define ASL_QUERY_OP_TRUE          0x0007
/*! @/defineblock */

#pragma mark -

/*! @defineblock Message Attributes
 * These attributes are known by ASL, and are generally
 * associated with all log messages.
 * Additional attributes may be added as desired.
 */
#define ASL_KEY_TIME               "Time"                 /* Timestamp.  Set automatically */
#define ASL_KEY_TIME_NSEC          "TimeNanoSec"          /* Nanosecond time. */
#define ASL_KEY_HOST               "Host"                 /* Sender's address (set by the server). */
#define ASL_KEY_SENDER             "Sender"               /* Sender's identification string.  Default is process name. */
#define ASL_KEY_FACILITY           "Facility"             /* Sender's facility.  Default is "user". */
#define ASL_KEY_PID                "PID"                  /* Sending process ID encoded as a string.  Set automatically. */
#define ASL_KEY_UID                "UID"                  /* UID that sent the log message (set by the server). */
#define ASL_KEY_GID                "GID"                  /* GID that sent the log message (set by the server). */
#define ASL_KEY_LEVEL              "Level"                /* Log level number encoded as a string.  See levels above. */
#define ASL_KEY_MSG                "Message"              /* Message text. */
#define ASL_KEY_READ_UID           "ReadUID"              /* User read access (-1 is any user). */
#define ASL_KEY_READ_GID           "ReadGID"              /* Group read access (-1 is any group). */
#define ASL_KEY_EXPIRE_TIME        "ASLExpireTime"        /* Expiration time for messages with long TTL. */
#define ASL_KEY_MSG_ID             "ASLMessageID"         /* 64-bit message ID number (set by the server). */
#define ASL_KEY_SESSION            "Session"              /* Session (set by the launchd). */
#define ASL_KEY_REF_PID            "RefPID"               /* Reference PID for messages proxied by launchd */
#define ASL_KEY_REF_PROC           "RefProc"              /* Reference process for messages proxied by launchd */
#define ASL_KEY_AUX_TITLE          "ASLAuxTitle"          /* Auxiliary title string */
#define ASL_KEY_AUX_UTI            "ASLAuxUTI"            /* Auxiliary Uniform Type ID */
#define ASL_KEY_AUX_URL            "ASLAuxURL"            /* Auxiliary Uniform Resource Locator */
#define ASL_KEY_AUX_DATA           "ASLAuxData"           /* Auxiliary in-line data */
#define ASL_KEY_OPTION             "ASLOption"            /* Internal */
#define ASL_KEY_MODULE             "ASLModule"            /* Internal */
#define ASL_KEY_SENDER_INSTANCE	   "SenderInstance"       /* Sender instance UUID. */
#define ASL_KEY_SENDER_MACH_UUID   "SenderMachUUID"       /* Sender Mach-O UUID. */
#define ASL_KEY_FINAL_NOTIFICATION "ASLFinalNotification" /* syslogd posts value as a notification when message has been processed */
#define ASL_KEY_OS_ACTIVITY_ID     "OSActivityID"         /* Current OS Activity for the logging thread */
/*! @/defineblock */

#pragma mark -

/*! @defineblock ASL Object Types
 * The library uses only one opaque type - asl_object_t.
 * Many of the routines can operate on several different types.
 * For example, asl_search() can be used to search a list of messages,
 * an ASL database directory or data file, or the main ASL database.
 * It can even be used to check a single message against a query
 * message, or against another message to check for exact match.
 *
 * The first three types are container objects - messages, queries,
 * and lists of messages or queries.  The following types are
 * abstractions for ASL data files and ASL data stores (directories
 * containing data files).
 *
 * ASL_TYPE_CLIENT is a high-level object that abstracts ASL
 * interactions.  It may access ASL stores or files directly,
 * and it may communicate with ASL daemons.
 * 
 */
#define ASL_TYPE_UNDEF  0xffffffff
#define ASL_TYPE_MSG    0
#define ASL_TYPE_QUERY  1
#define ASL_TYPE_LIST   2
#define ASL_TYPE_FILE   3
#define ASL_TYPE_STORE  4
#define ASL_TYPE_CLIENT 5
/*! @/defineblock */

#pragma mark -

/*! @defineblock search directions
 * Used for asl_store_match(), asl_file_match(), and asl_match().
 */
#define ASL_MATCH_DIRECTION_FORWARD	1
#define ASL_MATCH_DIRECTION_REVERSE	-1
/*! @/defineblock */

#pragma mark -

/*! @defineblock Filter Masks
 * Used in client-side filtering, which determines which
 * messages are sent by the client to the syslogd server.
 */
#define ASL_FILTER_MASK_EMERG   0x01
#define ASL_FILTER_MASK_ALERT   0x02
#define ASL_FILTER_MASK_CRIT    0x04
#define ASL_FILTER_MASK_ERR     0x08
#define ASL_FILTER_MASK_WARNING 0x10
#define ASL_FILTER_MASK_NOTICE  0x20
#define ASL_FILTER_MASK_INFO    0x40
#define ASL_FILTER_MASK_DEBUG   0x80
/*! @/defineblock */

#pragma mark -

/*! @defineblock Filter Mask Macros
 * Macros to create bitmasks for filter settings - see asl_set_filter().
 */
#define	ASL_FILTER_MASK(level) (1 << (level))
#define	ASL_FILTER_MASK_UPTO(level) ((1 << ((level) + 1)) - 1)
/*! @/defineblock */

#pragma mark -

/*! @defineblock Client Creation Options
 * Options for asl_open().
 * Note that ASL_OPT_NO_DELAY no longer has any effect.
 */
#define ASL_OPT_STDERR		0x00000001
#define ASL_OPT_NO_DELAY    0x00000002
#define ASL_OPT_NO_REMOTE   0x00000004
/*! @/defineblock */

#pragma mark -

/*! @defineblock File and Store Open Options
 * Options for asl_open_path().
 */
#define ASL_OPT_OPEN_WRITE   0x00000001
#define ASL_OPT_CREATE_STORE 0x00000002
/*! @/defineblock */

#pragma mark -

/*! @defineblock File Descriptor Types
 * Instructions on how to treat the file descriptor in asl_log_descriptor().
 */
#define ASL_LOG_DESCRIPTOR_READ  1
#define ASL_LOG_DESCRIPTOR_WRITE 2

#pragma mark -

/*! @defineblock Output file message and time formats.
 * These select internally defined formats for printed log messages for
 * asl_add_output_file().  Custom message and time formats may also be
 * used.  These pre-defined formats and custom formats are described in detail
 * in the syslog(1) manual page.
 */
#define ASL_MSG_FMT_RAW "raw"
#define ASL_MSG_FMT_STD "std"
#define ASL_MSG_FMT_BSD "bsd"
#define ASL_MSG_FMT_XML "xml"
#define ASL_MSG_FMT_MSG "msg"

#define ASL_TIME_FMT_SEC "sec"
#define ASL_TIME_FMT_UTC "utc"
#define ASL_TIME_FMT_LCL "lcl"

#pragma mark -

/*! @defineblock Text Encoding Types
 * These are used by the library when formatting messages to be written 
 * to file descriptors associated with an ASL client handle with 
 * asl_add_output_file().  The syslog(1) manual page describes text encoding
 * in detail.  ASL_ENCODE_ASL corresponds to the "vis" encoding option
 * described in the syslog(1) manual.  ASL_ENCODE_XML should be used in
 * combination with ASL_MSG_FMT_XML to ensure that special XML characters
 * are correctly encoded.
 */
#define ASL_ENCODE_NONE 0
#define ASL_ENCODE_SAFE 1
#define ASL_ENCODE_ASL  2
#define ASL_ENCODE_XML  3

#pragma mark -

/*!
 * ASL_PREFILTER_LOG is a macro similar to asl_log(), but it first checks
 * if the message will simply be ignored due to local filter settings.
 * This prevents the variable argument list from being evaluated.
 * Note that the message may still be processed if it will be written
 * to a file or stderr.
 *
 * @param client
 *    (input) An ASL_TYPE_CLIENT object.
 * @param msg
 *    (input) An asl_object_t of type ASL_TYPE_MSG (default attributes will be supplied if msg is NULL).
 * @param level
 *    (input) Log level (ASL_LEVEL_DEBUG to ASL_LEVEL_EMERG).
 * @param format
 *    (input) A printf() - style format string followed by a list of arguments.
 */
#define ASL_PREFILTER_LOG(client, msg, level, format, ...) \
	do { \
		asl_object_t _client = (client); \
		asl_object_t _msg = (msg); \
		uint32_t _asl_eval = _asl_evaluate_send(_client, _msg, (level)); \
		if (_asl_eval != 0) _asl_lib_log(_client, _asl_eval, _msg, (format), ## __VA_ARGS__); \
	} while (0)

#pragma mark -

__BEGIN_DECLS

/* ASL Library SPI - do not call directly */
int _asl_lib_log(asl_object_t client, uint32_t eval, asl_object_t msg, const char *format, ...) __printflike(4, 5);
uint32_t _asl_evaluate_send(asl_object_t client, asl_object_t msg, int level);

/*!
 * Initialize a connection to the ASL server.
 *
 * This call is optional in many cases.  The library will perform any
 * necessary initializations on the fly.  A call to asl_open() is required
 * if optional settings must be made before messages are sent to the server.
 * These include setting the client filter and managing additional output
 * file descriptors.  Note that the default setting of the client filter is
 * ASL_FILTER_MASK_UPTO(ASL_LEVEL_NOTICE), so ASL_LEVEL_DEBUG and ASL_LEVEL_INFO
 * messages are not sent to the server by default.
 * A separate client connection is required for multiple threads or
 * dispatch queues.
 *
 * Options (defined above) may be set using the opts parameter. They are:
 *
 *   ASL_OPT_STDERR    - adds stderr as an output file descriptor
 *
 *   ASL_OPT_NO_REMOTE - disables the remote-control mechanism for adjusting
 *                       filter levers for processes using e.g. syslog -c ...
 *
 * @param ident
 *    (input) Sender name.
 * @param facility
 *    (input) Facility name.
 * @param opts
 *    (input) Options (see Client Creation Options).
 * @result Returns an ASL client handle (asl_object_t of type ASL_TYPE_CLIENT).
 */
asl_object_t asl_open(const char *ident, const char *facility, uint32_t opts) __API_DEPRECATED("os_log(3) has replaced asl(3); see os_log_create(3)", macosx(10.4,10.12), ios(2.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/*!
 * Open an ASL database or ASL data file for read or write access.
 *
 * Opens an ASL database if the path specifies a directory, or
 * an ASL data file if the path specifies a file. Opens the system
 * ASL database if path is NULL.
 *
 * If the ASL_OPT_OPEN_READ option is specified, the database or data file may be
 * searched with asl_search() or asl_match(). asl_next() and asl_prev() may be used
 * to iterate over the messages in the database or file.
 *
 * If the ASL_OPT_OPEN_WRITE option is specified, an existing file or database is
 * opened for writing.  New messages may be added to the file or database using
 * asl_append(), asl_send(), asl_log(), or asl_vlog().  Existing messages in the
 * store or file may not be deleted or modified.
 *
 * If the path does not exist, asl_open_path() will create a new database if
 * ASL_OPT_CREATE_STORE is set in the options, or a new data file otherwise.
 * The file will be created with the user's effective UID and GID as owner and
 * group.  The mode will be 0644.  If a different mode, UID, or GID is desired,
 * an empty file or directory may be pre-created with the desired settings.
 *
 * @param path
 *    (input) Location of the ASL database or ASL data file in the filesystem.
 *    A value of NULL may be used to open the system's database.
 * @param opts
 *    (input) Options (see File and Store Open Options).
 * @result Returns an ASL object of type ASL_TYPE_STORE or ASL_TYPE_FILE, or NULL on failure.
 */
asl_object_t asl_open_path(const char *path, uint32_t opts) __API_DEPRECATED("os_log(3) has replaced asl(3); this functionality is no longer supported", macosx(10.10,10.12), ios(8.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/*!
 * Shuts down a connection to the server.
 * This routine is identical to asl_release().
 *
 * @param obj
 *    (input) An ASL object.
 */
void asl_close(asl_object_t obj) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.4,10.12), ios(2.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/*!
 * Write log messages to the given file descriptor.
 *
 * Log messages will be written to this file as well as to the server.
 * This is equivalent to calling:
 * asl_add_output_file(asl, descriptor, ASL_MSG_FMT_STD, ASL_TIME_FMT_LCL, ASL_FILTER_MASK_UPTO(ASL_LEVEL_DEBUG), ASL_ENCODE_SAFE)
 *
 * @param client
 *    (input) An ASL client handle (asl_object_t of type ASL_TYPE_CLIENT).
 * @param descriptor
 *    (input) A file descriptor.
 * @result Returns 0 on success, non-zero on failure.
*/
int asl_add_log_file(asl_object_t client, int descriptor) __API_DEPRECATED("os_log(3) has replaced asl(3); this functionality is no longer supported", macosx(10.4,10.12), ios(2.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/*!
 * Write log messages to the given file descriptor.
 *
 * Log messages will be written to this file as well as to the server.
 * This routine extends the basic interface offered by asl_add_log_file(),
 * allowing control of the format used to write log message written to the file.
 * control of the time zone used when printing time values, and allowing
 * individual filtering control for each log file.
 *
 * @param client
 *    (input) An ASL client handle (asl_object_t of type ASL_TYPE_CLIENT).
 * @param descriptor
 *    (input) A file descriptor.
 * @param mfmt
 *    (input) A character string specifying the message format.
 * @param tfmt
 *    (input) A character string specifying the time format.
 * @param filter
 *    (input) A filter value.
 * @param text_encoding
 *    (input) A text encoding type.
 * @result Returns 0 on success, non-zero on failure.
 */
int asl_add_output_file(asl_object_t client, int fd, const char *mfmt, const char *tfmt, int filter, int text_encoding) __API_DEPRECATED("os_log(3) has replaced asl(3); this functionality is no longer supported", macosx(10.10,10.12), ios(7.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/*!
 * Write log messages to the given file descriptor.
 *
 * Sets or changes a filter value for filtering messages written to a file associated
 * with an ASL client handle using asl_add_output_file() or asl_add_log_file(). 
 *
 * @param client
 *    (input) An ASL client handle (asl_object_t of type ASL_TYPE_CLIENT).
 * @param descriptor
 *    (input) A file descriptor.
 * @param filter
 *    (input) A filter value.
 * @result Returns the previous filter value.
 */
int asl_set_output_file_filter(asl_object_t client, int fd, int filter) __API_DEPRECATED("os_log(3) has replaced asl(3); this functionality is no longer supported", macosx(10.10,10.12), ios(7.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/*!
 * Stop writing log messages to the given file descriptor.
 * The file descriptor is not closed by this routine.
 *
 * @param client
 *    (input) An ASL client handle (asl_object_t of type ASL_TYPE_CLIENT).
 * @param descriptor
 *    (input) A file descriptor.
 * @result Returns 0 on success, non-zero on failure.
 */
int asl_remove_log_file(asl_object_t client, int descriptor) __API_DEPRECATED("os_log(3) has replaced asl(3); this functionality is no longer supported", macosx(10.4,10.12), ios(2.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/*!
 * Set a filter for messages being sent to the server.
 * The filter is a bitmask representing priorities.  The ASL_FILTER_MASK
 * macro may be used to convert a priority level into a bitmask for that
 * level.  The ASL_FILTER_MASK_UPTO macro creates a bitmask for all
 * priorities up to and including a given priority.
 * Messages with priority levels that do not have a corresponding bit 
 * set in the filter are not sent to the server, although they will be
 * sent to any file descripters added with asl_add_log_file().
 * The default setting is ASL_FILTER_MASK_UPTO(ASL_LEVEL_NOTICE).
 * Returns the previous filter value.
 *
 * @param client
 *    (input) An ASL client handle (asl_object_t of type ASL_TYPE_CLIENT).
 * @param f
 *    (input) A filter value.
 * @result Returns the previous filter value.
 */
int asl_set_filter(asl_object_t client, int f) __API_DEPRECATED("os_log(3) has replaced asl(3); see log(1)'s config command", macosx(10.4,10.12), ios(2.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/*
 * Examine attribute keys.
 *
 * @param msg
 *    (input) An ASL message or query (asl_object_t of type ASL_TYPE_MSG or ASL_TYPE_QUERY).
 * @param n
 *    (input) An index value.
 * @result Returns the key of the nth attribute in a message (beginning at zero),
 * or NULL if n is greater than the largest message index.
 */
const char *asl_key(asl_object_t msg, uint32_t n) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.4,10.12), ios(2.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/*
 * Examine attribute keys.
 *
 * @param msg
 *    (input) An ASL message or query (asl_object_t of type ASL_TYPE_MSG or ASL_TYPE_QUERY).
 * @param key
 *    (output) key at the given index.  May be NULL.
 * @param val
 *    (output) val at the given index.  May be NULL.
 * @param op
 *    (output) op at the given index.  May be NULL.
 * @param n
 *    (input) An index value.
 * @result returns 0 for success, non-zero for failure.
 */
int asl_fetch_key_val_op(asl_object_t msg, uint32_t n, const char **key, const char **val, uint32_t *op) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(8.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/*!
 * Create a new log message, query message, message list, or a connection to the system database.
 *
 * @param type
 *    (input) ASL_TYPE_MSG, ASL_TYPE_QUERY, ASL_TYPE_LIST, or ASL_TYPE_CLIENT.
 * @result Returns a newly allocated asl_object_t of the specified type.
 *
 * @discussion
 *    New objects of type ASL_TYPE_CLIENT will be created with default settings for 
 *    a client connection, equivalent to asl_open(NULL, NULL, 0).
 *    The Sender and Facility values associated with an ASL_TYPE_CLIENT may
 *    be reset using asl_set().
 */
asl_object_t asl_new(uint32_t type) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.4,10.12), ios(2.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/*!
 * Set or re-set a message or query attribute.
 * May also be used to set values associated with an ASL_TYPE_CLIENT object,
 * such as Sender and Facility.
 *
 * @param obj
 *    (input) An ASL object of type ASL_TYPE_MSG, ASL_TYPE_QUERY, or ASL_TYPE_CLIENT.
 * @param key
 *    (input) Attribute key.
 * @param value
 *    (input) Attribute value.
 * @result returns 0 for success, non-zero for failure.
 */
int asl_set(asl_object_t obj, const char *key, const char *value) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.4,10.12), ios(2.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/*!
 * Remove a key/value attribute.
 *
 * @param obj
 *    (input) An ASL object of type ASL_TYPE_MSG, ASL_TYPE_QUERY, or ASL_TYPE_CLIENT.
 * @param key
 *    (input) Attribute key.
 * returns 0 for success, non-zero for failure.
 */
int asl_unset(asl_object_t obj, const char *key) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.4,10.12), ios(2.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/*!
 * Get the value associated with an attribute key.
 *
 * @param obj
 *    (input) An ASL object of type ASL_TYPE_MSG, ASL_TYPE_QUERY, or ASL_TYPE_CLIENT.
 * @param key
 *    (input) Attribute key.
 * @result Returns the attribute value, or NULL if the object does not contain the key.
 */
const char *asl_get(asl_object_t msg, const char *key) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.4,10.12), ios(2.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/*!
 * Log a message with a particular log level.
 *
 * @param obj
 *    (input) An asl_object_t or NULL.
 * @param msg
 *    (input) An asl_object_t of type ASL_TYPE_MSG (default attributes will be supplied if msg is NULL).
 * @param level
 *    (input) Log level (ASL_LEVEL_DEBUG to ASL_LEVEL_EMERG).
 * @param format
 *    (input) A printf() - style format string followed by a list of arguments.
 * @result Returns 0 for success, non-zero for failure.
 *
 * @discussion
 *    The input object may be of any type.
 *    In typical usage, obj is of type ASL_TYPE_CLIENT or obj is NULL.
 *    NULL causes the library to use the default ASL client handle.
 *    This routine prepares a message for tranmission to the ASL server daemon (syslogd),
 *    The message is sent to the server subject to filter settings.  The message may also
 *    be formatted and printed to various output files.
 *
 *    For ASL_TYPE_MSG, this routine will set all key/value pairs in the input object as
 *    they would appear if the message were being sent to the server.  This includes
 *    setting alues for ASL_KEY_TIME, ASL_KEY_TIME_NSEC, ASL_KEY_HOST, and so on.
 *
 *    If the object is of type ASL_TYPE_STORE or ASL_TYPE_FILE, a message will be 
 *    constructed (as above) and saved in the file or data store.  No filtering is done.
 *
 *    If obj is of type ASL_TYPE_LIST, a message is created and appended to the list.
 *
 *    The object type ASL_TYPE_QUERY is supported, but the key/value pairs set in the
 *    object will have an operator value of zero.
 */
int asl_log(asl_object_t client, asl_object_t msg, int level, const char *format, ...) __printflike(4, 5) __not_tail_called __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.4,10.12), ios(2.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/*!
 * Log a message with a particular log level.
 *
 * This API is a simplified version of asl_log().  It uses the default (NULL) ASL client handle,
 * and does not have a msg parameter to supply additonal key/value pairs to be attached to the
 * message sent to the syslogd server.
 *
 * @param level
 *    (input) Log level (ASL_LEVEL_DEBUG to ASL_LEVEL_EMERG).
 * @param format
 *    (input) A printf() - style format string followed by a list of arguments.
 * @result Returns 0 for success, non-zero for failure.
 */
int asl_log_message(int level, const char *format, ...) __printflike(2, 3) __not_tail_called __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(8.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/*!
 * Log a message with a particular log level.
 * Similar to asl_log, but takes a va_list argument.
 *
 * @param asl
 *    (input) An ASL object or NULL.
 * @param msg
 *    (input) An asl_object_t of type ASL_TYPE_MSG (default attributes will be supplied if msg is NULL).
 * @param level
 *    (input) Log level (ASL_LEVEL_DEBUG to ASL_LEVEL_EMERG).
 * @param format
 *    (input) A printf() - style format string followed by a list of arguments.
 * @param ap
 *    (input) A va_list containing the values for the format string.
 * @result Returns 0 for success, non-zero for failure.
 * @discussion 
 *    See the discussion for asl_log() for a description of how this routine treats different
 *    types of input object.
 *
 */
int asl_vlog(asl_object_t obj, asl_object_t msg, int level, const char *format, va_list ap) __printflike(4, 0) __not_tail_called __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.4,10.12), ios(2.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/*!
 * Log a message.
 *
 * This routine may be used instead of asl_log() or asl_vlog() if asl_set() 
 * has been used to set all of a message's attributes.
 *
 * @param asl
 *    (input) An ASL object or NULL.
 * @param msg
 *    (input) An asl_object_t of type ASL_TYPE_MSG.
 * @result Returns 0 for success, non-zero for failure.
 * @discussion
 *    See the discussion for asl_log() for a description of how this routine treats different
 *    types of input object.
 */
int asl_send(asl_object_t obj, asl_object_t msg) __not_tail_called __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.4,10.12), ios(2.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/*!
 * DEPRECATED: Free an ASL object and all internal resources associated with it.
 * This routine is identical to asl_release(), which should be used instead.
 * Note that we don't issue a deprecation warning - yet.
 *
 * @param obj
 *    (input) An ASL object to free.
 */
void asl_free(asl_object_t obj) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.4,10.12), ios(2.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/*!
 * Increment the internal reference count of an ASL object.
 *
 * @param obj
 *    (input) An ASL object to retain.
 * @result Returns the object.
 */
asl_object_t asl_retain(asl_object_t obj) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(8.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/*!
 * Decrement the internal reference count of an ASL object.
 * Frees the object when the reference count becomes zero.
 *
 * @param obj
 *    (input) An ASL object to release.
 */
void asl_release(asl_object_t obj) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(8.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/*!
 * Get the internal type of an ASL object.
 *
 * @param obj
 *    (input) An ASL object.
 * @result Returns the object type.
 */
uint32_t asl_get_type(asl_object_t obj) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(8.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/*!
 * Set arbitrary parameters of a query.
 * This is similar to asl_set, but allows richer query operations.
 * See ASL_QUERY_OP_* above.
 *
 * @param msg
 *    (input) An ASL object of type ASL_TYPE_QUERY.
 * @param key
 *    (input) Attribute key 
 * @param value
 *    (input) Attribute value
 * @param op
 *    (input) An operation (ASL_QUERY_OP_*)
 * @result Returns 0 for success, non-zero for failure
 */
int asl_set_query(asl_object_t msg, const char *key, const char *value, uint32_t op) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.4,10.12), ios(2.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/*!
 * Search for messages matching the criteria described by an query object.
 * The caller should set the attributes to match using asl_set_query() or asl_set().
 * The operation ASL_QUERY_OP_EQUAL is used for attributes set with asl_set().
 *
 * @param obj
 *    (input) An ASL object to search.
 * @param query
 *    (input) An asl_object_t of type ASL_TYPE_QUERY or ASL_TYPE_MSG.
 *            query may be NULL, which matches anything.
 * @result Returns an ASL object containing messages matching the query, or NULL if there are no matches.
 *
 * @discussion
 *    The object to search may be of any type.
 *    ASL_TYPE_CLIENT searches the main ASL database.
 *    ASL_TYPE_STORE searches an ASL database in the filesystem.
 *    ASL_TYPE_FILE searches an ASL data file in the filesystem.
 *    ASL_TYPE_LIST searches for matches in a list of messages.
 *
 *    A NULL query matches anything.
 *
 *    If obj is of type ASL_TYPE_MSG and query is of type ASL_TYPE_QUERY, obj is matched against the query,
 *    and a list containing the "obj" object is returned if the match succeeds.
 *
 *    If both obj and query are objects of type ASL_TYPE_MSG or both are of type ASL_TYPE_QUERY,
 *    they are tested for exact match. A list containing the "obj" object is returned if the match is exact.
 *
 *    If obj is of type ASL_TYPE_QUERY and query is of type ASL_TYPE_MSG, the routine returns NULL.
 *
 */
asl_object_t asl_search(asl_object_t obj, asl_object_t query) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.4,10.12), ios(2.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/*!
 * DEPRECATED: Iterate over messages in an asl_object_t (same as an aslresponse).
 * This routine is identical to asl_next().
 *
 * @param list
 *    (input) An asl_object_t (aslresponse).
 * @result Returns the next message contained in an ASL object, or NULL when there are no more messages.
 *
 * @discussion
 *    This routine is deprecated in favor of asl_next().
 */
asl_object_t aslresponse_next(asl_object_t obj) __OSX_AVAILABLE_BUT_DEPRECATED_MSG(__MAC_10_4,__MAC_10_10,__IPHONE_2_0,__IPHONE_7_0, "Use asl_next instead");

/*!
 * DEPRECATED: Free an asl_object_t.
 * This routine is identical to asl_release().
 *
 * @param list
 *    (input) An asl_object_t (aslresponse).
 *
 * @discussion
 *    This routine is deprecated in favor of asl_release().
 */
void aslresponse_free(asl_object_t obj) __OSX_AVAILABLE_BUT_DEPRECATED_MSG(__MAC_10_4,__MAC_10_10,__IPHONE_2_0,__IPHONE_7_0, "Use asl_release instead");

/*!
 * Append messages to an object of type ASL_TYPE_LIST.  The input "obj"
 * parameter may be of type ASL_TYPE_MSG or ASL_TYPE_QUERY, in which case
 * the object is appended to the list, or "obj" may be of type ASL_TYPE_LIST,
 * in which case each object in that list is appended to the "list" object.
 * Does nothing if either list or obj are NULL.
 *
 * @param obj
 *    (input) An object of type ASLTYPE_CLIENT or ASL_TYPE_LIST, or an object of type
 *    ASL_TYPE_FILE or ASL_TYPE_STORE that is open for write operations.
 * @param obj_to_add
 *    (input) An object of type ASL_TYPE_MSG, ASL_TYPE_QUERY or type ASL_TYPE_LIST.
 */
void asl_append(asl_object_t obj, asl_object_t obj_to_add) __not_tail_called __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(8.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/*!
 * Prepend messages to an object of type ASL_TYPE_LIST.  The input "obj"
 * parameter may be of type ASL_TYPE_MSG or ASL_TYPE_QUERY, in which case
 * the object is prepended to the list, or "obj" may be of type ASL_TYPE_LIST,
 * in which case each object in that list is prepended to the "list" object.
 * Does nothing if either list or obj are NULL.
 *
 * @param obj
 *    (input) An object of type ASL_TYPE_LIST.
 * @param obj_to_add
 *    (input) An object of type ASL_TYPE_MSG, ASL_TYPE_QUERY or type ASL_TYPE_LIST.
 */
void asl_prepend(asl_object_t obj, asl_object_t obj_to_add) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(8.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/*!
 * Get the number of key/value pairs in an object of type ASL_TYPE_MSG or ASL_TYPE_QUERY,
 * or the number of components in an object of type ASL_TYPE_LIST.
 *
 * @param obj
 *    (input) An asl_object_t of type ASL_TYPE_MSG, ASL_TYPE_QUERY, or ASL_TYPE_LIST.
 * @result The number of components in the object.
 *    Returns zero if object is empty or NULL, or if the type is not
 *    ASL_TYPE_MSG, ASL_TYPE_QUERY, or ASL_TYPE_LIST.
 */
size_t asl_count(asl_object_t obj) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(8.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/*!
 * Retreive a message from an object of type ASL_TYPE_LIST.
 *
 * @param obj
 *    (input) An asl_object_t of type ASL_TYPE_LIST
 * @result Returns the message (an object of type ASL_TYPE_MSG or ASL_TYPE_QUERY) at the specified index.
 *    Returns NULL if the index is out of range or if list is not an object of type ASL_TYPE_LIST.
 */
asl_object_t asl_get_index(asl_object_t list, size_t index) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(8.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/*!
 * Remove the message at a specified index from an object of type ASL_TYPE_LIST.
 *
 * @param list
 *    (input) An object of type ASL_TYPE_LIST.
 */
void asl_remove_index(asl_object_t list, size_t index) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(8.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/*!
 * Creates an auxiliary file that may be used to save arbitrary data.  The ASL message msg
 * will be saved at the time that the auxiliary file is closed with asl_close_auxiliary_file().
 * The log entry will include any keys and values found in msg, and it will include the title
 * and Uniform Type Identifier specified.  If NULL is supplied as a value for the uti parameter,
 * the type "public.data" is used.  Console.app will display a hyperlink to the file.
 * Output parameter out_descriptor will contain a readable and writable file descriptor for the new
 * auxiliary file. 
 *
 * By default, the file will be world-readable.  If the message contains a ReadUID and/or a
 * ReadGID key, then the values for those keys will determine read access to the file.
 *
 * The file will be deleted at the same time that the message expires from the ASL data store.
 * The aslmanager utility manages message expiry.  If msg contains a value for ASLExpireTime,
 * then the message and the file will not be deleted before that time.  The value may be in
 * seconds after the Epoch, or it may be ctime() format, e.g "Thu Jun 24 18:22:48 2010".
 * 
 * @param msg
 *    (input) An object of type ASL_TYPE_MSG.
 * @param tite
 *    (input) A title string for the file.
 * @param uti
 *    (input) Uniform Type Identifier for the file.
 * @param out_descriptor
 *    (output) A writable file descriptor.
 * @result Returns 0 for success, non-zero for failure
 */
int asl_create_auxiliary_file(asl_object_t msg, const char *title, const char *uti, int *out_descriptor) __API_DEPRECATED("os_log(3) has replaced asl(3); this functionality is no longer supported", macosx(10.7,10.12), ios(5.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/*!
 * Close an auxiliary file opened by asl_create_auxiliary_file() when writing is complete.
 * syslogd will log the message provided to asl_create_auxiliary_file() when this routine
 * is called.
 *
 * @param descriptor
 *    (input) The file descriptor
 * @result Returns 0 for success, non-zero for failure
 */
int asl_close_auxiliary_file(int descriptor) __API_DEPRECATED("os_log(3) has replaced asl(3); this functionality is no longer supported", macosx(10.7,10.12), ios(5.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/*!
 * Sends an ASL message to syslogd along with a title string, Uniform Resource Locator, 
 * and Uniform Type Identifier specified.  Console.app will hyperlink the title string to
 * the specified URL.  If NULL is supplied as a value for the uti parameter, the default
 * type "public.data" is used.
 *
 * @param msg
 *    (input) An object of type ASL_TYPE_MSG.
 * @param title
 *    (input) A title string for the file
 * @param uti
 *    (input) Uniform Type Identifier for the file
 * @param url
 *    (input) Uniform Type Locator
 * @result Returns 0 for success, non-zero for failure
 */
int asl_log_auxiliary_location(asl_object_t msg, const char *title, const char *uti, const char *url) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.7,10.12), ios(5.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/*!
 * Creates an object of type ASL_TYPE_CLIENT for logging to a file descriptor.
 * The file must be opened for read and write access.  This routine may be used in conjunction
 * with asl_create_auxiliary_file() to save ASL format log messages to an auxiliary file.
 *
 * When logging to the file is complete, the returned object should be released with asl_release().
 * The file descriptor should be closed using asl_close_auxiliary_file() if it was returned by
 * asl_create_auxiliary_file(), or close() otherwise.
 *
 * The returned client object is thread-safe.  It contains a lock that is aquired by
 * the calling thread.  Note that this may cause unexpected syncronization behavior
 * if multiple threads log to the returned object, or in applications that use the
 * object in multiple dispatch queues.
 *
 * Note that per-message read access controls (ReadUID and ReadGID) and message expire
 * times (ASLExpireTime) keys have no effect for messages written to this file.
 *
 * Also note that files are NOT truncated.  This is a change in OS X 10.9 and iOS 7.0.
 * Previous versions of this routine truncated the file before writing.  Callers
 * may use ftruncate() to truncate the file if desired.  If an existing non-empty
 * file is used, it must be an ASL format data file.
 *
 * @param descriptor
 *    (input) A file descriptor
 * @param ident
 *    (input) Sender name
 * @param facility
 *    (input) Facility name
 * @result An object of type ASL_TYPE_CLIENT.
 */
asl_object_t asl_open_from_file(int descriptor, const char *ident, const char *facility) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.7,10.12), ios(5.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/*!
 * This API provides functionality to use file descriptors to send logging
 * data to ASL.
 *
 * asl is retained by ASL and must still be closed by the caller by calling
 * asl_close() if the caller loses reference to it.  msg is copied by ASL and
 * similarly must still be freed by the caller by calling asl_free() if the
 * caller loses reference to it.  Any changes made to it after calling
 * asl_log_descriptor() are not applicable to the message used. descriptor
 * is treated differently based on the value of fd_type.
 *
 * If fd_type is ASL_LOG_DESCRIPTOR_READ, the descriptor must be open for read
 * access.  ASL uses GCD to read from the descriptor as data becomes available.
 * These data are line buffered and passed to asl_log.  When EOF is read, the
 * descriptor is closed.
 *
 * Example:
 * asl_log_descriptor(c, m, ASL_LEVEL_NOTICE, STDIN_FILENO, ASL_LOG_DESCRIPTOR_READ);
 *
 * If fd_type is ASL_LOG_DESCRIPTOR_WRITE, the descriptor is closed and a new
 * writable descriptor is created with the same fileno.  Any data written to
 * this new descriptor are line buffered and passed to asl_log.  When EOF is
 * sent, no further data are read.  The caller is responsible for closing the
 * new descriptor.  One common use for this API is to redirect writes to stdout
 * or stderr to ASL by passing STDOUT_FILENO or STDERR_FILENO as descriptor.
 *
 * Example:
 * asl_log_descriptor(c, m, ASL_LEVEL_NOTICE, STDOUT_FILENO, ASL_LOG_DESCRIPTOR_WRITE);
 * asl_log_descriptor(c, m, ASL_LEVEL_ERR, STDERR_FILENO, ASL_LOG_DESCRIPTOR_WRITE);
 *
 * @param client
 *    (input) An ASL object of type ASL_TYPE_CLIENT.
 * @param msg
 *    (input) An asl_object_t of type ASL_TYPE_MSG (default attributes will be supplied if msg is NULL).
 * @param level
 *    (input) Log level (ASL_LEVEL_DEBUG to ASL_LEVEL_EMERG)
 * @param descriptor
 *    (input) An open file descriptor to read from
 * @param fd_type
 *    (input) Either ASL_LOG_DESCRIPTOR_READ or ASL_LOG_DESCRIPTOR_WRITE
 * @result Returns 0 for success, non-zero for failure
 */
int asl_log_descriptor(asl_object_t asl, asl_object_t msg, int level, int descriptor, uint32_t fd_type) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.8,10.12), ios(5.1,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

#pragma mark -


/*!
 * Creates a string representation of an ASL message.
 *
 * This utility creates a character string suitable for printing an ASL message.
 * The returned string ends with a newline character.  The caller is responsible
 * for freeing the returned string.
 * The message is formatted according to the specified format string.  Timestamps
 * are formatted accoring to the specified time format string.  Special characters
 * are enoded as specified by the text_encoding parameter.
 *
 * @param msg
 *    (input) An asl_object_t of type ASL_TYPE_MSG.
 * @param fmt
 *    (input) A format specification string.  See "Output file message and time formats"
 *    for standard formats.  See the syslog(1) man page for more discussion on formats.
 * @param fmt
 *    (input) A time format specification string.  See "Output file message and time formats"
 *    for standard formats.  See the syslog(1) man page for more discussion on time formats.
 * @param text_encoding
 *    (input) Text encoding control (for special characters).  See "Text Encoding Types".
* @result Returns a character string, or NULL in case of a failure.
 */
char *asl_format(asl_object_t msg, const char *msg_fmt, const char *time_fmt, uint32_t text_encoding) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(8.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/*!
 * Encodes a buffer with embedded nul characters into a nul terminated C string.
 * The result must be freed by the caller.
 *
 * This utility is used to encode the value associated with ASL_KEY_AUX_DATA
 * in an ASL_TYPE_MSG object.  An ASL_KEY_AUX_DATA key/value pair is used to hold the
 * data written to a file descriptor created by asl_create_auxiliary_file on iOS
 * systems, where the ASL database is stored in memory.
 *
 * @param buf
 *    (input) Pointer to a data buffer.
 * @param len
 *    (input) Length (in octets) of data buffer.
 * @result Returns an encoded character string.
 */
char *asl_encode_buffer(const char *buf, size_t len) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(8.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/*!
 * Decodes a C string previously created by asl_encode_buffer back into a buffer,
 * possibly containing embedded nul characters.  Allocates memory for the buffer
 * and returns a pointer in an output parameter "buf".
 * The caller is responsible for freeing the buffer.
 *
 * This routine should be used to decode the value associated with an
 * ASL_KEY_AUX_DATA key in an ASL_TYPE_MSG object.
 *
 * @param in
 *    (input) Pointer to nul-terminated string created by asl_encode_buffer.
 * @param buf
 *    (output) Pointer to a newly allocated data buffer.
 * @param len
 *    (input) Length (in octets) of data buffer.
 * @result Returns 0 on success, non-zero on failure.
 */
int asl_decode_buffer(const char *in, char **buf, size_t *len) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(8.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/*!
 * Iterate forward through messages in an asl_object_t.
 * The asl_object_t object maintains an internal position index for the underlying
 * collection of ASL messages, whether the asl_object_t represents a list, a
 * data file, or an ASL database.  The position index is moved forward and the
 * "next" message is returned.
 *
 * @param obj
 *    (input) An asl_object_t.
 * @result Returns the next message (an object of type ASL_TYPE_MSG or ASL_TYPE_QUERY) from the object,
 *    which should be of type ASL_TYPE_CLIENT, ASL_TYPE_LIST, ASL_TYPE_STORE, or ASL_TYPE_FILE.
 *    Returns NULL when there are no more messages or if obj is not a type that holds messages.
 */
asl_object_t asl_next(asl_object_t obj) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(8.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/*!
 * Iterate backwards through messages in an asl_object_t.
 * The asl_object_t object maintains an internal position index for the underlying
 * collection of ASL messages, whether the asl_object_t represents a list, a
 * data file, or an ASL database.  The position index is moved backward and the
 * "previous" message is returned.  
 *
 * @param data
 *    (input) An asl_object_t.
 * @result Returns the previous message (an object of type ASL_TYPE_MSG or ASL_TYPE_QUERY) from the object,
 *    which should be of type ASL_TYPE_CLIENT, ASL_TYPE_LIST, ASL_TYPE_STORE, or ASL_TYPE_FILE.
 *    Returns NULL when there are no more messages or if obj is not a type that holds messages.
 */
asl_object_t asl_prev(asl_object_t obj) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(8.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/*!
 * Reset internal interation index in an asl_object_t.
 *
 * @param obj
 *    (input) An object of type ASL_TYPE_CLIENT, ASL_TYPE_LIST, ASL_TYPE_STORE, or ASL_TYPE_FILE.
 * @param position
 *    (input) Use 0 to position the internal interation index at the beginning of the asl_object_t object,
 *    and SIZE_MAX to position it at the end.  Other values of position may cause unpredictable behavior.
 */
void asl_reset_iteration(asl_object_t obj, size_t position) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(8.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/*!
 * Searches an asl_object_t.
 * The search is controlled by a list of queries, and returns a list with matching messages.
 * A message is returned if it matches any of the queries in the query list.
 * A NULL querylist matches anything.
 *
 * The caller may provide a starting ASL message ID, a direction, and a count.
 * A start ID value of 0 means that matching should commence at the beginning of the target obj.
 * A value of SIZE_MAX indicates that matching should commence at the end (most recent message)
 * in the target.  If a non-zero count value is supplied, the routine will return when it has
 * found that many messages, or it has checked all messages.  If a non-zero duration is supplied,
 * the routine will return after the specified time (in microseconds).
 * If both count and duration are non-zero, the routine will return when the desired number of
 * items has been matched or when the specified duration has been exceeded, whichever occurs first.
 * The search direction may be ASL_MATCH_DIRECTION_FORWARD or ASL_MATCH_DIRECTION_REVERSE.
 * The routine sets the value of the out parameter last to be an index of the last message
 * checked while matching.  To fetch matching messages in batches (using a small count or
 * duration value), the start value for each iteration should be set to (last + 1) if searching
 * forward, or (last - 1)for reverse search.
 *
 * @param data
 *    (input) An asl_object_t object.
 * @param querylist
 *    (input) An asl_object_t object containing zero or more queries.
 * @param last
 *    (output) An internal position index of the last message checked while matching in the asl_object_t object.
 * @param start
 *    (input) A position index specifying where matching should commence.
 * @param count
 *    (input) The maximum number of messages to be returned in the res output list (zero indicates no limit).
 * @param duration
 *    (input) A limit (in microseconds) on the time to be spent searching for results.  Zero indicates no time limit.
 * @param direction
 *    (input) ASL_MATCH_DIRECTION_FORWARD or ASL_MATCH_DIRECTION_REVERSE.
 * @result Returns an ASL object containing messages matching the querylist, or NULL if there are no matches.
 */
asl_object_t asl_match(asl_object_t data, asl_object_t querylist, size_t *last, size_t start, size_t count, uint32_t duration, int32_t direction) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(8.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

__END_DECLS

#endif /* __ASL_H__ */
