//
// Copyright 2019 Karol Herbst
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

#ifndef CLOVER_NIR_INVOCATION_HPP
#define CLOVER_NIR_INVOCATION_HPP

#include "core/binary.hpp"
#include <util/disk_cache.h>

struct nir_shader;

namespace clover {
   class device;
   namespace nir {
      void check_for_libclc(const device &dev);

      // converts libclc spirv into nir
      nir_shader *load_libclc_nir(const device &dev, std::string &r_log);

      struct disk_cache *create_clc_disk_cache(void);

      // converts a given spirv binary to nir
      binary spirv_to_nir(const binary &bin, const device &dev, std::string &r_log);
   }
}

#endif
