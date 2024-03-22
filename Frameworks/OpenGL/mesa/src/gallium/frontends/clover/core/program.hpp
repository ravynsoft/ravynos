//
// Copyright 2012 Francisco Jerez
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//

#ifndef CLOVER_CORE_PROGRAM_HPP
#define CLOVER_CORE_PROGRAM_HPP

#include <map>

#include "core/object.hpp"
#include "core/context.hpp"
#include "core/binary.hpp"

namespace clover {
   typedef std::vector<std::pair<std::string, std::string>> header_map;

   class program : public ref_counter, public _cl_program {
   private:
      typedef adaptor_range<
         evals, const std::vector<intrusive_ref<device>> &> device_range;

   public:
      enum class il_type { none, source, spirv };

      program(clover::context &ctx,
              std::string &&il,
              enum il_type il_type);
      program(clover::context &ctx,
              const ref_vector<device> &devs = {},
              const std::vector<binary> &binaries = {});

      program(const program &prog) = delete;
      program &
      operator=(const program &prog) = delete;

      void compile(const ref_vector<device> &devs, const std::string &opts,
                   const header_map &headers = {});
      void link(const ref_vector<device> &devs, const std::string &opts,
                const ref_vector<program> &progs);

      const std::string &source() const;
      enum il_type il_type() const;

      device_range devices() const;

      struct build {
         build(const binary &b = {}, const std::string &opts = {},
               const std::string &log = {}) : bin(b), opts(opts), log(log) {}

         cl_build_status status() const;
         cl_program_binary_type binary_type() const;

         binary bin;
         std::string opts;
         std::string log;
      };

      const build &build(const device &dev) const;

      const std::vector<binary::symbol> &symbols() const;

      unsigned kernel_ref_count() const;

      const intrusive_ref<clover::context> context;

      friend class kernel;

   private:
      std::vector<intrusive_ref<device>> _devices;
      std::map<const device *, struct build> _builds;
      std::string _source;
      ref_counter _kernel_ref_counter;
      enum il_type _il_type;
   };
}

#endif
