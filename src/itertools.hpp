#ifndef ITERTOOLS_H
#define ITERTOOLS_H

#include "generator.hpp"
#include "types.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <numeric>
#include <optional>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#pragma once

namespace tupletools {

/*
The actual tupletools:
 */

// The below three functions are drivers behind one's basic argument forwarding
// of a tuple.

template<size_t... Ixs, class Func>
constexpr auto
index_apply_impl(Func&& func, std::index_sequence<Ixs...>)
{
    return std::invoke(
        std::forward<Func>(func), std::integral_constant<size_t, Ixs>{}...);
}

template<size_t N, class Func>
constexpr auto
index_apply(Func&& func)
{
    return index_apply_impl(
        std::forward<Func>(func), std::make_index_sequence<N>{});
}

/*
Essentially a clone of std::apply; an academic exercise using expression
folding.
 */
template<class Tup, class Func, const size_t N = tuple_size<Tup>::value>
constexpr auto
apply(Func&& func, Tup&& tup)
{
    return index_apply<N>([&](auto... Ixs) {
        return std::invoke(
            std::forward<Func>(func), std::get<Ixs>(std::forward<Tup>(tup))...);
    });
}

/*
Begets a tuple of size N of type T, filled with value value.

@param value: fill value

@returns: tuple filled N times with value.
 */
template<const size_t N, class T>
constexpr auto
make_tuple_of(T value)
{
    auto func = [&value](auto t) { return value; };
    auto tup = index_apply<N>(
        [&value](auto... Ixs) { return std::make_tuple(Ixs...); });
    return index_apply<N>([&](auto... Ixs) {
        return std::make_tuple(func(std::get<Ixs>(tup))...);
    });
}

/*
High order functor that allows for a functor of type "Func" to be applied
element-wise to a tuple of type "Tup". "func" must accept two parameters of
types (size_t, tup_i).

Each item within "tup" is iterated upon, and must therefor each element must be
of a coalesceable type; the aforesaid elements must be of a type that allows
proper type coalescence.

@param tup: tuple of coalesceable type.
@param func:    high order function that takes two arguments of types
                (size_t,tup_i), wherein "tup_i" is a coalescably-typed element
                of "tup"

@returns: string representation of tup.
 */

template<class Tup, class Func, const size_t N = tuple_size_v<Tup>>
void
for_each(Tup&& tup, Func&& func)
{
    index_apply<N>([func = std::forward<Func>(func), &tup](auto... Ixs) {
        (func(
             Ixs,
             std::forward<decltype(std::get<Ixs>(tup))>(std::get<Ixs>(tup))),
         ...);
    });
    return;
}

template<class Tup, const size_t N = tuple_size<Tup>::value>
constexpr auto
reverse(Tup&& tup)
{
    return index_apply<N>([&tup](auto... Ixs) {
        return std::make_tuple(std::get<N - (Ixs + 1)>(tup)...);
    });
}

template<int Ix1, int Ix2, class Tup, const size_t N = tuple_size<Tup>::value>
constexpr auto
swap(Tup&& tup)
{
    auto tmp = std::get<Ix1>(tup);
    std::get<Ix1>(tup) = std::get<Ix2>(tup);
    std::get<Ix2>(tup) = tmp;
    return tup;
}

template<class Tup, const size_t N = tuple_size<Tup>::value>
constexpr auto
roll(Tup&& tup, bool reverse = false)
{
    if (reverse) {
        tupletools::for_each(tup, [&](const auto n, auto v) {
            swap<0, N - (n + 1)>(tup);
        });
    } else {

        tupletools::for_each(tup, [&](const auto n, auto v) {
            swap<n, N - 1>(tup);
        });
    }
    return tup;
}

template<
    class... Tuples,
    const size_t N = std::min({std::tuple_size<Tuples>{}...})>
constexpr auto
transpose(Tuples&&... tups)
{
    auto row = [&](auto Ixs) {
        return std::make_tuple(std::get<Ixs>(tups)...);
    };
    return index_apply<N>(
        [&](auto... Ixs) { return std::make_tuple(row(Ixs)...); });
}

template<class Func, class Tup>
constexpr auto
transform(Func&& func, Tup&& tup)
{
    auto f = [func = std::forward<Func>(func)](auto&&... args) {
        return std::forward_as_tuple(
            func(std::forward<decltype(args)>(args))...);
    };
    return tupletools::apply(f, std::forward<Tup>(tup));
}

template<class Tup, const size_t N = tuple_size<Tup>::value>
constexpr auto
deref(Tup&& tup)
{
    auto func = [](auto&& x) -> decltype(*x) { return *x; };
    return transform(func, tup);
}

template<class Tup>
constexpr auto
increment_ref(Tup&& tup)
{
    return tupletools::for_each(std::forward<Tup>(tup), [](auto n, auto&& v) {
        ++v;
    });
}

template<
    class Pred,
    class Tup1,
    class Tup2,
    const size_t N = tupletools::tuple_size_v<Tup1>,
    const size_t M = tupletools::tuple_size_v<Tup2>>
constexpr auto
where(Pred&& pred, Tup1&& tup1, Tup2&& tup2)
{
    static_assert(N == M, "Tuples must be the same size!");
    return index_apply<N>([&](auto... Ixs) {
        auto tup = std::make_tuple(std::invoke(
            std::forward<Pred>(pred),
            std::get<Ixs>(std::forward<Tup1>(tup1)),
            std::get<Ixs>(std::forward<Tup2>(tup2)))...);
        return tup;
    });
}

template<
    class Pred,
    class Tup1,
    class Tup2,
    const size_t N = tupletools::tuple_size_v<Tup1>,
    const size_t M = tupletools::tuple_size_v<Tup2>>
constexpr bool
any_where(Pred&& pred, Tup1&& tup1, Tup2&& tup2)
{
    static_assert(N == M, "Tuples must be the same size!");
    return index_apply<N>([&](auto... Ixs) {
        return (
            (std::invoke(
                std::forward<Pred>(pred),
                std::get<Ixs>(std::forward<Tup1>(tup1)),
                std::get<Ixs>(std::forward<Tup2>(tup2)))) ||
            ...);
    });
}

template<
    class Pred,
    class Tup1,
    class Tup2,
    const size_t N = tupletools::tuple_size_v<Tup1>,
    const size_t M = tupletools::tuple_size_v<Tup2>>
constexpr bool
all_where(Pred&& pred, Tup1&& tup1, Tup2&& tup2)
{
    static_assert(N == M, "Tuples must be the same size!");
    return index_apply<N>([&](auto... Ixs) {
        return (
            (std::invoke(
                std::forward<Pred>(pred),
                std::get<Ixs>(std::forward<Tup1>(tup1)),
                std::get<Ixs>(std::forward<Tup2>(tup2)))) &&
            ...);
    });
}

/*
Returns the string format representation of a coalesceably-typed tuple;
The elements of aforesaid tuple must be of a type that allows proper
type coalescence.

@param tup: tuple of coalesceable type.
@returns: string representation of tup.
 */
template<typename Tup, const size_t N = tuple_size<Tup>::value>
std::string
to_string(Tup&& tup)
{
    std::string s = "(";
    tupletools::
        for_each(std::forward<Tup>(tup), [&tup, &s](auto&& n, auto&& v) {
            s += std::to_string(v);
            s += n < N - 1 ? ", " : "";
            return false;
        });
    return s + ")";
}

template<class Tup>
constexpr bool
any_of(Tup&& tup)
{
    bool b = false;
    auto func = [&](auto&& n, auto&& v) -> bool {
        if (v) {
            b = true;
            return true;
        }
        return false;
    };
    tupletools::
        for_each(std::forward<Tup>(tup), std::forward<decltype(func)>(func));
    return b;
}

template<class Tup>
constexpr bool
all_of(Tup&& tup)
{
    bool b = true;
    auto func = [&](auto&& n, auto&& v) -> bool {
        if (!v) {
            b = false;
            return true;
        }
        return false;
    };
    tupletools::
        for_each(std::forward<Tup>(tup), std::forward<decltype(func)>(func));
    return b;
}

template<class Tup>
constexpr bool
disjunction_of(Tup&& tup)
{
    bool b = true;
    bool prev_b = true;
    auto func = [&](auto&& n, auto&& v) -> bool {
        if (!prev_b && v) {
            b = false;
            return true;
        } else {
            prev_b = v;
            return false;
        }
    };
    tupletools::
        for_each(std::forward<Tup>(tup), std::forward<decltype(func)>(func));
    return b;
}

template<class Tup, std::enable_if_t<!is_tuple_v<Tup>, int> = 0>
constexpr auto
make_tuple_if(Tup&& tup)
{
    return std::make_tuple(tup);
}

template<class Tup, std::enable_if_t<is_tuple_v<Tup>, int> = 0>
constexpr auto
make_tuple_if(Tup&& tup)
{
    return tup;
}

template<class... T>
struct flatten_impl
{};

template<class T>
struct flatten_impl<T>
{
    template<class U>
    constexpr auto operator()(U&& value)
    {
        return std::forward<U>(value);
    }
};

template<class T>
struct flatten_impl<std::tuple<T>>
{
    template<class Tup>
    constexpr auto operator()(Tup&& tup)
    {
        return flatten_impl<tupletools::remove_cvref_t<T>>{}(std::get<0>(tup));
    }
};

template<class T, class... Ts>
struct flatten_impl<std::tuple<T, Ts...>>
{
    template<
        class Tup,
        const size_t N = sizeof...(Ts),
        std::enable_if_t<(N >= 1), int> = 0>
    constexpr auto operator()(Tup&& tup)
    {
        auto tup_first =
            flatten_impl<tupletools::remove_cvref_t<T>>{}(std::get<0>(tup));

        auto t_tup_args = index_apply<N>([&tup](auto... Ixs) {
            return std::make_tuple(std::get<Ixs + 1>(tup)...);
        });
        auto tup_args =
            flatten_impl<tupletools::remove_cvref_t<decltype(t_tup_args)>>{}(
                t_tup_args);

        return std::
            tuple_cat(make_tuple_if(tup_first), make_tuple_if(tup_args));
    }
};

template<class Tup>
constexpr auto
flatten(Tup&& tup)
{
    return flatten_impl<tupletools::remove_cvref_t<Tup>>{}(
        std::forward<Tup>(tup));
}
}; // namespace tupletools

