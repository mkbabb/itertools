#ifndef ITERTOOLS_H
#define ITERTOOLS_H

#include "itertools/range_iterator.hpp"
#include "itertools/tupletools.hpp"
#include "itertools/types.hpp"
#include "itertools/views/all.hpp"

#include <algorithm>
#include <chrono>
#include <functional>
#include <iostream>
#include <map>
#include <numeric>
#include <optional>
#include <sstream>
#include <vector>

#pragma once

namespace itertools {

template<class Iterable>
constexpr Iterable
roll(Iterable&& iter, int axis = -1)
{
    if (axis == 0) {
        return iter;
    } else if (axis == -1) {
        axis = iter.size() - 1;
    }
    int i = 0;
    while (i < axis) {
        std::swap(iter[axis], iter[i]);
        i += 1;
    };
    return iter;
}

template<class Func>
struct y_combinator
{
    Func func;
    y_combinator(Func func)
      : func(std::move(func))
    {}
    template<class... Args>
    auto operator()(Args&&... args) const
    {
        return func(std::ref(*this), std::forward<Args>(args)...);
    }
};

};     // namespace itertools
#endif // ITERTOOLS_H
