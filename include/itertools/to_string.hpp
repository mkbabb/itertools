#include "itertools.hpp"

#pragma once

namespace itertools {
namespace detail {

int
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
        std::string new_line = std::string("\n", line_count < 0 ? 0 : line_count);
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
}