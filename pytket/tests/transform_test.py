# Copyright 2019-2022 Cambridge Quantum Computing
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from pathlib import Path
from pytket.circuit import Circuit, OpType, PauliExpBox  # type: ignore
from pytket.pauli import Pauli  # type: ignore
from pytket.passes import RemoveRedundancies, KAKDecomposition, ThreeQubitSquash, CommuteThroughMultis, PauliSquash, FullPeepholeOptimise, GlobalisePhasedX  # type: ignore
from pytket.predicates import CompilationUnit  # type: ignore
from pytket.transform import Transform, CXConfigType, PauliSynthStrat  # type: ignore
from pytket.qasm import circuit_from_qasm

from sympy import Symbol  # type: ignore
import numpy as np
import json
import pytest


def get_test_circuit() -> Circuit:
    # alpha = Symbol("alpha")
    # beta = Symbol("beta")
    alpha = 0.356
    beta = 1.183
    c = Circuit(4)
    c.Rx(0.5, 0)
    c.Rx(0.5, 1)
    c.H(2)
    c.Rx(0.5, 3)
    c.CX(3, 2)
    c.CX(2, 1)
    c.CX(1, 0)
    c.Rz(alpha, 0)
    c.CX(1, 0)
    c.CX(2, 1)
    c.CX(3, 2)
    c.Rx(1.5, 0)
    c.Rx(1.5, 1)
    c.H(2)
    c.Rx(1.5, 3)
    c.H(0)
    c.Rx(0.5, 1)
    c.Rx(0.5, 2)
    c.Rx(0.5, 3)
    c.CX(3, 2)
    c.CX(2, 1)
    c.CX(1, 0)
    c.Rz(beta, 0)
    c.CX(1, 0)
    c.CX(2, 1)
    c.CX(3, 2)
    c.H(0)
    c.Rx(1.5, 1)
    c.Rx(1.5, 2)
    c.Rx(1.5, 3)
    return c


def get_KAK_test_circuit() -> Circuit:
    c = Circuit(4)
    c.CX(0, 1)
    c.CX(1, 0)
    c.CX(0, 1)
    c.CX(1, 0)
    c.CX(2, 3)
    c.CX(3, 2)
    c.CX(2, 3)
    c.CX(3, 2)
    c.CX(0, 2)
    c.CX(2, 0)
    c.CX(0, 2)
    c.CX(2, 0)
    c.CX(1, 3)
    c.CX(3, 1)
    c.CX(1, 3)
    c.CX(3, 1)
    return c


def get_KAK_test_fidelity_circuit() -> Circuit:
    c = Circuit(2)
    c.add_gate(OpType.TK1, [3.51402, 0.552635, 3.56255], [0])
    c.add_gate(OpType.TK1, [0.567177, 0.482056, 3.66929], [1])
    c.CX(0, 1)
    c.add_gate(OpType.TK1, [0.496564, 0.5, 3.5], [0])
    c.add_gate(OpType.TK1, [3.91171, 0, 0], [1])
    c.CX(0, 1)
    c.add_gate(OpType.TK1, [0.5, 0.5, 0.5], [0])
    c.add_gate(OpType.TK1, [3.7823, 0, 0], [1])
    c.CX(0, 1)
    c.add_gate(OpType.TK1, [0.691597, 0.286125, 3.05058], [0])
    c.add_gate(OpType.TK1, [0.1989, 0.279667, 0.818303], [1])
    return c


def test_remove_redundancies() -> None:
    c = get_test_circuit()
    c.CX(0, 1)
    c.Rz(0.0, 1)
    c.CX(0, 1)
    Transform.RemoveRedundancies().apply(c)
    assert c.n_gates_of_type(OpType.Rx) == 8
    assert c.n_gates_of_type(OpType.CX) == 12


def test_reduce_singles() -> None:
    c = get_test_circuit()
    Transform.ReduceSingles().apply(c)
    assert c.n_gates_of_type(OpType.TK1) == 12
    assert c.n_gates_of_type(OpType.CX) == 12


def test_commute() -> None:
    c = get_test_circuit()
    Transform.CommuteThroughMultis().apply(c)
    assert c.n_gates_of_type(OpType.Rx) == 12
    assert c.n_gates_of_type(OpType.Rz) == 2
    assert c.n_gates_of_type(OpType.CX) == 12
    Transform.ReduceSingles().apply(c)
    assert c.n_gates_of_type(OpType.TK1) == 12


