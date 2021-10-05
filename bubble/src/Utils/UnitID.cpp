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

#include "UnitID.hpp"

#include <sstream>

namespace tket {

std::string UnitID::repr() const {
  std::stringstream str;
  str << data_->name_;
  if (!data_->index_.empty()) {
    str << "[" << std::to_string(data_->index_[0]);
    for (unsigned i = 1; i < data_->index_.size(); i++) {
      str << ", " << std::to_string(data_->index_[i]);
    }
    str << "]";
  }
  return str.str();
}

/* The functions below use the "construct on first use" idiom to return "global"
 * objects. They use pointers to ensure that the objects returned last
 * throughout static initialization and deinitialization. There is no memory
 * leak since the objects are only constructed once and the memory is reclaimed
 * on program termination.
 */

const std::string& q_default_reg() {
  static std::unique_ptr<const std::string> regname =
      std::make_unique<const std::string>("q");
  return *regname;
}

const std::string& c_default_reg() {
  static std::unique_ptr<const std::string> regname =
      std::make_unique<const std::string>("c");
  return *regname;
}

const std::string& node_default_reg() {
  static std::unique_ptr<const std::string> regname =
      std::make_unique<const std::string>("node");
  return *regname;
}

}  // namespace tket