namespace itertools {

struct range_container_terminus
{
    bool complete = false;
};

template<class Range, class Iterator, class... Args>
class range_container
{
public:
    range_container(Range&& rng, Args&&... args)
      : iter{std::forward<Range>(rng), std::forward<Args>(args)...}
      , range{rng}
    {}

    Iterator begin()
    {
        return iter;
    }

    auto end()
    {
        return range_container_terminus{true};
    }

    Range range;
    Iterator iter;
};

using namespace tupletools;

template<
    class Range,
    class BeginIter = decltype(std::declval<Range>().begin()),
    class EndIter = decltype(std::declval<Range>().end())>
class range_container_iterator
{
public:
    using iterator_category = std::forward_iterator_tag;
    range_container_terminus terminus;

    explicit range_container_iterator(Range&& rng) noexcept
      : begin_iter{std::begin(rng)}
      , end_iter{std::end(rng)}
      , terminus{false}
    {}

    explicit range_container_iterator(
        Range&& rng,
        BeginIter&& begin_iter,
        EndIter&& end_iter) noexcept
      : begin_iter{begin_iter}
      , end_iter{end_iter}
      , terminus{false}
    {}

    bool is_complete()
    {
        auto b = this->begin_iter == this->end_iter;
        this->terminus.complete = b;
        return b;
    }

