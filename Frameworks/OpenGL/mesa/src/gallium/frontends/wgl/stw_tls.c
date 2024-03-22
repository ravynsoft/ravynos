/**************************************************************************
 *
 * Copyright 2009-2013 VMware, Inc.
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
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#include <windows.h>
#include <tlhelp32.h>

#include "util/compiler.h"
#include "util/u_debug.h"
#include "stw_tls.h"

static DWORD tlsIndex = TLS_OUT_OF_INDEXES;


/**
 * Static mutex to protect the access to g_pendingTlsData global and
 * stw_tls_data::next member.
 */
static CRITICAL_SECTION g_mutex = {
   (PCRITICAL_SECTION_DEBUG)-1, -1, 0, 0, 0, 0
};

/**
 * There is no way to invoke TlsSetValue for a different thread, so we
 * temporarily put the thread data for non-current threads here.
 */
static struct stw_tls_data *g_pendingTlsData = NULL;


static struct stw_tls_data *
stw_tls_data_create(DWORD dwThreadId);

static struct stw_tls_data *
stw_tls_lookup_pending_data(DWORD dwThreadId);


bool
stw_tls_init(void)
{
   tlsIndex = TlsAlloc();
   if (tlsIndex == TLS_OUT_OF_INDEXES) {
      return false;
   }

   /*
    * DllMain is called with DLL_THREAD_ATTACH only for threads created after
    * the DLL is loaded by the process.  So enumerate and add our hook to all
    * previously existing threads.
    *
    * XXX: Except for the current thread since it there is an explicit
    * stw_tls_init_thread() call for it later on.
    */
#ifndef _GAMING_XBOX
   DWORD dwCurrentProcessId = GetCurrentProcessId();
   DWORD dwCurrentThreadId = GetCurrentThreadId();
   HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, dwCurrentProcessId);
   if (hSnapshot != INVALID_HANDLE_VALUE) {
      THREADENTRY32 te;
      te.dwSize = sizeof te;
      if (Thread32First(hSnapshot, &te)) {
         do {
            if (te.dwSize >= FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) +
                             sizeof te.th32OwnerProcessID) {
               if (te.th32OwnerProcessID == dwCurrentProcessId) {
                  if (te.th32ThreadID != dwCurrentThreadId) {
                     struct stw_tls_data *data;
                     data = stw_tls_data_create(te.th32ThreadID);
                     if (data) {
                        EnterCriticalSection(&g_mutex);
                        data->next = g_pendingTlsData;
                        g_pendingTlsData = data;
                        LeaveCriticalSection(&g_mutex);
                     }
                  }
               }
            }
            te.dwSize = sizeof te;
         } while (Thread32Next(hSnapshot, &te));
      }
      CloseHandle(hSnapshot);
   }
#endif /* _GAMING_XBOX */

   return true;
}


/**
 * Install windows hook for a given thread (not necessarily the current one).
 */
static struct stw_tls_data *
stw_tls_data_create(DWORD dwThreadId)
{
   struct stw_tls_data *data;

   if (0) {
      debug_printf("%s(0x%04lx)\n", __func__, dwThreadId);
   }

   data = calloc(1, sizeof *data);
   if (!data) {
      goto no_data;
   }

   data->dwThreadId = dwThreadId;

#ifndef _GAMING_XBOX
   data->hCallWndProcHook = SetWindowsHookEx(WH_CALLWNDPROC,
                                             stw_call_window_proc,
                                             NULL,
                                             dwThreadId);
#else
   data->hCallWndProcHook = NULL;
#endif
   if (data->hCallWndProcHook == NULL) {
      goto no_hook;
   }

   return data;

no_hook:
   free(data);
no_data:
   return NULL;
}

/**
 * Destroy the per-thread data/hook.
 *
 * It is important to remove all hooks when unloading our DLL, otherwise our
 * hook function might be called after it is no longer there.
 */
static void
stw_tls_data_destroy(struct stw_tls_data *data)
{
   assert(data);
   if (!data) {
      return;
   }

   if (0) {
      debug_printf("%s(0x%04lx)\n", __func__, data->dwThreadId);
   }

#ifndef _GAMING_XBOX
   if (data->hCallWndProcHook) {
      UnhookWindowsHookEx(data->hCallWndProcHook);
      data->hCallWndProcHook = NULL;
   }
#endif

   free(data);
}

bool
stw_tls_init_thread(void)
{
   struct stw_tls_data *data;

   if (tlsIndex == TLS_OUT_OF_INDEXES) {
      return false;
   }

   data = stw_tls_data_create(GetCurrentThreadId());
   if (!data) {
      return false;
   }

   TlsSetValue(tlsIndex, data);

   return true;
}

void
stw_tls_cleanup_thread(void)
{
   struct stw_tls_data *data;

   if (tlsIndex == TLS_OUT_OF_INDEXES) {
      return;
   }

   data = (struct stw_tls_data *) TlsGetValue(tlsIndex);
   if (data) {
      TlsSetValue(tlsIndex, NULL);
   } else {
      /* See if there this thread's data in on the pending list */
      data = stw_tls_lookup_pending_data(GetCurrentThreadId());
   }

   if (data) {
      stw_tls_data_destroy(data);
   }
}

void
stw_tls_cleanup(void)
{
   if (tlsIndex != TLS_OUT_OF_INDEXES) {
      /*
       * Destroy all items in g_pendingTlsData linked list.
       */
      EnterCriticalSection(&g_mutex);
      while (g_pendingTlsData) {
         struct stw_tls_data * data = g_pendingTlsData;
         g_pendingTlsData = data->next;
         stw_tls_data_destroy(data);
      }
      LeaveCriticalSection(&g_mutex);

      TlsFree(tlsIndex);
      tlsIndex = TLS_OUT_OF_INDEXES;
   }
}

/*
 * Search for the current thread in the g_pendingTlsData linked list.
 *
 * It will remove and return the node on success, or return NULL on failure.
 */
static struct stw_tls_data *
stw_tls_lookup_pending_data(DWORD dwThreadId)
{
   struct stw_tls_data ** p_data;
   struct stw_tls_data *data = NULL;

   EnterCriticalSection(&g_mutex);
   for (p_data = &g_pendingTlsData; *p_data; p_data = &(*p_data)->next) {
      if ((*p_data)->dwThreadId == dwThreadId) {
         data = *p_data;
	 
	 /*
	  * Unlink the node.
	  */
         *p_data = data->next;
         data->next = NULL;
         
	 break;
      }
   }
   LeaveCriticalSection(&g_mutex);

   return data;
}

struct stw_tls_data *
stw_tls_get_data(void)
{
   struct stw_tls_data *data;
   
   if (tlsIndex == TLS_OUT_OF_INDEXES) {
      return NULL;
   }
   
   data = (struct stw_tls_data *) TlsGetValue(tlsIndex);
   if (!data) {
      DWORD dwCurrentThreadId = GetCurrentThreadId();

      /*
       * Search for the current thread in the g_pendingTlsData linked list.
       */
      data = stw_tls_lookup_pending_data(dwCurrentThreadId);

      if (!data) {
         /*
          * This should be impossible now.
          */
	 assert(!"Failed to find thread data for thread id");

         /*
          * DllMain is called with DLL_THREAD_ATTACH only by threads created
          * after the DLL is loaded by the process
          */
         data = stw_tls_data_create(dwCurrentThreadId);
         if (!data) {
            return NULL;
         }
      }

      TlsSetValue(tlsIndex, data);
   }

   assert(data);
   assert(data->dwThreadId == GetCurrentThreadId());
   assert(data->next == NULL);

   return data;
}
