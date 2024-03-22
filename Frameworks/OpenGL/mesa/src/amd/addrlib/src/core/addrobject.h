/*
************************************************************************************************************************
*
*  Copyright (C) 2007-2022 Advanced Micro Devices, Inc.  All rights reserved.
*  SPDX-License-Identifier: MIT
*
***********************************************************************************************************************/


/**
****************************************************************************************************
* @file  addrobject.h
* @brief Contains the Object base class definition.
****************************************************************************************************
*/

#ifndef __ADDR_OBJECT_H__
#define __ADDR_OBJECT_H__

#include "addrtypes.h"
#include "addrcommon.h"

namespace Addr
{

/**
****************************************************************************************************
* @brief This structure contains client specific data
****************************************************************************************************
*/
struct Client
{
    ADDR_CLIENT_HANDLE  handle;
    ADDR_CALLBACKS      callbacks;
};
/**
****************************************************************************************************
* @brief This class is the base class for all ADDR class objects.
****************************************************************************************************
*/
class Object
{
public:
    Object();
    Object(const Client* pClient);
    virtual ~Object();

    VOID* operator new(size_t size, VOID* pMem) noexcept;
    VOID  operator delete(VOID* pObj);
    /// Microsoft compiler requires a matching delete implementation, which seems to be called when
    /// bad_alloc is thrown. But currently C++ exception isn't allowed so a dummy implementation is
    /// added to eliminate the warning.
    VOID  operator delete(VOID* pObj, VOID* pMem) { ADDR_ASSERT_ALWAYS(); }

    VOID* Alloc(size_t size) const;
    VOID  Free(VOID* pObj) const;

    VOID DebugPrint(const CHAR* pDebugString, ...) const;

    const Client* GetClient() const {return &m_client;}

protected:
    Client m_client;

    static VOID* ClientAlloc(size_t size, const Client* pClient);
    static VOID  ClientFree(VOID* pObj, const Client* pClient);

private:
    // disallow the copy constructor
    Object(const Object& a);

    // disallow the assignment operator
    Object& operator=(const Object& a);
};

} // Addr
#endif
