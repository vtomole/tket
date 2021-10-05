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

#include "ClassicalOps.hpp"

#include "OpType/OpType.hpp"

namespace tket {

static uint32_t u32_from_boolvec(const std::vector<bool> &x) {
  unsigned n = x.size();
  if (n > 32) {
    throw std::domain_error("Vector of bool exceeds maximum size (32)");
  }
  uint32_t X = 0;
  for (unsigned i = 0; i < n; i++) {
    if (x[i]) X |= (1u << i);
  }
  return X;
}

ClassicalOp::ClassicalOp(
    OpType type, unsigned n_i, unsigned n_io, unsigned n_o,
    const std::string &name)
    : Op(type), n_i_(n_i), n_io_(n_io), n_o_(n_o), name_(name), sig_() {
  for (unsigned i = 0; i < n_i; i++) {
    sig_.push_back(EdgeType::Boolean);
  }
  for (unsigned j = 0; j < n_io + n_o; j++) {
    sig_.push_back(EdgeType::Classical);
  }
}

std::string ClassicalOp::get_name(bool latex) const {
  std::stringstream name;
  if (latex) name << "\\text{";
  name << name_;
  if (latex) name << "}";
  return name.str();
}

bool ClassicalOp::is_equal(const Op &op_other) const {
  const ClassicalOp &other = dynamic_cast<const ClassicalOp &>(op_other);

  if (n_i_ != other.get_n_i()) return false;
  if (n_io_ != other.get_n_io()) return false;
  if (n_o_ != other.get_n_o()) return false;
  unsigned N = n_i_ + n_io_;
  uint32_t xlim = 1u << N;
  std::vector<bool> v(N);
  for (uint32_t x = 0; x < xlim; x++) {
    for (unsigned i = 0; i < N; i++) {
      v[i] = (x >> i) & 1;
    }
    if (eval(v) != other.eval(v)) return false;
  }
  return true;
}

ClassicalTransformOp::ClassicalTransformOp(
    unsigned n, const std::vector<uint32_t> &values, const std::string &name)
    : ClassicalOp(OpType::ClassicalTransform, 0, n, 0, name), values_(values) {
  if (n > 32) {
    throw std::domain_error("Too many inputs/outputs (maximum is 32)");
  }
}

std::vector<bool> ClassicalTransformOp::eval(const std::vector<bool> &x) const {
  if (x.size() != n_io_) {
    throw std::domain_error("Incorrect input size");
  }
  uint32_t val = values_[u32_from_boolvec(x)];
  std::vector<bool> y(n_io_);
  for (unsigned j = 0; j < n_io_; j++) {
    y[j] = (val >> j) & 1;
  }
  return y;
}

std::string SetBitsOp::get_name(bool latex) const {
  std::stringstream name;
  if (latex) name << "\\text{";
  name << name_ << "(";
  for (auto v : values_) {
    name << v;  // "0" or "1"
  }
  name << ")";
  if (latex) name << "}";
  return name.str();
}

std::vector<bool> SetBitsOp::eval(const std::vector<bool> &x) const {
  if (!x.empty()) {
    throw std::domain_error("Non-empty input");
  }
  return values_;
}

std::vector<bool> CopyBitsOp::eval(const std::vector<bool> &x) const {
  if (x.size() != n_i_) {
    throw std::domain_error("Incorrect input size");
  }
  return x;
}

std::string RangePredicateOp::get_name(bool latex) const {
  std::stringstream name;
  if (latex) name << "\\text{";
  name << name_ << "([" << a << "," << b << "])";
  if (latex) name << "}";
  return name.str();
}

std::vector<bool> RangePredicateOp::eval(const std::vector<bool> &x) const {
  if (x.size() != n_i_) {
    throw std::domain_error("Incorrect input size");
  }
  uint32_t X = u32_from_boolvec(x);
  std::vector<bool> y(1);
  y[0] = (X >= a && X <= b);
  return y;
}

bool RangePredicateOp::is_equal(const Op &op_other) const {
  const RangePredicateOp &other =
      dynamic_cast<const RangePredicateOp &>(op_other);

  if (n_i_ != other.n_i_) return false;
  return (a == other.a && b == other.b);
}

ExplicitPredicateOp::ExplicitPredicateOp(
    unsigned n, const std::vector<bool> &values, const std::string &name)
    : PredicateOp(OpType::ExplicitPredicate, n, name), values_(values) {
  if (n > 32) {
    throw std::domain_error("Too many inputs");
  }
}

std::vector<bool> ExplicitPredicateOp::eval(const std::vector<bool> &x) const {
  if (x.size() != n_i_) {
    throw std::domain_error("Incorrect input size");
  }
  std::vector<bool> y(1);
  y[0] = values_[u32_from_boolvec(x)];
  return y;
}

ExplicitModifierOp::ExplicitModifierOp(
    unsigned n, const std::vector<bool> &values, const std::string &name)
    : ModifyingOp(OpType::ExplicitModifier, n, name), values_(values) {
  if (n > 31) {
    throw std::domain_error("Too many inputs");
  }
}

std::vector<bool> ExplicitModifierOp::eval(const std::vector<bool> &x) const {
  if (x.size() != n_i_ + 1) {
    throw std::domain_error("Incorrect input size");
  }
  std::vector<bool> y(1);
  y[0] = values_[u32_from_boolvec(x)];
  return y;
}

MultiBitOp::MultiBitOp(std::shared_ptr<const ClassicalOp> op, unsigned n)
    : ClassicalOp(
          OpType::MultiBit, n * op->get_n_i(), n * op->get_n_io(),
          n * op->get_n_o(), op->get_name()),
      op_(op),
      n_(n) {
  op_signature_t op_sig = op->get_signature();
  const std::size_t op_sig_size = op_sig.size();
  sig_.clear();
  sig_.reserve(n_ * op_sig.size());
  for (unsigned i = 0; i < n_; i++) {
    std::copy_n(op_sig.begin(), op_sig_size, std::back_inserter(sig_));
  }
}

std::string MultiBitOp::get_name(bool latex) const {
  std::stringstream name;
  if (latex) name << "\\text{";
  name << name_ << " (*" << n_ << ")";
  if (latex) name << "}";
  return name.str();
}

std::vector<bool> MultiBitOp::eval(const std::vector<bool> &x) const {
  if (x.size() != n_i_ + n_io_) {
    throw std::domain_error("Incorrect input size");
  }
  unsigned n_op_inputs = op_->get_n_i() + op_->get_n_io();
  unsigned n_op_outputs = op_->get_n_io() + op_->get_n_o();
  std::vector<bool> y(n_io_ + n_o_);
  for (unsigned i = 0; i < n_; i++) {
    std::vector<bool> x_i(n_op_inputs);
    for (unsigned j = 0; j < n_op_inputs; j++) {
      x_i[j] = x[n_op_inputs * i + j];
    }
    std::vector<bool> y_i = op_->eval(x_i);
    for (unsigned j = 0; j < n_op_outputs; j++) {
      y[n_op_outputs * i + j] = y_i[j];
    }
  }
  return y;
}

bool MultiBitOp::is_equal(const Op &op_other) const {
  const MultiBitOp &other = dynamic_cast<const MultiBitOp &>(op_other);

  if (n_ != other.n_) return false;
  return (*op_ == *(other.op_));
}

std::shared_ptr<ClassicalTransformOp> ClassicalX() {
  static const std::vector<uint32_t> values = {1, 0};
  static const std::shared_ptr<ClassicalTransformOp> op =
      std::make_shared<ClassicalTransformOp>(1, values, "ClassicalX");
  return op;
}

std::shared_ptr<ClassicalTransformOp> ClassicalCX() {
  static const std::vector<uint32_t> values = {0, 3, 2, 1};
  static const std::shared_ptr<ClassicalTransformOp> op =
      std::make_shared<ClassicalTransformOp>(2, values, "ClassicalCX");
  return op;
}

std::shared_ptr<ExplicitPredicateOp> NotOp() {
  static const std::vector<bool> values = {1, 0};
  static const std::shared_ptr<ExplicitPredicateOp> op =
      std::make_shared<ExplicitPredicateOp>(1, values, "NOT");
  return op;
}

std::shared_ptr<ExplicitPredicateOp> AndOp() {
  static const std::vector<bool> values = {0, 0, 0, 1};
  static const std::shared_ptr<ExplicitPredicateOp> op =
      std::make_shared<ExplicitPredicateOp>(2, values, "AND");
  return op;
}

std::shared_ptr<ExplicitPredicateOp> OrOp() {
  static const std::vector<bool> values = {0, 1, 1, 1};
  static const std::shared_ptr<ExplicitPredicateOp> op =
      std::make_shared<ExplicitPredicateOp>(2, values, "OR");
  return op;
}

std::shared_ptr<ExplicitPredicateOp> XorOp() {
  static const std::vector<bool> values = {0, 1, 1, 0};
  static const std::shared_ptr<ExplicitPredicateOp> op =
      std::make_shared<ExplicitPredicateOp>(2, values, "XOR");
  return op;
}

std::shared_ptr<ExplicitModifierOp> AndWithOp() {
  static const std::vector<bool> values = {0, 0, 0, 1};
  static const std::shared_ptr<ExplicitModifierOp> op =
      std::make_shared<ExplicitModifierOp>(1, values, "AND");
  return op;
}

std::shared_ptr<ExplicitModifierOp> OrWithOp() {
  static const std::vector<bool> values = {0, 1, 1, 1};
  static const std::shared_ptr<ExplicitModifierOp> op =
      std::make_shared<ExplicitModifierOp>(1, values, "OR");
  return op;
}

std::shared_ptr<ExplicitModifierOp> XorWithOp() {
  static const std::vector<bool> values = {0, 1, 1, 0};
  static const std::shared_ptr<ExplicitModifierOp> op =
      std::make_shared<ExplicitModifierOp>(1, values, "XOR");
  return op;
}

}  // namespace tket