def test_global_phasedx() -> None:
    c = Circuit(3).add_gate(OpType.NPhasedX, [0.4, 2.3], [0, 1])
    Transform.GlobalisePhasedX().apply(c)
    assert c.n_gates_of_type(OpType.NPhasedX) == 2
    assert c.n_gates_of_type(OpType.Rz) == 2


def test_KAK() -> None:
    c = get_KAK_test_circuit()
    Transform.KAKDecomposition().apply(c)
    assert c.n_gates_of_type(OpType.CX) == 8


def test_fidelity_KAK() -> None:
    c = get_KAK_test_circuit()
    Transform.KAKDecomposition(cx_fidelity=0.6).apply(c)
    assert c.n_gates_of_type(OpType.CX) == 0


def test_fidelity_KAK_pass() -> None:
    """makes sure applying the Transform and the KAK pass
    produces the same result
    """
    c1 = get_KAK_test_fidelity_circuit()
    Transform.KAKDecomposition(cx_fidelity=0.94).apply(c1)
    res1 = c1.n_gates_of_type(OpType.CX)
    c2 = get_KAK_test_fidelity_circuit()
    KAKDecomposition(cx_fidelity=0.94).apply(c2)
    res2 = c2.n_gates_of_type(OpType.CX)
    assert res1 == res2


def test_fidelity_KAK2() -> None:
    c = get_KAK_test_fidelity_circuit()
    Transform.KAKDecomposition(cx_fidelity=0.6).apply(c)
    assert c.n_gates_of_type(OpType.CX) == 0

    c = get_KAK_test_fidelity_circuit()
    Transform.KAKDecomposition(cx_fidelity=0.7).apply(c)
    assert c.n_gates_of_type(OpType.CX) == 1

    c = get_KAK_test_fidelity_circuit()
    Transform.KAKDecomposition(cx_fidelity=0.94).apply(c)
    assert c.n_gates_of_type(OpType.CX) == 2

    c = get_KAK_test_fidelity_circuit()
    Transform.KAKDecomposition(cx_fidelity=0.99).apply(c)
    assert c.n_gates_of_type(OpType.CX) == 3


def test_three_qubit_squash() -> None:
    c = Circuit(3)
    for i in range(50):
        c.Rz(0.125, i % 3)
        c.CX((i + 2) % 3, (i + 1) % 3)
    assert Transform.ThreeQubitSquash().apply(c)
    assert c.n_gates_of_type(OpType.CX) <= 14


def test_basic_rebases() -> None:
    c = get_test_circuit()
    Transform.RebaseToTket().apply(c)
    assert c.n_gates_of_type(OpType.Rz) == 0
    assert c.n_gates_of_type(OpType.Rx) == 0
    Transform.RebaseToRzRx().apply(c)
    assert c.n_gates_of_type(OpType.U1) == 0
    assert c.n_gates_of_type(OpType.U3) == 0
    Transform.RebaseToCliffordSingles().apply(c)
    assert c.n_gates_of_type(OpType.Rz) == 2
    assert c.n_gates_of_type(OpType.Rx) == 0
    assert c.n_gates_of_type(OpType.U1) == 0
    assert c.n_gates_of_type(OpType.U3) == 0


def test_post_routing() -> None:
    c = get_test_circuit()
    Transform.OptimisePostRouting().apply(c)
    assert c.n_gates_of_type(OpType.TK1) == 12
    assert c.n_gates_of_type(OpType.CX) == 12


def test_phase_gadget() -> None:
    c = get_test_circuit()
    Transform.OptimisePhaseGadgets(CXConfigType.Tree).apply(c)
    assert c.n_gates_of_type(OpType.CX) == 12
    assert c.depth_by_type(OpType.CX) == 8


def test_Cliffords() -> None:
    c = get_test_circuit()
    c2 = c.copy()
    Transform.OptimisePhaseGadgets().apply(c)
    Transform.OptimiseCliffords().apply(c)
    assert c.n_gates_of_type(OpType.CX) == 8
    assert np.allclose(c.get_statevector(), c2.get_statevector())


def test_Pauli_gadget() -> None:
    c = get_test_circuit()
    Transform.OptimisePauliGadgets(CXConfigType.Tree).apply(c)
    assert c.n_gates_of_type(OpType.CX) == 6


