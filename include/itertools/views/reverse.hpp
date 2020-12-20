#include <itertools/range_container.hpp>
#include <itertools/tupletools.hpp>

#pragma once

namespace itertools {
namespace views {
namespace detail {

template<class Range>
class reverse_container : range_container<Range>
{
  public:
    template<class BeginIt, class EndIt>
    class iterator : public bi_range_container_iterator<BeginIt, EndIt>
    {
      public:
        iterator(BeginIt&& begin_it, EndIt&& end_it)
          : bi_range_container_iterator<BeginIt, EndIt>(std::forward<BeginIt>(begin_it),
                                                        std::forward<EndIt>(end_it))
        {}

        decltype(auto) operator*()
        {
            auto tmp = *--this->it;
            this->it = ++this->it;
            return tmp;
        }

        decltype(auto) operator++()
        {
            --this->it;
            return *this;
        }

        decltype(auto) operator--()
        {
            ++this->it;
            return *this;
        }
    };

    template<class BeginIt, class EndIt>
    iterator(BeginIt&&, EndIt&&) -> iterator<BeginIt, EndIt>;

    bool was_cached = false;

    using super = range_container<Range>;

    reverse_container(Range&& range)
      : range_container<Range>(std::forward<Range>(range))
    {}

    decltype(auto) init_range()
    {
        if (was_cached || !(this->begin_ || this->end_)) {
            this->begin_ = super::begin();
            this->end_ = super::end();
            was_cached = false;
        } else {
            was_cached = true;
        }
        return std::forward_as_tuple(*this->begin_, *this->end_);
    }

    auto begin()
    {
        auto [begin, end] = init_range();
        return iterator(end, begin);
    }

    auto end()
    {
        auto [begin, end] = init_range();
        return iterator(begin, end);
    }
};

template<class Range>
reverse_container(Range&&) -> reverse_container<Range>;

template<class Range>
constexpr decltype(auto)
reverse(Range&& range)
{
    return reverse_container(std::forward<Range>(range));
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