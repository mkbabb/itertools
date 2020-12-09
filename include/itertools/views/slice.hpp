#include <itertools/views/filter.hpp>

#pragma once

namespace itertools { namespace views {

namespace detail {
template<class Range>
constexpr auto
slice(Range&& range, size_t start = 0, size_t stop = std::numeric_limits<size_t>::max())
{
    auto pred = [=, pos = 0UL](auto&&) mutable {
        auto b = (pos >= start and pos < stop);
        pos += 1;
        return b;
    };
    using Pred = decltype(pred);
    return filter<Pred, Range>(std::forward<Pred>(pred), std::forward<Range>(range));
};
}

auto
slice(size_t start = 0, size_t stop = std::numeric_limits<size_t>::max())
{
    return [=]<class Range>(Range&& range) {
        return detail::slice<Range>(std::forward<Range>(range), start, stop);
    };
}

}}