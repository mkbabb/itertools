#include "itertools/algorithm/find_if.hpp"
#include "itertools/range_iterator.hpp"
#include "reverse.hpp"

#pragma once

namespace itertools {
namespace views {

template<class Pred, class Range>
class filter_container : cached_container<Range>
{
  public:
    template<class Iter>
    class iterator : public range_iterator<Iter>
    {
      public:
        filter_container* base;

        iterator(filter_container* base, Iter&& it)
          : range_iterator<Iter>(std::forward<Iter>(it))
          , base(base)
        {}

        auto operator++() -> decltype(auto)
        {
            this->it = itertools::find_if(++this->it, *base->end_, base->pred);
            return *this;
        }

        auto operator--() -> decltype(auto)
        {
            while (--this->it != *base->end_) {
                if (std::invoke(base->pred, *this->it)) {
                    break;
                }
            }
            return *this;
        }
    };

    template<class Iter>
    iterator(filter_container*, Iter&&) -> iterator<Iter>;

    Pred pred;

    filter_container(Pred&& pred, Range&& range)
      : cached_container<Range>(std::forward<Range>(range))
      , pred(std::forward<Pred>(pred))
    {}

    auto begin() { return iterator(this, this->cache_begin()); }

    auto end() { return iterator(this, this->cache_end()); }
};

template<class Pred, class Range>
filter_container(Pred&&, Range&&) -> filter_container<Pred, Range>;

namespace detail {
template<class Pred, class Range>
constexpr filter_container<Pred, Range>
filter(Pred pred, Range&& range)
{
    return filter_container<Pred, Range>(std::move(pred), std::forward<Range>(range));
};
}

template<class Pred>
constexpr auto
filter(Pred&& pred)
{
    return [pred = std::forward<Pred>(pred)]<class Range>(Range&& range) {
        return detail::filter(pred, std::forward<Range>(range));
    };
}

}
}