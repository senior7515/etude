//
//  less:
//    std::less の拡張版
// 
//    etude::less<T, U> は std::less<T> の上位互換として使える関数オブジェクトです。
// 
//    etude::less<T, U> は、 T と U が < 演算子で比較可能な場合には、
//      bool operator()( T const&, U const& ) const;
//    という operator() が一つ定義され、
//    型 result_type や (first|second)_argument_type が適切に定義された、
//    空で trivial な関数オブジェクトになります。
//    比較不能の場合には、 operator() や typedef は定義されません。
//    
//    また、 T, U は省略可能であり、まず U のみが省略された場合、 U は T に定義されます。
//    この時、 etude::less<T> は、 T が比較可能な場合には、 std::less<T> と同じです。
//    また T, U が共に省略された場合、 etude::less<> は、あらゆる比較可能な型の組み合わせ
//    T_, U_ に対して bool operator()( T_ const&, U_ const& ) const; の定義され、
//    型 result_type の定義された、空で trivial な関数オブジェクトになります。
//    この場合は (first|second)_argument_type は定義されません。
//    
//  Copyright (C) 2011  Takaya Saito (SubaruG)
//    Distributed under the Boost Software License, Version 1.0.
//    http://www.boost.org/LICENSE_1_0.txt
//
#ifndef ETUDE_FUNCTINAL_INCLUDED_LESS_HPP_
#define ETUDE_FUNCTINAL_INCLUDED_LESS_HPP_

#include <type_traits>
#include "../types/is_simply_callable.hpp"
#include "../utility/compare_less.hpp"

namespace etude {
  
  // 実装用
  template< class T, class U, class = void >
  struct less_ {};
  
  template< class T, class U >
  struct less_< T, U,
    typename std::enable_if<
      std::is_convertible<
        decltype(
          etude::compare_less( std::declval<T>(), std::declval<U>() )
        ), bool
      >::value
    >::type
  >
  {
    typedef bool result_type;
    
    bool operator()( T && lhs, U && rhs ) const {
      return etude::compare_less( std::forward<T>(lhs), std::forward<U>(rhs) );
    }
    
  };
  
  // 本体（一般の場合）
  template<class T = void, class U = T>
  struct less :
    less_<
      typename std::conditional<std::is_reference<T>::value, T, T const&>::type,
      typename std::conditional<std::is_reference<U>::value, U, U const&>::type
    >
  {
    typedef T   first_argument_type;
    typedef U  second_argument_type;
    
    // result_type, operator() は比較可能なときのみ定義される
    
  };
  
  // 型指定無しの場合
  template<>
  struct less<>
  {
    typedef bool result_type;
    
    // 左右で同じ型を取る場合
    template< class T,
      class = typename std::enable_if<
        etude::is_simply_callable<
          etude::less<T>, bool ( T const&, T const& )
        >::value
      >::type
    >
    bool operator()( T const& lhs, T const& rhs ) const {
      return etude::less<T>()( lhs, rhs );
    }
    
    // 左右で異なる型を取る場合
    template< class T, class U,
      class = typename std::enable_if<
        etude::is_simply_callable<
          etude::less<T&&, U&&>, bool ( T&&, U&& )
        >::value
      >::type
    >
    bool operator()( T && lhs, U && rhs ) const {
      return etude::less<T&&, U&&>()(
        std::forward<T>(lhs), std::forward<U>(rhs)
      );
    }
    
  };
  
  
  
} // namespace etude

#endif  // #ifndef ETUDE_FUNCTINAL_INCLUDED_LESS_HPP_
