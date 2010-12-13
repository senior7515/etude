//
//  value_wrapper:
//    タプルを unpack しながら構築できるラッパークラス
// 
//    boost::noncopyable を = delete; を使って書きなおした版です。
//    オリジナルの boost::noncopyable と違い、可能なかぎり trivial になっています。
//    
//  Copyright (C) 2010  Takaya Saito (SubaruG)
//    Distributed under the Boost Software License, Version 1.0.
//    http://www.boost.org/LICENSE_1_0.txt
//
#ifndef ETUDE_UTILITY_INCLUDED_VALUE_WRAPPER_HPP_
#define ETUDE_UTILITY_INCLUDED_VALUE_WRAPPER_HPP_

#include "simple_wrapper.hpp"
#include "../types/indices.hpp"
#include "../types/tuple_element.hpp"
#include "../types/tuple_types.hpp"
#include "../types/tuple_indices.hpp"
#include "../types/is_convertible.hpp"

namespace etude {
 namespace value_wrapper_ { // ADL 回避
 
  // 構築指定用のタグ
  struct emplace_construct_t {};
  struct unpack_construct_t {};
  namespace {
    emplace_construct_t const emplace_construct = {};
    unpack_construct_t const unpack_construct = {};
  }
  
  // 直接 etude::simple_wrapper を使うのはアレなので
  // 実装用クラスを持ってくる
  using etude::simple_wrapper_::simple_wrapper_;
  
  // タプルの unpack をするにはもう一段実装用クラスをはさむ必要あり
  template<class T>
  class value_wrapper_
    : private simple_wrapper_<T>
  {
    typedef simple_wrapper_<T> base;
    
   public:
    using base::get;
    
    // デフォルト構築
    value_wrapper_() = default;
    
    // pack されてない引数から構築
    template<class... Args>
    value_wrapper_( emplace_construct_t, Args&&... args )
      : base( std::forward<Args>(args)... ) {}
    
    // pack された引数から構築
    template<class Tuple, std::size_t... Indices>
    value_wrapper_( Tuple && t, etude::indices<Indices...> ) :
      base (
        std::forward<typename etude::tuple_element<Indices, Tuple>::type>(
          std::get<Indices>(t)
        )...
      )
   {}
    
    // gcc4.5.0 では implicit move が実装されていないので
    value_wrapper_( value_wrapper_ const& ) = default;
    value_wrapper_( value_wrapper_ && )     = default;
    
  };
  
  // 本体
  template<class T, class Tag = void>
  class value_wrapper
    : private value_wrapper_<T>
  {
    typedef value_wrapper_<T> base;
    
   public:
    using base::get;
    
    // デフォルト構築
    value_wrapper() = default;
    
    // T からの構築
    template<class U,
      class = typename std::enable_if<
        std::is_convertible<U, T>::value
      >::type
    >
    value_wrapper( U && src )
      : base( emplace_construct, std::forward<U>(src) ) {}
    
    // pack されてない引数から構築
    template<class... Args,
      class = typename std::enable_if<
        std::is_constructible<T, Args...>::value
      >::type
    >
    value_wrapper( emplace_construct_t, Args&&... args )
      : base( emplace_construct, std::forward<Args>(args)... ) {}
    
    // pack された引数から構築
    template<class Tuple,
      class = typename std::enable_if<
        etude::is_convertible<
          typename etude::tuple_types<Tuple>::type, T
        >::value
      >::type
    >
    value_wrapper( unpack_construct_t, Tuple && t )
      : base( std::forward<Tuple>(t), etude::tuple_indices<Tuple>() ) {}
    
    // gcc4.5.0 では implicit move が（ｒｙ
    value_wrapper( value_wrapper const& ) = default;
    value_wrapper( value_wrapper && )     = default;
    
  };
  
  // 自由関数版 get
  template<class T, class Tag>
  inline T& get( value_wrapper<T, Tag>& x ) {
    return x.get();
  }
  template<class T, class Tag>
  inline T const& get( value_wrapper<T, Tag> const& x ) {
    return x.get();
  }
  // move 版
  template<class T, class Tag>
  inline T&& get( value_wrapper<T, Tag> && x ) {
    return std::forward<T>( x.get() );
  }
 
 }  // namespace value_wrapper_
 using namespace value_wrapper_;
}

#endif  // #ifndef ETUDE_UTILITY_INCLUDED_VALUE_WRAPPER_HPP_