def test_Pauli_gadget_xxphase3() -> None:
    c = Circuit(4)
    c.H(0).H(2).H(3)
    c.CX(2, 1).CX(1, 0)
    c.Rz(0.3, 0)
    c.CX(1, 0).CX(2, 1)
    c.H(1)
    c.CX(1, 0).CX(3, 2).CX(2, 0)
    c.H(1).H(3)
    c.Rz(0.8, 0)
    c.CX(3, 1).CX(2, 0)
    c.Rz(0.4, 1)
    c.CX(3, 1)
    c.H(1).H(3)
    c.CX(3, 2).CX(1, 0)
    c.H(0).H(1).H(2).H(3)

    Transform.SynthesisePauliGraph(cx_config=CXConfigType.MultiQGate).apply(c)
    assert c.n_gates_of_type(OpType.XXPhase3) == 2


def test_cons_sequencing() -> None:
    c = get_test_circuit()
    c2 = c.copy()
    t = Transform.OptimisePhaseGadgets() >> Transform.OptimiseCliffords()
    t.apply(c)
    assert c.n_gates_of_type(OpType.CX) == 8
    assert np.allclose(c.get_statevector(), c2.get_statevector())


def test_list_sequencing() -> None:
    c = get_test_circuit()
    t_list = [
        Transform.OptimisePhaseGadgets(CXConfigType.Star),
        Transform.OptimiseCliffords(),
        Transform.RebaseToTket(),
    ]
    Transform.sequence(t_list).apply(c)
    assert c.n_gates_of_type(OpType.CX) == 8
    assert c.n_gates_of_type(OpType.V) == 0
    assert c.n_gates_of_type(OpType.S) == 0
    assert c.n_gates_of_type(OpType.Z) == 0
    assert c.n_gates_of_type(OpType.X) == 0


def test_basic_repeat() -> None:
    c = Circuit(2)
    c.Rz(-0.34, 0)
    c.Rx(-0.63, 1)
    c.CX(0, 1)
    c.Rz(0.34, 0)
    c.Rx(0.63, 1)
    loop_body = Transform.RemoveRedundancies() >> Transform.CommuteThroughMultis()
    Transform.repeat(loop_body).apply(c)
    assert c.n_gates == 1
    assert c.n_gates_of_type(OpType.CX) == 1


def test_while_repeat() -> None:
    c = Circuit(2)
    c.Rz(-0.34, 0)
    c.Rx(-0.63, 1)
    c.CX(1, 0)
    c.CX(1, 0)
    c.CX(0, 1)
    c.Rz(0.34, 0)
    c.Rx(0.63, 1)
    assert (
        Transform.while_repeat(
            Transform.RebaseToCliffordSingles(), Transform.RemoveRedundancies()
        ).apply(c)
        == False
    )
    assert (
        Transform.while_repeat(
            Transform.CommuteThroughMultis(), Transform.RemoveRedundancies()
        ).apply(c)
        == True
    )
    assert c.n_gates_of_type(OpType.CX) == 1
    assert c.n_gates_of_type(OpType.Rz) == 0
    assert c.n_gates_of_type(OpType.Rx) == 0


