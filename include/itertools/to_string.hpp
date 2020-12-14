#include "itertools.hpp"
#include "tupletools.hpp"

#pragma once

namespace itertools {
namespace detail {

using namespace std::string_literals;
using namespace tupletools;

struct get_ndim_impl
{
    int ndim = 0;

    template<Rangeable Iter>
    constexpr void recurse(Iter&& iter)
    {
        ndim = 0;
        recurse(*(++iter.begin()));
        ndim += 1;
    }

    template<Tupleoid Tup>
    constexpr void recurse(Tup&& tup)
    {
        for_each(tup, [this]<class T>(auto&& n, T&& i) {
            recurse(std::forward<T>(i));
        });
        ndim += 1;
    }

    template<class T>
    requires(!Rangeloid<T>) constexpr void recurse(T&& x)
    {}

    template<class Iter>
    constexpr int operator()(Iter&& iter)
    {
        recurse(std::forward<Iter>(iter));
        return ndim;
    }
};

template<class Iter>
constexpr int
get_ndim(Iter&& iter)
{
    return detail::get_ndim_impl{}(std::forward<Iter>(iter));
}

template<class Formatter>
struct to_string_impl
{

    Formatter formatter;
    int ndim;
    int offset;
    std::string sep;
    std::string tmp;

    template<class Iter>
    explicit to_string_impl(
        Iter&& iter,
        Formatter&& formatter,
        std::string& sep,
        int offset = 0)
      : formatter{formatter}
      , sep{sep}
      , ndim{std::max(0, get_ndim(iter) - 1)}
      , offset{offset} {};

    template<class Iter>
    std::string operator()(Iter&& iter)
    {
        return recurse(std::forward<Iter>(iter), 0);
    }

    decltype(auto) create_hanging_indent(int ix)
    {
        auto space_count = ndim > 0 ? std::max(0, ix + offset + 1) : 0;
        std::string spaces(space_count, ' ');

        std::string new_lines(std::max(0, ndim - ix), '\n');

        return new_lines + spaces;
    }

    template<class T>
    requires(!Rangeloid<T>) std::string recurse(T&& x, int ix)
    {
        return formatter(std::forward<T>(x));
    }

    template<Tupleoid Tup>
    std::string recurse(Tup tup, int ix)
    {
        std::string buff = "";
        auto hanging_indent = create_hanging_indent(ix);

        auto size = tuple_size_v<Tup>;
        for_each(std::forward<Tup>(tup), [&, this]<class T>(auto n, T&& i) {
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

    template<Rangeable Iter>
    std::string recurse(Iter&& iter, int ix)
    {
        std::string buff = "";
        auto hanging_indent = create_hanging_indent(ix);

        for (auto&& [n, i] : views::enumerate(iter)) {
            using T = std::remove_cvref_t<decltype(i)>;

            if constexpr (Rangeloid<T>) {
                auto t_buff = recurse(i, ix + 1);

                buff += (n > 0) ? hanging_indent + t_buff : t_buff;
            } else {
                buff += formatter(i);
            }

            if (n < iter.size() - 1) {
                buff += sep;
            }
        };

        return "["s + buff + "]"s;
    };
};
};

template<class Iter, class Formatter>
std::string
to_string_f(Iter&& iter, Formatter&& formatter, std::string sep = ", ")
{
    return detail::to_string_impl{
        std::forward<Iter>(iter), std::forward<Formatter>(formatter), sep}(iter);
}

template<class Iter>
std::string
to_string(Iter&& iter)
{
    std::string sep = ", ";
    auto formatter = []<class T>(T&& s) -> std::string {
        return fmt::format("{}", std::forward<T>(s));
    };

    return detail::to_string_impl{std::forward<Iter>(iter), std::move(formatter), sep}(
        std::forward<Iter>(iter));
}

}