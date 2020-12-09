#ifndef RANGE_CONTAINER_H
#define RANGE_CONTAINER_H

#include "tupletools.hpp"
#include "types.hpp"

#include <numeric>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>

#pragma once

namespace itertools {
using namespace tupletools;

template<class Iterable, class IterableValue = tupletools::iterable_t<Iterable>>
constexpr std::vector<IterableValue>
to_vector(Iterable&& it)
{
    std::vector<IterableValue> vec;
    for (auto&& v : it) {
        vec.push_back(v);
    }
    return vec;
}

struct range_container_terminus
{
    bool complete = false;
};

template<
    class Range,
    class BeginIt = decltype(std::declval<Range>().begin()),
    class EndIt = decltype(std::declval<Range>().end())>
class range_container_iterator
{
public:
    using iterator_category = std::forward_iterator_tag;
    // This may need to change to tuple.
    using iterator_value = tupletools::iterator_t<BeginIt>;

    range_container_terminus terminus;

    range_container_iterator(Range&& range)
      : range_container_iterator{
            std::forward<Range>(range),
            std::begin(range),
            std::end(range)}
    {}

    range_container_iterator(Range&& range, BeginIt&& begin_it, EndIt&& end_it)
      : begin_it{begin_it}
      , end_it{end_it}
      , terminus{false}
    {}

    bool is_complete()
    {
        auto complete = this->begin_it == this->end_it;
        this->terminus.complete = complete;

        return complete;
    }

    auto operator++()
    {
        ++this->begin_it;
        this->is_complete();
        return *this;
    }

    bool operator==(range_container_terminus rhs)
    {
        return this->terminus.complete == rhs.complete;
    }

    auto operator*()
    {
        return *(this->begin_it);
    }

    auto operator->() noexcept
    {
        return this->begin_it;
    }

    BeginIt begin_it;
    EndIt end_it;
};

template<class Range, class BeginIt, class EndIt>
class range_tuple_iterator : public range_container_iterator<Range, BeginIt, EndIt>
{
public:
    range_tuple_iterator(Range&& range, BeginIt&& begin_it, EndIt&& end_it)
      : range_container_iterator<Range, BeginIt, EndIt>(
            std::forward<Range>(range),
            std::forward<BeginIt>(begin_it),
            std::forward<EndIt>(end_it))
    {}

    bool is_complete()
    {
        auto complete = tupletools::any_where(
            [](auto&& x, auto&& y) { return x == y; }, this->begin_it, this->end_it);
        this->terminus.complete = complete;
        return complete;
    }

    auto operator++()
    {
        tupletools::for_each(this->begin_it, [](auto&& n, auto&& v) { ++v; });
        this->is_complete();
        return *this;
    }

    auto operator*()
    {
        auto func = [](auto&& v) -> decltype(*v) { return *v; };
        return tupletools::transform(func, this->begin_it);
    }
};

template<class Range, class Iterator>
class range_container
{
public:
    size_t size_;

    range_container(Range&& range, Iterator&& it)
      : it{std::forward<Iterator>(it)}
      , range{range}
      , size_{1}
    {}

    auto begin()
    {
        return it;
    }

    auto end()
    {
        return range_container_terminus{true};
    }

    size_t size()
    {
        return size_;
    }

    template<class Func>
    auto operator|(Func&& func)
    {
        return func(*this);
    }

    Iterator it;
    Range range;
};

template<class Range>
class block_iterator : public range_container_iterator<Range>
{
public:
    using IteratorValue = tupletools::iterable_t<Range>;
    std::vector<IteratorValue> ret;

    block_iterator(Range&& range, size_t block_size) noexcept
      : range_container_iterator<Range>(std::forward<Range>(range))
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
            ret.push_back(*(this->begin_it));
            ++(this->begin_it);
        }

        return ret;
    }

    size_t block_size;
};

template<class Pred, class Range>
class filter_iterator : public range_container_iterator<Range>
{
public:
    using IterableValue = tupletools::iterable_t<Range>;

    filter_iterator(Pred&& pred, Range&& range) noexcept
      : range_container_iterator<Range>(std::forward<Range>(range))
      , pred{std::forward<Pred>(pred)}
    {}

    auto operator++()
    {
        ++(this->begin_it);

        while (!this->is_complete() && !this->pred(*this->begin_it)) {
            ++(this->begin_it);
        };

        return *this;
    }

    auto operator*()
    {
        auto to_filter = !this->pred(*this->begin_it);

        if (to_filter) {
            ++(*this);
        }

        return *(this->begin_it);
    }

    Pred pred;
};

template<class Func, class Range>
class transform_iterator : public range_container_iterator<Range>
{
public:
    transform_iterator(Func&& func, Range&& range) noexcept
      : range_container_iterator<Range>(std::forward<Range>(range))
      , func{std::forward<Func>(func)}
    {}

    auto operator*() noexcept
    {
        return std::invoke(
            std::forward<Func>(func),
            std::forward<decltype(this->begin_it)>(this->begin_it));
    }

    Func func;
};

