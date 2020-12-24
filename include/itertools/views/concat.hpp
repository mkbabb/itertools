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
using tuple_iter_value_t =
  std::remove_cvref_t<decltype(std::get<0>(std::declval<Range>()))>;
template<class Range>
class concat_container : public cached_tuple<Range>
{

  public:
    template<class Iter>
    class iterator : range_iterator<Iter>
    {
      private:
        template<int ix>
        bool is_complete() const
        {
            return std::get<ix>(*base->begin_) == std::get<ix>(*base->end_);
        }

        template<int ix>
        bool roll()
        {
            tupletools::roll<ix == 0>(*base->begin_);
            tupletools::roll<ix == 0>(*base->end_);
            return is_complete<ix>();
        }

        template<int ix>
        bool roll_if_complete()
        {
            if (is_complete<ix>()) {
                rolled = true;
                return this->roll<ix>();
            } else {
                rolled = false;
                return false;
            }
        }

        void advance(auto&& func)
        {
            if (is_reversed) {
                func.template operator()<N - 1>(this);
            } else {
                func.template operator()<0>(this);
            }
        }

      public:
        using begin_t = tuple_iter_value_t<typename cached_tuple<Range>::begin_t>;
        static constexpr int N = tupletools::tuple_size_v<Range>;

        concat_container* base;
        begin_t value;
        bool is_reversed = false;
        bool rolled = false;

        iterator(concat_container* base, Iter&& it)
          : range_iterator<Iter>{ std::forward<Iter>(it) }
          , base(base)
          , value(std::get<0>(it))
          , is_reversed{ it != *base->begin_ }
        {}

        template<class T>
        bool operator==(const iterator<T>& rhs) const
        {
            if (is_reversed) {
                return is_complete<N - 1>();
            } else {
                return is_complete<0>();
            }
        }

        auto operator++() -> decltype(auto)
        {
            if (is_reversed && rolled) {
                this->roll<0>();
            }

            constexpr auto func = []<int ix>(auto&& self) {
                ++std::get<ix>(self->it);
                self->template roll_if_complete<ix>();
                self->value = std::get<ix>(self->it);
            };

            advance(func);
            return *this;
        }

        auto operator--() -> decltype(auto)
        {
            if (!is_reversed && rolled) {
                this->roll<N - 1>();
            }

            constexpr auto func = []<int ix>(auto&& self) {
                --std::get<ix>(self->it);
                self->value = std::get<ix>(self->it);
                self->template roll_if_complete<ix>();
            };

            advance(func);
            return *this;
        }

        auto operator*() -> decltype(auto) { return *value; }
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

template<class Range>
concat_container<Range>
concat(Range&& range)
{
    return detail::concat_container(std::forward<Range>(range));
}

}

template<class T, class... Args>
requires(std::is_same_v<T, Args>&&...) constexpr decltype(auto)
  concat(T&& arg, Args&&... args)
{
    auto tup = std::forward_as_tuple(arg, args...);
    return detail::concat(std::move(tup));
}

}
}