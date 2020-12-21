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

    range_iterator(Iter&& it)
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

template<class Func, ForwardRange Range>
auto
operator|(Range&& rhs, Func&& lhs)
{
    return std::invoke(std::forward<Func>(lhs), std::forward<Range>(rhs));
}

} // namespace itertools
#endif // RANGE_CONTAINER_H