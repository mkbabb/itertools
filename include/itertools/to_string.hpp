#include "itertools.hpp"

#pragma once

namespace itertools {
namespace detail {

using namespace std::string_literals;
namespace tt = tupletools;

struct get_ndim_impl
{
    int ndim = 0;

    template<class Iterable, std::enable_if_t<tt::is_iterable_v<Iterable>, int> = 0>
    constexpr void recurse(Iterable&& iter)
    {
        ndim = 0;
        recurse(*(++iter.begin()));
        ndim += 1;
    }

    template<class Tup, std::enable_if_t<tt::is_tupleoid_v<Tup>, int> = 0>
    constexpr void recurse(Tup&& tup)
    {
        tt::for_each(tup, [this]<class T>(auto&& n, T&& i) {
            recurse(std::forward<T>(i));
        });
        ndim += 1;
    }

    template<
        class Iterable,
        std::enable_if_t<
            !(tt::is_iterable_v<Iterable> or tt::is_tupleoid_v<Iterable>),
            int> = 0>
    constexpr void recurse(Iterable&& iter)
    {}

    template<class Iterable>
    constexpr int operator()(Iterable&& iter)
    {
        recurse(std::forward<Iterable>(iter));
        return ndim;
    }
};

template<class Iterable>
constexpr int
get_ndim(Iterable&& iter)
{
    return detail::get_ndim_impl{}(std::forward<Iterable>(iter));
}

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

    Formatter formatter;
    int ndim;
    int offset;
    std::string sep;
    std::string tmp;

    template<class Iterable>
    explicit constexpr to_string_impl(
        Iterable&& iter,
        Formatter&& formatter,
        std::string& sep,
        int offset = 0)
      : formatter{formatter}
      , sep{sep}
      , ndim{std::max(0, get_ndim(iter) - 1)}
      , offset{offset} {};

    template<class Iterable>
    std::string operator()(Iterable&& iter)
    {
        return recurse(std::forward<Iterable>(iter), 0);
    }

    template<
        class Tup,
        std::enable_if_t<!(tt::is_tupleoid_v<Tup> or tt::is_iterable_v<Tup>), int> = 0>
    std::string recurse(Tup&& tup, int ix)
    {
        return formatter(std::forward<Tup>(tup));
    }

    decltype(auto) create_hanging_indent(int ix)
    {
        auto space_count = ndim > 0 ? std::max(0, ix + offset + 1) : 0;
        std::string spaces(space_count, ' ');

        std::string new_lines(std::max(0, ndim - ix), '\n');

        return new_lines + spaces;
    }

    template<class Tup, std::enable_if_t<tt::is_tupleoid_v<Tup>, int> = 0>
    std::string recurse(Tup&& tup, int ix)
    {
        std::string buff = "";
        auto hanging_indent = create_hanging_indent(ix);

        auto size = tt::tuple_size_v<Tup>;
        tt::for_each(std::forward<Tup>(tup), [&, this]<class T>(auto n, T&& i) {
            auto t_buff = to_string_impl{
                std::forward<T>(i),
                std::forward<Formatter>(formatter),
                sep,
                ix + 1}(std::forward<T>(i));

            buff += (n > 0) ? hanging_indent + t_buff : t_buff;

            if (n < size - 1) {
                buff += sep;
            }
        });

        return "("s + buff + ")"s;
    }

    template<class Iterable, std::enable_if_t<tt::is_iterable_v<Iterable>, int> = 0>
    std::string recurse(Iterable&& iter, int ix)
    {
        std::string buff = "";
        auto hanging_indent = create_hanging_indent(ix);

        auto n = 0;
        for (auto [i] : views::zip(iter)) {
            using T = decltype(i);

            if constexpr (is_iterable_v<T> or is_tupleoid_v<T>) {
                auto t_buff = recurse(std::forward<T>(i), ix + 1);

                buff += (n > 0) ? hanging_indent + t_buff : t_buff;
            } else {
                buff += formatter(std::forward<T>(i));
            }

            if (n < iter.size() - 1) {
                buff += sep;
            }
            n += 1;
        };

        return "["s + buff + "]"s;
    };
};
}; // namespace detail

template<class Iterable, class Formatter>
std::string
to_string_f(Iterable&& iter, Formatter&& formatter, std::string sep = ", ")
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

    return detail::
        to_string_impl{std::forward<Iterable>(iter), std::move(formatter), sep}(iter);
}
}