#include <itertools/range_container.hpp>

#pragma once

namespace itertools { namespace views {

template<class Pred, class Range>
class filter_iterator : public range_container_iterator<Range>
{
public:
    using IterableValue = tupletools::iterable_t<Range>;

    filter_iterator(Pred&& pred, Range&& range) noexcept
      : range_container_iterator<Range>(std::forward<Range>(range))
      , pred{std::forward<Pred>(pred)}
    {}

    auto operator++()
    {
        ++this->begin_it;

        while (!this->is_complete()) {
            auto good_value = pred(*this->begin_it);

            if (good_value) {
                break;
            } else {
                ++this->begin_it;
            }
        };

        return *this;
    }

    auto operator*()
    {
        auto good_value = pred(*this->begin_it);
        if (!good_value) {
            ++(*this);
        }
        return *this->begin_it;
    }

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
        return detail::
            filter<Func, Range>(std::forward<Func>(func), std::forward<Range>(range));
    };
}

}}