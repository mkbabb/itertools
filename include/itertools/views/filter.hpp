#include "reverse.hpp"

#include <itertools/range_container.hpp>

#pragma once

namespace itertools { namespace views {

constexpr auto identity = []<class T>(T&& t) -> decltype(auto) {
    return std::forward<T>(t);
};

template<class I, class S, class Pred, class Proj = decltype(identity)>
constexpr decltype(auto)
find_if(I first, S last, Pred pred, Proj proj = {})
{
    for (; first != last; ++first) {
        if (std::invoke(pred, std::invoke(proj, *first))) {
            return first;
        }
    }
    return first;
}

template<class Range, class Pred, class Proj = decltype(identity)>
constexpr decltype(auto)
find_if(Range&& range, Pred&& pred, Proj proj = {})
{
    return find_if(
        range.begin(), range.end(), std::forward<Pred>(pred), std::ref(proj));
}

// template<class Range, class BeginFunc, class EndFunc>
// class filter_container : public range_container<Range, BeginFunc, EndFunc>
// {
// public:
// }

template<class Pred, class BeginIt, class EndIt>
class filter_iterator : public bi_range_container_iterator<BeginIt, EndIt>
{
public:
    filter_iterator(Pred&& pred, BeginIt&& begin_it, EndIt&& end_it)
      : bi_range_container_iterator<
            BeginIt,
            EndIt>(std::forward<BeginIt>(begin_it), std::forward<EndIt>(end_it))
      , pred{pred}
    {}

    auto operator++() -> decltype(auto)
    {
        this->it = find_if(++this->it, this->end_it, pred);
        return *this;
    }

    auto operator--() -> decltype(auto)
    {
        while (this->it != this->end_it) {
            --this->it;
            if (std::invoke(pred, *this->it)) {
                break;
            }
        }
        return *this;
    }

    auto operator*() -> decltype(auto)
    {
        return *this->it;
    }

private:
    bool was_incremented = false;
    Pred pred;
};

namespace detail {
template<class Func, class Range>
constexpr auto
filter(Func func, Range&& range)
{
    auto clamp_range = [&func](auto&& range) {
        auto end = range.end();
        auto begin = find_if(range.begin(), end, std::forward<Func>(func));
        return std::make_tuple(begin, end);
    };

    auto begin_func = [&](auto&& range) {
        auto [begin, end] = clamp_range(range);

        return filter_iterator(
            std::forward<Func>(func), std::move(begin), std::move(end));
    };

    auto end_func = [&](auto&& range) {
        auto [begin, end] = clamp_range(range);

        return filter_iterator(
            std::forward<Func>(func), std::move(end), std::move(begin));
    };

    return range_container<Range, decltype(begin_func), decltype(end_func)>(
        std::forward<Range>(range), std::move(begin_func), std::move(end_func));
};
}

template<class Func>
constexpr auto
filter(Func func)
{
    return [func = std::forward<Func>(func)]<class Range>(Range&& range) {
        return detail::filter(func, std::forward<Range>(range));
    };
}

}}