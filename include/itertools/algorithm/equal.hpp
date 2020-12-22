#include "itertools/types.hpp"
#include "itertools/views/all.hpp"

#pragma once

namespace itertools {

template<tupletools::Rangeable R1, tupletools::Rangeable R2>
bool
equal(R1&& range1, R2&& range2)
{
    auto zp = views::zip(range1, range2);

    for (auto&& [x, y] : zp) {
        if (x != y) {
            return false;
        }
    }

    for (auto&& [x, y] : zp | views::reverse()) {
        if (x != y) {
            return false;
        }
    }
    return true;
}
}