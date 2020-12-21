#include "itertools/range_iterator.hpp"
#include "itertools/tupletools.hpp"
#include "itertools/types.hpp"
#include "zip.hpp"

#pragma once

namespace itertools {
namespace views {
namespace detail {

template<class Range>
class concat_container
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

    using begin_t = tuple_begin_t<Range&>;
    using end_t = tuple_end_t<Range&>;

    Range range;
    std::optional<begin_t> begin_;
    std::optional<end_t> end_;
    bool was_cached = false;

    concat_container(Range&& range)
      : range{ std::forward<Range>(range) }
    {}

    decltype(auto) init_range()
    {
        if (was_cached || !(begin_ || end_)) {
            begin_ = tuple_begin(range);
            end_ = tuple_end(range);
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