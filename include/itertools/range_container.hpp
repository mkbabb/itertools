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
{};

template<
    class Range,
    class BeginIt = decltype(std::declval<Range>().begin()),
    class EndIt = decltype(std::declval<Range>().end())>
class range_container_iterator
{
public:
    using iterator_category = std::forward_iterator_tag;

    range_container_iterator(Range&& range)
      : range_container_iterator{std::forward<Range>(range), range.begin(), range.end()}
    {}

    range_container_iterator(Range&& range, BeginIt&& begin_it, EndIt&& end_it)
      : begin_it{std::forward<BeginIt>(begin_it)}
      , end_it{std::forward<EndIt>(end_it)}
    {}

    bool is_complete()
    {
        return this->begin_it == this->end_it;
    }

    template<class T>
    bool operator==(T)
    {
        return this->is_complete();
    }

    auto operator++() -> decltype(auto)
    {
        ++this->begin_it;
        return *this;
    }

    auto operator*() -> decltype(auto)
    {
        return *this->begin_it;
    }

    auto operator->() -> decltype(auto)
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
      : range_container_iterator<Range, BeginIt, EndIt>{
            std::forward<Range>(range),
            std::forward<BeginIt>(begin_it),
            std::forward<EndIt>(end_it)}
    {}

    bool is_complete()
    {
        return tupletools::any_where(
            [](auto&& x, auto&& y) { return x == y; }, this->begin_it, this->end_it);
    }

    auto operator++() -> decltype(auto)
    {
        tupletools::for_each(this->begin_it, [](auto&& n, auto&& v) { ++v; });
        return *this;
    }

    auto operator*() -> decltype(auto)
    {
        auto func = [](auto&& v) { return *v; };
        return tupletools::transform(func, this->begin_it);
    }

    auto operator->() -> decltype(auto)
    {
        return this->begin_it;
    }
};

template<
    class Range,
    class Iterator,
    std::enable_if_t<tupletools::is_iterator_v<Iterator>, int> = 0>
class range_container
{
public:
    size_t size_;

    range_container(Range&& range, Iterator&& it)
      : it{std::forward<Iterator>(it)}
      , range{std::forward<Range>(range)}
      , size_{1}
    {}

    auto begin()
    {
        return it;
    }

    auto end()
    {
        return range_container_terminus{};
    }

    size_t size()
    {
        return size_;
    }

    template<class Func>
    auto operator|(Func&& rhs) -> decltype(auto)
    {
        return std::
            invoke(std::forward<Func>(rhs), std::forward<decltype(*this)>(*this));
    }

    template<class Func, class R, class I>
    friend auto operator|(Func&& lhs, range_container<R, I>&& rhs)
    {
        using RR = std::remove_cvref_t<decltype(rhs)>;
        return std::invoke(std::forward<Func>(lhs), std::forward<RR>(rhs));
    }

    Iterator it;
    Range range;
};

template<class, template<class, class...> class>
struct is_instance : public std::false_type
{};

template<class... Ts, template<class, class...> class U>
struct is_instance<U<Ts...>, U> : public std::true_type
{};

template<typename Range>
constexpr auto
to_range(Range&& range)
{
    using Iterator = range_container_iterator<Range>;

    auto it = Iterator(std::forward<Range>(range));
    return range_container(std::forward<Range>(range), std::move(it));
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