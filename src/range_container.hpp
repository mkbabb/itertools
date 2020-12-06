#ifndef RANGE_CONTAINER_H
#define RANGE_CONTAINER_H

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
    class BeginIterator = decltype(std::declval<Iterator>().begin_it),
    class EndIterator = decltype(std::declval<Iterator>().end_it)>
class range_container
{
public:
    size_t _size;

    range_container(Range&& range, BeginIterator&& begin_it, EndIterator&& end_it)
      : iter{std::forward<Range>(range), std::forward<BeginIterator>(begin_it), std::forward<EndIterator>(end_it)}
      , range{range}
      , _size{1}
    {}

    range_container(Range&& range, Iterator&& iter)
      : iter{std::forward<Iterator>(iter)}
      , range{range}
      , _size{1}
    {}

    Iterator begin()
    {
        return iter;
    }

    auto end()
    {
        return range_container_terminus{true};
    }

    size_t size()
    {
        return _size;
    }

    template<class Tfunc>
    auto operator|(Tfunc&& tfunc)
    {
        return tfunc(*this);
    }

    Iterator iter;
    Range range;
};

template<template<typename... Ts> class Iterator, class... Args>
class range_tuple
  : public range_container<
        std::tuple<Args...>,
        Iterator<
            std::tuple<Args...>,
            std::tuple<decltype(std::declval<Args>().begin())...>,
            std::tuple<decltype(std::declval<Args>().end())...>>>
{
    static constexpr size_t N = sizeof...(Args);
    static_assert(N > 0, "Zip argument count must be larger than 0.");

public:
    range_tuple(Args&&... args)
      : range_container<
            std::tuple<Args...>,
            Iterator<
                std::tuple<Args...>,
                std::tuple<decltype(std::declval<Args>().begin())...>,
                std::tuple<decltype(std::declval<Args>().end())...>>>(
            std::forward_as_tuple(args...),
            std::forward_as_tuple(args.begin()...),
            std::forward_as_tuple(args.end()...))
    {}
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
      : begin_it{std::begin(range)}
      , end_it{std::end(range)}
      , terminus{false}
    {}

    range_container_iterator(
        Range&& range,
        BeginIter&& begin_it,
        EndIter&& end_it) noexcept
      : begin_it{begin_it}
      , end_it{end_it}
      , terminus{false}
    {}

    bool is_complete()
    {
        auto complete = this->begin_it == this->end_it;
        this->terminus.complete = complete;
        return complete;
    }

    auto operator++()
    {
        ++this->begin_it;
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
        return *(this->begin_it);
    }

    auto operator->() noexcept
    {
        return this->begin_it;
    }

    BeginIter begin_it;
    EndIter end_it;
};

template<class Range, class BeginIter, class EndIter>
class range_tuple_iterator : public range_container_iterator<Range, BeginIter, EndIter>
{
public:
    range_tuple_iterator(Range&& range, BeginIter&& begin_it, EndIter&& end_it)
      : range_container_iterator<Range, BeginIter, EndIter>(
            std::forward<Range>(range),
            std::forward<BeginIter>(begin_it),
            std::forward<EndIter>(end_it))
    {}

    bool is_complete()
    {
        auto complete = tupletools::any_where(
            [](auto&& x, auto&& y) { return x == y; }, this->begin_it, this->end_it);
        this->terminus.complete = complete;
        return complete;
    }

    auto operator++()
    {
        tupletools::for_each(this->begin_it, [](auto n, auto&& v) { ++v; });
        this->is_complete();
        return *this;
    }

    auto operator*() noexcept
    {
        auto func = [](auto&& v) -> decltype(*v) { return *v; };
        return transform_copy(func, this->begin_it);
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
            ret.push_back(*(this->begin_it));
            ++(this->begin_it);
        }
        return ret;
    }

    size_t block_size;
};

