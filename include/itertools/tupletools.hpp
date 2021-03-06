#ifndef TUPLETOOLS_H
#define TUPLETOOLS_H

#include "types.hpp"

#include <algorithm>
#include <functional>
#include <string>
#include <tuple>
#include <type_traits>

#pragma once

namespace tupletools {

template<typename T>
constexpr std::reference_wrapper<T>
copy_if_rvalue(T& t)
{
    return t;
}

template<typename T>
constexpr T
copy_if_rvalue(T&& t)
{
    return std::move(t);
}

// The below three functions are drivers behind one's basic argument
// forwarding of a tuple.

template<size_t... Ixs, class Func>
constexpr decltype(auto)
index_apply_impl(Func&& func, std::index_sequence<Ixs...>)
{
    return std::invoke(std::forward<Func>(func),
                       std::integral_constant<size_t, Ixs>{}...);
}

template<size_t N, class Func>
constexpr decltype(auto)
index_apply(Func&& func)
{
    return index_apply_impl(std::forward<Func>(func), std::make_index_sequence<N>{});
}

/*
Essentially a clone of std::apply; an academic exercise using expression
folding.
 */
template<class Tup, class Func, const size_t N = tuple_size<Tup>::value>
constexpr decltype(auto)
apply(Func&& func, Tup&& tup)
{
    return index_apply<N>([&tup, func = std::forward<Func>(func)](auto... Ixs) {
        return std::invoke(func, std::get<Ixs>(std::forward<Tup>(tup))...);
    });
}

/*
Begets a tuple of size N of type T, filled with value value.

@param value: fill value

@returns: tuple filled N times with value.
 */
template<const size_t N, class T>
constexpr decltype(auto)
make_tuple_of(T value)
{
    auto func = [=](auto) { return value; };
    auto tup = index_apply<N>([=](auto... Ixs) { return std::make_tuple(Ixs...); });

    return index_apply<N>(
      [func = std::move(func)](auto... Ixs) { return std::make_tuple(func(Ixs)...); });
}

/*
High order functor that allows for a functor of type "Func" to be applied
element-wise to a tuple of type "Tup". "func" must accept two parameters of
types (size_t, tup_i).

Each item within "tup" is iterated upon, and must therefor each element must be
of a coalesceable type; the aforesaid elements must be of a type that allows
proper type coalescence.

@param tup: tuple of coalesceable type.
@param func:    high order function that takes two arguments of types
                (size_t,tup_i), wherein "tup_i" is a coalescably-typed element
                of "tup"

@returns: string representation of tup.
 */

template<class Tup, class Func, const size_t N = tuple_size_v<Tup>>
constexpr void
for_each(Tup&& tup, Func&& func)
{
    index_apply<N>([&tup, func = std::forward<Func>(func)](auto... Ixs) {
        (std::invoke(func, Ixs, std::get<Ixs>(std::forward<Tup>(tup))), ...);
    });
    return;
}

template<class Func, class Tup>
constexpr decltype(auto)
transform(Func&& func, Tup&& tup)
{
    auto f = [func = std::forward<Func>(func)]<class... Args>(Args && ... args)
    {
        return std::make_tuple(std::invoke(func, std::forward<Args>(args))...);
    };
    return std::apply(f, std::forward<Tup>(tup));
}

template<class Tup, const size_t N = tuple_size<Tup>::value>
constexpr auto
reverse(Tup&& tup)
{
    return index_apply<N>([&tup](auto... Ixs) {
        return std::make_tuple(std::get<N - (Ixs + 1)>(std::forward<Tup>(tup))...);
    });
}

template<int Ix1, int Ix2, class Tup>
constexpr void
swap(Tup&& tup)
{
    std::swap(std::get<Ix1>(std::forward<Tup>(tup)),
              std::get<Ix2>(std::forward<Tup>(tup)));
}

template<const bool reverse, class Tup, const size_t N = tuple_size<Tup>::value>
constexpr void
roll(Tup&& tup)
{
    tupletools::for_each(std::forward<Tup>(tup), [&tup](auto n, auto&&) {
        if constexpr (reverse) {
            swap<0, N - (n + 1)>(std::forward<Tup>(std::forward<Tup>(tup)));
        } else {
            swap<n, N - 1>(std::forward<Tup>(std::forward<Tup>(tup)));
        }
    });
}

template<class... Tups, const size_t N = std::min({ std::tuple_size<Tups>{}... })>
constexpr auto
transpose(Tups&&... tups)
{
    auto row = [&](auto Ixs) {
        return std::make_tuple(std::get<Ixs>(std::forward<Tups>(tups))...);
    };
    return index_apply<N>(
      [row = std::move(row)](auto... Ixs) { return std::make_tuple(row(Ixs)...); });
}

template<class Pred,
         class Tup1,
         class Tup2,
         const size_t N = tupletools::tuple_size_v<Tup1>,
         const size_t M = tupletools::tuple_size_v<Tup2>>
constexpr auto
where(Pred&& pred, Tup1&& tup1, Tup2&& tup2)
{
    static_assert(N == M, "Tups must be the same size!");

    return index_apply<N>([&](auto... Ixs) {
        auto tup =
          std::make_tuple(std::invoke(std::forward<Pred>(pred),
                                      std::get<Ixs>(std::forward<Tup1>(tup1)),
                                      std::get<Ixs>(std::forward<Tup2>(tup2)))...);
        return tup;
    });
}

template<class Pred,
         class Tup1,
         class Tup2,
         const size_t N = tupletools::tuple_size_v<Tup1>,
         const size_t M = tupletools::tuple_size_v<Tup2>>
constexpr bool
any_where(Pred&& pred, Tup1&& tup1, Tup2&& tup2)
{
    static_assert(N == M, "Tuples must be the same size!");
    return index_apply<N>([&tup1, &tup2, pred = std::forward<Pred>(pred)](auto... Ixs) {
        return ((std::invoke(pred,
                             std::get<Ixs>(std::forward<Tup1>(tup1)),
                             std::get<Ixs>(std::forward<Tup2>(tup2)))) ||
                ...);
    });
}

template<class Pred,
         class Tup1,
         class Tup2,
         const size_t N = tupletools::tuple_size_v<Tup1>,
         const size_t M = tupletools::tuple_size_v<Tup2>>
constexpr bool
all_where(Pred&& pred, Tup1&& tup1, Tup2&& tup2)
{
    static_assert(N == M, "Tuples must be the same size!");
    return index_apply<N>([&tup1, &tup2, pred = std::forward<Pred>(pred)](auto... Ixs) {
        return ((std::invoke(pred,
                             std::get<Ixs>(std::forward<Tup1>(tup1)),
                             std::get<Ixs>(std::forward<Tup2>(tup2)))) &&
                ...);
    });
}

/*
Returns the string format representation of a coalesceably-typed tuple;
The elements of aforesaid tuple must be of a type that allows proper
type coalescence.

@param tup: tuple of coalesceable type.
@returns: string representation of tup.
 */
template<typename Tup, const size_t N = tuple_size<Tup>::value>
std::string
to_string(Tup&& tup)
{
    std::string s = "(";

    tupletools::for_each(std::forward<Tup>(tup), [&](auto&& n, auto&& v) {
        s += std::to_string(v);
        s += n < N - 1 ? ", " : "";
        return false;
    });

    return s + ")";
}

template<class Tup>
constexpr bool
any_of(Tup&& tup)
{
    bool b = false;
    auto func = [&](auto&& n, auto&& v) -> bool {
        if (v) {
            b = true;
            return true;
        }
        return false;
    };
    tupletools::for_each(std::forward<Tup>(tup), func);
    return b;
}

template<class Tup>
constexpr bool
all_of(Tup&& tup)
{
    bool b = true;
    auto func = [&](auto&& n, auto&& v) -> bool {
        if (!v) {
            b = false;
            return true;
        }
        return false;
    };
    tupletools::for_each(std::forward<Tup>(tup), func);
    return b;
}

template<class Tup>
constexpr bool
disjunction_of(Tup&& tup)
{
    bool b = true;
    bool prev_b = true;

    auto func = [&](auto&& n, auto&& v) -> bool {
        if (!prev_b && v) {
            b = false;
            return true;
        } else {
            prev_b = v;
            return false;
        }
    };

    tupletools::for_each(std::forward<Tup>(tup), func);
    return b;
}

template<class Tup, std::enable_if_t<!is_tuple_v<Tup>, int> = 0>
constexpr auto
make_tuple_if(Tup&& tup)
{
    return std::make_tuple(tup);
}

template<class Tup, std::enable_if_t<is_tuple_v<Tup>, int> = 0>
constexpr auto
make_tuple_if(Tup&& tup)
{
    return tup;
}

template<class... T>
struct flatten_impl
{};

template<class T>
struct flatten_impl<T>
{
    template<class U>
    constexpr auto operator()(U&& value)
    {
        return std::forward<U>(value);
    }
};

template<class T>
struct flatten_impl<std::tuple<T>>
{
    template<class Tup>
    constexpr auto operator()(Tup&& tup)
    {
        return flatten_impl<std::remove_cvref_t<T>>{}(
          std::get<0>(std::forward<Tup>(tup)));
    }
};

template<class T, class... Ts>
struct flatten_impl<std::tuple<T, Ts...>>
{
    template<class Tup,
             const size_t N = sizeof...(Ts),
             std::enable_if_t<(N >= 1), int> = 0>
    constexpr auto operator()(Tup&& tup)
    {
        auto tup_first =
          flatten_impl<std::remove_cvref_t<T>>{}(std::get<0>(std::forward<Tup>(tup)));

        auto t_tup_args = index_apply<N>([&](auto... Ixs) {
            return std::forward_as_tuple(std::get<Ixs + 1>(std::forward<Tup>(tup))...);
        });
        auto tup_args =
          flatten_impl<std::remove_cvref_t<decltype(t_tup_args)>>{}(t_tup_args);

        return std::tuple_cat(make_tuple_if(tup_first), make_tuple_if(tup_args));
    }
};

template<class Tup>
constexpr auto
flatten(Tup&& tup)
{
    using T = std::remove_cvref_t<Tup>;
    return flatten_impl<T>{}(std::forward<T>(tup));
}
};     // namespace tupletools
#endif // TUPLETOOLS_H