def test_pauli_graph_synth() -> None:
    strats = [
        PauliSynthStrat.Individual,
        PauliSynthStrat.Pairwise,
        PauliSynthStrat.Sets,
    ]
    cx_counts = []
    for s in strats:
        c = Circuit(4)
        # JW double excitation Paulis
        paulis1 = [Pauli.X, Pauli.Y, Pauli.Y, Pauli.Y]
        paulis2 = [Pauli.Y, Pauli.X, Pauli.Y, Pauli.Y]
        paulis3 = [Pauli.Y, Pauli.Y, Pauli.X, Pauli.Y]
        paulis4 = [Pauli.Y, Pauli.Y, Pauli.Y, Pauli.X]
        paulis5 = [Pauli.X, Pauli.X, Pauli.X, Pauli.Y]
        paulis6 = [Pauli.X, Pauli.X, Pauli.Y, Pauli.X]
        paulis7 = [Pauli.X, Pauli.Y, Pauli.X, Pauli.X]
        paulis8 = [Pauli.Y, Pauli.X, Pauli.X, Pauli.X]
        pbox1 = PauliExpBox(paulis1, 0.1)
        pbox2 = PauliExpBox(paulis2, 0.2)
        pbox3 = PauliExpBox(paulis3, 0.3)
        pbox4 = PauliExpBox(paulis4, 0.4)
        pbox5 = PauliExpBox(paulis5, 0.4)
        pbox6 = PauliExpBox(paulis6, 0.4)
        pbox7 = PauliExpBox(paulis7, 0.4)
        pbox8 = PauliExpBox(paulis8, 0.4)
        c.add_pauliexpbox(pbox1, [0, 1, 2, 3])
        c.add_pauliexpbox(pbox2, [0, 1, 2, 3])
        c.add_pauliexpbox(pbox3, [0, 1, 2, 3])
        c.add_pauliexpbox(pbox4, [0, 1, 2, 3])
        c.add_pauliexpbox(pbox5, [0, 1, 2, 3])
        c.add_pauliexpbox(pbox6, [0, 1, 2, 3])
        c.add_pauliexpbox(pbox7, [0, 1, 2, 3])
        c.add_pauliexpbox(pbox8, [0, 1, 2, 3])
        Transform.SynthesisePauliGraph(s).apply(c)
        num_cxs = c.n_gates_of_type(OpType.CX)
        cx_counts.append(num_cxs)

    for (i, count) in enumerate(cx_counts):
        if i == 0:
            continue
        assert count < cx_counts[i - 1]


def test_cnry_decomp() -> None:
    # TKET-543

    circ = Circuit(2)
    circ.Ry(0.5, 0)
    state0 = circ.get_statevector()
    assert circ.n_gates == 1

    circ.add_gate(OpType.CnRy, 1.1, [0, 1])
    circ.add_gate(OpType.CnRy, 0.9, [0, 1])

    state1a = circ.get_statevector()
    unit1a = circ.get_unitary()

    cu1b = CompilationUnit(circ)
    assert circ.n_gates == 3
    RemoveRedundancies().apply(cu1b)
    circ = cu1b.circuit
    assert circ.n_gates == 2
    state1b = circ.get_statevector()
    unit1b = circ.get_unitary()

    # circ0 should not be equivalent to circ1b (in fact, the states are orthogonal):
    assert abs(np.vdot(state0, state1b)) < 1e-10  # type: ignore

    # circ1a and circ1b should be equivalent:
    assert np.allclose(state1a, state1b)
    assert np.allclose(unit1a, unit1b)


def test_optimise_cliffords() -> None:
    # TKET-846
    c = Circuit(4)
    c.CZ(0, 2)
    c.CZ(3, 1)
    c.V(2)
    c.V(3)
    c.CZ(0, 3)
    c.V(3)
    c.CZ(3, 1)
    c.CZ(2, 1)
    c.V(2)
    c.CZ(0, 2)
    c.X(2)
    c.V(1)
    c.CZ(3, 1)
    c.CZ(2, 1)
    c.CZ(3, 1)
    c.V(2)
    c.V(1)
    c.CZ(2, 1)
    c.X(2)
    c.CZ(2, 1)
    c.V(2)
    c.CZ(2, 1)
    c.CZ(0, 2)
    c.CZ(2, 1)

    c1 = c.copy()
    Transform.OptimiseCliffords(True).apply(c1)

    # Check circuits are equivalent.
    s = c.get_statevector()
    s1 = c1.get_statevector()
    assert np.allclose(s, s1)

    # Test with allow_swaps=false
    c2 = c.copy()
    Transform.OptimiseCliffords(False).apply(c2)
    assert np.allclose(s, c2.get_statevector())


def test_implicit_swaps_1() -> None:
    # TKET-858
    # 0->3, 1->0, 2->2, 3->1
    c = Circuit(4)
    c.X(0).CX(0, 1).CX(1, 0).CX(1, 3).CX(3, 1).X(2)
    s = c.get_statevector()
    Transform.OptimiseCliffords().apply(c)
    s1 = c.get_statevector()
    assert np.allclose(s, s1)


def test_implicit_swaps_2() -> None:
    # TKET-858
    # 0->1, 1->0
    c = Circuit(2)
    c.CX(0, 1)
    c.CX(1, 0)
    u = c.get_unitary()
    Transform.OptimiseCliffords().apply(c)
    u1 = c.get_unitary()
    assert np.allclose(u, u1)


