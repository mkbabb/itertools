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

constexpr auto default_begin_func = [](auto& range) { return range.begin(); };
constexpr auto default_end_func = [](auto& range) { return range.end(); };

template<class Range,
         class BeginFunc = decltype(default_begin_func),
         class EndFunc = decltype(default_end_func)>
class range_container
{
  public:
    using begin_t = std::invoke_result_t<BeginFunc, Range&>;
    using end_t = std::invoke_result_t<EndFunc, Range&>;

    Range range;
    BeginFunc begin_func;
    EndFunc end_func;

    begin_t* begin_ = nullptr;
    end_t* end_ = nullptr;

    range_container(Range&& range, BeginFunc&& begin_func = {}, EndFunc&& end_func = {})
      : range{ std::forward<Range>(range) }
      , begin_func{ std::forward<BeginFunc>(begin_func) }
      , end_func{ std::forward<EndFunc>(end_func) }
    {}

    auto begin() -> begin_t { return begin_func(range); }

    auto end() -> end_t { return end_func(range); }

    auto size() { return 1; }
};

template<class Range, class BeginFunc, class EndFunc>
range_container(Range&&, BeginFunc&&, EndFunc&&)
  -> range_container<Range, BeginFunc, EndFunc>;

template<class Range>
range_container(Range&&) -> range_container<Range>;

template<class Func, tupletools::ForwardRange Range>
auto
operator|(Range&& rhs, Func&& lhs)
{
    return std::invoke(std::forward<Func>(lhs), std::forward<Range>(rhs));
}

template<class Iter>
class range_container_iterator
{
  public:
    Iter it;

    range_container_iterator(Iter&& it)
      : it(std::forward<Iter>(it))
    {}

    template<class T, class U>
    bool operator==(const range_container_iterator<T>& rhs) const
    {
        return it == rhs.it;
    }

    template<class T, class U>
    bool operator!=(const range_container_iterator<T>& rhs) const
    {
        return !(*this == rhs);
    }

    decltype(auto) operator++()
    {
        ++this->it;
        return *this;
    }

    decltype(auto) operator++(int)
    {
        auto tmp = *this;
        ++*this;
        return tmp;
    }

    decltype(auto) operator*() { return *this->it; }

    decltype(auto) operator->() { return this->it; }
};

template<class Iter>
range_container_iterator(Iter&&) -> range_container_iterator<Iter>;

template<class BeginIt, class EndIt>
class bi_range_container_iterator : public range_container_iterator<BeginIt>
{
  public:
    EndIt end_it;

    bi_range_container_iterator(BeginIt&& begin_it, EndIt&& end_it)
      : range_container_iterator<BeginIt>(std::forward<BeginIt>(begin_it))
      , end_it(std::forward<EndIt>(end_it))
    {}

    decltype(auto) operator--()
    {
        --this->it;
        return *this;
    }

    decltype(auto) operator--(int)
    {
        auto tmp = *this;
        --*this;
        return tmp;
    }

    template<class T>
    bool operator==(T&&)
    {
        return this->it == end_it;
    }
};

template<class BeginIt, class EndIt>
bi_range_container_iterator(BeginIt&&, EndIt&&)
  -> bi_range_container_iterator<BeginIt, EndIt>;

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
        return tupletools::any_where(
          [](auto&& x, auto&& y) { return x == y; }, this->it, rhs.it);
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

template<class Iter>
range_tuple_iterator(Iter&&) -> range_tuple_iterator<Iter>;

template<template<typename... Ts> class Iter, class... Args>
decltype(auto)
make_tuple_iterator(Args&&... args)
{
    auto begin_func = []<class Range>(Range& range) -> decltype(auto) {
        auto it = tupletools::transform([](auto&& x) { return x.begin(); },
                                        std::forward<Range>(range));
        return Iter(std::move(it));
    };

    auto end_func = []<class Range>(Range& range) -> decltype(auto) {
        auto it = tupletools::transform([](auto&& x) { return x.end(); },
                                        std::forward<Range>(range));
        return Iter(std::move(it));
    };

    auto range =
      std::make_tuple(tupletools::copy_if_rvalue(std::forward<Args>(args))...);
    using Range = decltype(range);

    return range_container(
      std::forward<Range>(range), std::move(begin_func), std::move(end_func));
}

constexpr auto default_inserter = [](auto& container, auto&& y) {
    container.push_back(y);
};

template<template<typename... Ts> class Container,
         class Func = decltype(default_inserter)>
decltype(auto)
to(Func inserter = {})
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