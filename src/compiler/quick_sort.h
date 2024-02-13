#pragma once
#include <iterator>
#include <stdexcept>

// From https://tristanbrindle.com/posts/a-more-useful-compile-time-quicksort
namespace compiler {
namespace _sort {
    template <typename RAIt> constexpr RAIt _next(RAIt it, typename std::iterator_traits<RAIt>::difference_type n = 1) {
        return it + n;
    }

    template <typename RAIt> constexpr auto _distance(RAIt first, RAIt last) { return last - first; }

    template <class ForwardIt1, class ForwardIt2> constexpr void _iter_swap(ForwardIt1 a, ForwardIt2 b) {
        auto temp = std::move(*a);
        *a = std::move(*b);
        *b = std::move(temp);
    }

    template <class InputIt, class UnaryPredicate>
    constexpr InputIt _find_if_not(InputIt first, InputIt last, UnaryPredicate q) {
        for (; first != last; ++first) {
            if (!q(*first)) {
                return first;
            }
        }
        return last;
    }

    template <class ForwardIt, class UnaryPredicate>
    constexpr ForwardIt _partition(ForwardIt first, ForwardIt last, UnaryPredicate p) {
        first = _find_if_not(first, last, p);
        if (first == last) return first;

        for (ForwardIt i = _next(first); i != last; ++i) {
            if (p(*i)) {
                _iter_swap(i, first);
                ++first;
            }
        }
        return first;
    }

    template <class RAIt, class Compare = std::less<>>
    constexpr void quick_sort(RAIt first, RAIt last, Compare cmp = Compare{}) {
        auto const N = _distance(first, last);
        if (N <= 1) return;
        auto const pivot = *_next(first, N / 2);
        auto const middle1 = _partition(first, last, [=](auto const &elem) { return cmp(elem, pivot); });
        auto const middle2 = _partition(middle1, last, [=](auto const &elem) { return !cmp(pivot, elem); });
        quick_sort(first, middle1, cmp);
        quick_sort(middle2, last, cmp);
    }
} // namespace _sort

template <typename Range, typename Compare = std::less<>> constexpr auto sort(Range &&range, Compare cmp = Compare{}) {
    auto first = std::begin(range);
    auto last = std::end(range);
    _sort::quick_sort(first, last, cmp);
}
} // namespace compiler
