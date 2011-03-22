//
//  invoke:
//    標準ライブラリ <functional> における INVOKE をシミュレートする関数
// 
//    etude::invoke( f, t1, t2, ..., tN ) は、
//    N == 1 かつ t1 が etude::unpacked_tuple<T, Indices...> ならば
//      INVOKE( f, etude::move<Indices>(t1)... )
//    を、 f がメンバ関数ポインタで N == 2 かつ t2 が etude::unpacked_tuple<T, Indices...> ならば
//      INVOKE( f, t1, etude::move<Indices>(t2)... )
//    を、それ以外の場合には
//      INVOKE( f, t1, t2, ..., tN )
//    を呼び出します。
//    ここで、 INVOKE は標準ライブラリ <functional> の規格において定義されたものです。
//    
//    etude::invoke<R>( f, t1, t2, ..., tN ) は、
//    etude::invoke( f, t1, t2, ..., tN ) の戻り値を R に暗黙変換して返します。
//    これは標準ライブラリの <functional> の規格における
//    INVOKE( f, t1, t2, ..., tN, R ) の動作と同一です。
//    ただし R が void の場合、戻り値は暗黙変換ではなく明示的に void に変換されます
//    （つまり、戻り値は単純に無視されます）。
//    
//  Copyright (C) 2011  Takaya Saito (SubaruG)
//    Distributed under the Boost Software License, Version 1.0.
//    http://www.boost.org/LICENSE_1_0.txt
//
#ifndef ETUDE_FUNCTINAL_INCLUDED_INVOKE_HPP_
#define ETUDE_FUNCTINAL_INCLUDED_INVOKE_HPP_

#include <utility>
#include <type_traits>
#include <functional>

#include "wrap_if_mem_fn.hpp"
#include "unpacked_tuple.hpp"

#include <boost/preprocessor/empty.hpp>
#include "../types/identity.hpp"

namespace etude {
  
  // 規格 20.8.2 Requirements [func.require] で定義される INVOKE の処理を行う
  template<class F, class... Args,
    class R = decltype(
      etude::wrap_if_mem_fn( std::declval<F>() )( std::declval<Args>()... )
    )
  >
  inline R invoke( F && f, Args&&... args )
  {
    return etude::wrap_if_mem_fn( std::forward<F>(f) )( std::forward<Args>(args)... );
  }
  // ただし引数が etude::unpack の結果なら、 unpack して適用する
  template<class F, class Tuple, std::size_t... Indices,
    class R = decltype(
      etude::wrap_if_mem_fn( std::declval<F>() )(
        etude::move<Indices>( std::declval<unpacked_tuple<Tuple, Indices...>>() )...
      )
    )
  >
  inline R invoke( F && f, unpacked_tuple<Tuple, Indices...> t )
  {
    (void)t;  // 警告避け
    return etude::wrap_if_mem_fn( std::forward<F>(f) )( etude::move<Indices>(t)... );
  }
  // 更にメンバ関数ポインタかつ第三引数が unpack の結果なら、それも unpack する
  template<class F, class T1, class Tuple, std::size_t... Indices,
    class R = decltype(
      std::mem_fn( std::declval<F>() )( std::declval<T1>(),
        etude::move<Indices>( std::declval<unpacked_tuple<Tuple, Indices...>>() )...
      )
    ),
    class = typename std::enable_if<
      std::is_member_function_pointer<F>::value
    >::type
  >
  inline R invoke( F f, T1 && t1, unpacked_tuple<Tuple, Indices...> t )
  {
    (void)t;  // 警告避け
    return std::mem_fn(f)( std::forward<T1>(t1), etude::move<Indices>(t)... );
  }
  
  // 型指定版。戻り値を R に暗黙変換する。 R が void ならば単純に捨てる
  template<class R, class F, class... Args,
    class R_ = decltype(
      etude::invoke( std::declval<F>(), std::declval<Args>()... )
    ),
    class = typename std::enable_if<
      std::is_void<R>::value || std::is_convertible<R_, R>::value
    >::type
  >
  inline R invoke( F && f, Args&&... args )
  {
    return static_cast<R>(
      etude::invoke( std::forward<F>(f), std::forward<Args>(args)... )
    );
  }
  
  
  // 引数が関数ポインタの場合に型推論を補助
  template<class R, class... Args>
  inline R invoke( R (*f)( Args... ), typename etude::identity<Args>::type... args )
  {
    return f( std::forward<Args>(args)... );
  }
  // 型指定版
  template< class R, class R_, class... Args,
    class = typename std::enable_if<
      std::is_void<R>::value || std::is_convertible<R_, R>::value
    >::type
  >
  inline R invoke( R_ (*f)( Args... ), typename etude::identity<Args>::type... args )
  {
    return static_cast<R>( f( std::forward<Args>(args)... ) );
  }
  
  
  // メンバ関数ポインタの場合に型推論補助
  #define ETUDE_GEN_( qualifier )                                               \
    template<class R, class X, class T, class... Args>                          \
    inline R invoke( R ( X::*f )( Args... ) qualifier,                          \
      T && x, typename etude::identity<Args>::type... args )                    \
    {                                                                           \
      return std::mem_fn(f)( std::forward<T>(x), std::forward<Args>(args)... ); \
    }                                                                           \
    template< class R, class R_, class X, class T, class... Args,               \
      class = typename std::enable_if<                                          \
        std::is_void<R>::value || std::is_convertible<R_, R>::value             \
      >::type                                                                   \
    >                                                                           \
    inline R invoke( R_ ( X::*f )( Args... ) qualifier,                         \
      T && x, typename etude::identity<Args>::type... args )                    \
    {                                                                           \
      return static_cast<R>(                                                    \
        std::mem_fn(f)( std::forward<T>(x), std::forward<Args>(args)... )       \
      );                                                                        \
    }                                                                           \
    /* ETUDE_GEN_( qual ) */
    
    ETUDE_GEN_( BOOST_PP_EMPTY() )
    ETUDE_GEN_( const )
    ETUDE_GEN_( volatile )
    ETUDE_GEN_( const volatile )
    
    // 本来はここに & や && 系列も用意する
    
  #undef ETUDE_GEN_
  
} // namespace etude

#endif  // #ifndef ETUDE_FUNCTINAL_INCLUDED_INVOKE_HPP_
