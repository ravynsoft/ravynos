/*
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * Copyright (c) 2015 Apple Computer, Inc.  All Rights Reserved.
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

#ifndef IOKitUser_IOHIDDebugTrace_h
#define IOKitUser_IOHIDDebugTrace_h

#include <sys/kdebug.h>
#include <sys/syscall.h>
#include "IOHIDLibPrivate.h"

enum {
    kIOHIDEnableProfilingBit = 0,
    
    kIOHIDEnableProfilingMask = 1 << kIOHIDEnableProfilingBit,
};


enum {
    kHID_TraceBase                      = 0x2000,
    kHID_UserDev_Create                 = 0x2001,       //0x5238004
    kHID_UserDev_Release                = 0x2002,       //0x5238008
    kHID_UserDev_Start                  = 0x2003,       //0x523800c
    kHID_UserDev_AsyncSupport           = 0x2004,       //0x5238010
    kHID_UserDev_Unschedule             = 0x2005,       //0x5238014
    kHID_UserDev_ScheduleDispatch       = 0x2006,       //0x5238018
    kHID_UserDev_UnscheduleDispatch     = 0x2007,       //0x523801c
    kHID_UserDev_QueueCallback          = 0x2008,       //0x5238020
    kHID_UserDev_HandleReport           = 0x2009,       //0x5238024
    kHID_UserDev_HandleReportCallback   = 0x200A,       //0x5238028
    kHID_UserDev_SetReportCallback      = 0x200B,       //0x523802c
    kHID_ES_EventCallback               = 0x2010,       //0x5238040
    kHID_ES_FiltersClientsDone          = 0x2011,       //0x5238044
    kHID_ES_SystemFilterDone            = 0x2012,       //0x5238048
    kHID_ES_ClientsEnqueue              = 0x2013,       //0x523804c
    kHID_ES_Client_QueueCallback        = 0x2020,       //0x5238080
    kHID_ES_Conn_DispatchEvent          = 0x2030,       //0x52380c0
    kHID_ES_Service_Callback            = 0x2040,       //0x5238100
    kHID_ES_Service_Create              = 0x2041,       //0x5238104
    kHID_ES_Service_Open                = 0x2042,       //0x5238108
    kHID_ES_Service_Filters             = 0x2043,       //0x523810c
    kHID_ES_Service_Notification        = 0x2044,       //0x5238110
    kHID_ES_Service_Close               = 0x2045,       //0x5238114
    kHID_ES_Service_Async               = 0x2046,       //0x5238118
    kHID_ES_Service_CopyEvent           = 0x2047,       //0x523811c
    kHID_ES_Session_Callback            = 0x2050,       //0x5238140
    kHID_ES_Session_Dispatch            = 0x2051,       //0x5238144
    kHID_ES_Session_Filters             = 0x2052,       //0x5238148
    kHID_ES_Create                      = 0x2053,       //0x523814c
    kHID_ES_Open                        = 0x2054        //0x5238150
};

#define IOHID_DEBUG_CODE(code)          IOKDBG_CODE(DBG_IOHID, code)
#define IOHID_DEBUG_START(code)         IOHID_DEBUG_CODE(code) | DBG_FUNC_START
#define IOHID_DEBUG_END(code)           IOHID_DEBUG_CODE(code) | DBG_FUNC_END


#define HIDDEBUGTRACE(code, a, b, c, d)  _IOHIDDebugTrace (code, DBG_FUNC_NONE, a, b, c, d)
#define HIDPROFTRACE(code, a, b, c, d)   _IOHIDDebugTrace (code, DBG_FUNC_NONE, a, b, c, d)
#define HIDFUNCSTART(code, a, b, c, d)   _IOHIDDebugTrace (code, DBG_FUNC_START, a, b, c, d)
#define HIDFUNCEND(code, a, b, c, d)     _IOHIDDebugTrace (code, DBG_FUNC_END, a, b, c, d)
#define HIDEVENTPERF(event,point,time)   _IOHIDDebugEventAddPerfData (event, point, time)

#endif
