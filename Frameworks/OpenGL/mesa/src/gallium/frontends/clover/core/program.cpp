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

#include "core/compiler.hpp"
#include "core/program.hpp"

using namespace clover;

program::program(clover::context &ctx, std::string &&source,
                 enum il_type il_type) :
   context(ctx), _devices(ctx.devices()), _source(std::move(source)),
   _kernel_ref_counter(0), _il_type(il_type) {
}

program::program(clover::context &ctx,
                 const ref_vector<device> &devs,
                 const std::vector<binary> &binaries) :
   context(ctx), _devices(devs), _kernel_ref_counter(0),
   _il_type(il_type::none) {
   for_each([&](device &dev, const binary &bin) {
         _builds[&dev] = { bin };
      },
      devs, binaries);
}

void
program::compile(const ref_vector<device> &devs, const std::string &opts,
                 const header_map &headers) {
   if (_il_type != il_type::none) {
      _devices = devs;

      for (auto &dev : devs) {
         std::string log;

         try {
            const binary b =
               compiler::compile_program(*this, headers, dev, opts, log);
            _builds[&dev] = { b, opts, log };
         } catch (...) {
            _builds[&dev] = { binary(), opts, log };
            throw;
         }
      }
   }
}

void
program::link(const ref_vector<device> &devs, const std::string &opts,
              const ref_vector<program> &progs) {
   _devices = devs;

   for (auto &dev : devs) {
      const std::vector<binary> bs = map([&](const program &prog) {
         return prog.build(dev).bin;
         }, progs);
      std::string log = _builds[&dev].log;

      try {
         const binary b = compiler::link_program(bs, dev, opts, log);
         _builds[&dev] = { b, opts, log };
      } catch (...) {
         _builds[&dev] = { binary(), opts, log };
         throw;
      }
   }
}

enum program::il_type
program::il_type() const {
   return _il_type;
}

const std::string &
program::source() const {
   return _source;
}

program::device_range
program::devices() const {
   return map(evals(), _devices);
}

cl_build_status
program::build::status() const {
   if (!bin.secs.empty())
      return CL_BUILD_SUCCESS;
   else if (log.size())
      return CL_BUILD_ERROR;
   else
      return CL_BUILD_NONE;
}

cl_program_binary_type
program::build::binary_type() const {
   if (any_of(type_equals(binary::section::text_intermediate), bin.secs))
      return CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT;
   else if (any_of(type_equals(binary::section::text_library), bin.secs))
      return CL_PROGRAM_BINARY_TYPE_LIBRARY;
   else if (any_of(type_equals(binary::section::text_executable), bin.secs))
      return CL_PROGRAM_BINARY_TYPE_EXECUTABLE;
   else
      return CL_PROGRAM_BINARY_TYPE_NONE;
}

const struct program::build &
program::build(const device &dev) const {
   static const struct build null;
   return _builds.count(&dev) ? _builds.find(&dev)->second : null;
}

const std::vector<binary::symbol> &
program::symbols() const {
   if (_builds.empty())
      throw error(CL_INVALID_PROGRAM_EXECUTABLE);

   return _builds.begin()->second.bin.syms;
}

unsigned
program::kernel_ref_count() const {
   return _kernel_ref_counter.ref_count();
}
