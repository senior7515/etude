//
//  etude::is_move_constructible のテストです。
//    
//  Copyright (C) 2011  Takaya Saito (SubaruG)
//    Distributed under the Boost Software License, Version 1.0.
//    http://www.boost.org/LICENSE_1_0.txt
//

#include "../../etude/types/is_move_constructible.hpp"

#include <type_traits>
#define STATIC_ASSERT( expr ) static_assert( expr, #expr )

struct X {};
struct Y {
  Y( Y const& ) = delete;
  Y( Y&& ) = default;
};
struct Z {
  Z( Z const& ) = delete;
};

int main()
{
  // void, 関数
  STATIC_ASSERT(( !etude::is_move_constructible<void>::value ));
  STATIC_ASSERT(( !etude::is_move_constructible<void const>::value ));
  STATIC_ASSERT(( !etude::is_move_constructible<void()>::value ));
  
  // 参照
  STATIC_ASSERT((  etude::is_move_constructible<int&>::value ));
  STATIC_ASSERT((  etude::is_move_constructible<int&&>::value ));
  
  // scalar types
  STATIC_ASSERT((  etude::is_move_constructible<int>::value ));
  STATIC_ASSERT((  etude::is_move_constructible<int const>::value ));
  STATIC_ASSERT((  etude::is_move_constructible<double>::value ));
  STATIC_ASSERT((  etude::is_move_constructible<int*>::value ));
  STATIC_ASSERT((  etude::is_move_constructible<void(*)()>::value ));
  STATIC_ASSERT((  etude::is_move_constructible<int X::*>::value ));
  STATIC_ASSERT((  etude::is_move_constructible<void (X::*)()>::value ));
  
  // ユーザ定義型
  STATIC_ASSERT((  etude::is_move_constructible<X>::value ));
  STATIC_ASSERT((  etude::is_move_constructible<X const>::value ));
  STATIC_ASSERT((  etude::is_move_constructible<Y>::value ));
  STATIC_ASSERT(( !etude::is_move_constructible<Y const>::value ));
  STATIC_ASSERT(( !etude::is_move_constructible<Z>::value ));
  STATIC_ASSERT(( !etude::is_move_constructible<Z const>::value ));
  
}
