#include <itertools/range_container.hpp>

#pragma once

namespace itertools { namespace views {

template<class Pred, class Range>
class filter_iterator : public range_container_iterator<Range>
{
public:
    filter_iterator(Pred&& pred, Range&& range) noexcept
      : range_container_iterator<Range>(std::forward<Range>(range))
      , pred{std::forward<Pred>(pred)}
    {}

    bool invoke_predicate()
    {
        using T = decltype(*this->begin_it);
        return std::invoke(std::forward<Pred>(pred), std::forward<T>(*this->begin_it));
    }

    void filter_until()
    {
        while (!this->is_complete()) {
            auto good_value = invoke_predicate();

            if (good_value) {
                break;
            } else {
                ++this->begin_it;
            }
        };
    }

    auto operator++()
    {
        ++this->begin_it;
        filter_until();
        was_incremented = true;
        return *this;
    }

    auto operator*()
    {
        // We cache the predicate value of the previous increment call.
        // If a 'good value' was found, there's no need to check the predicate again.
        auto good_value = was_incremented || invoke_predicate();
        if (!good_value) {
            filter_until();
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
filter(Func&& func, Range&& range)
{
    auto it = filter_iterator<
        Func,
        Range>(std::forward<Func>(func), std::forward<Range>(range));
    using Iterator = decltype(it);

    return range_container<
        Range,
        Iterator>(std::forward<Range>(range), std::forward<Iterator>(it));
};
}

template<class Func>
constexpr auto
filter(Func&& func)
{
    return [func = std::forward<Func>(func)]<class Range>(Range&& range) {
        return detail::filter(func, std::forward<Range>(range));
    };
}

}}