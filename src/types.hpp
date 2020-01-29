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
struct remove_cvref
{
    typedef std::remove_cv_t<std::remove_reference_t<T>> type;
};

template<class T>
using remove_cvref_t = typename remove_cvref<T>::type;

template<class T>
struct is_tuple : std::false_type
{};

template<class... T>
struct is_tuple<std::tuple<T...>> : std::true_type
{};

template<class T>
constexpr bool is_tuple_v = is_tuple<remove_cvref_t<T>>::value;

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
constexpr bool is_tupleoid_v = is_tupleoid<remove_cvref_t<T>>::value;

template<typename T>
struct remove_cref
{
    using type = typename std::add_lvalue_reference_t<
        std::remove_const_t<std::remove_reference_t<T>>>;
};

template<class T>
using remove_cref_t = typename tupletools::remove_cref<T>::type;

template<typename T>
struct add_cref
{
    using type = typename std::add_lvalue_reference_t<
        std::add_const_t<std::remove_reference_t<T>>>;
};

template<class T>
using add_cref_t = typename tupletools::add_cref<T>::type;

template<typename T, typename = void>
struct is_iterator : std::false_type
{};

template<typename T>
struct is_iterator<
    T,
    std::void_t<
        decltype(++std::declval<T&>()),
        decltype(*std::declval<T&>()),
        decltype(std::declval<T&>() == std::declval<T&>())>> : std::true_type
{
    using deref_type = remove_cvref_t<decltype(*std::declval<T&>())>;
};

template<typename T>
using iterator_t = typename is_iterator<T>::deref_type;

template<typename T>
constexpr bool is_iterator_v = is_iterator<T>::value;

template<typename T, typename = void>
struct is_iterable : std::false_type
{};

template<typename T>
struct is_iterable<
    T,
    std::void_t<
        decltype(std::declval<T&>().begin()),
        decltype(std::declval<T&>().end())>> : std::true_type
{
    using deref_type = remove_cvref_t<decltype(*(std::declval<T&>().begin()))>;
};

template<typename T>
using iterable_t = typename is_iterable<T>::deref_type;

template<typename T>
constexpr bool is_iterable_v = is_iterable<T>::value;

template<typename T, typename = void>
struct is_container : std::false_type
{};

template<typename T>
struct is_container<T, std::void_t<is_tupleoid<T>, is_iterable<T>>>
  : std::true_type
{};

template<typename T>
constexpr bool is_container_v = is_container<T>::value;

template<typename T>
struct tuple_size
{
    const static size_t value =
        std::tuple_size<tupletools::remove_cvref_t<T>>::value;
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
  : std::integral_constant<
        bool,
        Pred<T0, T1>::value || any_of_t<Pred, T0, Ts...>::value>
{};

// const down-and-up-cast: returns "value" with const either removed or added.

template<typename T>
constexpr auto&&
const_downcast(T&& value)
{
    using downcasted = tupletools::remove_cref_t<T>;
    return std::forward<downcasted>(const_cast<downcasted>(value));
}

template<typename T>
constexpr auto&&
const_upcast(T&& value)
{
    using upcasted = tupletools::add_cref_t<T>;
    return std::forward<upcasted>(const_cast<upcasted>(value));
}

};     // namespace tupletools
#endif // TYPES_H