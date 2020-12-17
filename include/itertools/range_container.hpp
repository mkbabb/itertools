#ifndef RANGE_CONTAINER_H
#define RANGE_CONTAINER_H

#include "tupletools.hpp"
#include "types.hpp"

#include <iostream>
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

constexpr auto default_begin_func = []<class Range>(Range&& range) {
    return range_container_iterator(
        std::forward<Range>(range), range.begin(), range.end());
};

template<class Range, class BeginFunc = decltype(default_begin_func)>
class range_container
{
public:
    range_container(Range&& range, BeginFunc&& begin_func = BeginFunc())
      : range{std::forward<Range>(range)}
      , begin_func{begin_func}
    {}

    decltype(auto) begin()
    {
        return begin_func(std::forward<Range>(range));
    }

    decltype(auto) end()
    {
        return range_container_terminus{};
    }

    auto size()
    {

        return 1;
    }

    Range range;
    BeginFunc begin_func;
};

template<class Func, tupletools::ForwardRange Range>
decltype(auto)
operator|(Range&& rhs, Func&& lhs)
{
    return std::invoke(std::forward<Func>(lhs), std::forward<Range>(rhs));
}

template<
    class Range,
    class BeginIt = decltype(std::declval<Range>().begin()),
    class EndIt = decltype(std::declval<Range>().end())>
class range_container_iterator
{
public:
    using iterator_category = std::forward_iterator_tag;

    range_container_iterator(Range&& range)
      : begin_it{range.begin()}
      , end_it{range.end()}
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
    template<class T>
    bool operator!=(T)
    {
        return !(this->is_complete());
    }

    decltype(auto) operator++()
    {
        ++this->begin_it;
        return *this;
    }

    decltype(auto) operator--()
    {
        --this->begin_it;
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

    decltype(auto) operator++()
    {
        tupletools::for_each(this->begin_it, [](auto&&, auto&& v) { ++v; });
        return *this;
    }

    decltype(auto) operator--()
    {
        tupletools::for_each(this->begin_it, [](auto&&, auto&& v) { --v; });
        return *this;
    }

    decltype(auto) operator*()
    {
        auto func = [](auto&& v) -> decltype(auto) {
            return tupletools::copy_if_rvalue(*v);
        };
        return tupletools::transform(func, this->begin_it);
    }

    decltype(auto) operator->()
    {
        return this->begin_it;
    }
};

template<template<typename... Ts> class Iter, class... Args>
decltype(auto)
make_tuple_iterator(Args&&... args)
{
    auto begin_func = []<class Range>(Range&& range) -> decltype(auto) {
        auto begin_it = tupletools::
            transform([](auto&& x) { return x.begin(); }, std::forward<Range>(range));
        auto end_it = tupletools::
            transform([](auto&& x) { return x.end(); }, std::forward<Range>(range));

        return Iter(std::forward<Range>(range), std::move(begin_it), std::move(end_it));
    };

    auto range =
        std::make_tuple(tupletools::copy_if_rvalue(std::forward<Args>(args))...);
    using Range = decltype(range);

    return range_container(std::forward<Range>(range), std::move(begin_func));
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