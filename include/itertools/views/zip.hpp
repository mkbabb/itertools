#include "itertools/range_iterator.hpp"
#include "itertools/tupletools.hpp"

#include <tuple>

#pragma once

namespace itertools {
namespace views {

namespace detail {

constexpr auto tuple_begin = []<class Range>(Range& range) {
    return tupletools::transform([](auto&& x) { return x.begin(); }, range);
};

constexpr auto tuple_end = []<class Range>(Range& range) {
    return tupletools::transform([](auto&& x) { return x.end(); }, range);
};

template<tupletools::Tupleoid T>
using tuple_begin_t =
  std::remove_cvref_t<std::invoke_result_t<decltype(tuple_begin), T>>;

template<tupletools::Tupleoid T>
using tuple_end_t = std::remove_cvref_t<std::invoke_result_t<decltype(tuple_end), T>>;

template<class Range>
class zip_container
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

    using begin_t = tuple_begin_t<Range&>;
    using end_t = tuple_end_t<Range&>;

    Range range;

    zip_container(Range&& range)
      : range(std::forward<Range>(range))
    {}

    auto begin() -> iterator<begin_t> { return iterator(tuple_begin(this->range)); }

    auto end() -> iterator<end_t> { return iterator(tuple_end(this->range)); }
};

template<class Range>
zip_container(Range&&) -> zip_container<Range>;
}

// template<class... Args>
// requires(...&& std::is_reference_v<Args>) constexpr decltype(auto) zip(Args&&...
// args)
// {
//     auto range = std::forward_as_tuple(args...);
//     return detail::zip_container(std::move(range));
// }

template<class... Args>
constexpr decltype(auto)
zip(Args&&... args)
{
    auto tup = std::forward_as_tuple(args...);
    return detail::zip_container(std::move(tup));
}

template<class... Args>
constexpr decltype(auto)
zip_copy(Args&&... args)
{
    auto tup = std::make_tuple(args...);
    return detail::zip_container(std::move(tup));
}

template<class... Args>
constexpr decltype(auto)
zip_copy_if_rvalue(Args&&... args)
{
    auto tup = std::make_tuple(tupletools::copy_if_rvalue(std::forward<Args>(args))...);
    return detail::zip_container(std::move(tup));
}

}
}