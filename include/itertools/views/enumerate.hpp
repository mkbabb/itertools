#include "range.hpp"
#include "zip.hpp"

#pragma once

namespace itertools { namespace views {

template<class Iterable>
constexpr auto
enumerate(Iterable&& it)
{
    auto rng = range(std::numeric_limits<size_t>::max());
    return zip(std::move(rng), std::forward<Iterable>(it));
}

}}