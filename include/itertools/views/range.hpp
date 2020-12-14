#pragma once

namespace itertools { namespace views {

template<typename T = int>
class range;

template<typename T = int>
class range_iterator
{
public:
    explicit range_iterator(range<T>& seq)
      : seq(seq)
    {}

    auto operator++()
    {
        seq.current += seq.stride;
        return *this;
    }

    bool operator==(T rhs)
    {
        return seq.current == rhs;
    }

    decltype(auto) operator*() const
    {
        return seq.current;
    }

    range<T> seq;
};

template<typename T>
class range
{
public:
    using iterator = range_iterator<T>;

    constexpr explicit range(T stop)
      : start(0)
      , stop(stop)
    {
        if (start > stop) {
            std::swap(start, stop);
        }
        stride = start > stop ? -1 : 1;
        current = start;
    }

    constexpr range(T start, T stop)
      : start(start)
      , stop(stop)
    {
        stride = start > stop ? -1 : 1;
        current = start;
    }

    constexpr range(T start, T stop, T stride)
      : start(start)
      , stop(stop)
      , stride(stride)
      , current(start)
    {}

    int size() const
    {
        return size_;
    }

    iterator begin()
    {
        return iterator(*this);
    }

    T end()
    {
        return stop;
    }

    T start, stop, stride;
    T current;
    size_t size_ = 0;
};

}}