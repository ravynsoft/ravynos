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
 * DxgiFns.h --
 *    DXGI related functions
 */

#ifndef WRAP_DXGI_H
#define WRAP_DXGI_H

#include "DriverIncludes.h"

HRESULT APIENTRY _Present( DXGI_DDI_ARG_PRESENT * );
HRESULT APIENTRY _GetGammaCaps( DXGI_DDI_ARG_GET_GAMMA_CONTROL_CAPS * );
HRESULT APIENTRY _SetDisplayMode( DXGI_DDI_ARG_SETDISPLAYMODE * );
HRESULT APIENTRY _SetResourcePriority( DXGI_DDI_ARG_SETRESOURCEPRIORITY * );
HRESULT APIENTRY _QueryResourceResidency( DXGI_DDI_ARG_QUERYRESOURCERESIDENCY * );
HRESULT APIENTRY _RotateResourceIdentities( DXGI_DDI_ARG_ROTATE_RESOURCE_IDENTITIES * );
HRESULT APIENTRY _Blt( DXGI_DDI_ARG_BLT * );

#endif
