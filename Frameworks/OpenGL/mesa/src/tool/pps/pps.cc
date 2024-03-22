/*
 * Copyright © 2020 Collabora, Ltd.
 * Author: Antonio Caggiano <antonio.caggiano@collabora.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "pps.h"

#include <cerrno>
#include <cstring>

#include <sched.h>

namespace pps
{
bool check(int res, const char *msg)
{
   if (res < 0) {
      char *err_msg = std::strerror(errno);
      PERFETTO_ELOG("%s: %s", msg, err_msg);
      return false;
   }

   return true;
}

void make_thread_rt()
{
   // Use FIFO policy to avoid preemption while collecting counters
   int sched_policy = SCHED_FIFO;
   // Do not use max priority to avoid starving migration and watchdog threads
   int priority_value = sched_get_priority_max(sched_policy) - 1;
   sched_param priority_param { priority_value };
   sched_setscheduler(0, sched_policy, &priority_param);
}

} // namespace pps
