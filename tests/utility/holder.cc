//
//  holder のテストです。
//    
//  Copyright (C) 2010-11  Takaya Saito (SubaruG)
//    Distributed under the Boost Software License, Version 1.0.
//    http://www.boost.org/LICENSE_1_0.txt
//

#include "../../etude/utility/holder.hpp"
#include <type_traits>
#include <utility>

#include "../../etude/types/is_constructible.hpp"
#include "../../etude/types/is_trivially_destructible.hpp"
#define STATIC_ASSERT( expr ) static_assert( expr, #expr )

// チェック本体
template<class T>
void check()
{
  typedef etude::holder<T> holder;
  typedef typename std::remove_const<T>::type U;
  typedef typename std::add_pointer<T>::type             pointer;
  typedef typename std::add_pointer<T const>::type const_pointer;
  
  STATIC_ASSERT((  sizeof(T) ==  sizeof(holder) || std::is_reference<T>::value ));
  STATIC_ASSERT(( alignof(T) == alignof(holder) || std::is_reference<T>::value ));
  STATIC_ASSERT(( std::is_empty<T>::value == std::is_empty<holder>::value ));
  // STATIC_ASSERT(( std::is_trivially_copyable<T>::value == std::is_trivially_copyable<holder>::value )); // ない
  STATIC_ASSERT ((
    ( std::is_reference<T>::value && std::is_standard_layout<holder>::value ) ||
    std::is_standard_layout<T>::value == std::is_standard_layout<holder>::value
  ));
  // trivially destructible class か
  STATIC_ASSERT ((
    ( std::is_reference<T>::value && etude::is_trivially_destructible<holder>::value ) ||
    etude::is_trivially_destructible<T>::value == etude::is_trivially_destructible<holder>::value
  ));
  
  // get
  STATIC_ASSERT(( std::is_same<
    T &, decltype( std::declval<holder&>().get() )
  >::value ));
  STATIC_ASSERT(( std::is_same<
    T const&, decltype( std::declval<holder const&>().get() )
  >::value ));
  // move 時は const が外れる
  STATIC_ASSERT(( std::is_same<
    U &&, decltype( std::declval<holder&>().move() )
  >::value ));
  
  // get_ptr
  STATIC_ASSERT(( std::is_same<
    pointer, decltype( std::declval<holder&>().get_ptr() )
  >::value ));
  STATIC_ASSERT(( std::is_same<
    const_pointer, decltype( std::declval<holder const&>().get_ptr() )
  >::value ));
  
  // operator*
  STATIC_ASSERT(( std::is_same<
    T &, decltype( *std::declval<holder&>() )
  >::value ));
  STATIC_ASSERT(( std::is_same<
    T const&, decltype( *std::declval<holder const&>() )
  >::value ));
  STATIC_ASSERT(( std::is_same<
    U &&, decltype( *std::declval<holder&&>() )
  >::value ));
  
  // operator->
  STATIC_ASSERT(( std::is_same<
    pointer, decltype( std::declval<holder&>().operator->() )
  >::value ));
  STATIC_ASSERT(( std::is_same<
    const_pointer, decltype( std::declval<holder const&>().operator->() )
  >::value ));
  
  // 自由関数 get は operator* と同じ
  STATIC_ASSERT(( std::is_same<
    T &, decltype( get( std::declval<holder&>() ) )
  >::value ));
  STATIC_ASSERT(( std::is_same<
    T const&, decltype( get( std::declval<holder const&>() ) )
  >::value ));
  STATIC_ASSERT(( std::is_same<
    U &&, decltype( get( std::declval<holder&&>() ) )
  >::value ));
  
  // 自由関数 get のポインタ版
  STATIC_ASSERT(( std::is_same<
    pointer, decltype( get( std::declval<holder*>() ) )
  >::value ));
  STATIC_ASSERT(( std::is_same<
    const_pointer, decltype( get( std::declval<holder const*>() ) )
  >::value ));
  
  // 自由関数 get_pointer
  STATIC_ASSERT(( std::is_same<
    pointer, decltype( get_pointer( std::declval<holder&>() ) )
  >::value ));
  STATIC_ASSERT(( std::is_same<
    const_pointer, decltype( get_pointer( std::declval<holder const&>() ) )
  >::value ));
  
}

