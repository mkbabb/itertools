#include <itertools/views/filter.hpp>

#pragma once

namespace itertools {
namespace views {

namespace detail {

template<class Range>
decltype(auto)
stride(Range&& range, size_t stride = 1)
{
    auto pred = [=, pos = 0UL](auto&&) mutable {
        auto b = pos % stride == 0;

        if (b && pos == stride) {
            pos = 0;
        }

        pos += 1;
        return b;
    };
    return filter(std::move(pred), std::forward<Range>(range));
};
}

auto
stride(size_t stride = 1)
{
    return [=]<class Range>(Range&& range) {
        return detail::stride<Range>(std::forward<Range>(range), stride);
    };
}

}
}