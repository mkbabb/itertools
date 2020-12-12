#include <itertools/range_container.hpp>

#pragma once

namespace itertools { namespace views {

template<class Range, class BeginIt, class EndIt>
class concat_iterator : public range_tuple_iterator<Range, BeginIt, EndIt>
{
public:
    concat_iterator(Range&& range, BeginIt&& begin_it, EndIt&& end_it)
      : range_tuple_iterator<Range, BeginIt, EndIt>{
            std::forward<Range>(range),
            std::forward<BeginIt>(begin_it),
            std::forward<EndIt>(end_it)}
    {}

    bool is_first_complete()
    {
        return std::get<0>(this->begin_it) == std::get<0>(this->end_it);
    }

    bool is_complete()
    {
        if (is_first_complete()) {
            tupletools::roll(this->begin_it, true);
            tupletools::roll(this->end_it, true);

            return is_first_complete();
        } else {
            return false;
        }
    }

    auto operator++() -> decltype(auto)
    {
        ++std::get<0>(this->begin_it);
        this->is_complete();
        return *this;
    }

    auto operator*() -> decltype(auto)
    {
        return *std::get<0>(this->begin_it);
    }
};

template<class T, class... Args>
constexpr auto
concat(T&& arg, Args&&... args)
{
    static_assert(
        tupletools::is_all<T, Args...>::value,
        "All arguments must be of a homogenous type.");

    return make_tuple_iterator<
        concat_iterator,
        T,
        Args...>(std::forward<T>(arg), std::forward<Args>(args)...);
}

}}