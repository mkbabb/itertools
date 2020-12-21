#include "itertools/range_iterator.hpp"
#include "itertools/tupletools.hpp"
#include "itertools/types.hpp"
#include "zip.hpp"

#pragma once

namespace itertools {
namespace views {
namespace detail {

template<class Range>
using cached_tuple =
  cached_container<Range, tuple_begin_t<Range&>, tuple_end_t<Range&>>;

template<class Range>
class concat_container : cached_tuple<Range>
{
  public:
    template<class Iter>
    class iterator : range_iterator<Iter>
    {
      public:
        concat_container* base;

        iterator(concat_container* base, Iter&& it)
          : range_iterator<Iter>{ std::forward<Iter>(it) }
          , base(base)
        {}

        bool is_first_complete()
        {
            return std::get<0>(this->it) == std::get<0>(*base->end_);
        }

        bool is_complete()
        {
            if (is_first_complete()) {
                tupletools::roll<true>(this->it);
                tupletools::roll<true>(*base->end_);

                return is_first_complete();
            } else {
                return false;
            }
        }

        template<class T>
        bool operator==(T&)
        {
            return is_complete();
        }

        auto operator++() -> decltype(auto)
        {
            ++std::get<0>(this->it);
            is_complete();
            return *this;
        }

        auto operator*() -> decltype(auto) { return *std::get<0>(this->it); }
    };

    template<class Iter>
    iterator(concat_container*, Iter&&) -> iterator<Iter>;

    concat_container(Range&& range)
      : cached_tuple<Range>{ std::forward<Range>(range) }
    {}

    void init_begin() override { this->begin_ = tuple_begin(this->range); }

    void init_end() override { this->end_ = tuple_end(this->range); }

    auto begin() { return iterator(this, this->cache_begin()); }

    auto end() { return iterator(this, this->cache_end()); }
};

template<class Range>
concat_container(Range&&) -> concat_container<Range>;

}

template<class T, class... Args>
requires(std::is_same_v<T, Args>&&...) constexpr auto concat(T&& arg, Args&&... args)
{
    auto tup = std::forward_as_tuple(arg, args...);
    return detail::concat_container(std::move(tup));
}

}
}