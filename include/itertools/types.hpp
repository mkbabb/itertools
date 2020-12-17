#ifndef TYPES_H
#define TYPES_H

#include <tuple>
#include <type_traits>

#pragma once

namespace tupletools {

/*
A collection of compile-time tools for manipulating and tractalating std::tuple.
Tuple is an obstinate type: though it has been realtively surmounted in this
sense; many higher order abstractions are provided hereinafter.
 */

/*
Template-type meta-programming.
Used for either getting or setting types withal.
 */
template<class T>
struct is_tuple : std::false_type
{};

template<class... T>
struct is_tuple<std::tuple<T...>> : std::true_type
{};

template<class T>
constexpr bool is_tuple_v = is_tuple<std::remove_cvref_t<T>>::value;

template<class T>
struct is_tupleoid : std::false_type
{};
template<class... T>
struct is_tupleoid<std::tuple<T...>> : std::true_type
{};

template<class... T>
struct is_tupleoid<std::pair<T...>> : std::true_type
{};
template<class T>
constexpr bool is_tupleoid_v = is_tupleoid<std::remove_cvref_t<T>>::value;

template<typename T>
struct tuple_size
{
    const static size_t value = std::tuple_size<std::remove_cvref_t<T>>::value;
};

template<typename T>
constexpr size_t tuple_size_v = tupletools::tuple_size<T>::value;

template<class T, class... Ts>
struct is_any : std::bool_constant<(std::is_same_v<T, Ts> || ...)>
{};

template<class T, class... Ts>
struct is_all : std::bool_constant<(std::is_same_v<T, Ts> && ...)>
{};

template<template<typename, typename> class Pred, typename... Ts>
struct any_of_t : std::false_type
{};

template<
    template<typename, typename>
    class Pred,
    typename T0,
    typename T1,
    typename... Ts>
struct any_of_t<Pred, T0, T1, Ts...>
  : std::
        integral_constant<bool, Pred<T0, T1>::value || any_of_t<Pred, T0, Ts...>::value>
{};

template<typename T>
concept Tupleoid = is_tupleoid_v<T>;

template<typename T>
concept Rangeable = requires(T x)
{
    x.begin();
    x.end();
};

template<typename T>
concept Rangeloid = (Rangeable<T> or Tupleoid<T>);

template<typename T>
concept ForwardIterable = requires(T a, T b)
{
    ++a;
    *a;
};

template<typename T>
concept BidirectionalIterable = ForwardIterable<T>&& requires(T a, T b)
{
    --a;
};

template<typename T>
concept ForwardRange = Rangeable<T>&& requires(T a)
{
    requires ForwardIterable<decltype(a.begin())>;
};

template<typename T>
concept NestedRange = Rangeable<T>&& requires(T x)
{
    requires Rangeable<decltype(*(x.begin()))>;
};

template<Rangeable T>
using range_value_t = decltype(*std::declval<T>().begin());

template<class F, class... Args>
concept invocable = requires(F&& f, Args&&... args)
{
    std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
    /* not required to be equality preserving */
};

};     // namespace tupletools
#endif // TYPES_H