template<typename Range>
constexpr auto
to_range(Range&& range)
{
    auto it = range_container_iterator<Range>(range);
    using Iterator = decltype(it);

    return range_container<
        Range,
        Iterator>(std::forward<Range>(range), std::forward<Iterator>(it));
}

template<class Func, class Range>
constexpr auto
transform(Func&& func, Range&& range)
{
    auto it = transform_iterator(std::forward<Func>(func), std::forward<Range>(range));
    using Iterator = decltype(it);

    return range_container<
        Range,
        Iterator>(std::forward<Range>(range), std::forward<Iterator>(it));
};

template<class Func, class Range>
constexpr auto
filter(Func&& func, Range&& range)
{
    auto it = filter_iterator(std::forward<Func>(func), std::forward<Range>(range));
    using Iterator = decltype(it);

    return range_container<
        Range,
        Iterator>(std::forward<Range>(range), std::forward<Iterator>(it));
};

template<class Func, class Range>
constexpr auto
block(Range&& range, size_t block_size)
{
    auto it = block_iterator(std::forward<Range>(range), block_size);
    using Iterator = decltype(it);

    return range_container<
        Range,
        Iterator>(std::forward<Range>(range), std::forward<Iterator>(it));
};

template<class Range, class BeginIt, class EndIt>
class concat_iterator : public range_tuple_iterator<Range, BeginIt, EndIt>
{
public:
    concat_iterator(Range&& range, BeginIt&& begin_it, EndIt&& end_it)
      : range_tuple_iterator<Range, BeginIt, EndIt>(
            std::forward<Range>(range),
            std::forward<BeginIt>(begin_it),
            std::forward<EndIt>(end_it))
    {}

    bool is_complete()
    {
        auto complete = std::get<0>(this->begin_it) == std::get<0>(this->end_it);

        if (complete) {
            tupletools::roll(this->begin_it, true);
            tupletools::roll(this->end_it, true);

            complete = std::get<0>(this->begin_it) == std::get<0>(this->end_it);
        }

        this->terminus.complete = complete;
        return complete;
    }

    auto operator++() -> decltype(auto)
    {
        ++std::get<0>(this->begin_it);
        this->is_complete();
        return *this;
    }

    auto operator*() -> decltype(auto)
    {
        return *std::get<0>(this->begin_it);
    }
};

template<template<typename... Ts> class Iterator, class... Args>
auto
make_tuple_iterator(Args&&... args)
{
    auto range = std::forward_as_tuple(args...);
    auto begin_it = std::make_tuple(args.begin()...);
    auto end_it = std::make_tuple(args.end()...);

    using Range = decltype(range);
    using BeginIt = decltype(begin_it);
    using EndIt = decltype(end_it);

    auto it = Iterator<Range, BeginIt, EndIt>(
        std::forward<Range>(range),
        std::forward<BeginIt>(begin_it),
        std::forward<EndIt>(end_it));

    using TupleIt = decltype(it);

    return range_container<
        Range,
        TupleIt>(std::forward<Range>(range), std::forward<TupleIt>(it));
}

template<class... Args>
constexpr auto
zip(Args&&... args)
{
    return make_tuple_iterator<range_tuple_iterator, Args...>(
        std::forward<Args>(args)...);
}

template<class T, class... Args>
constexpr auto
concat(T&& arg, Args&&... args)
{
    static_assert(
        tupletools::is_all<T, Args...>::value,
        "All arguments must be of a homogenous type.");

    return make_tuple_iterator<
        concat_iterator,
        T,
        Args...>(std::forward<T>(arg), std::forward<Args>(args)...);
}

// end variadic container iterators.

template<typename T = int>
class range;

template<typename T = int>
class range_iterator
{
public:
    explicit range_iterator(range<T>& seq)
      : seq(seq)
    {}

    auto operator++()
    {
        seq.current += seq.stride;
        return *this;
    }

    bool operator==(T rhs)
    {
        return seq.current == rhs;
    }

    const T& operator*()
    {
        return seq.current;
    }

private:
    range<T> seq;
};

template<typename T>
class range
{
public:
    using iterator = range_iterator<T>;

    constexpr explicit range(T stop)
      : start(0)
      , stop(stop)
    {
        if (start > stop) {
            std::swap(start, stop);
        }
        stride = start > stop ? -1 : 1;
        current = start;
    }

    constexpr range(T start, T stop)
      : start(start)
      , stop(stop)
    {
        stride = start > stop ? -1 : 1;
        current = start;
    }

    constexpr range(T start, T stop, T stride)
      : start(start)
      , stop(stop)
      , stride(stride)
      , current(start)
    {}

    int size() const
    {
        return size_;
    }

    iterator begin()
    {
        return iterator(*this);
    }

    T end()
    {
        return stop;
    }

private:
    friend class range_iterator<T>;
    T start, stop, stride, current;
    size_t size_ = 0;
};

template<class Iterable>
constexpr auto
enumerate(Iterable&& it)
{
    auto rng = range(std::numeric_limits<size_t>::max());
    return zip(std::forward<decltype(rng)>(rng), std::forward<Iterable>(it));
}

} // namespace itertools
#endif // RANGE_CONTAINER_H