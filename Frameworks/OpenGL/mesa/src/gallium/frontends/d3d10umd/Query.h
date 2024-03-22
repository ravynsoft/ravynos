/**************************************************************************
 *
 * Copyright 2012-2021 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS, AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 **************************************************************************/

/*
 * Query.h --
 *    Functions that manipulate query resources.
 */

#ifndef QUERY_H
#define QUERY_H

#include "DriverIncludes.h"

struct Device;

SIZE_T APIENTRY CalcPrivateQuerySize(D3D10DDI_HDEVICE hDevice,
                            __in const D3D10DDIARG_CREATEQUERY *pCreateQuery);

void APIENTRY CreateQuery(D3D10DDI_HDEVICE hDevice,
                 __in const D3D10DDIARG_CREATEQUERY *pCreateQuery,
                 D3D10DDI_HQUERY hQuery, D3D10DDI_HRTQUERY hRTQuery);

void APIENTRY APIENTRY DestroyQuery(D3D10DDI_HDEVICE hDevice, D3D10DDI_HQUERY hQuery);

void APIENTRY QueryBegin(D3D10DDI_HDEVICE hDevice, D3D10DDI_HQUERY hQuery);

void APIENTRY QueryEnd(D3D10DDI_HDEVICE hDevice, D3D10DDI_HQUERY hQuery);

void APIENTRY QueryGetData(D3D10DDI_HDEVICE hDevice, D3D10DDI_HQUERY hQuery,
                  __out_bcount_full_opt (DataSize) void *pData,
                  UINT DataSize, UINT Flags);

void APIENTRY SetPredication(D3D10DDI_HDEVICE hDevice, D3D10DDI_HQUERY hQuery,
                    BOOL PredicateValue);

void APIENTRY CheckCounterInfo(D3D10DDI_HDEVICE hDevice,
                 __out D3D10DDI_COUNTER_INFO *pCounterInfo);

void APIENTRY CheckCounter(D3D10DDI_HDEVICE hDevice,
                  D3D10DDI_QUERY Query,
                  __out D3D10DDI_COUNTER_TYPE *pCounterType,
                  __out UINT *pActiveCounters,
                  __out_ecount_part_z_opt (*pNameLength, *pNameLength) LPSTR pName,
                  __inout_opt UINT *pNameLength,
                  __out_ecount_part_z_opt (*pUnitsLength, *pUnitsLength) LPSTR pUnits,
                  __inout_opt UINT *pUnitsLength,
                  __out_ecount_part_z_opt (*pDescriptionLength, *pDescriptionLength) LPSTR pDescription,
                  __inout_opt UINT* pDescriptionLength);

BOOL CheckPredicate(Device *pDevice);

#endif   /* QUERY_H */
