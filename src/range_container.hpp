#ifndef RANGE_CONTAINER_H
#define RANGE_CONTAINER_H

#include "generator.hpp"
#include "tupletools.hpp"
#include "types.hpp"

#include <numeric>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>

#pragma once

namespace itertools {
using namespace tupletools;

template<class Iterable, class IterableValue = tupletools::iterable_t<Iterable>>
constexpr std::vector<IterableValue>
to_vector(Iterable&& iter)
{
    std::vector<IterableValue> vec;
    for (auto&& v : iter) {
        vec.push_back(v);
    }
    return vec;
}

// The base iterators whereof all is built.

struct range_container_terminus
{
    bool complete = false;
};

template<
    class Range,
    class Iterator,
    class BeginIter,
    class EndIter,
    class... Args>
class range_base
{
public:
    range_base(Range&& range, Args&&... args)
      : iter{std::forward<Range>(range), std::forward<Args>(args)...}
      , range{range}
    {}

    range_base(Range&& range, BeginIter&& begin_iter, EndIter&& end_iter)
      : iter{std::forward<Range>(range),
             std::forward<BeginIter>(begin_iter),
             std::forward<EndIter>(end_iter)}
      , range{range}
    {}

    Iterator begin()
    {
        return iter;
    }

    auto end()
    {
        return range_container_terminus{true};
    }

    auto size()
    {
        return this->range.size();
    }

    template<class Tfunc>
    auto operator|(Tfunc&& tfunc)
    {
        return tfunc(*this);
    }

    Range range;
    Iterator iter;
};

template<class Range, class Iterator, class... Args>
class range_container
  : public range_base<
        Range,
        Iterator,
        decltype(std::declval<Range>().begin()),
        decltype(std::declval<Range>().end()),
        Args...>
{
public:
    range_container(Range&& range, Args&&... args)
      : range_base<
            Range,
            Iterator,
            decltype(std::declval<Range>().begin()),
            decltype(std::declval<Range>().end()),
            Args...>(std::forward<Range>(range), std::forward<Args>(args)...)
    {}
};

template<template<typename... Ts> class Iterator, class... Args>
class range_tuple
  : public range_base<
        std::tuple<Args...>,
        Iterator<
            std::tuple<Args...>,
            std::tuple<decltype(std::declval<Args>().begin())...>,
            std::tuple<decltype(std::declval<Args>().end())...>>,
        std::tuple<decltype(std::declval<Args>().begin())...>,
        std::tuple<decltype(std::declval<Args>().end())...>>
{
    static constexpr size_t N = sizeof...(Args);
    static_assert(N > 0, "Zip argument count must be larger than 0.");

public:
    range_tuple(Args&&... args)
      : range_base<
            std::tuple<Args...>,
            Iterator<
                std::tuple<Args...>,
                std::tuple<decltype(std::declval<Args>().begin())...>,
                std::tuple<decltype(std::declval<Args>().end())...>>,
            std::tuple<decltype(std::declval<Args>().begin())...>,
            std::tuple<decltype(std::declval<Args>().end())...>>(

            std::forward_as_tuple(args...),
            std::forward_as_tuple(std::begin(args)...),
            std::forward_as_tuple(std::end(args)...))
    {}

    auto size()
    {
        return 1;
    }
};

template<
    class Range,
    class BeginIter = decltype(std::declval<Range>().begin()),
    class EndIter = decltype(std::declval<Range>().end())>
class range_container_iterator
{
public:
    using iterator_category = std::forward_iterator_tag;
    range_container_terminus terminus;

    range_container_iterator(Range&& range) noexcept
      : begin_iter{std::begin(range)}
      , end_iter{std::end(range)}
      , terminus{false}
    {}

    range_container_iterator(
        Range&& range,
        BeginIter&& begin_iter,
        EndIter&& end_iter) noexcept
      : begin_iter{begin_iter}
      , end_iter{end_iter}
      , terminus{false}
    {}

    bool is_complete()
    {
        auto b = this->begin_iter == this->end_iter;
        this->terminus.complete = b;
        return b;
    }