// （CVつきの）値と参照についてチェック
template<class T>
void test()
{
  check<T>();
  check<T const>();
  check<T &>();
  check<T const&>();
  check<T &&>();
}

// 変換可能性
template<class T, class U>
void check_convertible( int )
{
  typedef etude::holder<T> holder;
  
  STATIC_ASSERT(( std::is_convertible<U, T>::value
    == std::is_convertible<U, holder>::value ));
  // 型変換できるか否か
  STATIC_ASSERT(( std::is_convertible<U, T>::value
    == std::is_convertible<etude::holder<U>, holder>::value ));
}
template<class T, class... Args>
typename std::enable_if<sizeof...(Args) != 1>::type check_convertible( ... ) {}

// 構築可能性
template<class T, class... Args>
void check_constructible()
{
  typedef etude::holder<T> holder;
  
  STATIC_ASSERT(( etude::is_constructible<T, Args...>::value
    == etude::is_constructible<holder, Args...>::value ));
  
  check_convertible<T, Args...>(0);
}


struct trivial_empty_class {};

struct nontrivial_empty_class {
  nontrivial_empty_class() {}
  ~nontrivial_empty_class() {}
};

struct trivial_class {
  int x, y;
};

struct nontrivial_class
{
  enum {
    copy_ctor,
    move_ctor,
    
    from_int,
    from_intptr,
    from_int_and_double,
    from_int_and_double_and_charptr
    
  } which_ctor_has_called;
  
  nontrivial_class( nontrivial_class const& )
    : which_ctor_has_called(copy_ctor) {}
  nontrivial_class( nontrivial_class && )
    : which_ctor_has_called(move_ctor) {}
  
  // 型変換： int から
  nontrivial_class( int )
    : which_ctor_has_called(from_int) {}
  // int* からは explicit
  explicit nontrivial_class( int* )
    : which_ctor_has_called(from_intptr) {}
  
  // explicit な複数引数コンストラクタ
  explicit nontrivial_class( int, double )
    : which_ctor_has_called(from_int_and_double) {}
  // non-explicit な複数引数コンストラクタ
  nontrivial_class( int, double, char* )
    : which_ctor_has_called(from_int_and_double_and_charptr) {}
  
  // standard layout でない
  virtual ~nontrivial_class() {}
  
};

#include <boost/assert.hpp>

int main()
{
  test<int>();
  test<trivial_empty_class>();
  test<nontrivial_empty_class>();
  test<trivial_class>();
  test<nontrivial_class>();
  
  check_constructible<int>();
  check_constructible<int, int>();
  check_constructible<int const&, int&>();
  
  check_constructible<trivial_class>();
  check_constructible<nontrivial_class>();
  
  check_constructible<nontrivial_class, int>();
  check_constructible<nontrivial_class, int*>();
  check_constructible<nontrivial_class, void const*>();
  
  check_constructible<nontrivial_class, double, double>();
  check_constructible<nontrivial_class, int*, double>();
  check_constructible<nontrivial_class, int, double, char*>();
  check_constructible<nontrivial_class, int, double, char const*>();
  
  // 実際に構築するチェック
  etude::holder<int> i;  // デフォルト構築時は初期化される
  BOOST_ASSERT( i.get() == int() );
  
  // 初期化させない場合は uninitialized を渡す
  etude::holder<int> j = etude::uninitialized;
  // BOOST_ASSERT( j.get() == int() );  // これはそうとは限らない
  
  // scalar type じゃない場合には etude::uninitialized は使えない
  // etude::holder<nontrivial_class> x = etude::uninitialized;
  
  etude::holder<void*> p = 0;  // リテラル 0 も正しく扱える
  BOOST_ASSERT( p.get() == 0 );
  
  // 参照の swap
  etude::holder<int&> r1 = *i, r2 = *j;
  BOOST_ASSERT( boost::addressof( *r1 ) == boost::addressof( *i ) );
  BOOST_ASSERT( boost::addressof( *r2 ) == boost::addressof( *j ) );
  swap( r1, r2 );
  BOOST_ASSERT( boost::addressof( *r1 ) == boost::addressof( *j ) );
  BOOST_ASSERT( boost::addressof( *r2 ) == boost::addressof( *i ) );
  
  // 再代入
  struct X
  {
    X& operator=( int ) {
      return *this;
    }
  };
  etude::holder<X const> x;
  x = 43;
}
