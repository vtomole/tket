# Copyright 2019-2021 Cambridge Quantum Computing
#
# You may not use this file except in compliance with the Licence.
# You may obtain a copy of the Licence in the LICENCE file accompanying
# these documents or at:
#
#     https://cqcl.github.io/pytket/build/html/licence.html

"""The architecture module provides an API to interact with the
 tket ::py:class:'Architecture' class, which for some set of identified physical qubits,
 defines which can run two-qubit gates between them. This module is provided in binary
 form during the PyPI installation."""

from pytket._tket.architecture import *  # type: ignore