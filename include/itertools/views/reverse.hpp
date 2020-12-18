#include <itertools/range_container.hpp>
#include <itertools/tupletools.hpp>

#pragma once

namespace itertools { namespace views {

template<class BeginIt, class EndIt>
class reverse_iterator : public bi_range_container_iterator<BeginIt, EndIt>
{
public:
    reverse_iterator(BeginIt begin_it, EndIt end_it) noexcept
      : bi_range_container_iterator<
            BeginIt,
            EndIt>(std::forward<BeginIt>(begin_it), std::forward<EndIt>(end_it))
      , was_cached{false}
    {}

    template<class T>
    bool operator==(T&&)
    {
        bool res = this->begin_it == this->end_it;
        if (res && !was_cached) {
            was_cached = true;
            return false;
        } else {
            return res;
        }
    }

    decltype(auto) operator++()
    {
        --this->begin_it;
        return *this;
    }

    decltype(auto) operator--()
    {
        ++this->begin_it;
        return *this;
    }

    bool was_cached;
};

namespace detail {
template<class Range>
constexpr auto
reverse(Range&& range)
{
    auto begin_func = [](auto&& range) {
        return reverse_iterator(--range.end(), range.begin());
    };
    auto end_func = [](auto&& range) {
        return reverse_iterator(range.begin(), --range.end());
    };

    return range_container(
        std::forward<Range>(range), std::move(begin_func), std::move(end_func));
};
}

constexpr auto
reverse()
{
    return []<class Range>(Range&& range) {
        return detail::reverse(std::forward<Range>(range));
    };
}

}}