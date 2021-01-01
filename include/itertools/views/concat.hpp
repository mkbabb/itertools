#include "itertools/algorithm/to.hpp"
#include "itertools/itertools.hpp"
#include "itertools/range_iterator.hpp"
#include "itertools/types.hpp"
#include "transform.hpp"

#pragma once

namespace itertools {
namespace views {
namespace detail {

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
class concat_container : public cached_vector<Range>
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

        bool roll(bool reverse = false)
        {
            itertools::roll(*base->begin_, reverse);
            itertools::roll(*base->end_, reverse);

            return is_complete();
        }

        bool roll_if_complete()
        {
            if (is_complete()) {
                rolled = true;
                return roll();
            } else {
                rolled = false;
                return false;
            }
        }

        auto get_ix() const { return is_reversed ? this->it.size() - 1 : 0; }

        auto& first() { return this->it[get_ix()]; }

      public:
        using value_t = iter_begin_t<range_value_t<Range>>;

        concat_container* base;
        value_t cached_value;
        bool is_reversed;
        bool rolled = false;

        iterator(concat_container* base, Iter&& it, bool is_reversed = false)
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
    iterator(concat_container*, Iter&&, bool) -> iterator<Iter>;

    concat_container(Range&& range)
      : cached_vector<Range>{ std::forward<Range>(range) }
    {}

    void init_begin() override { this->begin_ = vector_begin(this->range); }

    void init_end() override { this->end_ = vector_end(this->range); }

    auto begin() { return iterator(this, this->cache_begin(), false); }

    auto end() { return iterator(this, this->cache_end(), true); }
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

template<class Range>
auto
flat(Range&& range)
{
    auto v = range | itertools::to<std::vector>();
    return detail::concat_container(std::move(v));
}

template<class T, class... Args>
requires(std::is_same_v<T, Args>&&...) constexpr decltype(auto)
  concat(T&& arg, Args&&... args)
{
    constexpr int N = sizeof...(Args) + 1;
    std::vector<std::remove_cvref_t<T>> v;
    v.reserve(N);
    v.push_back(arg);
    (v.push_back(args), ...);

    return detail::concat(std::move(v));
}

}
}