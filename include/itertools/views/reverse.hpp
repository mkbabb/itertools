#include "concat.hpp"
#include "itertools/range_iterator.hpp"
#include "itertools/tupletools.hpp"

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
        constexpr iterator(Iter&& it)
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

    constexpr reverse_container(Range&& range)
      : range(std::forward<Range>(range))
    {}

    auto begin() { return iterator(std::end(range)); }

    auto end() { return iterator(std::begin(range)); }
};

template<class Range>
reverse_container(Range&&) -> reverse_container<Range>;

template<class Range>
concept concatted = is_instance_v<std::remove_cvref_t<Range>, concat_container>;

template<class Range>
constexpr reverse_container<Range>
reverse(Range&& range)
{
    if constexpr (concatted<Range>) {
        range.range = tupletools::reverse(range.range);
    }
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