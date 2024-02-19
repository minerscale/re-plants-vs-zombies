#pragma once

namespace compiler {
template <decltype(auto) arr> static constexpr auto SparseArray() {
    using T = decltype(arr)::value_type::second_type;

    constexpr auto max_key = std::max_element(arr.begin(), arr.end(), [](const auto &left, const auto &right) {
                                 return left.first < right.first;
                             })->first;

    std::array<T, max_key + 1> sparse_array{};

    for (auto it : arr) {
        sparse_array[it.first] = it.second;
    }

    return sparse_array;
}
} // namespace compiler