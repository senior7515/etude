//
//  pointee_greater:
//    etude::pointee_before を呼び出す関数オブジェクト
// 
//    etude::pointee_greater<T, U> は、
//    T const& 型の t 及び U const& 型の u に対し
//    etude::pointee_before( u, t ) を呼び出す、
//    空で trivial な関数オブジェクトです。
//    
//    この関数オブジェクトは、比較が etude::pointee_before になる以外は、
//    etude::greater と同様に定義されます。
//    
//  Copyright (C) 2011  Takaya Saito (SubaruG)
//    Distributed under the Boost Software License, Version 1.0.
//    http://www.boost.org/LICENSE_1_0.txt
//
#ifndef ETUDE_FUNCTINAL_INCLUDED_POINTEE_GREATER_HPP_
#define ETUDE_FUNCTINAL_INCLUDED_POINTEE_GREATER_HPP_

#include <type_traits>
#include <utility>
#include "../utility/pointee_before.hpp"
#include "../types/is_simply_callable.hpp"

namespace etude {
  
  // 実装
  template<class T, class U, class = void>
  struct pointee_greater_ {};
  
  template<class T, class U>
  struct pointee_greater_< T, U,
    typename std::enable_if<
      std::is_convertible<
        decltype(
          etude::pointee_before( std::declval<U const&>(), std::declval<T const&>() )
        ), bool
      >::value
    >::type
  >
  {
    typedef bool result_type;
    typedef T  first_argument_type;
    typedef U second_argument_type;
    
    typedef typename std::remove_reference<T>::type T_;
    typedef typename std::remove_reference<U>::type U_;
    
    bool operator()( T_ const& lhs, U_ const& rhs ) const {
      return etude::pointee_before( rhs, lhs );
    }
    
  };
  
  
  // 本体（一般の場合）
  template<class T = void, class U = T>
  struct pointee_greater
    : pointee_greater_<T, U> {};
  
  // 型指定無しの場合
  template<>
  struct pointee_greater<>
  {
    typedef bool result_type;
    
    // etude::pointee_greater<T, U> に転送
    
    // 型が同じ場合
    template< class T,
      class = typename std::enable_if<
        etude::is_simply_callable<
          etude::pointee_greater<T>, bool ( T const&, T const& )
        >::value
      >::type
    >
    bool operator()( T const& lhs, T const& rhs ) const {
      return etude::pointee_greater<T>()( lhs, rhs );
    }
    // 型が違う場合
    template< class T, class U,
      class = typename std::enable_if<
        etude::is_simply_callable<
          etude::pointee_greater<T, U>, bool ( T const&, U const& )
        >::value
      >::type
    >
    bool operator()( T const& lhs, U const& rhs ) const {
      return etude::pointee_greater<T, U>()( lhs, rhs );
    }
    
  };

} // namespace etude

#endif  // #ifndef ETUDE_FUNCTINAL_INCLUDED_POINTEE_GREATER_HPP_