template<class Range, class Pred>
class filter_iterator : public range_container_iterator<Range>
{
public:
    using IterableValue = tupletools::iterable_t<Range>;

    filter_iterator(Range&& range, Pred&& pred) noexcept
      : range_container_iterator<Range>(std::forward<Range>(range))
      , pred{std::forward<Pred>(pred)}
    {}

    auto operator++()
    {
        ++(this->begin_it);

        while (!this->is_complete() &&
               !this->pred(std::forward<IterableValue>(*this->begin_it))) {
            ++(this->begin_it);
        };

        return *this;
    }

    auto operator*()
    {
        auto to_filter = !this->pred(std::forward<IterableValue>(*this->begin_it));

        if (to_filter) {
            ++(*this);
        }

        return *(this->begin_it);
    }

    Pred pred;
};

template<class Range, class Func>
class transform_iterator : public range_container_iterator<Range>
{
public:
    using IterableValue = tupletools::iterable_t<Range>;

    transform_iterator(Range&& range, Func&& func) noexcept
      : range_container_iterator<Range>(std::forward<Range>(range))
      , func{func}
    {}

    auto operator*() noexcept
    {
        return this->func(std::forward<IterableValue>(*this->begin_it));
    }
    Func func;
};

template<typename R>
constexpr auto
to_range(R&& range)
{
    using Range = tupletools::remove_cvref_t<decltype(range)>;
    auto&& it = range_container_iterator<Range>(range);

    using Iterator = tupletools::remove_cvref_t<decltype(it)>;

    return range_container<
        Range,
        Iterator>(std::forward<Range>(range), std::forward<Iterator>(it));
}

constexpr auto transform = [](auto&& func, auto&& range) {
    using Func = tupletools::remove_cvref_t<decltype(func)>;
    using Range = tupletools::remove_cvref_t<decltype(range)>;

    auto iter =
        transform_iterator(std::forward<Range>(range), std::forward<Func>(func));
    using Iterator = tupletools::remove_cvref_t<decltype(iter)>;

    return range_container<
        Range,
        Iterator>(std::forward<Range>(range), std::forward<Iterator>(iter));
};

constexpr auto filter = [](auto&& func, auto&& range) {
    using Func = tupletools::remove_cvref_t<decltype(func)>;
    using Range = tupletools::remove_cvref_t<decltype(range)>;

    auto iter = filter_iterator(std::forward<Range>(range), std::forward<Func>(func));
    using Iterator = tupletools::remove_cvref_t<decltype(iter)>;

    return range_container<
        Range,
        Iterator>(std::forward<Range>(range), std::forward<Iterator>(iter));
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
    concat_iterator(Range&& range, BeginIter&& begin_it, EndIter&& end_it) noexcept
      : range_tuple_iterator<Range, BeginIter, EndIter>(
            std::forward<Range>(range),
            std::forward<BeginIter>(begin_it),
            std::forward<EndIter>(end_it))
    {}

    bool is_complete()
    {
        auto complete = std::get<0>(this->begin_it) == std::get<0>(this->end_it);

        if (complete) {
            tupletools::roll(this->begin_it, true);
            tupletools::roll(this->end_it, true);

            complete = std::get<0>(this->begin_it) == std::get<0>(this->end_it);
        }
        this->terminus.complete = complete;
        return complete;
    }

    auto operator++()
    {
        ++std::get<0>(this->begin_it);
        this->is_complete();
        return *this;
    }

    auto& operator*() noexcept
    {
        return *std::get<0>(this->begin_it);
    }
};

template<class... Args>
constexpr auto
zip(Args&&... args)
{
    return range_tuple<range_tuple_iterator, Args...>(std::forward<Args>(args)...);
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
    auto rng = range(std::numeric_limits<size_t>::max());
    return zip(std::forward<decltype(rng)>(rng), std::forward<Iterable>(iter));
}

} // namespace itertools
#endif // RANGE_CONTAINER_H