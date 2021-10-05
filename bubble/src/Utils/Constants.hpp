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

#ifndef _TKET_Constants_H_
#define _TKET_Constants_H_

/**
 * @file
 * @brief Generally useful typedefs and constants
 */

#include <complex>

namespace tket {

// Typedefs

/** Complex number */
typedef std::complex<double> Complex;

// Numeric constants
/** A fixed square root of -1 */
constexpr Complex i_(0, 1);
/** Complex zero */
constexpr Complex czero(0, 0);

/** Default tolerance for floating-point comparisons */
constexpr double EPS = 1e-11;

/** \f$ \pi \f$ */
constexpr double PI =
    3.141'592'653'589'793'238'462'643'383'279'502'884'197'169'399'375'105'820'974;

}  // namespace tket

#endif  // CONSTANTS_H_
