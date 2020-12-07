#ifndef ITERTOOLS_H
#define ITERTOOLS_H

#include "itertools/range_container.hpp"
#include "itertools/tupletools.hpp"
#include "itertools/types.hpp"

#include <algorithm>
#include <chrono>
#include <functional>
#include <map>
#include <numeric>
#include <optional>
#include <sstream>
#include <vector>

#pragma once

namespace itertools {
/*
High order functor, whereof: applies a binary function of type BinaryFunction,
func, over an iterable range of type Iterable, iter. func must accept two
values of (size_t, IterableValue), wherein IterableValue is the
iterable's container value.

@param iter: iterable function of type Iterable.
@param func: low order functor of type BinaryFunction; returns a boolean.

@returns iter
 */
template<
    class Iterable,
    class BinaryFunction,
    class IterableValue = tupletools::iterable_t<Iterable>>
constexpr Iterable
for_each(Iterable&& iter, BinaryFunction&& func)
{
    for (auto&& [n, i] : enumerate(iter)) {
        std::invoke(
            std::forward<BinaryFunction>(func),
            std::forward<decltype(n)>(n),
            std::forward<decltype(i)>(i));
    }
    return iter;
}

template<
    class Iterable,
    class BinaryFunction,
    class IterableValue = tupletools::iterable_t<Iterable>>
constexpr Iterable
map(Iterable&& iter, BinaryFunction&& func)
{
    for (auto&& [n, i] : enumerate(iter)) {
        std::optional<IterableValue> opt = std::invoke(
            std::forward<BinaryFunction>(func),
            std::forward<decltype(n)>(n),
            std::forward<decltype(i)>(i));
        if (opt) {
            iter[n] = *opt;
        } else {
            break;
        }
    }
    return iter;
}

template<typename ReductionValue = int, class Iterable, class NaryFunction>
constexpr ReductionValue
reduce(Iterable&& iter, ReductionValue init, NaryFunction&& func)
{
    itertools::for_each(iter, [&](auto n, auto&& v) {
        init = func(n, std::forward<decltype(v)>(v), init);
        return false;
    });
    return init;
}

template<typename ReductionValue, class Iterable>
constexpr ReductionValue
sum(Iterable&& iter)
{
    return itertools::reduce<ReductionValue>(iter, 0, [](auto n, auto v, auto i) {
        return i + v;
    });
}

template<typename ReductionValue, class Iterable>
constexpr ReductionValue
mul(Iterable&& iter)
{
    return itertools::reduce<ReductionValue>(iter, 1, [](auto n, auto v, auto i) {
        return i * v;
    });
}

template<class Iterable>
constexpr Iterable
swap(Iterable&& iter, int ix1, int ix2)
{
    auto t = iter[ix1];
    iter[ix1] = iter[ix2];
    iter[ix2] = t;
    return iter;
}

template<class Iterable>
constexpr Iterable
roll(Iterable&& iter, int axis = -1)
{
    if (axis == 0) {
        return iter;
    } else if (axis == -1) {
        axis = iter.size() - 1;
    }
    int i = 0;
    while (i < axis) {
        itertools::swap(std::forward<Iterable>(iter), axis, i++);
    };
    return iter;
}

template<class Func>
struct y_combinator
{
    Func func;
    y_combinator(Func func)
      : func(std::move(func))
    {}
    template<class... Args>
    auto operator()(Args&&... args) const
    {
        return func(std::ref(*this), std::forward<Args>(args)...);
    }
    template<class... Args>
    auto operator()(Args&&... args)
    {
        return func(std::ref(*this), std::forward<Args>(args)...);
    }
};

template<class... Funcs, const size_t N = sizeof...(Funcs)>
auto
time_multiple(size_t iterations, Funcs&&... funcs)
{
    using namespace std::chrono;
    using integral_time_t = decltype(std::declval<microseconds>().count());

    std::map<int, std::vector<microseconds>> times;
    std::map<int, std::vector<integral_time_t>> extremal_times;

    for (auto i : range(N)) {
        times[i].reserve(iterations);
    }

    auto tup = std::make_tuple(std::forward<Funcs>(funcs)...);

    auto func = [&](auto&& n, auto&& v) {
        auto start = high_resolution_clock::now();
        v();
        auto stop = high_resolution_clock::now();
        auto time = duration_cast<microseconds>(stop - start);
        times[n].push_back(time);
        return false;
    };

    for (auto i : range(iterations)) {
        tupletools::for_each(tup, func);
    }

    itertools::for_each(times, [&](auto&& n, auto&& v) {
        auto [key, value] = v;
        std::sort(value.begin(), value.end());

        integral_time_t avg =
            itertools::reduce<integral_time_t>(
                value, 0, [](auto n, auto v, auto i) { return v.count() + i; }) /
            iterations;

        extremal_times[key] = {value[0].count(), value[value.size() - 1].count(), avg};
        return false;
    });
    return std::make_tuple(times, extremal_times);
}

namespace detail {

struct get_ndim_impl
{
    size_t _ndim = 0;