def test_implicit_swaps_3() -> None:
    # TKET-858
    # 0->1, 1->0, 2->2
    c0 = Circuit(3).CX(0, 1).H(0).H(1).CX(0, 1)
    c1 = c0.copy()
    Transform.OptimiseCliffords().apply(c1)

    s0 = c0.get_statevector()
    s1 = c1.get_statevector()
    assert np.allclose(s0, s1)

    u0 = c0.get_unitary()
    u1 = c1.get_unitary()
    assert np.allclose(u0, u1)


def test_implicit_swaps_4() -> None:
    # TKET-858
    # 0->1, 1->0, 2->2, 3->3, 4->4
    # Unitary with all rows and columns distinct.
    c = (
        Circuit(5)
        .X(0)
        .V(1)
        .X(2)
        .V(3)
        .X(4)
        .CX(0, 1)
        .H(2)
        .S(3)
        .H(4)
        .H(0)
        .CX(4, 1)
        .H(3)
        .Rz(0.2, 1)
        .S(3)
        .CX(1, 0)
        .V(3)
        .CX(2, 0)
        .CX(4, 0)
        .CX(1, 0)
        .CX(2, 0)
        .CX(4, 1)
        .CX(4, 0)
        .H(2)
        .H(0)
        .V(2)
        .H(4)
        .CX(0, 1)
        .S(2)
        .V(4)
        .V(0)
        .Vdg(1)
        .H(2)
        .S(4)
        .S(0)
        .V(1)
        .S(2)
        .H(4)
        .H(0)
        .S(1)
        .S(4)
        .S(0)
        .H(1)
        .V(4)
        .H(0)
        .S(1)
        .S(4)
        .H(1)
        .H(4)
        .CX(0, 1)
        .S(4)
        .H(0)
        .V(4)
        .CX(0, 2)
        .CX(0, 3)
        .H(0)
        .CX(3, 1)
        .Rz(0.4, 0)
        .H(0)
        .CX(3, 1)
        .CX(0, 3)
        .CX(0, 2)
        .Vdg(3)
        .H(0)
        .V(2)
        .V(3)
        .CX(0, 1)
        .S(2)
        .S(3)
        .H(0)
        .H(1)
        .H(2)
        .H(3)
        .V(0)
        .V(1)
        .S(2)
        .S(3)
        .S(0)
        .S(1)
        .V(2)
        .V(3)
        .H(0)
        .H(1)
        .S(2)
        .CX(4, 3)
        .S(0)
        .S(1)
        .H(2)
        .V(1)
        .S(2)
        .CX(3, 1)
        .CX(1, 0)
        .CX(1, 0)
        .V(0)
        .CX(3, 1)
        .S(0)
        .Vdg(1)
        .CX(4, 3)
        .H(0)
        .V(1)
        .Vdg(3)
        .Vdg(4)
        .S(0)
        .S(1)
        .V(3)
        .V(4)
        .H(1)
        .S(3)
        .S(4)
        .S(1)
        .H(3)
        .H(4)
        .S(3)
        .S(4)
    )
    u0 = c.get_unitary()
    Transform.OptimiseCliffords().apply(c)
    u1 = c.get_unitary()
    assert np.allclose(u0, u1)


def test_commute_through_multis() -> None:
    # TKET-1253
    c = Circuit(2)
    c.add_gate(OpType.PhasedISWAP, [0.7, 1.6], [0, 1])
    c.Sdg(0)
    cu = CompilationUnit(c)
    assert not CommuteThroughMultis().apply(cu)


def test_cu3_removal() -> None:
    # TKET-1261
    c0 = Circuit(2)
    c0.add_gate(OpType.CU3, [0, 0, 0.5], [0, 1])
    assert not RemoveRedundancies().apply(c0)
    c1 = Circuit(2)
    c1.add_gate(OpType.CU3, [4, 0.6, 1.4], [0, 1])
    assert RemoveRedundancies().apply(c1)


