#include "iota.hpp"
#include "zip.hpp"

#pragma once

namespace itertools {
namespace views {

template<class Range>
constexpr decltype(auto)
enumerate(Range&& range)
{
    auto rng = views::iota(std::numeric_limits<int>::max());
    return zip_copy_if_rvalue(std::move(rng), std::forward<Range>(range));
}

}
}