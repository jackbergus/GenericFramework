// template_typing.h
// This file is part of narcissus
//
// Copyright (C)  2025 - Giacomo
//
// narcissus is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  narcissus is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with narcissus. If not, see <http://www.gnu.org/licenses/>.

//
// Created by gyankos on 27/12/25.
//

#ifndef REFLECTION_TEMPLATE_TYPING_H
#define REFLECTION_TEMPLATE_TYPING_H
#include <iosfwd>
#include <list>
#include <map>
#include <memory>
#include <queue>
#include <stack>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>

template<typename T, typename Enable = void>
struct is_smart_pointer
{
    enum { value = false };
};

template<typename _Tp>
  struct remove_arg
{ using type = _Tp; };


template <typename T, typename = void>
struct is_streamable2 : std::false_type {};

template <typename T>
struct is_streamable2<T, typename std::enable_if<
  std::is_convertible<
    decltype(std::declval<std::ostream &>() << std::declval<T>()),
    std::ostream &
  >::value
>::type> : std::true_type {};

template<typename Test, template<class...> class Ref>
struct is_specialization : std::false_type {};

template<template<class...> class Ref, class... Args>
struct is_specialization<Ref<Args...>, Ref>: std::true_type {};

template<typename>
struct is_std_array : std::false_type {};

template<typename T,  std::size_t A>
struct is_std_array<std::array<T,A>> : std::true_type {};

template<typename T,  std::size_t A>
  struct remove_arg<std::array<T,A>>
{ using type = T; };

template<typename T>
  struct std_array_size
{ constexpr static uint64_t size = 0; };

template<typename T,  std::size_t A>
  struct std_array_size<std::array<T,A>>
{ static constexpr uint64_t size = A; };

template <typename T> using is_vector = is_specialization<T, std::vector>;

template<typename T>
  struct remove_arg<std::vector<T>>
{ using type = T; };

template <typename T> using is_list = is_specialization<T, std::list>;

template<typename T>
  struct remove_arg<std::list<T>>
{ using type = T; };

template <typename T> using is_map = is_specialization<T, std::map>;
template <typename T> using is_unordered_map = is_specialization<T, std::unordered_map>;
template <typename T> using is_queue = is_specialization<T, std::queue>;
template <typename T> using is_tuple = is_specialization<T, std::tuple>;

template<typename T>
  struct remove_arg<std::queue<T>>
{ using type = T; };

template <typename T> using is_stack = is_specialization<T, std::stack>;

template<typename T>
  struct remove_arg<std::stack<T>>
{ using type = T; };

template <typename T> using is_deque = is_specialization<T, std::deque>;

template<typename T>
  struct remove_arg<std::deque<T>>
{ using type = T; };

template <typename T> using is_variant = is_specialization<T, std::variant>;
template <typename T> using is_pair = is_specialization<T, std::pair>;
template <typename T> using is_tuple = is_specialization<T, std::tuple>;
template <typename T> using is_shared = is_specialization<T, std::shared_ptr>;


template<typename T>
  struct remove_arg<std::shared_ptr<T>>
{ using type = T; };

template<typename T>
struct is_smart_pointer<T, typename std::enable_if<std::is_same<typename std::remove_cv<T>::type, std::shared_ptr<typename T::element_type>>::value>::type>
{
  enum { value = true };
};



template <typename T> using is_unique = is_specialization<T, std::unique_ptr>;

template<typename T>
struct is_smart_pointer<T, typename std::enable_if<std::is_same<typename std::remove_cv<T>::type, std::unique_ptr<typename T::element_type>>::value>::type>
{
    enum { value = true };
};

template<typename T>
  struct remove_arg<std::unique_ptr<T>>
{ using type = T; };

template <typename T> using is_weak = is_specialization<T, std::weak_ptr>;


template<typename T>
struct is_smart_pointer<T, typename std::enable_if<std::is_same<typename std::remove_cv<T>::type, std::weak_ptr<typename T::element_type>>::value>::type>
{
    enum { value = true };
};

template<typename T>
  struct remove_arg<std::weak_ptr<T>>
{ using type = T; };

template<class T, class U>
struct is_same : std::false_type {};

template<class T>
struct is_same<T, T> : std::true_type {};

// template<typename T>
// struct remove_all_pointers : std::conditional_t<
//     std::is_pointer_v<T>,
//     remove_all_pointers<
//         std::remove_pointer_t<T>
//     >,
//     std::type_identity<T>
// >
// {};
//
// template<typename T>
// using remove_all_pointers_t = typename remove_all_pointers<T>::type;
//
// template<typename T, typename Enable = void>
// struct is_actual_pointer : std::false_type {};
//
// template<typename T>
// struct is_actual_pointer<T, typename std::enable_if<!std::is_same<std::remove_cvref_t<T>, typename remove_all_pointers<std::remove_cvref_t<T>>::type>::value || std::is_pointer<std::remove_cvref_t<T>>::value
//                                                     >::type> : std::true_type {};

// #include <narcissus/reflection/field.h>
// #include <field_reflection.hpp>

// template<typename T, int x, int to>
// struct static_for
// {
//     std::vector<std::unique_ptr<Field>> field_enum() {
//         auto result = static_for<T, x+1,to>().field_enum();
//         std::function<std::any(const std::any&)> f = [](const std::any& val) { return field_reflection::get_field<x>(std::any_cast<T>(val));};
//         result.emplace_back(flatten_type_to_enum<field_reflection::field_type<T, x>>(x, field_reflection::field_name<T, x>, (f)));
//         return result;
//     }
// };
//
// template<typename T, int to>
// struct static_for<T, to,to>
// {
//     std::vector<std::unique_ptr<Field>> field_enum() {
//         return {};
//     }
// };


#endif //REFLECTION_TEMPLATE_TYPING_H