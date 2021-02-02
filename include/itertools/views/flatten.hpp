#include "itertools/algorithm/begin_end.hpp"
#include "itertools/range_iterator.hpp"
#include "itertools/tupletools.hpp"

#pragma once

namespace itertools {
namespace views {

using namespace tupletools;

template<class Range,
         class OuterBegin = iter_begin_t<Range>,
         class OuterEnd = iter_end_t<Range>,
         class InnerBegin = decltype(std::declval<OuterBegin>()->begin()),
         class InnerEnd = decltype(std::declval<OuterEnd>()->end())>
using cached_flattened_container =
  cached_container<Range,
                   std::tuple<OuterBegin, InnerBegin, InnerEnd>,
                   std::tuple<OuterEnd, InnerEnd, InnerBegin>>;

auto
advance_nested_it(auto& outer_it, bool is_reversed = false)
{
    if (is_reversed) {
        --outer_it;
        auto tup = std::make_tuple(outer_it->end(), outer_it->begin());
        ++outer_it;
        return tup;
    } else {
        return std::make_tuple(outer_it->begin(), outer_it->end());
    }
}

template<class Range>
class flatten_container : cached_flattened_container<Range>
{
  public:
    template<class Iter>
    class iterator : public range_iterator<Iter>
    {
      private:
        auto& get_nested() { return (is_reversed ? *base->end_ : *base->begin_); }
        auto& outer() { return std::get<0>(get_nested()); }
        auto& end() { return std::get<2>(get_nested()); }

        bool is_complete() { return this->it == end(); }

        bool is_outer_complete()
        {
            return std::get<0>(*base->begin_) == std::get<0>(*base->end_);
        }

        void advance(auto&& advance_func)
        {
            if (this->is_complete() and !this->is_outer_complete()) {

                advance_func(this);

                if (!this->is_outer_complete()) {
                    inner_advance();
                }
            }
        }

        void inner_advance()
        {
            std::tie(this->it, end()) = advance_nested_it(outer(), is_reversed);
        }

      public:
        using value_t = std::remove_cvref_t<Iter>;

        flatten_container* base;
        value_t cached_value;
        bool is_reversed;

        iterator(flatten_container* base, Iter&& it, bool is_reversed = false)
          : range_iterator<Iter>{ std::forward<Iter>(it) }
          , base(base)
          , cached_value(it)
          , is_reversed(is_reversed)
        {}

        template<class T>
        bool operator==(const iterator<T>& rhs) const
        {
            return this->it == rhs.it;
        }

        decltype(auto) operator++()
        {

            ++this->it;
            constexpr auto func = [](auto&& self) { ++(self->outer()); };

            advance(func);
            cached_value = this->it;

            return *this;
        }

        decltype(auto) operator--()
        {
            --this->it;
            constexpr auto func = [](auto&& self) { --(self->outer()); };

            cached_value = this->it;
            advance(func);

            return *this;
        }
        auto operator*() -> decltype(auto) { return *cached_value; }
        auto operator->() -> decltype(auto) { return this->it; }
    };

    template<class Iter>
    iterator(flatten_container*, Iter&&, bool = false) -> iterator<Iter>;

    flatten_container(Range&& range)
      : cached_flattened_container<Range>(std::forward<Range>(range))
    {}

    void init_begin() override
    {
        auto outer = this->range.begin();
        auto [inner_begin, inner_end] = advance_nested_it(outer);

        this->begin_ = std::make_tuple(outer, inner_begin, inner_end);
    }
    void init_end() override
    {
        auto outer = this->range.end();
        auto [inner_begin, inner_end] = advance_nested_it(outer, true);

        this->end_ = std::make_tuple(outer, inner_begin, inner_end);
    }

    auto begin() { return iterator(this, std::get<1>(this->cache_begin())); }

    auto end() { return iterator(this, std::get<1>(this->cache_end()), true); }
};

template<class Range>
flatten_container(Range&&) -> flatten_container<Range>;

template<NestedRange Range>
constexpr flatten_container<Range>
flatten(Range&& range)
{
    return flatten_container(std::forward<Range>(range));
}

template<class Range>
constexpr auto
flatten(Range&& range)
{
    return range;
}

}
}