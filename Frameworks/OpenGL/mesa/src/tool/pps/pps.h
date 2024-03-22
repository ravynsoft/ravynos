/*
 * Copyright © 2020 Collabora, Ltd.
 * Author: Antonio Caggiano <antonio.caggiano@collabora.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <perfetto.h>

#define PPS_LOG PERFETTO_LOG
#define PPS_LOG_IMPORTANT PERFETTO_ILOG
#define PPS_LOG_ERROR PERFETTO_ELOG
#define PPS_LOG_FATAL PERFETTO_FATAL

namespace pps
{
enum class State {
   Stop,  // initial state, or stopped by the tracing service
   Start, // running, sampling data
};

/// @brief Checks whether a return value is valid
/// @param res Result from a syscall
/// @param msg Message to prepend to strerror
/// @return True if ok, false otherwise
bool check(int res, const char *msg);

void make_thread_rt();

/// @param num Numerator
/// @param den Denominator
/// @return A ratio between two floating point numbers, or 0 if the denominator is 0
constexpr double ratio(double num, double den)
{
   return den > 0.0 ? num / den : 0.0;
}

} // namespace pps
