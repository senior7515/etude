//
//  etude/memory/operator_delete.hpp に対するテスト
//
//  Copyright (C) 2010-11  Takaya Saito (SubaruG)
//    Distributed under the Boost Software License, Version 1.0.
//    http://www.boost.org/LICENSE_1_0.txt
//
#include "../../etude/memory/operator_new.hpp"

#include <boost/assert.hpp>
#include <memory>
#include <type_traits>

#define STATIC_ASSERT( expr ) static_assert( expr, #expr )

struct hoge
{
  static void* operator new( std::size_t size ) {
    void* const p = ::operator new( size );
    instances_() += 1;
    return p;
  }
  static void operator delete( void* p ) {
    ::operator delete(p);
    instances_() -= 1;
  }
  
  static void* operator new []( std::size_t size ) {
    void* const p = ::operator new[]( size );
    arrays_() += 1;
    return p;
  }
  static void operator delete[]( void* p ) {
    ::operator delete[](p);
    arrays_() -= 1;
  }
  
  static int count_instance() {
    return instances_();
  }
  static int count_array() {
    return arrays_();
  }
  
  int x;
  
 private:
  static int& instances_() {
    static int instances = 0;
    return instances;
  }
  static int& arrays_() {
    static int arrays = 0;
    return arrays;
  }
  
};

int main()
{
  // メモリリークチェック用の定数軍
  int const blocksize = 128 * 1024; // 128kiB のメモリブロックを、
  int const n = 1024 * 1024;  // 1 Mi 回数確保する（つまり累計 128 GiBだけ確保する）
  
  // 非テンプレートのメモリ確保
  for( int i = 0; i < n; ++i ) {
    auto p = etude::operator_new(blocksize);
    STATIC_ASSERT(( std::is_same<decltype(p), etude::raw_storage_ptr>::value ));
  }
  
  // 普通の型に対するテンプレート版のメモリ確保
  for( int i = 0; i < n; ++i ) {
    auto p = etude::operator_new<int>();  // サイズは省略できる（便利！）
    
    STATIC_ASSERT(( std::is_same<
        decltype(p), etude::raw_storage_pointer<int>::type
      >::value
    ));
  }
  // 普通の配列の場合
  for( int i = 0; i < n; ++i ) {
    // auto p = etude::operator_new<int[]>();  // これは無理
    auto p = etude::operator_new<int[]>( 5 * sizeof(int) );  // バイト単位でサイズを指定
    
    STATIC_ASSERT(( std::is_same<
        decltype(p), etude::raw_storage_pointer<int[]>::type
      >::value
    ));
  }
  
  // operator new, operator delete がある型について
  for( int i = 0; i < n; ++i ) {
    auto p = etude::operator_new<hoge>( sizeof(hoge) );  // サイズは指定することもできる
    BOOST_ASSERT( hoge::count_instance() == 1 );
    
    STATIC_ASSERT(( std::is_same<
        decltype(p), etude::raw_storage_pointer<hoge>::type
      >::value
    ));
  }
  BOOST_ASSERT( hoge::count_instance() == 0 );
  // 配列の場合
  for( int i = 0; i < n; ++i ) {
    auto p = etude::operator_new<hoge[]>( 5 * sizeof(hoge) );
    BOOST_ASSERT( hoge::count_array() == 1 );
    
    STATIC_ASSERT(( std::is_same<
        decltype(p), etude::raw_storage_pointer<hoge[]>::type
      >::value
    ));
  }
  BOOST_ASSERT( hoge::count_array() == 0 );
  
  // 固定長配列も一応扱えるが
  for( int i = 0; i < n; ++i ) {
    auto p = etude::operator_new<hoge[5]>( sizeof(hoge[5]) ); // サイズは省略できなくなった
    // 一般に new hoge[5] で確保されるメモリサイズとは別物なので。
    BOOST_ASSERT( hoge::count_array() == 1 );
    
    STATIC_ASSERT(( std::is_same<
        decltype(p), etude::raw_storage_pointer<hoge[5]>::type
      >::value
    ));
  }
  BOOST_ASSERT( hoge::count_array() == 0 );
}
