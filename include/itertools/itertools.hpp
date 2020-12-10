#ifndef ITERTOOLS_H
#define ITERTOOLS_H

#include "itertools/range_container.hpp"
#include "itertools/tupletools.hpp"
#include "itertools/types.hpp"
#include "itertools/views/all.hpp"

#include <algorithm>
#include <chrono>
#include <functional>
#include <map>
#include <numeric>
#include <optional>
#include <sstream>
#include <vector>

#pragma once

namespace itertools {
/*
High order functor, whereof: applies a binary function of type BinaryFunction,
func, over an iterable range of type Iterable, iter. func must accept two
values of (size_t, IterableValue), wherein IterableValue is the
iterable's container value.

@param iter: iterable function of type Iterable.
@param func: low order functor of type BinaryFunction; returns a boolean.

@returns iter
 */
template<
    class Iterable,
    class BinaryFunction,
    class IterableValue = tupletools::iterable_t<Iterable>>
constexpr Iterable
for_each(Iterable&& iter, BinaryFunction&& func)
{
    for (auto&& [n, i] : views::enumerate(iter)) {
        std::invoke(
            std::forward<BinaryFunction>(func),
            std::forward<decltype(n)>(n),
            std::forward<decltype(i)>(i));
    }
    return iter;
}

template<
    class Iterable,
    class BinaryFunction,
    class IterableValue = tupletools::iterable_t<Iterable>>
constexpr Iterable
map(Iterable&& iter, BinaryFunction&& func)
{
    for (auto&& [n, i] : views::enumerate(iter)) {
        std::optional<IterableValue> opt = std::invoke(
            std::forward<BinaryFunction>(func),
            std::forward<decltype(n)>(n),
            std::forward<decltype(i)>(i));
        if (opt) {
            iter[n] = *opt;
        } else {
            break;
        }
    }
    return iter;
}

template<typename ReductionValue = int, class Iterable, class NaryFunction>
constexpr ReductionValue
reduce(Iterable&& iter, ReductionValue init, NaryFunction&& func)
{
    itertools::for_each(iter, [&](auto n, auto&& v) {
        init = func(n, std::forward<decltype(v)>(v), init);
        return false;
    });
    return init;
}

template<typename ReductionValue, class Iterable>
constexpr ReductionValue
sum(Iterable&& iter)
{
    return itertools::reduce<ReductionValue>(iter, 0, [](auto n, auto v, auto i) {
        return i + v;
    });
}

template<typename ReductionValue, class Iterable>
constexpr ReductionValue
mul(Iterable&& iter)
{
    return itertools::reduce<ReductionValue>(iter, 1, [](auto n, auto v, auto i) {
        return i * v;
    });
}

template<class Iterable>
constexpr Iterable
roll(Iterable&& iter, int axis = -1)
{
    if (axis == 0) {
        return iter;
    } else if (axis == -1) {
        axis = iter.size() - 1;
    }
    int i = 0;
    while (i < axis) {
        std::swap(iter[axis], iter[i]);
        i += 1;
    };
    return iter;
}

template<class Func>
struct y_combinator
{
    Func func;
    y_combinator(Func func)
      : func(std::move(func))
    {}
    template<class... Args>
    auto operator()(Args&&... args) const
    {
        return func(std::ref(*this), std::forward<Args>(args)...);
    }
};

template<class... Funcs, const size_t N = sizeof...(Funcs)>
auto
time_multiple(size_t iterations, Funcs&&... funcs)
{
    using namespace std::chrono;
    using integral_time_t = decltype(std::declval<microseconds>().count());

    std::map<int, std::vector<microseconds>> times;
    std::map<int, std::vector<integral_time_t>> extremal_times;

    for (auto i : views::range(N)) {
        times[i].reserve(iterations);
    }

    auto tup = std::make_tuple(std::forward<Funcs>(funcs)...);

    auto func = [&](auto&& n, auto&& v) {
        auto start = high_resolution_clock::now();
        v();
        auto stop = high_resolution_clock::now();
        auto time = duration_cast<microseconds>(stop - start);
        times[n].push_back(time);
        return false;
    };

    for (auto i : views::range(iterations)) {
        tupletools::for_each(tup, func);
    }

    itertools::for_each(times, [&](auto&& n, auto&& v) {
        auto [key, value] = v;
        std::sort(value.begin(), value.end());

        integral_time_t avg =
            itertools::reduce<integral_time_t>(
                value, 0, [](auto n, auto v, auto i) { return v.count() + i; }) /
            iterations;

        extremal_times[key] = {value[0].count(), value[value.size() - 1].count(), avg};
        return false;
    });
    return std::make_tuple(times, extremal_times);
}

namespace detail {

struct get_ndim_impl
{
    size_t _ndim = 0;

    template<
        class Iterable,
        std::enable_if_t<tupletools::is_iterable_v<Iterable>, int> = 0>
    constexpr void recurse(Iterable&& iter)
    {
        _ndim = 0;
        auto it = std::begin(iter);
        ++it;
        recurse(std::forward<decltype(*it)>(*it));
        _ndim++;
    }

    template<class Tup, std::enable_if_t<tupletools::is_tupleoid_v<Tup>, int> = 0>
    constexpr void recurse(Tup&& tup)
    {
        tupletools::for_each(tup, [&](auto&& n, auto&& v) {
            recurse(std::forward<decltype(v)>(v));
        });
    }

    template<
        class Iterable,
        std::enable_if_t<
            !tupletools::is_iterable_v<Iterable> &&
                !tupletools::is_tupleoid_v<Iterable>,
            int> = 0>
    constexpr void recurse(Iterable&& iter)
    {}

    template<class Iterable>
    constexpr size_t operator()(Iterable&& iter)
    {
        recurse(std::forward<Iterable>(iter));
        return _ndim + 1;
    }
};
}; // namespace detail

template<class Iterable>
constexpr size_t
get_ndim(Iterable&& iter)
{
    return detail::get_ndim_impl{}(std::forward<Iterable>(iter));
}

template<typename Iterable>
std::string
join(Iterable&& iter, std::string&& sep)
{
    std::ostringstream result;
    for (auto&& [n, i] : views::enumerate(iter)) {
        result << (n > 0 ? sep : "") << i;
    }
    return result.str();
}

std::string
ltrim(std::string s)
{
    auto end =
        std::find_if(s.begin(), s.end(), [](int ch) { return !std::isspace(ch); });
    s.erase(s.begin(), end);
    return s;
}

std::string
rtrim(std::string s)
{
    auto begin = std::find_if(s.rbegin(), s.rend(), [](int ch) {
                     return !std::isspace(ch);
                 }).base();
    s.erase(begin, s.end());
    return s;
}

std::string
trim(std::string s)
{
    ltrim(s);
    rtrim(s);
    return s;
}

};     // namespace itertools
#endif // ITERTOOLS_H