    auto operator++()
    {
        ++this->begin_iter;
        this->is_complete();
        return *this;
    }

    bool operator==(range_container_terminus rhs)
    {
        return this->terminus.complete == rhs.complete;
    }

    bool operator!=(range_container_terminus rhs)
    {
        return this->terminus.complete != rhs.complete;
    }

    auto operator*()
    {
        return *(this->begin_iter);
    }

    auto operator-> () noexcept
    {
        return this->begin_iter;
    }

    BeginIter begin_iter;
    EndIter end_iter;
};

template<class Range, class BeginIter, class EndIter>
class range_tuple_iterator
  : public range_container_iterator<Range, BeginIter, EndIter>
{
public:
    range_tuple_iterator(
        Range&& range,
        BeginIter&& begin_iter,
        EndIter&& end_iter)
      : range_container_iterator<Range, BeginIter, EndIter>(
            std::forward<Range>(range),
            std::forward<BeginIter>(begin_iter),
            std::forward<EndIter>(end_iter))
    {}

    bool is_complete()
    {
        auto b = tupletools::any_where(
            [](auto&& x, auto&& y) { return x == y; },
            this->begin_iter,
            this->end_iter);
        this->terminus.complete = b;
        return b;
    }

    auto operator++()
    {
        tupletools::for_each(this->begin_iter, [](auto n, auto&& v) { ++v; });
        this->is_complete();
        return *this;
    }

    auto operator*() noexcept
    {
        auto func = [](auto&& v) -> decltype(*v) { return *v; };
        return transform_copy(func, this->begin_iter);
    }
};

template<class Range, class BeginIter, class EndIter>
class range_tuple_iterator_ref
  : public range_tuple_iterator<Range, BeginIter, EndIter>
{
public:
    range_tuple_iterator_ref(
        Range&& range,
        BeginIter&& begin_iter,
        EndIter&& end_iter) noexcept
      : range_tuple_iterator<Range, BeginIter, EndIter>(
            std::forward<Range>(range),
            std::forward<BeginIter>(begin_iter),
            std::forward<EndIter>(end_iter))
    {}

    auto operator*() noexcept
    {
        auto func = [](auto&& v) -> decltype(*v) { return *v; };
        return transform(func, this->begin_iter);
    }
};

// end base iterators.

// Standard, non-variadic, container iterators.
template<class Range>
class block_iterator : public range_container_iterator<Range>
{
public:
    using IteratorValue = tupletools::iterable_t<Range>;
    std::vector<IteratorValue> ret;

    block_iterator(Range&& range, size_t block_size) noexcept
      : range_container_iterator<Range>(std::forward<Range>(range))
    {
        this->block_size = block_size;
        ret.reserve(block_size);
    }

    auto operator++()
    {
        auto i = 0;
        while (!this->is_complete() && i++ < this->block_size) {
            ret.pop_back();
        }
        return *this;
    }

    auto operator*() noexcept
    {
        auto i = 0;
        while (!this->is_complete() && i++ < this->block_size) {
            ret.push_back(*(this->begin_iter));
            ++(this->begin_iter);
        }
        return ret;
    }

    size_t block_size;
};

template<class Func, class Range>
class filter_iterator : public range_container_iterator<Range>
{
public:
    using IterableValue = tupletools::iterable_t<Range>;

    filter_iterator(Range&& range, Func&& func) noexcept
      : range_container_iterator<Range>(std::forward<Range>(range))
      , func{std::forward<Func>(func)}
    {}

    auto operator++() noexcept
    {
        ++(this->begin_iter);
        while (!this->is_complete() &&
               !this->func(std::forward<IterableValue>(*this->begin_iter))) {
            ++(this->begin_iter);
        };
        return *this;
    }

    auto operator*()
    {
        if (!this->func(std::forward<IterableValue>(*this->begin_iter))) {
            ++(*this);
        }
        return *(this->begin_iter);
    }

    Func func;
};

template<class Func, class Range>
class transmog_iterator : public range_container_iterator<Range>
{
public:
    transmog_iterator(Range&& range, Func&& func) noexcept
      : range_container_iterator<Range>(std::forward<Range>(range))
      , func{func}
    {}

