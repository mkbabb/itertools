#include <itertools/range_iterator.hpp>

#pragma once

namespace itertools { namespace views {

template<class Func, class Range>
class intersperse_iterator : public range_iterator<Range>
{
public:
    intersperse_iterator(Func func, Range&& range) noexcept
      : range_iterator<Range>(std::forward<Range>(range))
      , func{func}
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

    auto operator--() -> decltype(auto)
    {
        if (!do_intersperse()) {
            ++this->begin_it;
        }
        pos -= 1;
    }

    auto operator*()
    {
        if (do_intersperse()) {
            return func();
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
constexpr decltype(auto)
intersperse(Func func, Range&& range)
{
    auto it =
        intersperse_iterator<Func, Range>(std::move(func), std::forward<Range>(range));
    return range_container(std::move(it));
};
}

template<tupletools::invocable Func>
constexpr decltype(auto)
intersperse(Func&& func)
{
    return [func = std::forward<Func>(func)]<class Range>(Range&& range) {
        return detail::intersperse(func, std::forward<Range>(range));
    };
}

template<class T>
constexpr decltype(auto)
intersperse(T value)
{
    auto func = [=]() -> T { return value; };
    return intersperse(std::move(func));
}

}}