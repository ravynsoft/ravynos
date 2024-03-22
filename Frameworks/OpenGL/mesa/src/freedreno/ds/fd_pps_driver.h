/*
 * Copyright © 2021 Google, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "pps/pps_driver.h"

#include "common/freedreno_dev_info.h"
#include "drm/freedreno_drmif.h"
#include "drm/freedreno_ringbuffer.h"
#include "perfcntrs/freedreno_dt.h"
#include "perfcntrs/freedreno_perfcntr.h"

namespace pps
{

class FreedrenoDriver : public Driver
{
public:
   bool is_dump_perfcnt_preemptible() const override;
   uint64_t get_min_sampling_period_ns() override;
   bool init_perfcnt() override;
   void enable_counter(uint32_t counter_id) override;
   void enable_all_counters() override;
   void enable_perfcnt(uint64_t sampling_period_ns) override;
   void disable_perfcnt() override;
   bool dump_perfcnt() override;
   uint64_t next() override;
   uint32_t gpu_clock_id() const override;
   uint64_t gpu_timestamp() const override;
   bool cpu_gpu_timestamp(uint64_t &cpu_timestamp,
                          uint64_t &gpu_timestamp) const override;

private:
   struct fd_device *dev;
   struct fd_pipe *pipe;
   const struct fd_dev_id *dev_id;
   uint32_t max_freq;
   uint32_t next_counter_id;
   uint32_t next_countable_id;
   uint64_t last_dump_ts = 0;
   uint64_t last_capture_ts;

   bool has_suspend_count;
   uint32_t suspend_count;

   const struct fd_dev_info *info;

   /**
    * The memory mapped i/o space for counter readback:
    */
   void *io;

   const struct fd_perfcntr_group *perfcntrs;
   unsigned num_perfcntrs;

   /**
    * The number of counters assigned per perfcntr group, the index
    * into this matches the index into perfcntrs
    */
   std::vector<unsigned> assigned_counters;

   /*
    * Values that can be used by derived counters evaluation
    */
   float time;  /* time since last sample in fraction of second */
//   uint32_t cycles;  /* the number of clock cycles since last sample */

   void setup_a6xx_counters();

   void configure_counters(bool reset, bool wait);
   void collect_countables();

   /**
    * Split out countable mutable state from the class so that copy-
    * constructor does something sane when lambda derive function
    * tries to get the countable value.
    */
   struct CountableState {
      uint64_t last_value, value;
      const struct fd_perfcntr_countable *countable;
      const struct fd_perfcntr_counter   *counter;
   };

   std::vector<struct CountableState> state;

   /**
    * Performance counters on adreno consist of sets of counters in various
    * blocks of the GPU, where each counter can be can be muxed to collect
    * one of a set of countables.
    *
    * But the countables tend to be too low level to be directly useful to
    * visualize.  Instead various combinations of countables are combined
    * with various formulas to derive the high level "Counter" value exposed
    * via gfx-pps.
    *
    * This class serves to decouple the logic of those formulas from the
    * details of collecting countable values.
    */
   class Countable {
   public:
      Countable(FreedrenoDriver *d, std::string name);

      operator int64_t() const { return get_value(); };

      void configure(struct fd_ringbuffer *ring, bool reset) const;
      void collect() const;
      void resolve() const;

   private:

      uint64_t get_value() const;

      uint32_t id;
      FreedrenoDriver *d;
      std::string name;
   };

   Countable countable(std::string name);

   std::vector<Countable> countables;

   /**
    * A derived "Counter" (from pps's perspective)
    */
   class DerivedCounter : public Counter {
   public:
      DerivedCounter(FreedrenoDriver *d, std::string name, Counter::Units units,
                     std::function<int64_t()> derive);
   };

   DerivedCounter counter(std::string name, Counter::Units units,
                          std::function<int64_t()> derive);
};

} // namespace pps
