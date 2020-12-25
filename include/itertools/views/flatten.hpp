#include "itertools/algorithm/begin_end.hpp"
#include "itertools/range_iterator.hpp"
#include "itertools/tupletools.hpp"

#pragma once

namespace itertools {
namespace views {

using namespace tupletools;

template<class Range, class begin_t = iter_begin_t<Range>>
using cached_flattened_container =
  cached_container<Range,
                   decltype(std::declval<begin_t>()->begin()),
                   decltype(std::declval<begin_t>()->end())>;

namespace itt = itertools;

template<class Range>
class flatten_container : cached_flattened_container<Range>
{
  public:
    using begin_t = iter_begin_t<Range>;
    using end_t = iter_end_t<Range>;
    template<class Iter>
    class iterator : public range_iterator<Iter>
    {
      public:
        flatten_container* base;
        begin_t outer_begin;
        end_t outer_end;
        bool is_reversed;

        iterator(flatten_container* base, Iter&& it)
          : range_iterator<Iter>{ std::forward<Iter>(it) }
          , base(base)
          , outer_begin(base->range.begin())
          , outer_end(base->range.end())
          , is_reversed{ it != *base->begin_ }
        {}

        bool is_complete() const { return *base->begin_ == *base->end_; }

        bool is_outer_complete() { return outer_begin == outer_end; }

        void inner_advance()
        {
            *base->begin_ = outer_begin->begin();
            *base->end_ = outer_begin->end();
        }

        template<class T>
        bool operator==(const iterator<T>& rhs) const
        {
            return is_complete();
        }

        decltype(auto) operator++()
        {
            ++this->it;

            if (this->is_complete()) {
                ++outer_begin;

                if (!this->is_outer_complete()) {
                    inner_advance();
                }
            }
            return *this;
        }

        decltype(auto) operator--()
        {
            --this->it;

            if (this->is_complete()) {
                --outer_begin;

                if (!this->is_outer_complete()) {
                    inner_advance();
                }
            }
            return *this;
        }
    };

    template<class Iter>
    iterator(flatten_container*, Iter&&) -> iterator<Iter>;

    flatten_container(Range&& range)
      : cached_flattened_container<Range>(std::forward<Range>(range))
    {}

    void init_begin() override { this->begin_ = this->range.begin()->begin(); }
    void init_end() override { this->end_ = this->range.begin()->end(); }

    auto begin() { return iterator(this, this->cache_begin()); }

    auto end() { return iterator(this, this->cache_end()); }
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