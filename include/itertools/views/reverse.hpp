#include <itertools/range_iterator.hpp>
#include <itertools/tupletools.hpp>

#pragma once

namespace itertools {
namespace views {
namespace detail {

template<class Range>
class reverse_container
{
  public:
    template<class Iter>
    class iterator : public range_iterator<Iter>
    {
      public:
        iterator(Iter&& it)
          : range_iterator<Iter>(std::forward<Iter>(it))
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

    template<class Iter>
    iterator(Iter&&) -> iterator<Iter>;

    Range range;

    reverse_container(Range&& range)
      : range(std::forward<Range>(range))
    {}

    auto begin() { return iterator(range.end()); }

    auto end() { return iterator(range.begin()); }
};

template<class Range>
reverse_container(Range&&) -> reverse_container<Range>;

template<class Range>
constexpr reverse_container<Range>
reverse(Range&& range)
{
    return reverse_container<Range>(std::forward<Range>(range));
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