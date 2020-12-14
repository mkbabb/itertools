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

    range_container_iterator(Range&& range, BeginIt begin_it, EndIt end_it)
      : begin_it{begin_it}
      , end_it{end_it}
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

    decltype(auto) operator++()
    {
        ++this->begin_it;
        return *this;
    }

    decltype(auto) operator*()
    {
        return *this->begin_it;
    }

    decltype(auto) operator->()
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
    range_tuple_iterator(Range&& range, BeginIt begin_it, EndIt end_it)
      : range_container_iterator<
            Range,
            BeginIt,
            EndIt>(std::forward<Range>(range), begin_it, end_it)
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

    decltype(auto) operator++()
    {
        tupletools::for_each(this->begin_it, [](auto&&, auto&& v) { ++v; });
        return *this;
    }

    decltype(auto) operator*()
    {
        auto func = [](auto&& v) -> decltype(auto) { return *v; };
        return tupletools::transform(func, this->begin_it);
    }

    decltype(auto) operator->()
    {
        return this->begin_it;
    }
};

template<class Range, class Iterator>
class range_container
{
public:
    range_container(Range&& range, Iterator it)
      : range{std::forward<Range>(range)}
      , it{it}
    {}

    decltype(auto) begin()
    {
        return it;
    }

    decltype(auto) end()
    {
        return range_container_terminus{};
    }

    auto size()
    {

        return 1;
    }

    // template<class Func>
    // auto operator|(Func&& rhs) -> decltype(auto)
    // {
    //     return std::
    //         invoke(std::forward<Func>(rhs), std::forward<decltype(*this)>(*this));
    // }

    Iterator it;
    Range range;
};

template<class Func, tupletools::ForwardRange Range>
decltype(auto)
operator|(Range&& rhs, Func&& lhs)
{
    return std::invoke(std::forward<Func>(lhs), std::forward<Range>(rhs));
}

template<template<typename... Ts> class Iterator, class... Args>
decltype(auto)
make_tuple_iterator(Args&&... args)
{
    auto range = std::forward_as_tuple(args...);

    using Range = decltype(range);

    auto it = Iterator(
        std::forward<Range>(range),
        std::make_tuple(args.begin()...),
        std::make_tuple(args.end()...));

    return range_container(std::forward<Range>(range), std::move(it));
}

constexpr auto default_inserter = [](auto&& container, auto y) {
    container.push_back(y);
};

template<
    template<typename... Ts>
    class Container,
    class Func = decltype(default_inserter)>
decltype(auto)
to(Func inserter = Func())
{
    return [=]<tupletools::Rangeable Range>(Range&& range) {
        using Value = std::remove_cvref_t<tupletools::range_value_t<Range>>;
        auto container = Container<Value>{};

        for (auto&& x : range) {
            inserter(container, std::forward<decltype(x)>(x));
        }

        return container;
    };
}

} // namespace itertools
#endif // RANGE_CONTAINER_H