    auto operator++()
    {
        ++this->begin_iter;
        this->is_complete();
        return *this;
    }

    bool operator==(range_container_terminus rhs)
    {
        return this->terminus.complete == rhs.complete;
    }

    bool operator!=(range_container_terminus rhs)
    {
        return this->terminus.complete != rhs.complete;
    }

    auto& operator*()
    {
        return *(this->begin_iter);
    }

    auto operator-> () noexcept
    {
        return this->begin_iter;
    }

    BeginIter begin_iter;
    EndIter end_iter;
};

template<class Range>
class block_iterator : public range_container_iterator<Range>
{
public:
    using IteratorValue = tupletools::iterable_t<Range>;
    std::vector<IteratorValue> ret;

    block_iterator(Range&& rng, size_t block_size) noexcept
      : range_container_iterator<Range>(std::forward<Range>(rng))
    {
        this->block_size = block_size;
        ret.reserve(block_size);
    }

    auto operator++()
    {
        auto i = 0;
        while (!this->is_complete() && i++ < this->block_size) {
            ret.pop_back();
        }
        return *this;
    }

    auto operator*() noexcept
    {
        auto i = 0;
        while (!this->is_complete() && i++ < this->block_size) {
            ret.push_back(*(this->begin_iter));
            ++(this->begin_iter);
        }
        return ret;
    }

