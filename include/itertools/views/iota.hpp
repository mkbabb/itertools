#include "itertools/range_iterator.hpp"
#include <iostream>

#pragma once

namespace itertools {
namespace views {

template<typename T = int>
class iota
{
  public:
    class iterator
    {
      public:
        explicit iterator(iota* base)
          : base(base)
        {}

        auto operator++()
        {
            base->current += base->stride;
            return *this;
        }

        auto operator--()
        {
            base->current -= base->stride;
            return *this;
        }

        bool operator==(const iterator&) const { return base->current == base->stop; }

        const T operator*() { return base->current; }

        iota* base;
    };

    constexpr explicit iota(T stop)
      : start(0)
      , stop(stop)
    {
        if (start > stop) {
            std::swap(start, stop);
        }
        stride = start > stop ? -1 : 1;
        current = start;
    }

    constexpr iota(T start, T stop)
      : start(start)
      , stop(stop)
    {
        stride = start > stop ? -1 : 1;
        current = start;
    }

    constexpr iota(T start, T stop, T stride)
      : start(start)
      , stop(stop)
      , stride(stride)
      , current(start)
    {}

    int size() const { return size_; }

    auto begin() { return iterator(this); }

    auto end() { return iterator(this); }

    T start, stop, stride;
    T current;
    size_t size_ = 0;
};

}
}