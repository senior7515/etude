//
//  pack のテストです。
//    
//  Copyright (C) 2011  Takaya Saito (SubaruG)
//    Distributed under the Boost Software License, Version 1.0.
//    http://www.boost.org/LICENSE_1_0.txt
//

#include "../../etude/functional/pack.hpp"

#define STATIC_ASSERT( expr ) static_assert( expr, #expr )

#include <type_traits>
#include <boost/test/minimal.hpp>

#include <tuple>
#include <utility>
#include "../../etude/functional/unpack.hpp"

// 型のチェック
template<class... Args>
void check( Args&&... args )
{
  auto t = etude::pack( std::forward<Args>(args)... );
  STATIC_ASSERT((
    std::is_same< decltype(t), std::tuple<Args&&...> >::value
  ));
  
  // move しないで unpack
  auto t2 = etude::pack( etude::unpack(t) );
  STATIC_ASSERT((
    std::is_same< decltype(t2), std::tuple<Args&...> >::value
  ));
  
  // move して unpack
  auto t3 = etude::pack( etude::unpack( std::move(t) ) );
  STATIC_ASSERT((
    std::is_same< decltype(t3), std::tuple<Args&&...> >::value
  ));
 
}

// 簡単なテスト用の関数
int sum( std::tuple<int, int, int> t ) {
  return std::get<0>(t) + std::get<1>(t) + std::get<2>(t);
}

#include <functional> // for std::ref
#include <boost/utility/addressof.hpp>

int test_main( int, char** )
{
  check();
  
  int i = 2;
  int const j = 3;
  check( 1, i, j );
  
  // 値のチェック
  BOOST_CHECK( sum( etude::pack( 1, i, j ) ) == 1 + i + j );
  
  auto t = std::make_tuple( 1, std::ref(i), std::ref(j) );
  BOOST_CHECK( sum( etude::pack( etude::unpack(t) ) ) == 1 + i + j );
  
  // アドレスのチェック
  auto t2 = etude::pack( etude::unpack( t ) );
  
  BOOST_CHECK(
    boost::addressof( std::get<0>(t) ) == boost::addressof( std::get<0>(t2) )
  );
  BOOST_CHECK(
    boost::addressof( std::get<1>(t) ) == boost::addressof( std::get<1>(t2) )
  );
  BOOST_CHECK(
    boost::addressof( std::get<2>(t) ) == boost::addressof( std::get<2>(t2) )
  );
  
  // 拡張 unpack （ pack( a, b, unpack(t) ) の形）のチェック
  auto t3 = etude::pack( i, etude::unpack( std::move(t) ) );
  BOOST_CHECK(
    boost::addressof( i ) == boost::addressof( std::get<0>(t3) )
  );
  BOOST_CHECK(
    boost::addressof( std::get<0>(t) ) == boost::addressof( std::get<1>(t3) )
  );
  BOOST_CHECK(
    boost::addressof( std::get<1>(t) ) == boost::addressof( std::get<2>(t3) )
  );
  BOOST_CHECK(
    boost::addressof( std::get<2>(t) ) == boost::addressof( std::get<3>(t3) )
  );
  
  return 0;
}