    auto operator*() noexcept
    {
        return func(
            std::forward<decltype(*(this->begin_iter))>(*(this->begin_iter)));
    }
    Func func;
};

template<typename R>
constexpr auto
to_range(R&& range)
{
    using Range = tupletools::remove_cvref_t<decltype(range)>;
    using Iterator = range_container_iterator<Range>;
    return range_container<Range, Iterator>(std::forward<Range>(range));
}

constexpr auto transmog = [](auto&& func, auto&& range) {
    using Func = decltype(func);
    using Range = tupletools::remove_cvref_t<decltype(range)>;
    using Iterator = transmog_iterator<Func, Range>;
    return range_container<
        Range,
        Iterator,
        Func>(std::forward<Range>(range), std::forward<Func>(func));
};

constexpr auto transmog_hasty = [](auto&& func, auto&& range) {
    using Func = decltype(func);
    using Range = tupletools::remove_cvref_t<decltype(range)>;
    using IterableValue = tupletools::iterable_t<Range>;

    auto&& vec = itertools::to_vector(std::forward<Range>(range));
    auto&& t_vec =
        std::invoke(std::forward<Func>(func), std::forward<decltype(vec)>(vec));

    return itertools::to_range(std::forward<decltype(t_vec)>(t_vec));
};

constexpr auto filter = [](auto&& func, auto&& range) {
    using Func = decltype(func);
    using Range = tupletools::remove_cvref_t<decltype(range)>;
    using Iterator = filter_iterator<Func, Range>;
    return range_container<
        Range,
        Iterator,
        Func>(std::forward<Range>(range), std::forward<Func>(func));
};

constexpr auto block = [](auto&& range, size_t block_size) {
    using Range = tupletools::remove_cvref_t<decltype(range)>;
    using Iterator = block_iterator<Range>;
    return range_container<
        Range,
        Iterator,
        size_t>(std::forward<Range>(range), block_size);
};

// end non-variadic container iterators.

// Variadic container iterators.
template<class Range, class BeginIter, class EndIter>
class concat_iterator : public range_tuple_iterator<Range, BeginIter, EndIter>
{
public:
    concat_iterator(
        Range&& range,
        BeginIter&& begin_iter,
        EndIter&& end_iter) noexcept
      : range_tuple_iterator<Range, BeginIter, EndIter>(
            std::forward<Range>(range),
            std::forward<BeginIter>(begin_iter),
            std::forward<EndIter>(end_iter))
    {}

    bool is_complete()
    {
        auto b = std::get<0>(this->begin_iter) == std::get<0>(this->end_iter);

        if (b) {
            tupletools::roll(this->begin_iter, true);
            tupletools::roll(this->end_iter, true);

            b = std::get<0>(this->begin_iter) == std::get<0>(this->end_iter);
        }
        this->terminus.complete = b;
        return b;
    }

    auto operator++()
    {
        ++std::get<0>(this->begin_iter);
        this->is_complete();
        return *this;
    }

