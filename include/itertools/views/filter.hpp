#include "itertools/algorithm/find_if.hpp"
#include "itertools/range_container.hpp"
#include "reverse.hpp"

#pragma once

namespace itertools {
namespace views {

template<class Pred, class Range>
class filter_container : public range_container<Range>
{
  public:
    Pred pred;

    template<class BeginIt, class EndIt>
    class iterator : public bi_range_container_iterator<BeginIt, EndIt>
    {
      public:
        filter_container* base;

        iterator(filter_container* base, BeginIt&& begin_it, EndIt&& end_it)
          : bi_range_container_iterator<BeginIt, EndIt>(std::forward<BeginIt>(begin_it),
                                                        std::forward<EndIt>(end_it))
          , base(base)
        {}

        auto operator++() -> decltype(auto)
        {
            this->it = itertools::find_if(++this->it, this->end_it, base->pred);
            return *this;
        }

        auto operator--() -> decltype(auto)
        {
            while (this->it != this->end_it) {
                if (std::invoke(base->pred, *(--this->it))) {
                    break;
                }
            }
            return *this;
        }
    };

    template<class BeginIt, class EndIt>
    iterator(filter_container*, BeginIt&&, EndIt&&) -> iterator<BeginIt, EndIt>;

    // using begin_t = typename range_container<Range>::begin_t;
    // using end_t = typename range_container<Range>::end_t;

    // begin_t* begin_ = nullptr;
    // end_t* end_ = nullptr;

    filter_container(Pred&& pred, Range&& range)
      : range_container<Range>(std::forward<Range>(range))
      , pred(std::forward<Pred>(pred))
    {}

    auto clamp_range()
    {
        if (this->begin_ == nullptr && this->end_ == nullptr) {
            *this->end_ = range_container<Range>::end();
            *this->begin_ =
              itertools::find_if(range_container<Range>::begin(), *this->end_, pred);
        }
        return std::forward_as_tuple(*this->begin_, *this->end_);
    };

    auto begin()
    {
        auto [begin, end] = clamp_range();
        return iterator(this, begin, end);
    }

    auto end()
    {
        auto [begin, end] = clamp_range();
        return iterator(this, end, begin);
    }
};

template<class Pred, class Range>
filter_container(Pred&&, Range&&) -> filter_container<Pred, Range>;

namespace detail {
template<class Func, class Range>
constexpr auto
filter(Func func, Range&& range)
{
    return filter_container(std::forward<Func>(func), std::forward<Range>(range));
};
}

template<class Func>
constexpr auto
filter(Func&& func)
{
    return [func = std::forward<Func>(func)]<class Range>(Range&& range) {
        return detail::filter(func, std::forward<Range>(range));
    };
}

}
}