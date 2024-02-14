#pragma once
#include "hash.h"
#include <algorithm>

namespace compiler {
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
        for (size_t i = 0; i < Size; ++i) {
            auto [name, value] = raw[i];
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