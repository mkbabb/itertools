#include "range.hpp"
#include "zip.hpp"

#pragma once

namespace itertools { namespace views {

template<class Range>
constexpr decltype(auto)
enumerate(Range&& range)
{
    auto rng = views::range(std::numeric_limits<int>::max());
    return zip(std::move(rng), std::forward<Range>(range));
}

}}