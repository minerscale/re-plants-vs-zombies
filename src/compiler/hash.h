#pragma once

#include "quick_sort.h"

#include <algorithm>
#include <optional>
#include <ranges>
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

template <typename V, typename T, std::size_t Size> class OrderedHashMap {
    using HashFnMapItem = std::tuple<decltype(compiler::hash(std::declval<T>())), V>;

    struct HashItemComparator {
        constexpr bool operator()(const HashFnMapItem &lhs, const HashFnMapItem &rhs) const {
            return std::get<0>(lhs) < std::get<0>(rhs);
        }
    };

    std::array<HashFnMapItem, Size> arr;

public:
    constexpr explicit OrderedHashMap(std::tuple<T, V> const (&raw)[Size]) noexcept : arr() {
        for (const auto &[i, item] : raw | std::views::enumerate) {
            auto [name, value] = item;
            auto hash = compiler::hash(name);
            arr[i] = {hash, value};
        }
        compiler::sort(arr, HashItemComparator());
    }

    constexpr std::optional<V> find(T str) const {
        auto hash = compiler::hash(str);
        const auto it = std::ranges::lower_bound(arr, std::make_tuple(hash, V{}), HashItemComparator());

        if (it != arr.end() && std::get<0>(*it) == hash) {
            return std::get<1>(*it);
        }

        return std::nullopt;
    }

    consteval V operator[](T str) const {
        auto v = find(str);
        if (v.has_value()) {
            return v.value();
        }
        static_assert(!std::is_same_v<V, V>, "Key not found");
        return V{};
    }
};

} // namespace compiler
