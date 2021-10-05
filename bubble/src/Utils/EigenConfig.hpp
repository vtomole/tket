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

#ifndef _TKET_EigenConfig_H_
#define _TKET_EigenConfig_H_

/**
 * @file
 * @brief Include this file rather than including the Eigen headers directly
 */

#if defined(__clang__)
#pragma GCC diagnostic push

#if __has_warning("-Wdeprecated-copy")
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#endif

#endif

#include <Eigen/Dense>
#include <Eigen/SparseCore>
#include <unsupported/Eigen/KroneckerProduct>
#include <unsupported/Eigen/MatrixFunctions>

#if defined(__clang__)
#pragma GCC diagnostic pop
#endif

namespace Eigen {

}  // namespace Eigen

#endif  // _EIGEN_CONFIG_H_
