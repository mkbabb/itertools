#include "itertools/range_container.hpp"

#pragma once

namespace itertools {
namespace views {

template<class Func, class Range>
class transform_container : public range_container<Range>
{
  public:
    Func func;

    template<class Iter>
    class iterator : public range_container_iterator<Iter>
    {
      public:
        transform_container* base;

        iterator(transform_container* base, Iter&& it)
          : range_container_iterator<Iter>(std::forward<Iter>(it))
          , base(base)
        {}

        decltype(auto) operator*() { return std::invoke(base->func, *this->it); }
    };

    template<class Iter>
    iterator(transform_container*, Iter&&) -> iterator<Iter>;

    using super = range_container<Range>;

    transform_container(Func&& func, Range&& range)
      : range_container<Range>(std::forward<Range>(range))
      , func(std::forward<Func>(func))
    {}

    auto begin() { return iterator(this, super::begin()); }

    auto end() { return iterator(this, super::end()); }
};

template<class Func, class Range>
transform_container(Func&&, Range&&) -> transform_container<Func, Range>;

namespace detail {
template<class Func, class Range>
constexpr transform_container<Func, Range>
transform(Func func, Range&& range)
{
    return transform_container(std::move(func), std::forward<Range>(range));
};
}

template<class Func>
constexpr auto
transform(Func&& func)
{
    return [func = std::forward<Func>(func)]<class Range>(Range&& range) {
        return detail::transform(func, std::forward<Range>(range));
    };
}

}
}