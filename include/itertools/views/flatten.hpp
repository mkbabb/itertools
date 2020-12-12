#include <itertools/range_container.hpp>
#include <itertools/tupletools.hpp>

#pragma once

namespace itertools { namespace views {

using namespace tupletools;

template<class Range, class IBeginIt, class IEndIt>
class flatten_iterator : public range_container_iterator<Range>
{
public:
    flatten_iterator(Range&& range)
      : range_container_iterator<Range>{std::forward<Range>(range)}
      , i_begin_it{std::forward<IBeginIt>(this->begin_it->begin())}
      , i_end_it{std::forward<IEndIt>(this->begin_it->end())}
    {}

    bool is_internal_complete()
    {
        return i_begin_it == i_end_it;
    }

    void outer_advance()
    {
        i_begin_it = this->begin_it->begin();
        i_end_it = this->begin_it->end();
    }

    auto operator++() -> decltype(auto)
    {
        ++i_begin_it;

        if (is_internal_complete()) {
            ++this->begin_it;

            if (!this->is_complete()) {
                outer_advance();
            }
        }
        return *this;
    }

    auto operator*() -> decltype(auto)
    {
        return *i_begin_it;
    }

    IBeginIt i_begin_it;
    IEndIt i_end_it;
};

template<class Range>
constexpr auto
flatten(Range&& range)
{
    using IBeginIt = decltype(std::declval<Range>().begin()->begin());
    using IEndIt = decltype(std::declval<Range>().begin()->end());

    auto it = flatten_iterator<Range, IBeginIt, IEndIt>(std::forward<Range>(range));
    using Iterator = decltype(it);

    return range_container<
        Range,
        Iterator>(std::forward<Range>(range), std::forward<Iterator>(it));
}

// template<class Range, std::enable_if_t<!is_nested_iterable_v<Range>, bool> = true>
// constexpr auto
// flatten(Range&& range)
// {
//     return to_range(std::forward<Range>(range));
// }

}}