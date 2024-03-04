/*
<name TBD>, a handmade game about simulating things in space
Copyright (C) 2024 Nathan Smith

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <cmath>
#include <cstdint>

struct Time64 {
  uint32_t wholeSeconds;
  uint32_t fraction;
};

uint64_t Time64ToU64(Time64 t) {
  return (uint64_t)(t.wholeSeconds) << 32 | t.fraction;
}

Time64 Time64AddFloat(Time64 t, float addend) {
  uint64_t addendU64{(uint64_t)(addend * std::pow(2, 32) + 0.5f)};
  uint64_t res{Time64ToU64(t) + addendU64};
  return Time64{(uint32_t)(res >> 32), (uint32_t)(res & 0xFFFFFFFF)};
}
