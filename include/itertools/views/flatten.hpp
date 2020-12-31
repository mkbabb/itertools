#include "itertools/algorithm/begin_end.hpp"
#include "itertools/range_iterator.hpp"
#include "itertools/tupletools.hpp"

#pragma once

namespace itertools {
namespace views {

using namespace tupletools;

template<class Outer, class Inner>
struct nested_iter
{
    Outer outer;
    Inner inner;

    nested_iter(Outer outer, Inner inner)
      : outer(outer)
      , inner(inner)
    {}
};

// template<class Outer, class Inner>
// nested_iter(Outer&&, Inner&&) -> nested_iter<Outer, Inner>;

template<class Range,
         class OuterBegin = iter_begin_t<Range>,
         class OuterEnd = iter_end_t<Range>,
         class InnerBegin = decltype(std::declval<OuterBegin>()->begin()),
         class InnerEnd = decltype(std::declval<OuterEnd>()->end())>
using cached_flattened_container = cached_container<Range,
                                                    nested_iter<OuterBegin, InnerBegin>,
                                                    nested_iter<OuterEnd, InnerEnd>>;

namespace itt = itertools;

template<class Range>
class flatten_container : cached_flattened_container<Range>
{
  public:
    template<class BeginIter, class EndIter>
    class iterator : public range_iterator<BeginIter>
    {
      public:
        flatten_container* base;
        EndIter end;

        bool is_reversed;

        iterator(flatten_container* base,
                 bool is_reversed,
                 BeginIter&& it,
                 EndIter&& end)
          : range_iterator<BeginIter>{ std::forward<BeginIter>(it) }
          , end{ std::forward<EndIter>(end) }
          , base(base)
          , is_reversed{ is_reversed }
        {}

        auto& get_outer() { return (is_reversed ? *base->end_ : *base->begin_).outer; }

        bool is_complete() const { return this->it == end; }

        bool is_outer_complete()
        {
            return (*base->begin_).outer == (*base->end_).outer;
        }

        void inner_advance()
        {
            auto& oit = get_outer();

            if (is_reversed) {
                this->it = (--oit)->end();
                end = oit->begin();
            } else {
                this->it = oit->begin();
                end = oit->end();
            }
        }

        template<class T, class U>
        bool operator==(const iterator<T, U>& rhs) const
        {
            return is_complete();
        }

        decltype(auto) operator++()
        {
            ++this->it;

            if (this->is_complete()) {
                ++get_outer();

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
                --get_outer();

                if (!this->is_outer_complete()) {
                    inner_advance();
                }
            }
            return *this;
        }
    };

    template<class BeginIter, class EndIter>
    iterator(flatten_container*, bool, BeginIter&&, EndIter&&)
      -> iterator<BeginIter, EndIter>;

    flatten_container(Range&& range)
      : cached_flattened_container<Range>(std::forward<Range>(range))
    {}

    void init_begin() override
    {
        auto outer = this->range.begin();
        auto inner = outer->begin();
        nested_iter iter(outer, inner);

        this->begin_ = std::move(iter);
    }
    void init_end() override
    {
        auto outer = this->range.end();
        auto inner = (--outer)->end();

        nested_iter iter(outer, inner);

        this->end_ = std::move(iter);
    }

    auto begin()
    {
        auto& iter = this->cache_begin();
        auto& begin = iter.inner;
        auto end = iter.outer->end();

        return iterator(this, false, begin, std::move(end));
    }

    auto end()
    {
        auto& iter = this->cache_end();
        auto& begin = iter.inner;
        auto end = iter.outer->begin();

        return iterator(this, true, begin, std::move(end));
    }
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