    auto& operator*() noexcept
    {
        return *std::get<0>(this->begin_iter);
    }
};

template<class... Args>
constexpr auto
zip(Args&&... args)
{
    return range_tuple<range_tuple_iterator, Args...>(
        std::forward<Args>(args)...);
}

template<class... Args>
constexpr auto
zip_ref(Args&&... args)
{
    return range_tuple<range_tuple_iterator_ref, Args...>(
        std::forward<Args>(args)...);
}

template<class T, class... Args>
constexpr auto
concat(T&& arg, Args&&... args)
{
    static_assert(
        tupletools::is_all<T, Args...>::value,
        "All arguments must be of a homogenous type.");

    return range_tuple<
        concat_iterator,
        T,
        Args...>(std::forward<T>(arg), std::forward<Args>(args)...);
}

// end variadic container iterators.

/**
 * @brief Helper function that allows certain iterator modules to be applied to
 * a given range_container.
 *
 * @tparam Func
 * @tparam Piped
 * @param func
 * @param piped
 * @return constexpr auto
 */
template<class Func, class Piped>
constexpr auto
piper(Func&& func, Piped piped)
{
    return [func = std::forward<Func>(func),
            piped = std::forward<Piped>(piped)](auto&& v) {
        return piped(func, v);
    };
}

template<class Range>
constexpr auto
sort(Range&& range)
{
    return itertools::to_range(range) | itertools::piper(
                                            [](auto&& v) {
                                                std::sort(begin(v), end(v));
                                                return v;
                                            },
                                            itertools::transmog_hasty);
}

/**
 * @brief Spawns values using the input range as a source. Each time it's
 * called, range's iterator is incremented. If its reached the end of range's
 * iterator, it shall return an empty, falsy, optional.
 *
 * @tparam Range
 * @tparam tupletools::iterable_t<Range>
 * @param range
 * @return constexpr auto
 */
template<class Range, class IterableValue = tupletools::iterable_t<Range>>
constexpr auto
spawn(Range&& range)
{
    return [bg = range.begin(), ed = range.end()](
               auto&&... args) mutable -> std::optional<IterableValue> {
        if (!(bg == ed)) {
            ++bg;
            return *bg;
        }
        return {};
    };
}

template<typename T = size_t>
class range;

template<typename T = size_t>
class range_iterator : public std::iterator<std::forward_iterator_tag, T>
{
public:
    using iterator_category = std::forward_iterator_tag;

    constexpr explicit range_iterator(range<T>& seq)
      : _seq(seq)
    {}

    constexpr auto operator++()
    {
        _seq._current += _seq._stride;
        return *this;
    }

    constexpr bool operator==(T rhs)
    {
        return _seq._current == rhs;
    }

    constexpr bool operator!=(T rhs)
    {
        return !(*this == rhs);
    }

    constexpr const T& operator*() noexcept
    {
        return _seq._current;
    }

private:
    range<T> _seq;
};

template<typename T>
class range
{
public:
    using iterator = range_iterator<T>;

    constexpr explicit range(T stop)
      : _start(0)
      , _stop(stop)
    {
        if (_start > _stop) {
            std::swap(_start, _stop);
        }
        _stride = _start > _stop ? -1 : 1;
        _current = _start;
    }

    constexpr range(T start, T stop)
      : _start(start)
      , _stop(stop)
    {
        _stride = _start > _stop ? -1 : 1;
        _current = _start;
    }

    constexpr range(T start, T stop, T stride)
      : _start(start)
      , _stop(stop)
      , _stride(stride)
      , _current(_start)
    {}

    T start()
    {
        return _start;
    }
    T stop()
    {
        return _stop;
    }
    T stride()
    {
        return _stride;
    }
    T& current()
    {
        return _current;
    }
    size_t size() const
    {
        return _size;
    }

    iterator begin()
    {
        return iterator(*this);
    }
    T end()
    {
        return _stop;
    }

private:
    friend class range_iterator<T>;
    T _start, _stop, _stride, _current;
    size_t _size = 0;
};

template<class Iterable>
constexpr auto
enumerate(Iterable&& iter)
{
    auto _range = range(std::numeric_limits<size_t>::max());
    return zip(
        std::forward<decltype(_range)>(_range), std::forward<Iterable>(iter));
}

template<class Iterable>
constexpr auto
enumerate_ref(Iterable&& iter)
{
    auto _range = range(std::numeric_limits<size_t>::max());
    return zip_ref(
        std::forward<decltype(_range)>(_range), std::forward<Iterable>(iter));
}

/*

Coroutine generator function. Works with both negative and positive stride
values.

@param start: T starting value.
@param stop: T stopping value.
@param stride: T stride value whereof start is incremented.

@co_yield: incremented starting value of type T.
 */
template<typename T = size_t>
generator<T>
grange(T start, T stop, T stride = 1)
{
    stride = start > stop ? -1 : 1;
    do {
        co_yield start;
        start += stride;
    } while (start < stop);
}

template<typename T = size_t>
generator<T>
grange(T stop)
{
    T start = 0;
    if (start > stop) {
        std::swap(start, stop);
    }
    return grange<T>(start, stop);
}

} // namespace itertools
#endif // RANGE_CONTAINER_H