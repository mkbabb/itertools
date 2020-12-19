#include <itertools/range_container.hpp>
#include <itertools/tupletools.hpp>

#pragma once

namespace itertools {
namespace views {
namespace detail {

template<class BeginIt, class EndIt>
class reverse_iterator : public bi_range_container_iterator<BeginIt, EndIt>
{
  public:
    reverse_iterator(BeginIt&& begin_it, EndIt&& end_it) noexcept
      : bi_range_container_iterator<BeginIt, EndIt>(std::forward<BeginIt>(begin_it),
                                                    std::forward<EndIt>(end_it))
    {}

    decltype(auto) operator*()
    {
        auto tmp = *this;
        return *(--tmp.it);
    }

    decltype(auto) operator++()
    {
        --this->it;
        return *this;
    }

    decltype(auto) operator++(int)
    {
        auto tmp = *this;
        ++*this;
        return tmp;
    }

    decltype(auto) operator--()
    {
        ++this->it;
        return *this;
    }
    decltype(auto) operator--(int)
    {
        auto tmp = *this;
        --*this;
        return tmp;
    }
};

template<class Range>
constexpr auto
reverse(Range&& range)
{
    auto begin_func = [&](auto& range) {
        return reverse_iterator(range.end(), range.begin());
    };

    auto end_func = [&](auto& range) {
        return reverse_iterator(range.begin(), range.end());
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

}
}