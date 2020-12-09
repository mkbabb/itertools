#include <itertools/views/filter.hpp>

#pragma once

namespace itertools { namespace views {

namespace detail {
template<class Range>
auto
stride(Range&& range, size_t stride = 1) -> decltype(auto)
{
    auto pred = [=, pos = 0UL](auto&&) mutable {
        auto b = pos % stride == 0;
        pos += 1;
        return b;
    };
    using Pred = decltype(pred);

    return filter<Pred, Range>(std::forward<Pred>(pred), std::forward<Range>(range));
};
}

auto
stride(size_t stride = 1)
{
    return [=]<class Range>(Range&& range) {
        return detail::stride<Range>(std::forward<Range>(range), stride);
    };
}

}}