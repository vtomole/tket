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

from conans import ConanFile


class TketBenchmarksConan(ConanFile):
    name = "tket-benchmarks"
    version = "0.1.0"
    requires = "gtest/1.10.0", "benchmark/1.5.1"

    # TODO
