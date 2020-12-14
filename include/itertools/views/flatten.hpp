#include <itertools/range_container.hpp>
#include <itertools/tupletools.hpp>

#pragma once

namespace itertools { namespace views {

using namespace tupletools;

template<
    class Range,
    class BeginIt = decltype(std::declval<Range>().begin()),
    class EndIt = decltype(std::declval<Range>().end()),
    class IBeginIt = decltype(std::declval<BeginIt>()->begin()),
    class IEndIt = decltype(std::declval<BeginIt>()->end())>
class flatten_iterator : public range_container_iterator<Range, IBeginIt, IEndIt>
{
public:
    flatten_iterator(Range&& range)
      : range_container_iterator<
            Range,
            IBeginIt,
            IEndIt>{std::forward<Range>(range),
            std::forward<IBeginIt>(range.begin()->begin()),
            std::forward<IEndIt>(range.begin()->end())}
      , outer_begin_it{std::forward<BeginIt>(range.begin())}
      , outer_end_it{std::forward<EndIt>(range.end())}
    {}

    bool is_outer_complete()
    {
        return outer_begin_it == outer_end_it;
    }

    void inner_advance()
    {
        this->begin_it = outer_begin_it->begin();
        this->end_it = outer_begin_it->end();
    }

    decltype(auto) operator++()
    {
        ++this->begin_it;

        if (this->is_complete()) {
            ++outer_begin_it;

            if (!this->is_outer_complete()) {
                inner_advance();
            }
        }
        return *this;
    }

    BeginIt outer_begin_it;
    EndIt outer_end_it;
};

template<NestedRange Range>
constexpr auto
flatten(Range&& range)
{
    auto it = flatten_iterator<Range>(std::forward<Range>(range));
    using Iter = decltype(it);

    return range_container<Range, Iter>(std::forward<Range>(range), std::move(it));
}

template<class Range>
constexpr auto
flatten(Range&& range)
{
    return range;
}

}}