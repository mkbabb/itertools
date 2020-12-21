#include "itertools/tupletools.hpp"
#include <functional>

#pragma once

namespace itertools {

constexpr auto identity = []<class T>(T&& t) -> decltype(auto) {
    return std::forward<T>(t);
};

template<tupletools::ForwardIterable I,
         tupletools::ForwardIterable S,
         class Pred,
         class Proj = decltype(identity)>
constexpr decltype(auto)
find_if(I first, S last, Pred&& pred, Proj&& proj = {})
{
    for (; first != last; ++first) {
        if (std::invoke(std::forward<Pred>(pred),
                        std::invoke(std::forward<Proj>(proj), *first))) {
            return first;
        }
    }
    return first;
}

template<tupletools::ForwardRange Range, class Pred, class Proj = decltype(identity)>
constexpr decltype(auto)
find_if(Range&& range, Pred&& pred, Proj proj = {})
{
    return find_if(
      range.begin(), range.end(), std::forward<Pred>(pred), std::ref(proj));
}

}