    template<
        class Iterable,
        std::enable_if_t<tupletools::is_iterable_v<Iterable>, int> = 0>
    constexpr void recurse(Iterable&& iter)
    {
        _ndim = 0;
        auto it = std::begin(iter);
        ++it;
        recurse(std::forward<decltype(*it)>(*it));
        _ndim++;
    }

    template<class Tup, std::enable_if_t<tupletools::is_tupleoid_v<Tup>, int> = 0>
    constexpr void recurse(Tup&& tup)
    {
        tupletools::for_each(tup, [&](auto&& n, auto&& v) {
            recurse(std::forward<decltype(v)>(v));
        });
    }

    template<
        class Iterable,
        std::enable_if_t<
            !tupletools::is_iterable_v<Iterable> &&
                !tupletools::is_tupleoid_v<Iterable>,
            int> = 0>
    constexpr void recurse(Iterable&& iter)
    {}

    template<class Iterable>
    constexpr size_t operator()(Iterable&& iter)
    {
        recurse(std::forward<Iterable>(iter));
        return _ndim + 1;
    }
};
}; // namespace detail

template<class Iterable>
constexpr size_t
get_ndim(Iterable&& iter)
{
    return detail::get_ndim_impl{}(std::forward<Iterable>(iter));
}

template<typename Char, typename Traits, typename Allocator>
std::basic_string<Char, Traits, Allocator>
operator*(const std::basic_string<Char, Traits, Allocator> s, size_t n)
{
    std::basic_string<Char, Traits, Allocator> tmp = "";
    for (auto i : range(n)) {
        tmp += s;
    }
    return tmp;
}

template<typename Char, typename Traits, typename Allocator>
std::basic_string<Char, Traits, Allocator>
operator*(size_t n, const std::basic_string<Char, Traits, Allocator>& s)
{
    return s * n;
}

template<typename Iterable>
std::string
join(Iterable&& iter, std::string&& sep)
{
    std::ostringstream result;
    for (auto&& [n, i] : enumerate(iter)) {
        result << (n > 0 ? sep : "") << i;
    }
    return result.str();
}

std::string
ltrim(std::string s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
                return !std::isspace(ch);
            }));
    return s;
}

std::string
rtrim(std::string s)
{
    s.erase(
        std::find_if(s.rbegin(), s.rend(), [](int ch) { return !std::isspace(ch); })
            .base(),
        s.end());
    return s;
}

std::string
trim(std::string s)
{
    ltrim(s);
    rtrim(s);
    return s;
}

namespace detail {

constexpr int
bidirectional_match(
    const std::string& buff,
    const std::string& token,
    size_t pos = 0,
    bool backwards = false)
{
    int neg = 1;
    int end = buff.size();
    int token_begin = 0;
    int token_end = token.size() - 1;
    int t_begin = 0;

    if (backwards) {
        neg = -1;
        end = 0;
        token_begin = token.size() - 1;
        token_end = -1;
    }
    t_begin = token_begin;

    while (pos != end) {
        if (buff[pos] == token[t_begin]) {
            t_begin += neg;
        } else {
            t_begin = token_begin;
        }
        if (t_begin == token_end) {
            return pos;
        }
        pos += neg;
    }
    return -1;
}

std::string
summarize_string(
    const std::string& buff,
    const std::string& sep,
    size_t max_items = 3,
    size_t max_len = 80)
{
    if (buff.size() < max_len) {
        return buff;
    }
    int pos = 0;
    size_t max_len2 = max_len / 2;
    size_t i = 0;
    size_t begin = 0;
    size_t end = buff.size() - 1;
    while ((i++ < max_items) && (begin < max_len2)) {
        pos = bidirectional_match(buff, sep, pos, false);
        if (pos == -1) {
            break;
        } else {
            pos += sep.size() - 1;
            begin = pos;
        }
    }
    i = 0;
    pos = buff.size() - 1;
    while ((i++ < max_items) && (buff.size() - (end + 1) < max_len2)) {
        pos = bidirectional_match(buff, sep, pos, true);
        if (pos == -1) {
            break;
        } else {
            pos -= sep.size();
            end = pos;
        }
    }
    if (begin == 0 || end == buff.size() - 1) {
        return buff;
    } else {
        std::string summary =
            buff.substr(0, begin - 1) + " ... " + buff.substr(end, buff.size() - 1);
        return summary;
    }
}

template<class Formatter>
struct to_string_impl
{
    Formatter _formatter;
    size_t _prev_ix, _prev_len, _ndim, _max_len = 80;
    std::string _sep, _trim_sep;

