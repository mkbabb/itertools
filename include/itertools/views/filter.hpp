#include "itertools/algorithm/find_if.hpp"
#include "itertools/range_iterator.hpp"
#include "reverse.hpp"

#pragma once

namespace itertools {
namespace views {

template<class Pred, class Range>
class filter_container
{
  public:
    Pred pred;

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

    using begin_t = iter_begin_t<Range>;
    using end_t = iter_end_t<Range>;

    Range range;
    std::optional<begin_t> begin_;
    std::optional<end_t> end_;
    bool was_cached = false;

    filter_container(Pred&& pred, Range&& range)
      : range(std::forward<Range>(range))
      , pred(std::forward<Pred>(pred))
    {}

    decltype(auto) init_range()
    {
        if (was_cached || !(begin_ || end_)) {
            end_ = range.end();
            begin_ = itertools::find_if(range.begin(), *end_, pred);
            was_cached = false;
        } else {
            was_cached = true;
        }

        return std::forward_as_tuple(*begin_, *end_);
    }

    auto begin()
    {
        auto [begin, end] = init_range();
        return iterator(this, begin);
    }

    auto end()
    {
        auto [begin, end] = init_range();
        return iterator(this, end);
    }
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