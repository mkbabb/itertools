#include "itertools/range_iterator.hpp"
#include <tuple>

#pragma once

namespace itertools {
namespace views {

namespace detail {
template<class Range>
class range_tuple
{
  public:
    template<class Iter>
    class iterator : public range_iterator<Iter>
    {
      public:
        iterator(Iter&& it)
          : range_iterator<Iter>(std::forward<Iter>(it))
        {}

        template<class T>
        bool operator==(const iterator<T>& rhs) const
        {
            return tupletools::any_where(
              [](auto&& x, auto&& y) { return x == y; }, this->it, rhs.it);
        }

        decltype(auto) operator++()
        {
            tupletools::for_each(this->it, [](auto&&, auto&& v) { ++v; });
            return *this;
        }

        decltype(auto) operator--()
        {
            tupletools::for_each(this->it, [](auto&&, auto&& v) { --v; });
            return *this;
        }

        decltype(auto) operator*()
        {
            auto func = [](auto&& v) -> decltype(auto) {
                return tupletools::copy_if_rvalue(*v);
            };
            return tupletools::transform(func, this->it);
        }
    };

    template<class Iter>
    iterator(Iter&&) -> iterator<Iter>;

    Range range;

    range_tuple(Range&& range)
      : range(std::forward<Range>(range))
    {}

    auto begin()
    {
        auto tup = tupletools::transform([](auto&& x) { return x.begin(); },
                                         std::forward<Range>(this->range));
        return iterator(std::move(tup));
    }

    auto end()
    {
        auto tup = tupletools::transform([](auto&& x) { return x.end(); },
                                         std::forward<Range>(this->range));
        return iterator(std::move(tup));
    }
};

template<class Range>
range_tuple(Range&&) -> range_tuple<Range>;
}

template<class... Args>
constexpr decltype(auto)
zip(Args&&... args)
{
    auto range = std::forward_as_tuple(args...);
    return detail::range_tuple(std::move(range));
}

template<class... Args>
constexpr decltype(auto)
zip_copy_if_rvalue(Args&&... args)
{
    auto range =
      std::make_tuple(tupletools::copy_if_rvalue(std::forward<Args>(args))...);
    return detail::range_tuple(std::move(range));
}

}
}