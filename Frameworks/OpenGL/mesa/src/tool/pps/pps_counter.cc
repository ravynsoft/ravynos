/*
 * Copyright © 2019-2020 Collabora, Ltd.
 * Author: Antonio Caggiano <antonio.caggiano@collabora.com>
 * Author: Rohan Garg <rohan.garg@collabora.com>
 * Author: Robert Beckett <bob.beckett@collabora.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "pps_counter.h"

#include <cassert>
#include <cstring>

#include "pps_algorithm.h"

namespace pps
{
Counter::Counter(int32_t id, const std::string &name, int32_t group)
   : id {id}
   , name {name}
   , group {group}
{
   assert(id >= 0 && "Invalid counter ID");
   assert(group >= 0 && "Invalid group ID");
}

bool Counter::operator==(const Counter &other) const
{
   return id == other.id;
}

} // namespace pps
