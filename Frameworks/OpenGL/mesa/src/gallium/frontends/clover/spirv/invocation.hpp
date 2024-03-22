//
// Copyright 2018 Pierre Moreau
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

#ifndef CLOVER_SPIRV_INVOCATION_HPP
#define CLOVER_SPIRV_INVOCATION_HPP

#include <unordered_set>

#include "core/context.hpp"
#include "core/binary.hpp"
#include "core/program.hpp"

namespace clover {
   namespace spirv {
      // Returns whether the binary starts with the SPIR-V magic word.
      //
      // The first word is interpreted as little endian and big endian, but
      // only one of them has to match.
      bool is_binary_spirv(const std::string &binary);

      // Returns whether the given binary is considered valid for the given
      // OpenCL version.
      //
      // It uses SPIRV-Tools validator to do the validation, and potential
      // warnings and errors are appended to |r_log|.
      bool is_valid_spirv(const std::string &binary,
                          const cl_version opencl_version,
                          std::string &r_log);

      // Converts an integer SPIR-V version into its textual representation.
      std::string version_to_string(uint32_t version);

      // Creates a clover binary out of the given SPIR-V binary.
      binary compile_program(const std::string &binary,
                             const device &dev, std::string &r_log,
                             bool validate = true);

      // Combines multiple clover objects into a single one, resolving
      // link dependencies between them.
      binary link_program(const std::vector<binary> &objects, const device &dev,
                          const std::string &opts, std::string &r_log);

      // Returns a textual representation of the given binary.
      std::string print_module(const std::string &binary,
                               const cl_version opencl_version);

      // Returns a set of supported SPIR-V extensions.
      std::unordered_set<std::string> supported_extensions();

      // Returns a vector (sorted in increasing order) of supported SPIR-V
      // versions.
      std::vector<cl_name_version> supported_versions();

      // Converts a version number from SPIR-V's encoding to OpenCL's one.
      cl_version to_opencl_version_encoding(uint32_t version);

      // Converts a version number from OpenCL's encoding to SPIR-V's one.
      uint32_t to_spirv_version_encoding(cl_version version);
   }
}

#endif
