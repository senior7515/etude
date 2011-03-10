//
//  ライブラリテスト用の小物をまとめたヘッダ
//    全体的に使用頻度が高そうなものを。
//
//  Copyright (C) 2011  Takaya Saito (SubaruG)
//    Distributed under the Boost Software License, Version 1.0.
//    http://www.boost.org/LICENSE_1_0.txt
//
#ifndef ETUDE_TEST_INCLUDED_TEST_UTILITIES_HPP_
#define ETUDE_TEST_INCLUDED_TEST_UTILITIES_HPP_

#include <utility>
#include <type_traits>

// copyable 関連

// gcc 4.5.0 にないので自作する
template<class T>
struct is_copy_constructible
  : std::is_constructible<T, T const&>::type {};

template<class T>
struct is_move_constructible
  : std::is_constructible<T, T&&>::type {};

#include "../etude/types/is_assignable.hpp"

template<class T>
struct is_copy_assignable
  : etude::is_assignable<T, T const&>::type {};

template<class T>
struct is_move_assignable
  : etude::is_assignable<T, T&&>::type {};

// is_trivially_copyable も無い
#include "../etude/types/bool_constant.hpp"

template<class T>
struct is_trivially_copyable :
  etude::bool_constant<
    std::has_trivial_destructor<T>::value &&
    std::has_trivial_copy_constructor<T>::value &&
    std::has_trivial_assign<T>::value
  >::type
{};

#endif  // #ifndef ETUDE_TEST_INCLUDED_TEST_UTILITIES_HPP_