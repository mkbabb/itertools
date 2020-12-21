#include "filter.hpp"

#pragma once

namespace itertools {
namespace views {

constexpr auto drop_while = [](auto&& pred) {
    auto dropper = [pred = std::forward<decltype(pred)>(pred),
                    dropped = true](auto&& x) mutable {
        dropped = pred(x) and dropped;
        return !dropped;
    };
    return views::filter(dropper);
};
}
}