    size_t block_size;
};

template<class Func, class Range>
class filter_iterator : public range_container_iterator<Range>
{
public:
    filter_iterator(Range&& rng, Func&& func) noexcept
      : range_container_iterator<Range>(std::forward<Range>(rng))
      , func{func}
    {}

    auto operator++() noexcept
    {
        do {
            if (this->is_complete()) {
                break;
            } else {
                ++(this->begin_iter);
            }
        } while (!this->func(*(this->begin_iter)));

        return *this;
    }

    Func func;
};

template<class Func, class Range>
class transmog_iterator : public range_container_iterator<Range>
{
public:
    transmog_iterator(Range&& rng, Func&& func) noexcept
      : range_container_iterator<Range>(std::forward<Range>(rng))
      , func{func}
    {}

    auto operator*() noexcept
    {
        return func(
            std::forward<decltype(*(this->begin_iter))>(*(this->begin_iter)));
    }
    Func func;
};

template<class Func, class Range>
constexpr auto
transmog(Func&& func, Range&& rng)
{
    using Iterator = transmog_iterator<Func, Range>;
    return range_container<
        Range,
        Iterator,
        Func>(std::forward<Range>(rng), std::forward<Func>(func));
}

template<class Func, class Range>
constexpr auto
filter(Func&& func, Range&& rng)
{
    using Iterator = filter_iterator<Func, Range>;
    return range_container<
        Range,
        Iterator,
        Func>(std::forward<Range>(rng), std::forward<Func>(func));
}

template<class Range, class F = size_t>
constexpr auto
block(Range&& rng, F block_size)
{
    using Iterator = block_iterator<Range>;
    return range_container<
        Range,
        Iterator,
        size_t>(std::forward<Range>(rng), block_size);
}

template<template<typename... Ts> class Iterator, class... Args>
class range_tuple
{
    static constexpr size_t N = sizeof...(Args);
    static_assert(N > 0, "Zip argument count must be larger than 0.");

public:
    using Range = std::tuple<Args...>;
    using BeginIter = std::tuple<decltype(std::declval<Args>().begin())...>;
    using EndIter = std::tuple<decltype(std::declval<Args>().end())...>;

    using iterator = Iterator<Range, BeginIter, EndIter>;

    range_tuple(Args&&... args)
      : iter{std::forward_as_tuple(args...),
             std::forward_as_tuple(std::begin(args)...),
             std::forward_as_tuple(std::end(args)...)}
      , range{args...}
    {}

