#include <itertools/range_container.hpp>

#pragma once

namespace itertools { namespace views {

template<class Func, class Range>
class intersperse_iterator : public range_container_iterator<Range>
{
public:
    intersperse_iterator(Func&& func, Range&& range) noexcept
      : range_container_iterator<Range>(std::forward<Range>(range))
      , func{std::forward<Func>(func)}
      , pos{0}
    {}

    bool do_intersperse()
    {
        return pos % 2 == 1;
    }

    auto operator++() -> decltype(auto)
    {
        if (!do_intersperse()) {
            ++this->begin_it;
        }
        pos += 1;
    }

    auto operator*()
    {
        if (do_intersperse()) {
            return this->func();
        } else {
            return *this->begin_it;
        }
    }

private:
    size_t pos;
    Func func;
};

namespace detail {
template<class Func, class Range>
constexpr auto
intersperse(Func&& func, Range&& range)
{
    auto it = intersperse_iterator<
        Func,
        Range>(std::forward<Func>(func), std::forward<Range>(range));
    using Iterator = decltype(it);

    return range_container<
        Range,
        Iterator>(std::forward<Range>(range), std::forward<Iterator>(it));
};
}

template<class Func, class T = decltype(Func::operator())>
constexpr auto
intersperse(Func&& func)
{
    return [func = std::forward<Func>(func)]<class Range>(Range&& range) {
        return detail::intersperse(func, std::forward<Range>(range));
    };
}

template<class T>
constexpr auto
intersperse(T&& value)
{
    auto func = [value = std::forward<T>(value)]() -> const auto
    {
        return value;
    };
    using Func = decltype(func);

    return intersperse<Func, void>(std::move(func));
}

}}