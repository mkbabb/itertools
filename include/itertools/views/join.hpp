#include "intersperse.hpp"
#include "transform.hpp"

#pragma once

namespace itertools { namespace views {

constexpr auto join = [](std::string delim) {
    return [=]<class Range>(Range&& range) {
        auto t_string = [](auto&& x) { return fmt::format("{}", x); };
        return range | views::transform(t_string) | views::intersperse(delim);
    };
};

}}