def test_symbol_squash() -> None:
    # Test simplification of symbolic angles when squashing.
    a = Symbol("a")
    circ = Circuit(1)
    circ.Ry(0.5, 0).Rz(a, 0).Ry(0.5, 0)
    circ1 = circ.copy()
    assert PauliSquash().apply(circ1)
    cmds = circ1.get_commands()
    assert len(cmds) == 1
    op = cmds[0].op
    assert op.type == OpType.TK1
    assert len(str(op)) <= 100
    for x in np.arange(0.0, 4.0, 0.4):
        smap = {a: x}
        c = circ.copy()
        c.symbol_substitution(smap)
        u = c.get_unitary()
        c1 = circ1.copy()
        c1.symbol_substitution(smap)
        u1 = c1.get_unitary()
        # PauliSquash does not preserve global phase.
        v = u @ u1.conjugate().transpose()
        assert np.allclose(v, v[0, 0] * np.eye(2, 2, dtype=complex))


def symbolic_test_circ(n: int) -> Circuit:
    a = Symbol("a")
    circ = Circuit(n)
    for i in range(n - 1, 0, -1):
        circ.CX(i, i - 1)
    circ.Rz(-a, 0)
    for i in range(n - 1):
        circ.CX(i + 1, i)
    circ.H(0).V(0)
    circ.H(n - 1).V(n - 1)
    for i in range(n - 1, 0, -1):
        circ.CX(i, i - 1)
    circ.Rz(a, 0)
    return circ


def test_symbol_pauli_squash_1() -> None:
    # Test simplification of symbolic angles when squashing
    circ = symbolic_test_circ(2)
    circ1 = circ.copy()
    assert PauliSquash().apply(circ1)
    for x in np.arange(0.0, 4.0, 0.4):
        smap = {Symbol("a"): x}
        c = circ.copy()
        c.symbol_substitution(smap)
        u = c.get_unitary()
        c1 = circ1.copy()
        c1.symbol_substitution(smap)
        u1 = c1.get_unitary()
        # PauliSquash does not preserve global phase.
        v = u @ u1.conjugate().transpose()
        assert np.allclose(v, v[0, 0] * np.eye(4, 4, dtype=complex))


def test_symbol_pauli_squash_2() -> None:
    # Test simplification of symbolic angles when squashing
    circ = symbolic_test_circ(3)
    circ1 = circ.copy()
    assert PauliSquash().apply(circ1)
    for x in np.arange(0.0, 4.0, 0.4):
        smap = {Symbol("a"): x}
        c = circ.copy()
        c.symbol_substitution(smap)
        u = c.get_unitary()
        c1 = circ1.copy()
        c1.symbol_substitution(smap)
        u1 = c1.get_unitary()
        # PauliSquash does not preserve global phase.
        v = u @ u1.conjugate().transpose()
        assert np.allclose(v, v[0, 0] * np.eye(8, 8, dtype=complex))


def test_determinism() -> None:
    # TKET-1362
    c = circuit_from_qasm(
        Path(__file__).resolve().parent / "qasm_test_files" / "test11.qasm"
    )
    c0 = c.copy()
    c1 = c.copy()
    assert c0 == c1
    FullPeepholeOptimise().apply(c0)
    FullPeepholeOptimise().apply(c1)
    assert c0 == c1


def test_full_peephole_optimise() -> None:
    with open(
        Path(__file__).resolve().parent / "json_test_files" / "circuit.json", "r"
    ) as f:
        circ = Circuit.from_dict(json.load(f))

    n_cz = circ.n_gates_of_type(OpType.CZ)

    circ0 = circ.copy()
    FullPeepholeOptimise().apply(circ0)
    perm0 = circ0.implicit_qubit_permutation()
    assert any(a != b for a, b in perm0.items())
    n_cx0 = circ0.n_gates_of_type(OpType.CX)
    assert n_cx0 < n_cz

    circ1 = circ.copy()
    FullPeepholeOptimise(allow_swaps=False).apply(circ1)
    perm1 = circ1.implicit_qubit_permutation()
    assert all(a == b for a, b in perm1.items())
    n_cx1 = circ1.n_gates_of_type(OpType.CX)
    assert n_cx1 < n_cz


if __name__ == "__main__":
    test_remove_redundancies()
    test_reduce_singles()
    test_commute()
    test_KAK()
    test_basic_rebases()
    test_post_routing()
    test_phase_gadget()
    test_Cliffords()
    test_Pauli_gadget()
    test_cons_sequencing()
    test_list_sequencing()
    test_basic_repeat()
    test_while_repeat()
    test_implicit_swaps_1()
    test_implicit_swaps_2()
    test_implicit_swaps_3()
