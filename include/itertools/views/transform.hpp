#include <itertools/range_container.hpp>
#include <itertools/tupletools.hpp>

#pragma once

namespace itertools { namespace views {

template<class Func, class Range>
class transform_iterator : public range_container_iterator<Range>
{
public:
    transform_iterator(Func&& func, Range&& range) noexcept
      : range_container_iterator<Range>(std::forward<Range>(range))
      , func{func}
    {}

    decltype(auto) operator*()
    {
        using T = std::remove_cvref_t<decltype(*this->begin_it)>;
        return std::invoke(std::forward<Func>(func), std::forward<T>(*this->begin_it));
    }

private:
    Func func;
};

namespace detail {
template<class Func, class Range>
constexpr auto
transform(Func func, Range&& range)
{
    auto it =
        transform_iterator<Func, Range>(std::move(func), std::forward<Range>(range));
    using Iter = decltype(it);

    return range_container<Range, Iter>(std::forward<Range>(range), std::move(it));
};
}

template<class Func>
constexpr auto
transform(Func func)
{
    return [func = std::forward<Func>(func)]<class Range>(Range&& range) {
        return detail::transform(func, std::forward<Range>(range));
    };
}

}}