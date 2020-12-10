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
    auto operator|(Func&& func) -> decltype(auto)
    {
        return std::
            invoke(std::forward<Func>(func), std::forward<decltype(*this)>(*this));
    }

    Iterator it;
    Range range;
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

} // namespace itertools
#endif // RANGE_CONTAINER_H