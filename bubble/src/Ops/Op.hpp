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

#ifndef _TKET_Ops_Op_H_
#define _TKET_Ops_Op_H_

/**
 * @file
 * @brief Operations
 */

#include <memory>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "OpPtr.hpp"
#include "OpType/OpDesc.hpp"
#include "OpType/OpTypeFunctions.hpp"
#include "Utils/Constants.hpp"
#include "Utils/Exceptions.hpp"
#include "Utils/Expression.hpp"
#include "Utils/UnitID.hpp"

namespace tket {

/** Wrong number of parameters for an operation */
class InvalidParameterCount : public std::logic_error {
 public:
  InvalidParameterCount()
      : std::logic_error("Gate has an invalid number of parameters") {}
};

/** A specific entry or exit port of an op */
typedef unsigned port_t;

/**
 * Abstract class representing an operation type
 */
class Op : public std::enable_shared_from_this<Op> {
 public:
  /**
   * Inverse (of a unitary operation)
   *
   * @throw NotValid if operation is not unitary
   */
  virtual Op_ptr dagger() const { throw NotValid(); }

  /**
   * Transpose of a unitary operation
   */
  virtual Op_ptr transpose() const { throw NotValid(); };

  /**
   * Operation with values for symbols substituted
   *
   * @param sub_map map from symbols to values
   *
   * @return New operation with symbols substituted, or a null pointer if
   *         the operation type does not support symbols.
   */
  virtual Op_ptr symbol_substitution(
      const SymEngine::map_basic_basic &sub_map) const = 0;

  /** Sequence of phase parameters, if applicable */
  virtual std::vector<Expr> get_params() const { throw NotValid(); }

  /** Sequence of phase parameters reduced to canonical range, if applicable */
  virtual std::vector<Expr> get_params_reduced() const { throw NotValid(); }

  /* Number of qubits */
  virtual unsigned n_qubits() const { throw NotValid(); }

  /** String representation */
  virtual std::string get_name(bool latex = false) const;

  /** Command representation */
  virtual std::string get_command_str(const unit_vector_t &args) const;

  /** Get operation descriptor */
  OpDesc get_desc() const { return desc_; }

  /** Get operation type */
  OpType get_type() const { return type_; }

  /** Set of all free symbols occurring in operation parameters. */
  virtual SymSet free_symbols() const = 0;

  /** Vector specifying type of data for each port on op */
  virtual op_signature_t get_signature() const = 0;

  /**
   * Test whether operation is identity up to a phase and return phase if so.
   *
   * @return phase, as multiple of pi, if operation is identity up to phase
   * @throw NotValid if operation is not a \ref Gate
   */
  virtual std::optional<double> is_identity() const { throw NotValid(); }

  virtual ~Op() {}

  bool operator==(const Op &other) const {
    return type_ == other.type_ && is_equal(other);
  }

  /**
   * Checks equality between two instances of the same class.
   * The Op object passed as parameter must always be of the same type as this.
   *
   * For base class Op, it is sufficient that they have same type
   */
  virtual bool is_equal(const Op &) const { return true; }

 protected:
  explicit Op(const OpType &type) : desc_(type), type_(type) {}
  const OpDesc desc_; /**< Operation descriptor */
  const OpType type_; /**< Operation type */
};

// friend to stream op (print)
std::ostream &operator<<(std::ostream &os, Op const &operation);

}  // namespace tket

#endif  // OPS_H_
