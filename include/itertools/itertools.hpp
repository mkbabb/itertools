#ifndef ITERTOOLS_H
#define ITERTOOLS_H

#include "itertools/range_iterator.hpp"
#include "itertools/tupletools.hpp"
#include "itertools/types.hpp"
#include "itertools/views/iota.hpp"

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

template<class Iter>
void
roll(Iter&& it, bool reverse = false)
{
    int N = it.size();

    for (int i : views::iota(N)) {
        auto [ix1, ix2] = ([=] {
            if (reverse) {
                return std::make_tuple(N - (i + 1), 0);
            } else {
                return std::make_tuple(N - 1, i);
            }
        })();

        std::swap(it[ix1], it[ix2]);
    }
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