    iterator begin()
    {
        return iter;
    }

    auto end()
    {
        return range_container_terminus{true};
    }

    Range range;
    iterator iter;
};

template<class Range, class BeginIter, class EndIter>
class range_tuple_iterator
  : public range_container_iterator<Range, BeginIter, EndIter>
{
public:
    range_tuple_iterator(
        Range&& rng,
        BeginIter&& begin_iter,
        EndIter&& end_iter) noexcept
      : range_container_iterator<Range, BeginIter, EndIter>(
            std::forward<Range>(rng),
            std::forward<BeginIter>(begin_iter),
            std::forward<EndIter>(end_iter))
    {}

    bool is_complete()
    {
        auto b = tupletools::any_where(
            [](auto&& x, auto&& y) { return x == y; },
            this->begin_iter,
            this->end_iter);
        this->terminus.complete = b;
        return b;
    }

    auto operator++()
    {
        tupletools::increment_ref(this->begin_iter);
        this->is_complete();
        return *this;
    }

    auto operator*() noexcept
    {
        return tupletools::deref(this->begin_iter);
    }
};

template<class Range, class BeginIter, class EndIter>
class concat_iterator : public range_tuple_iterator<Range, BeginIter, EndIter>
{
public:
    concat_iterator(
        Range&& rng,
        BeginIter&& begin_iter,
        EndIter&& end_iter) noexcept
      : range_tuple_iterator<Range, BeginIter, EndIter>(
            std::forward<Range>(rng),
            std::forward<BeginIter>(begin_iter),
            std::forward<EndIter>(end_iter))
    {}

    bool is_complete()
    {
        auto b = std::get<0>(this->begin_iter) == std::get<0>(this->end_iter);

        if (b) {
            tupletools::roll(this->begin_iter, true);
            tupletools::roll(this->end_iter, true);

            b = std::get<0>(this->begin_iter) == std::get<0>(this->end_iter);
        }
        this->terminus.complete = b;
        return b;
    }

    auto operator++()
    {
        ++std::get<0>(this->begin_iter);
        this->is_complete();
        return *this;
    }

    auto& operator*() noexcept
    {
        return *std::get<0>(this->begin_iter);
    }
};

template<class... Args>
constexpr auto
zip(Args&&... args)
{
    return range_tuple<range_tuple_iterator, Args...>(
        std::forward<Args>(args)...);
}

template<class T, class... Args>
constexpr auto
concat(T&& arg, Args&&... args)
{
    static_assert(
        tupletools::is_all<T, Args...>::value,
        "All arguments must be of a homogenous type.");

    return range_tuple<
        concat_iterator,
        T,
        Args...>(std::forward<T>(arg), std::forward<Args>(args)...);
}

/*

Coroutine generator function. Works with both negative and positive stride
values.

@param start: T starting value.
@param stop: T stopping value.
@param stride: T stride value whereof start is incremented.

@co_yields: incremented starting value of type T.
 */
template<typename T = size_t>
generator<T>
grange(T start, T stop, T stride = 1)
{
    stride = start > stop ? -1 : 1;
    do {
        co_yield start;
        start += stride;
    } while (start < stop);
}

template<typename T = size_t>
generator<T>
grange(T stop)
{
    T start = 0;
    if (start > stop) {
        std::swap(start, stop);
    }
    return grange<T>(start, stop);
}

template<typename T = size_t>
class range;

template<typename T = size_t>
class range_iterator : public std::iterator<std::forward_iterator_tag, T>
{
public:
    using iterator_category = std::forward_iterator_tag;

    constexpr explicit range_iterator(range<T>& seq)
      : _seq(seq)
    {}

    constexpr auto operator++()
    {
        _seq._current += _seq._stride;
        return *this;
    }

    constexpr bool operator==(T rhs)
    {
        return _seq._current == rhs;
    }

    constexpr bool operator!=(T rhs)
    {
        return !(*this == rhs);
    }

