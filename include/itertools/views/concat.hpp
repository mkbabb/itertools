#include "itertools/algorithm/to.hpp"
#include "itertools/range_iterator.hpp"
#include "itertools/tupletools.hpp"
#include "itertools/types.hpp"
#include "transform.hpp"
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
class concat_container_tuple : public cached_tuple<Range>
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
        using value_t = tuple_iter_value_t<typename cached_tuple<Range>::begin_t>;
        static constexpr int N = tupletools::tuple_size_v<Range>;

        concat_container_tuple* base;
        value_t value;
        bool is_reversed;
        bool rolled = false;

        iterator(concat_container_tuple* base, Iter&& it, bool is_reversed = false)
          : range_iterator<Iter>{ std::forward<Iter>(it) }
          , base(base)
          , value(std::get<0>(it))
          , is_reversed(is_reversed)
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
    iterator(concat_container_tuple*, Iter&&, bool = false) -> iterator<Iter>;

    concat_container_tuple(Range&& range)
      : cached_tuple<Range>{ std::forward<Range>(range) }
    {}

    void init_begin() override { this->begin_ = tuple_begin(this->range); }
    void init_end() override { this->end_ = tuple_end(this->range); }

    auto begin() { return iterator(this, this->cache_begin()); }
    auto end() { return iterator(this, this->cache_end(), true); }
};

template<class Range>
concat_container_tuple(Range&&) -> concat_container_tuple<Range>;

auto vector_begin = [](auto&& range) {
    return range | views::transform([](auto&& x) { return x.begin(); }) |
           to<std::vector>();
};

auto vector_end = [](auto&& range) {
    return range | views::transform([](auto&& x) { return x.end(); }) |
           to<std::vector>();
};

template<class Range, class Value = range_value_t<Range>>
using cached_vector = cached_container<Range,
                                       std::vector<iter_begin_t<Value>>,
                                       std::vector<iter_end_t<Value>>>;

template<class Range>
class concat_container_vector : public cached_vector<Range>
{

  public:
    template<class Iter>
    class iterator : range_iterator<Iter>
    {
      private:
        bool is_complete() const
        {
            return (*base->begin_)[get_ix()] == (*base->end_)[get_ix()];
        }

        bool roll(bool reverse = true)
        {
            itertools::roll(*base->begin_, reverse);
            itertools::roll(*base->end_, reverse);

            return is_complete();
        }

        bool roll_if_complete()
        {
            if (is_complete()) {
                rolled = true;
                return roll(!is_reversed);
            } else {
                rolled = false;
                return false;
            }
        }

        auto get_ix() const { return is_reversed ? this->it.size() - 1 : 0; }

        auto& first() { return this->it[get_ix()]; }

      public:
        using value_t = iter_begin_t<range_value_t<Range>>;

        concat_container_vector* base;
        value_t cached_value;
        bool is_reversed;
        bool rolled = false;

        iterator(concat_container_vector* base, Iter&& it, bool is_reversed = false)
          : range_iterator<Iter>{ std::forward<Iter>(it) }
          , base(base)
          , cached_value(it[0])
          , is_reversed(is_reversed)
        {}

        template<class T>
        bool operator==(const iterator<T>& rhs) const
        {
            return is_complete();
        }

        auto operator++() -> decltype(auto)
        {
            if (is_reversed && rolled) {
                roll(true);
            }

            ++first();
            roll_if_complete();
            cached_value = first();

            return *this;
        }

        auto operator--() -> decltype(auto)
        {
            if (!is_reversed && rolled) {
                roll(false);
            }

            --first();
            cached_value = first();
            roll_if_complete();

            return *this;
        }

        auto operator*() -> decltype(auto) { return *cached_value; }
        auto operator->() -> decltype(auto) { return cached_value; }
    };

    template<class Iter>
    iterator(concat_container_vector*, Iter&&, bool = false) -> iterator<Iter>;

    concat_container_vector(Range&& range)
      : cached_vector<Range>{ std::forward<Range>(range) }
    {}

    void init_begin() override { this->begin_ = vector_begin(this->range); }
    void init_end() override { this->end_ = vector_end(this->range); }

    auto begin() { return iterator(this, this->cache_begin()); }
    auto end() { return iterator(this, this->cache_end(), true); }
};

template<class Range>
concat_container_vector(Range&&) -> concat_container_vector<Range>;

template<class Range>
concat_container_tuple<Range>
concat(Range&& range)
{
    return concat_container_tuple(std::forward<Range>(range));
}

}

template<class T, class... Args>
requires(std::is_same_v<T, Args>&&...) constexpr decltype(auto)
  concat(T&& arg, Args&&... args)
{
    auto tup = std::forward_as_tuple(arg, args...);
    return detail::concat(std::move(tup));
}

template<class Range>
auto
flat(Range&& range)
{
    auto v = range | itertools::to<std::vector>();
    return detail::concat_container_vector(std::move(v));
}

}
}