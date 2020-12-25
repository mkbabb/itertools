#include <type_traits>

#include "itertools/types.hpp"

#pragma once

namespace itertools {

template<tupletools::Rangeable Range>
decltype(auto)
begin(Range&& range)
{
    return std::begin(range);
}

template<tupletools::RangeablePtr Range>
decltype(auto)
begin(Range range)
{
    return itertools::begin(*range);
}

template<tupletools::Rangeable Range>
decltype(auto)
end(Range&& range)
{
    return std::end(range);
}

template<tupletools::RangeablePtr Range>
decltype(auto)
end(Range range)
{
    return itertools::end(*range);
}

}