    template<class Iterable>
    explicit constexpr to_string_impl(
        Iterable&& iter,
        Formatter&& formatter,
        std::string& sep)

      : _formatter{formatter}
      , _sep{sep}
      , _trim_sep{trim(sep)}
    {
        _ndim = get_ndim(iter);
    };

    template<class Iterable>
    std::string operator()(Iterable&& iter)
    {
        return recurse(std::forward<Iterable>(iter), 0);
    }

    constexpr size_t get_lastline_size(std::string& buff)
    {
        size_t line_size = 0;
        int i = buff.size() - 1;
        while (i-- > 0) {
            if (buff[i] == '\n') {
                return line_size;
            } else {
                line_size++;
            }
        }
        return line_size;
    }

    std::string format_newline(std::string& buff, size_t line_count)
    {
        std::string new_line = std::string("\n") * (line_count < 0 ? 0 : line_count);
        std::string spacing = _trim_sep + new_line;

        buff += spacing;
        _prev_len = spacing.size();
        return buff;
    }

    std::string format_indent(std::string& buff)
    {
        int i = 0;
        int N = buff.size();
        while (i++ < N) {
            if (i < N - 1) {
                if (buff[i] == '\n' && buff[i + 1] != '\n') {
                    buff.insert(i + 1, " ");
                    N++;
                }
            }
        }
        return buff;
    }

    template<
        class Tup,
        std::enable_if_t<tupletools::is_tupleoid_v<Tup>, int> = 0,
        const size_t N = tupletools::tuple_size_v<Tup>>
    std::string formatter_wrap(Tup&& tup, size_t ix)
    {
        std::string buff;
        std::string t_buff;
        bool nested_tupleoid = false;

        tupletools::for_each(std::forward<Tup>(tup), [&](auto n, auto&& iter_n) {
            t_buff = to_string_impl{
                std::forward<decltype(iter_n)>(iter_n),
                std::forward<Formatter>(_formatter),
                _sep}(std::forward<decltype(iter_n)>(iter_n));

            if (n < N - 1) {
                t_buff += _sep;

                if (nested_tupleoid || is_iterable_v<decltype(iter_n)> ||
                    is_tupleoid_v<decltype(iter_n)>) {
                    t_buff += '\n';
                    nested_tupleoid = true;
                }
            }

            buff += t_buff;
        });

        buff = "(" + format_indent(buff) + ")";
        if (ix > 0) {
            format_newline(buff, 2);
            _prev_ix = ix + 1;
        }
        return buff;
    }

    template<
        class Iterable,
        std::enable_if_t<!tupletools::is_tupleoid_v<Iterable>, int> = 0>
    std::string formatter_wrap(Iterable&& iter, size_t ix)
    {
        return std::
            invoke(std::forward<Formatter>(_formatter), std::forward<Iterable>(iter));
    }

    template<
        class Iterable,
        std::enable_if_t<!tupletools::is_iterable_v<Iterable>, int> = 0>
    std::string recurse(Iterable&& iter, size_t ix)
    {
        return formatter_wrap(std::forward<Iterable>(iter), ix);
    }

    template<
        class Iterable,
        std::enable_if_t<tupletools::is_iterable_v<Iterable>, int> = 0>
    std::string recurse(Iterable&& iter, size_t ix)
    {
        _prev_ix = ix;
        std::string buff = "";
        size_t ndim = iter.size();

        auto n = 0;
        for (auto&& iter_n : iter) {
            buff += recurse(std::forward<decltype(iter_n)>(iter_n), ix + 1);

            if (!is_iterable_v<decltype(iter_n)> && !is_tupleoid_v<decltype(iter_n)>) {
                buff += n < ndim - 1 ? _sep : "";
            }

            n++;
        };

        buff = _prev_ix > ix ? buff.substr(0, buff.size() - _prev_len) : buff;

        buff = "[" + format_indent(buff) + "]";
        if (ix > 0) {
            format_newline(buff, _ndim - ix);
            buff = summarize_string(buff, _sep, 80);
        }
        return buff;
    };
};
}; // namespace detail

template<class Iterable, class Formatter>
std::string
to_string_f(Iterable&& iter, Formatter&& formatter, std::string&& sep = ", ")
{
    return detail::to_string_impl{
        std::forward<Iterable>(iter), std::forward<Formatter>(formatter), sep}(iter);
}

template<class Iterable>
std::string
to_string(Iterable&& iter)
{
    std::string sep = ", ";
    auto formatter = [](auto&& s) -> std::string { return std::to_string(s); };

    return detail::to_string_impl{
        std::forward<Iterable>(iter),
        std::forward<decltype(formatter)>(formatter),
        sep}(iter);
}
};     // namespace itertools
#endif // ITERTOOLS_H