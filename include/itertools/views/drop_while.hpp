#include "itertools/range_iterator.hpp"

#pragma once

namespace itertools {
namespace views {

template<class Pred, class Range>
class drop_while_container : cached_container<Range>
{
  public:
    Pred pred;

    drop_while_container(Pred&& pred, Range&& range)
      : cached_container<Range>(std::forward<Range>(range))
      , pred(std::forward<Pred>(pred))
    {}

    void init_begin() override { this->end_ = std::end(this->range); }

    void init_end() override
    {
        this->begin_ = itertools::find_if(std::begin(this->range), *this->end_, pred);
    }

    auto begin() { return range_iterator(this->cache_begin()); }

    auto end() { return range_iterator(this->cache_end()); }
};

template<class Pred, class Range>
drop_while_container(Pred&&, Range&&) -> drop_while_container<Pred, Range>;

namespace detail {
template<class Pred, class Range>
constexpr auto
drop_while(Pred pred, Range&& range)
{
    auto p = [pred = std::move(pred)]<class T>(T&& x) {
        return !pred(std::forward<T>(x));
    };
    return drop_while_container(std::move(p), std::forward<Range>(range));
};
}

template<class Pred>
constexpr auto
drop_while(Pred&& pred)
{
    return [pred = std::forward<Pred>(pred)]<class Range>(Range&& range) {
        return detail::drop_while(pred, std::forward<Range>(range));
    };
}

}
}