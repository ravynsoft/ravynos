/*
 * Copyright © 2020 Collabora, Ltd.
 * Author: Antonio Caggiano <antonio.caggiano@collabora.com>
 * Author: Rohan Garg <rohan.garg@collabora.com>
 * Author: Robert Beckett <bob.beckett@collabora.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <variant>
#include <vector>

namespace pps
{
struct CounterGroup {
   std::string name;

   uint32_t id;

   /// List of counters ID belonging to this group
   std::vector<int32_t> counters;

   std::vector<CounterGroup> subgroups;
};

class Driver;

class Counter
{
   public:
   /// @brief A counter value can be of different types depending on what it represents:
   /// cycles, cycles-per-instruction, percentages, bytes, and so on.
   enum class Units {
      Percent,
      Byte,
      Hertz,
      None,
   };

   using Value = std::variant<int64_t, double>;

   /// @param c Counter which we want to retrieve a value
   /// @param d Driver used to sample performance counters
   /// @return The value of the counter
   using Getter = Value(const Counter &c, const Driver &d);

   Counter() = default;
   virtual ~Counter() = default;

   /// @param id ID of the counter
   /// @param name Name of the counter
   /// @param group Group ID this counter belongs to
   Counter(int32_t id, const std::string &name, int32_t group);

   bool operator==(const Counter &c) const;

   /// @param get New getter function for this counter
   void set_getter(const std::function<Getter> &get);

   /// @brief d Driver used to sample performance counters
   /// @return Last sampled value for this counter
   Value get_value(const Driver &d) const;

   /// Id of the counter
   int32_t id = -1;

   /// Name of the counter
   std::string name = "";

   /// ID of the group this counter belongs to
   int32_t group = -1;

   /// Offset of this counter within GPU counter list
   /// For derived counters it is negative and remains unused
   int32_t offset = -1;

   /// Whether it is a derived counter or not
   bool derived = false;

   /// Returns the value of this counter
   std::function<Getter> getter;

   /// The unit of the counter
   Units units;
};

/// @param get New getter function for this counter
inline void Counter::set_getter(const std::function<Getter> &get)
{
   getter = get;
}

/// @brief d Driver used to sample performance counters
/// @return Last sampled value for this counter
inline Counter::Value Counter::get_value(const Driver &d) const
{
   return getter(*this, d);
}

/// @return The underlying u32 value
template<typename T> constexpr uint32_t to_u32(T &&elem)
{
   return static_cast<uint32_t>(elem);
}

} // namespace pps
