#pragma once

#include "quick_sort.h"

#include <algorithm>
#include <optional>
#include <string>
#include <type_traits>

namespace compiler {

template <typename T> constexpr T fnv_hash(const char *str, const T basis, const T prime) {
    size_t Val = basis;
    for (size_t Idx = 0; Idx < std::char_traits<char>::length(str); ++Idx) {
        Val ^= static_cast<size_t>(str[Idx]);
        Val *= prime;
    }
    return Val;
}

template <typename T> constexpr T get_fnv_prime() {}
template <> constexpr uint32_t get_fnv_prime() { return 16777619; }

template <> constexpr uint64_t get_fnv_prime() { return 1099511628211; }

constexpr auto hash(const char *str) {
    constexpr size_t FNV_hash_prime = get_fnv_prime<size_t>();
    constexpr size_t FNV_offset_basis = fnv_hash("chongo <Landon Curt Noll> /\\../\\", (size_t)0, FNV_hash_prime);
    return fnv_hash(str, FNV_offset_basis, FNV_hash_prime);
}

} // namespace compiler
