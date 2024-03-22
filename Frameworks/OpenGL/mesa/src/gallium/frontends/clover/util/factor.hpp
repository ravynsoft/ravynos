//
// Copyright 2013 Francisco Jerez
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

#ifndef CLOVER_UTIL_FACTOR_HPP
#define CLOVER_UTIL_FACTOR_HPP

#include "util/range.hpp"

namespace clover {
   namespace factor {
      ///
      /// Calculate all prime integer factors of \p x.
      ///
      /// If \p limit is non-zero, terminate early as soon as enough
      /// factors have been collected to reach the product \p limit.
      ///
      template<typename T>
      std::vector<T>
      find_integer_prime_factors(T x, T limit = 0)
      {
         const T max_d = (limit > 0 && limit < x ? limit : x);
         const T min_x = x / max_d;
         std::vector<T> factors;

         for (T d = 2; d <= max_d && x > min_x; d++) {
            if (x % d == 0) {
               for (; x % d == 0; x /= d);
               factors.push_back(d);
            }
         }

         return factors;
      }

      namespace detail {
         ///
         /// Walk the power set of prime factors of the n-dimensional
         /// integer array \p grid subject to the constraints given by
         /// \p limits.
         ///
         template<typename T>
         std::pair<T, std::vector<T>>
         next_grid_factor(const std::pair<T, std::vector<T>> &limits,
                          const std::vector<T> &grid,
                          const std::vector<std::vector<T>> &factors,
                          std::pair<T, std::vector<T>> block,
                          unsigned d = 0, unsigned i = 0) {
            if (d >= factors.size()) {
               // We're done.
               return {};

            } else if (i >= factors[d].size()) {
               // We're done with this grid dimension, try the next.
               return next_grid_factor(limits, grid, factors,
                                       std::move(block), d + 1, 0);

            } else {
               T f = factors[d][i];

               // Try the next power of this factor.
               block.first *= f;
               block.second[d] *= f;

               if (block.first <= limits.first &&
                   block.second[d] <= limits.second[d] &&
                   grid[d] % block.second[d] == 0) {
                  // We've found a valid grid divisor.
                  return block;

               } else {
                  // Overflow, back off to the zeroth power,
                  while (block.second[d] % f == 0) {
                     block.second[d] /= f;
                     block.first /= f;
                  }

                  // ...and carry to the next factor.
                  return next_grid_factor(limits, grid, factors,
                                          std::move(block), d, i + 1);
               }
            }
         }
      }

      ///
      /// Find the divisor of the integer array \p grid that gives the
      /// highest possible product not greater than \p product_limit
      /// subject to the constraints given by \p coord_limit.
      ///
      template<typename T>
      std::vector<T>
      find_grid_optimal_factor(T product_limit,
                               const std::vector<T> &coord_limit,
                               const std::vector<T> &grid) {
         const std::vector<std::vector<T>> factors =
            map(find_integer_prime_factors<T>, grid, coord_limit);
         const auto limits = std::make_pair(product_limit, coord_limit);
         auto best = std::make_pair(T(1), std::vector<T>(grid.size(), T(1)));

         for (auto block = best;
              block.first != 0 && best.first != product_limit;
              block = detail::next_grid_factor(limits, grid, factors, block)) {
            if (block.first > best.first)
               best = block;
         }

         return best.second;
      }
   }
}

#endif
