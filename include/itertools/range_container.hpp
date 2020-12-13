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

    template<class T>
    bool operator==(T)
    {
        return this->begin_it == this->end_it;
    }
    template<class T>
    bool operator!=(T)
    {
        return !(this->begin_it == this->end_it);
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
      : range_container_iterator<Range, BeginIt, EndIt>(
            std::forward<Range>(range),
            std::forward<BeginIt>(begin_it),
            std::forward<EndIt>(end_it))
    {}

    bool is_complete()
    {
        return tupletools::any_where(
            [](auto&& x, auto&& y) { return x == y; }, this->begin_it, this->end_it);
    }

    bool operator==(range_container_terminus)
    {
        return is_complete();
    }

    auto operator++()
    {
        tupletools::for_each(this->begin_it, [](auto&&, auto&& v) { ++v; });
        return *this;
    }

    auto operator*()
    {
        auto func = [](auto&& v) -> decltype(*v) { return *v; };
        return tupletools::transform(func, this->begin_it);
    }

    auto operator->()
    {
        return this->begin_it;
    }
};

template<class Range, class Iterator>
class range_container
{
public:
    range_container(Range&& range, Iterator&& it)
      : it{std::forward<Iterator>(it)}
      , range{std::forward<Range>(range)}
    {}

    auto begin()
    {
        return it;
    }

    auto end()
    {
        return range_container_terminus{};
    }

    auto size()
    {
        if constexpr (tupletools::is_sized_v<Range>) {
            return range.size();
        } else {
            return 1;
        }
    }

    template<class Func>
    auto operator|(Func&& rhs) -> decltype(auto)
    {
        return std::
            invoke(std::forward<Func>(rhs), std::forward<decltype(*this)>(*this));
    }

    // template<class Func, class R, class I>
    // friend auto operator|(Func&& lhs, range_container<R, I>&& rhs)
    // {
    //     using RR = std::remove_cvref_t<decltype(rhs)>;
    //     return std::invoke(std::forward<Func>(lhs), std::forward<RR>(rhs));
    // }

    Iterator it;
    Range range;
};

template<template<typename... Ts> class Iterator, class... Args>
decltype(auto)
make_tuple_iterator(Args&&... args)
{
    auto range = std::make_tuple(args...);
    // auto begin_it = std::make_tuple(args.begin()...);
    // auto end_it = std::make_tuple(args.end()...);

    using Range = decltype(range);
    // using BeginIt = decltype(begin_it);
    // using EndIt = decltype(end_it);

    auto it = Iterator(
        std::forward<Range>(range),
        std::make_tuple(args.begin()...),
        std::make_tuple(args.end()...));

    return range_container(std::forward<Range>(range), std::move(it));
}

constexpr auto default_inserter = [](auto&& x, auto&& y) {
    x.push_back(std::forward<decltype(y)>(y));
};

template<
    template<typename... Ts>
    class Container,
    class Func = decltype(default_inserter)>
decltype(auto)
to(Func inserter = Func())
{
    return [=]<class Range>(Range&& range) {
        using Value = tupletools::iterable_t<Range>;
        auto container = Container<Value>{};

        for (auto&& x : range) {
            inserter(container, std::forward<decltype(x)>(x));
        }

        return container;
    };
}

} // namespace itertools
#endif // RANGE_CONTAINER_H