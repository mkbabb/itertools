#include <itertools/range_container.hpp>

#pragma once

namespace itertools { namespace views {

template<class Func, class Range>
class transform_iterator : public range_container_iterator<Range>
{
public:
    transform_iterator(Func&& func, Range&& range) noexcept
      : range_container_iterator<Range>(std::forward<Range>(range))
      , func{std::forward<Func>(func)}
    {}

    auto operator*() -> decltype(auto)
    {
        using T = decltype(*this->begin_it);
        return std::invoke(std::forward<Func>(func), std::forward<T>(*this->begin_it));
    }

private:
    Func func;
};

namespace detail {
template<class Func, class Range>
constexpr auto
transform(Func&& func, Range&& range)
{
    auto it = transform_iterator<
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
transform(Func&& func)
{
    return [func = std::forward<Func>(func)]<class Range>(Range&& range) {
        return detail::transform(func, std::forward<Range>(range));
    };
}

}}