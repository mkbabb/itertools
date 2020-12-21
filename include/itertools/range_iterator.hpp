#ifndef RANGE_CONTAINER_H
#define RANGE_CONTAINER_H

#include "tupletools.hpp"
#include "types.hpp"

#include <numeric>
#include <string>

#include <type_traits>

#pragma once

namespace itertools {
using namespace tupletools;

struct range_container_terminus
{};

template<Rangeable R>
using iter_begin_t = decltype(std::declval<R>().begin());

template<Rangeable R>
using iter_end_t = decltype(std::declval<R>().end());

template<class Iter>
class range_iterator
{
  public:
    Iter it;

    constexpr range_iterator(Iter&& it)
      : it{ std::forward<Iter>(it) }
    {}

    range_iterator(const range_iterator& other) = default;
    range_iterator& operator=(const range_iterator& other)
    {
        this->it = other.it;
        return *this;
    }
    ~range_iterator() = default;

    bool operator==(const range_iterator<Iter>& rhs) const { return it == rhs.it; }

    decltype(auto) operator++()
    {
        ++this->it;
        return *this;
    }

    decltype(auto) operator*() { return *this->it; }

    decltype(auto) operator->() { return this->it; }
};

template<class Iter>
range_iterator(Iter&&) -> range_iterator<Iter>;

template<class Range,
         class begin_t = iter_begin_t<Range>,
         class end_t = iter_end_t<Range>>
class cached_container
{
  public:
    Range range;
    std::optional<begin_t> begin_;
    std::optional<end_t> end_;
    bool was_cached = false;

    cached_container(Range&& range)
      : range{ std::forward<Range>(range) }
    {}

    virtual void init_begin()
    {
        if constexpr (ForwardRange<Range>) {
            this->begin_ = std::begin(range);
        }
    }

    virtual void init_end()
    {
        if constexpr (ForwardRange<Range>) {
            this->end_ = std::end(range);
        }
    }

    void cache()
    {
        if (was_cached || !(begin_ || end_)) {
            init_begin();
            init_end();
            was_cached = false;
        } else {
            was_cached = true;
        }
    }

    begin_t& cache_begin()
    {
        cache();
        return *begin_;
    }

    end_t& cache_end()
    {
        cache();
        return *end_;
    }
};

template<ForwardRange Range, class Func>
requires invocable<Func, Range> constexpr auto
operator|(Range&& rhs, Func&& lhs)
{
    return std::invoke(std::forward<Func>(lhs), std::forward<Range>(rhs));
}

template<class Func, class Funk>
constexpr auto
operator|(Funk&& rhs, Func&& lhs)
{
    return [&]<class... Args>(Args && ... args)
    {
        return std::invoke(
          std::forward<Funk>(rhs),
          std::invoke(std::forward<Func>(lhs), std::forward<Args>(args)...));
    };
}

} // namespace itertools
#endif // RANGE_CONTAINER_H