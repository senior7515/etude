//
//  etude/noncopyable.hpp に対するテスト
//    というより仕様
//
//  Copyright (C) 2011  Takaya Saito (SubaruG)
//    Distributed under the Boost Software License, Version 1.0.
//    http://www.boost.org/LICENSE_1_0.txt
//
#include "../etude/noncopyable.hpp"

#include <type_traits>
#include "../etude/types/is_constructible.hpp"
#define STATIC_ASSERT( expr ) static_assert( expr, #expr )

// for is_xxx_constructible/assignable
#include "test_utilities.hpp"

int main()
{
  // etude::noncopyable<> に対して
  STATIC_ASSERT(( !is_copy_constructible<etude::noncopyable<>>::value ));
  STATIC_ASSERT((  is_move_constructible<etude::noncopyable<>>::value ));
  
  STATIC_ASSERT(( !is_copy_assignable<etude::noncopyable<>>::value ));
  STATIC_ASSERT((  is_move_assignable<etude::noncopyable<>>::value ));
  
  STATIC_ASSERT(( std::is_empty<etude::noncopyable<>>::value ));
  
  STATIC_ASSERT(( std::has_trivial_default_constructor<etude::noncopyable<>>::value ));
  STATIC_ASSERT(( etude::is_trivially_destructible<etude::noncopyable<>>::value ));


  // base class chaining
  struct X {};
  
  STATIC_ASSERT(( std::is_base_of<X, etude::noncopyable<X>>::value ));

  STATIC_ASSERT(( !is_copy_constructible<etude::noncopyable<X>>::value ));
  STATIC_ASSERT((  is_move_constructible<etude::noncopyable<X>>::value ));
  
  STATIC_ASSERT(( !is_copy_assignable<etude::noncopyable<X>>::value ));
  STATIC_ASSERT((  is_move_assignable<etude::noncopyable<X>>::value ));
  
  // ctor 転送
  STATIC_ASSERT(( !std::is_convertible<X&&, etude::noncopyable<X>>::value ));
  STATIC_ASSERT(( etude::is_constructible<etude::noncopyable<X>, X&&>::value ));
  STATIC_ASSERT(( !etude::is_constructible<etude::noncopyable<X>, int>::value ));


  // noncopyable wrapper
  STATIC_ASSERT(( !is_copy_constructible<etude::noncopyable<int>>::value ));
  STATIC_ASSERT((  is_move_constructible<etude::noncopyable<int>>::value ));
  
  STATIC_ASSERT(( !is_copy_assignable<etude::noncopyable<int>>::value ));
  STATIC_ASSERT((  is_move_assignable<etude::noncopyable<int>>::value ));
  
  // ctor 転送
  STATIC_ASSERT(( !std::is_convertible<int, etude::noncopyable<int>>::value ));
  STATIC_ASSERT(( etude::is_constructible<etude::noncopyable<int>, int>::value ));
  STATIC_ASSERT(( !etude::is_constructible<etude::noncopyable<int>, void*>::value ));

}
