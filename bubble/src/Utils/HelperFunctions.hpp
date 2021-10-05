// Copyright 2019-2021 Cambridge Quantum Computing
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef _TKET_HelperFunctions_H_
#define _TKET_HelperFunctions_H_

#include <cstdint>
#include <deque>
#include <map>
#include <vector>

namespace tket {

typedef std::vector<std::deque<bool>> GrayCode;
/**
 * Construct a GrayCode over `n` bits
 * @param n bits
 */
GrayCode gen_graycode(unsigned n);

/**
 * Check equality of each element of first and second
 * when first and second can provide begin, end iterators
 **/
template <typename T>
bool check_iterators_equality(const T& first, const T& second) {
  auto it1 = first.begin();
  auto f_end = first.end();

  auto it2 = second.begin();
  auto s_end = second.end();

  for (; it1 != f_end; ++it1) {
    if (it2 == s_end || !(*it1 == *it2)) {
      return false;
    }
    ++it2;
  }

  // check there were the same number of elements
  return (it2 == s_end);
}

/**
 * Reverse bits 0,1,...,w-1 of the number v, assuming v < 2^w and w <= 32.
 */
uint32_t reverse_bits(uint32_t v, unsigned w);

}  // namespace tket

#endif  //_TKET_HelperFunctions_H_
