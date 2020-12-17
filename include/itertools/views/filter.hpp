#include <itertools/range_container.hpp>

#pragma once

namespace itertools { namespace views {

constexpr auto identity = []<class T>(T&& t) { return std::forward<T>(t); };

template<class I, class S, class Pred, class Proj = decltype(identity)>
constexpr decltype(auto)
find_if(I first, S last, Pred&& pred, Proj proj = {})
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

template<class Pred, class Range>
class filter_iterator : public range_container_iterator<Range>
{
public:
    filter_iterator(Pred&& pred, Range&& range)
      : range_container_iterator<Range>(std::forward<Range>(range))
      , pred{pred}
    {}

    auto operator++() -> decltype(auto)
    {
        this->begin_it = find_if(this->begin_it, this->end_it, this->pred);
        was_incremented = true;

        return *this;
    }

    auto operator--() -> decltype(auto)
    {
        while (true) {
            --this->begin_it;
            if (std::invoke(pred, *this->begin_it)) {
                break;
            }
        }
        return *this;
    }

    auto operator*() -> decltype(auto)
    {
        // We cache the predicate value of the previous increment call.
        // If a 'good value' was found, there's no need to check the predicate again.
        auto good_value = was_incremented || std::invoke(pred, *this->begin_it);
        if (!good_value) {
            ++(*this);
            was_incremented = false;
        }
        return *this->begin_it;
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
    auto begin_func = [func = std::forward<Func>(func)](auto&& range) {
        return filter_iterator<
            Func,
            Range>(std::move(func), std::forward<Range>(range));
    };

    return range_container(std::forward<Range>(range), std::move(begin_func));
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