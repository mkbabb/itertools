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

template<class Range, class BeginFunc, class EndFunc>
class range_container
{
public:
    range_container(Range&& range, BeginFunc&& begin_func, EndFunc&& end_func)
      : range{std::forward<Range>(range)}
      , begin_func{begin_func}
      , end_func{end_func}
    {}

    decltype(auto) begin()
    {
        return begin_func(std::forward<Range>(range));
    }

    decltype(auto) end()
    {
        return end_func(std::forward<Range>(range));
    }

    auto size()
    {

        return 1;
    }

    Range range;
    BeginFunc begin_func;
    EndFunc end_func;
};

template<class Func, tupletools::ForwardRange Range>
decltype(auto)
operator|(Range&& rhs, Func&& lhs)
{
    return std::invoke(std::forward<Func>(lhs), std::forward<Range>(rhs));
}

template<class Iter>
class range_container_iterator
{
public:
    // using iterator_category = std::forward_iterator_tag;
    // using iter_value_t = decltype(*std::declval<Iter>());
    // using iter_reference_t = decltype(*std::declval<Iter&>());

    // range_container_iterator(Range&& range)
    //   : begin_it{range.begin()}
    //   , end_it{range.end()}
    // {}

    // range_container_iterator(Range&& range, BeginIt&& begin_it, EndIt&& end_it)
    //   : begin_it{std::forward<BeginIt>(begin_it)}
    //   , end_it{std::forward<EndIt>(end_it)}
    // {}

    range_container_iterator(Iter&& it)
      : it(std::forward<Iter>(it))
    {}

    template<class T>
    bool operator==(const range_container_iterator<T>& rhs) const
    {
        return it == rhs.it;
    }

    template<class T>
    bool operator!=(const range_container_iterator<T>& rhs) const
    {
        return !(*this == rhs);
    }

    decltype(auto) operator++()
    {
        ++this->it;
        return *this;
    }

    decltype(auto) operator--()
    {
        --this->it;
        return *this;
    }

    decltype(auto) operator*()
    {
        return *this->it;
    }

    decltype(auto) operator->()
    {
        return this->it;
    }

    Iter it;
};

template<class BeginIt, class EndIt>
class bi_range_container_iterator : public range_container_iterator<BeginIt>
{
public:
    bi_range_container_iterator(BeginIt&& begin_it, EndIt&& end_it)
      : range_container_iterator<BeginIt>(std::forward<BeginIt>(begin_it))
      , end_it(std::forward<EndIt>(end_it))
    {}

    template<class T>
    bool operator==(T&&)
    {
        return this->it == end_it;
    }

    EndIt end_it;
};

template<class Iter>
class range_tuple_iterator : public range_container_iterator<Iter>
{
public:
    range_tuple_iterator(Iter&& it)
      : range_container_iterator<Iter>(std::forward<Iter>(it))
    {}

    template<class T>
    bool operator==(const range_container_iterator<T>& rhs) const
    {
        return tupletools::
            any_where([](auto&& x, auto&& y) { return x == y; }, this->it, rhs.it);
    }

    template<class T>
    bool operator!=(const range_container_iterator<T>& rhs) const
    {
        return !(*this == rhs);
    }

    decltype(auto) operator++()
    {
        tupletools::for_each(this->it, [](auto&&, auto&& v) { ++v; });
        return *this;
    }

    decltype(auto) operator--()
    {
        tupletools::for_each(this->it, [](auto&&, auto&& v) { --v; });
        return *this;
    }

    decltype(auto) operator*()
    {
        auto func = [](auto&& v) -> decltype(auto) {
            return tupletools::copy_if_rvalue(*v);
        };
        return tupletools::transform(func, this->it);
    }
};

template<template<typename... Ts> class Iter, class... Args>
decltype(auto)
make_tuple_iterator(Args&&... args)
{
    auto begin_func = []<class Range>(Range&& range) -> decltype(auto) {
        auto it = tupletools::
            transform([](auto&& x) { return x.begin(); }, std::forward<Range>(range));
        return Iter(std::move(it));
    };

    auto end_func = []<class Range>(Range&& range) -> decltype(auto) {
        auto it = tupletools::
            transform([](auto&& x) { return x.end(); }, std::forward<Range>(range));
        return Iter(std::move(it));
    };

    auto range =
        std::make_tuple(tupletools::copy_if_rvalue(std::forward<Args>(args))...);
    using Range = decltype(range);

    return range_container(
        std::forward<Range>(range), std::move(begin_func), std::move(end_func));
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