    constexpr const T& operator*() noexcept
    {
        return _seq._current;
    }

private:
    range<T> _seq;
};

template<typename T>
class range
{
public:
    using iterator = range_iterator<T>;

    constexpr explicit range(T stop)
      : _start(0)
      , _stop(stop)
    {
        if (_start > _stop) {
            std::swap(_start, _stop);
        }
        _stride = _start > _stop ? -1 : 1;
        _current = _start;
    }

    constexpr range(T start, T stop)
      : _start(start)
      , _stop(stop)
    {
        _stride = _start > _stop ? -1 : 1;
        _current = _start;
    }

    constexpr range(T start, T stop, T stride)
      : _start(start)
      , _stop(stop)
      , _stride(stride)
      , _current(_start)
    {}

    T start()
    {
        return _start;
    }
    T stop()
    {
        return _stop;
    }
    T stride()
    {
        return _stride;
    }
    T& current()
    {
        return _current;
    }
    size_t size() const
    {
        return _size;
    }

    iterator begin()
    {
        return iterator(*this);
    }
    T end()
    {
        return _stop;
    }

private:
    friend class range_iterator<T>;
    T _start, _stop, _stride, _current;
    size_t _size = 0;
};

template<class Iterable>
constexpr auto
enumerate(Iterable&& iter)
{
    auto _range = range<size_t>(iter.size());
    return zip(
        std::forward<decltype(_range)>(_range), std::forward<Iterable>(iter));
}

/*
High order functor, whereof: applies a binary function of type BinaryFunction,
func, over an iterable range of type Iterable, iter. func must accept two
values of (size_t, IterableValue), wherein IterableValue is the
iterable's container value. func must also return a boolean value upon each
iteration: returning true, subsequently forcing a break in the loop, or false,
to continue onward.

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
    for (auto&& [n, i] : enumerate(iter)) {
        std::invoke(
            std::forward<BinaryFunction>(func),
            std::forward<decltype(n)>(n),
            std::forward<decltype(i)>(i));
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
    return itertools::reduce<
        ReductionValue>(iter, 0, [](auto n, auto v, auto i) { return i + v; });
}

template<typename ReductionValue, class Iterable>
constexpr ReductionValue
mul(Iterable&& iter)
{
    return itertools::reduce<
        ReductionValue>(iter, 1, [](auto n, auto v, auto i) { return i * v; });
}

template<class Iterable>
constexpr Iterable
swap(Iterable&& iter, int ix1, int ix2)
{
    assert(ix1 < iter.size() && ix2 < iter.size());
    auto t = iter[ix1];
    iter[ix1] = iter[ix2];
    iter[ix2] = t;
    return iter;
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
        itertools::swap(std::forward<Iterable>(iter), axis, i++);
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
    template<class... Args>
    auto operator()(Args&&... args)
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

    for (auto i : range(N)) {
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

    for (auto i : range(iterations)) {
        tupletools::for_each(tup, func);
    }

    itertools::for_each(times, [&](auto&& n, auto&& v) {
        auto [key, value] = v;
        std::sort(value.begin(), value.end());

        integral_time_t avg =
            itertools::reduce<integral_time_t>(
                value,
                0,
                [](auto n, auto v, auto i) { return v.count() + i; }) /
            iterations;

        extremal_times[key] = {value[0].count(),
                               value[value.size() - 1].count(),
                               avg};
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
        std::advance(it, 1);
        recurse(std::forward<decltype(*it)>(*it));
        _ndim++;
    }

    template<
        class Tup,
        std::enable_if_t<tupletools::is_tupleoid_v<Tup>, int> = 0>
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

template<typename Char, typename Traits, typename Allocator>
std::basic_string<Char, Traits, Allocator> operator*(
    const std::basic_string<Char, Traits, Allocator> s,
    size_t n)
{
    std::basic_string<Char, Traits, Allocator> tmp = "";
    for (auto i : range(n)) {
        tmp += s;
    }
    return tmp;
}

template<typename Char, typename Traits, typename Allocator>
std::basic_string<Char, Traits, Allocator> operator*(
    size_t n,
    const std::basic_string<Char, Traits, Allocator>& s)
{
    return s * n;
}

template<typename Iterable>
std::string
join(Iterable&& iter, std::string&& sep)
{
    std::ostringstream result;
    for (auto&& [n, i] : enumerate(iter)) {
        result << (n > 0 ? sep : "") << i;
    }
    return result.str();
}

std::string
ltrim(std::string s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
                return !std::isspace(ch);
            }));
    return s;
}

std::string
rtrim(std::string s)
{
    s.erase(
        std::find_if(
            s.rbegin(), s.rend(), [](int ch) { return !std::isspace(ch); })
            .base(),
        s.end());
    return s;
}

std::string
trim(std::string s)
{
    ltrim(s);
    rtrim(s);
    return s;
}

namespace detail {

constexpr int
bidirectional_match(
    const std::string& buff,
    const std::string& token,
    size_t pos = 0,
    bool backwards = false)
{
    int neg = 1;
    int end = buff.size();
    int token_begin = 0;
    int token_end = token.size() - 1;
    int t_begin = 0;

    if (backwards) {
        neg = -1;
        end = 0;
        token_begin = token.size() - 1;
        token_end = -1;
    }
    t_begin = token_begin;

    while (pos != end) {
        if (buff[pos] == token[t_begin]) {
            t_begin += neg;
        } else {
            t_begin = token_begin;
        }
        if (t_begin == token_end) {
            return pos;
        }
        pos += neg;
    }
    return -1;
}

std::string
summarize_string(
    const std::string& buff,
    const std::string& sep,
    size_t max_items = 3,
    size_t max_len = 80)
{
    if (buff.size() < max_len) {
        return buff;
    }
    int pos = 0;
    size_t max_len2 = max_len / 2;
    size_t i = 0;
    size_t begin = 0;
    size_t end = buff.size() - 1;
    while ((i++ < max_items) && (begin < max_len2)) {
        pos = bidirectional_match(buff, sep, pos, false);
        if (pos == -1) {
            break;
        } else {
            pos += sep.size() - 1;
            begin = pos;
        }
    }
    i = 0;
    pos = buff.size() - 1;
    while ((i++ < max_items) && (buff.size() - (end + 1) < max_len2)) {
        pos = bidirectional_match(buff, sep, pos, true);
        if (pos == -1) {
            break;
        } else {
            pos -= sep.size();
            end = pos;
        }
    }
    if (begin == 0 || end == buff.size() - 1) {
        return buff;
    } else {
        std::string summary = buff.substr(0, begin - 1) + " ... " +
                              buff.substr(end, buff.size() - 1);
        return summary;
    }
}

template<class Formatter>
struct to_string_impl
{
    Formatter _formatter;
    size_t _prev_ix, _prev_len, _ndim, _max_len = 80;
    std::string _sep, _trim_sep;

    template<class Iterable>
    explicit constexpr to_string_impl(
        Iterable&& iter,
        Formatter&& formatter,
        std::string& sep)

      : _formatter{formatter}
      , _sep{sep}
      , _trim_sep{trim(sep)}
    {
        _ndim = get_ndim(iter);
    };

    template<class Iterable>
    std::string operator()(Iterable&& iter)
    {
        return recurse(std::forward<Iterable>(iter), 0);
    }

    constexpr size_t get_lastline_size(std::string& buff)
    {
        size_t line_size = 0;
        int i = buff.size() - 1;
        while (i-- > 0) {
            if (buff[i] == '\n') {
                return line_size;
            } else {
                line_size++;
            }
        }
        return line_size;
    }

    std::string format_newline(std::string& buff, size_t line_count)
    {
        std::string new_line =
            std::string("\n") * (line_count < 0 ? 0 : line_count);
        std::string spacing = _trim_sep + new_line;
        buff += spacing;
        _prev_len = spacing.size();
        return buff;
    }

    std::string format_indent(std::string& buff)
    {
        int i = 0;
        int N = buff.size();
        while (i++ < N) {
            if (i < N - 1) {
                if (buff[i] == '\n' && buff[i + 1] != '\n') {
                    buff.insert(i + 1, " ");
                    N++;
                }
            }
        }
        return buff;
    }

    template<
        class Tup,
        std::enable_if_t<tupletools::is_tupleoid_v<Tup>, int> = 0,
        const size_t N = tupletools::tuple_size_v<Tup>>
    std::string formatter_wrap(Tup&& tup, size_t ix)
    {
        std::string buff;
        std::string t_buff;
        bool nested_tupleoid = false;

        tupletools::
            for_each(std::forward<Tup>(tup), [&](auto n, auto&& iter_n) {
                t_buff = to_string_impl{std::forward<decltype(iter_n)>(iter_n),
                                        std::forward<Formatter>(_formatter),
                                        _sep}(
                    std::forward<decltype(iter_n)>(iter_n));
                if (n < N - 1) {
                    t_buff += _sep;
                    if (nested_tupleoid || is_iterable_v<decltype(iter_n)> ||
                        is_tupleoid_v<decltype(iter_n)>) {
                        t_buff += '\n';
                        nested_tupleoid = true;
                    }
                }
                buff += t_buff;
            });

        buff = "(" + format_indent(buff) + ")";
        if (ix > 0) {
            format_newline(buff, 2);
            _prev_ix = ix + 1;
        }
        return buff;
    }

    template<
        class Iterable,
        std::enable_if_t<!tupletools::is_tupleoid_v<Iterable>, int> = 0>
    std::string formatter_wrap(Iterable&& iter, size_t ix)
    {
        return std::invoke(
            std::forward<Formatter>(_formatter), std::forward<Iterable>(iter));
    }

    template<
        class Iterable,
        std::enable_if_t<!tupletools::is_iterable_v<Iterable>, int> = 0>
    std::string recurse(Iterable&& iter, size_t ix)
    {
        return formatter_wrap(std::forward<Iterable>(iter), ix);
    }

    template<
        class Iterable,
        std::enable_if_t<tupletools::is_iterable_v<Iterable>, int> = 0>
    std::string recurse(Iterable&& iter, size_t ix)
    {
        _prev_ix = ix;
        std::string buff = "";
        size_t ndim = iter.size();

        for (auto [n, iter_n] : itertools::enumerate(iter)) {
            buff += recurse(std::forward<decltype(iter_n)>(iter_n), ix + 1);
            if (!is_iterable_v<decltype(iter_n)> &&
                !is_tupleoid_v<decltype(iter_n)>) {
                buff += n < ndim - 1 ? _sep : "";
            }
        };

        buff = _prev_ix > ix ? buff.substr(0, buff.size() - _prev_len) : buff;

        buff = "[" + format_indent(buff) + "]";
        if (ix > 0) {
            format_newline(buff, _ndim - ix);
            buff = summarize_string(buff, _sep, 80);
        }
        return buff;
    };
};
}; // namespace detail

template<class Iterable, class Formatter>
std::string
to_string_f(Iterable&& iter, Formatter&& formatter, std::string&& sep = ", ")
{
    return detail::to_string_impl{std::forward<Iterable>(iter),
                                  std::forward<Formatter>(formatter),
                                  sep}(iter);
}

template<class Iterable>
std::string
to_string(Iterable&& iter)
{
    std::string sep = ", ";
    auto formatter = [](auto&& s) -> std::string {
        return std::string(fmt::format("{}", s));
    };
    return detail::to_string_impl{std::forward<Iterable>(iter),
                                  std::forward<decltype(formatter)>(formatter),
                                  sep}(iter);
}
};     // namespace itertools
#endif // ITERTOOLS_H
