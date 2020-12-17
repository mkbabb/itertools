#include "itertools/range_container.hpp"

#pragma once

namespace itertools { namespace views {

template<class... Args>
constexpr decltype(auto)
zip(Args&&... args)
{
    return make_tuple_iterator<range_tuple_iterator, Args...>(
        std::forward<Args>(args)...);
}

}}