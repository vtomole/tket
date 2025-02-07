// Copyright 2019-2022 Cambridge Quantum Computing
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

#pragma once

#include "Transform.hpp"

namespace tket {

namespace Transforms {

// decomposes multiq gates not in the gate set to CXs, then replaces CXs with
// the replacement (if CX is not allowed) then converts singleq gates no in
// the gate set to U3 and replaces them using provided function Expects: any
// gates Produces: gates in multiqs and singleqs
Transform rebase_factory(
    const OpTypeSet& multiqs, const Circuit& cx_replacement,
    const OpTypeSet& singleqs,
    const std::function<Circuit(const Expr&, const Expr&, const Expr&)>&
        tk1_replacement);

// Multiqs: CX
// Singleqs: TK1
Transform rebase_tket();

// Multiqs: CZ
// Singleqs: PhasedX, Rz
Transform rebase_cirq();

// Multiqs: ZZMax
// Singleqs: PhasedX, Rz
Transform rebase_HQS();

// Multiqs: XXPhase
// Singleqs: PhasedX, Rz
Transform rebase_UMD();

// Multiqs: CZ
// Singleqs: Rx, Rz
Transform rebase_quil();

// Multiqs: SWAP, CX, CZ
// Singleqs: H, X, Z, S, T, Rx, Rz
Transform rebase_pyzx();

// Multiqs: SWAP, CRz, CX, CZ
// Singleqs: H, X, Y, Z, S, T, V, Rx, Ry, Rz
Transform rebase_projectq();

// Used for UniversalFrameRandomisation
// Multiqs: CX
// Singleqs: Rz, H
Transform rebase_UFR();

// Multiqs: ECR
// Singleqs: Rz, SX
Transform rebase_OQC();

// converts a TK1 gate to a PhasedXRz gate
Circuit tk1_to_PhasedXRz(
    const Expr& alpha, const Expr& beta, const Expr& gamma);

Circuit tk1_to_rzrx(const Expr& alpha, const Expr& beta, const Expr& gamma);

Circuit tk1_to_rzh(const Expr& alpha, const Expr& beta, const Expr& gamma);

Circuit tk1_to_rzsx(const Expr& alpha, const Expr& beta, const Expr& gamma);

Circuit tk1_to_tk1(const Expr& alpha, const Expr& beta, const Expr& gamma);

}  // namespace Transforms

}  // namespace tket
