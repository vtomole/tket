# Copyright 2019-2021 Cambridge Quantum Computing
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

"""The circuit module provides an API to interact with the
 tket :py:class:`Circuit` data structure.
  This module is provided in binary form during the PyPI installation."""

from pytket._tket.circuit import *  # type: ignore

# prefixes for assertion bits
from pytket._tket.circuit import _DEBUG_ZERO_REG_PREFIX, _DEBUG_ONE_REG_PREFIX  # type: ignore
