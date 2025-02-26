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

"""
The routing module provides access to the tket :py:class:`Architecture` structure and
methods for modifying circuits to satisfy the architectural constraints. It also
provides acess to the :py:class:`Placement` constructors for relabelling Circuit qubits
and has some methods for routing circuits. This module is provided in binary form during
the PyPI installation.
"""

from pytket._tket.routing import *  # type: ignore
