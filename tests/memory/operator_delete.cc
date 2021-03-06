//
//  etude/memory/operator_delete.hpp に対するテスト
//
//  Copyright (C) 2010-11  Takaya Saito (SubaruG)
//    Distributed under the Boost Software License, Version 1.0.
//    http://www.boost.org/LICENSE_1_0.txt
//
#include "../../etude/memory/operator_delete.hpp"
#include "../../etude/memory/default_deallocate.hpp"

#include <boost/assert.hpp>
#include <memory>

template<class T>
void test()
{
  {
    T* p = new T();
    p->~T();
    etude::operator_delete<T>(p);
  }
  
  int const n = 10;
  
  {
    T* p = new T[n];
    for( int i = n; i --> 0; ) {
      (p+i)->~T();
    }
    etude::operator_delete<T[]>(p);
  }
  
  // 多次元
  {
    auto* p = new T[n][n];
    for( int i = n; i --> 0; ) {
      for( int j = n; j --> 0; ) {
        p[i][j].~T();
      }
    }
    etude::operator_delete<T[n][n]>(p);
  }
  
  // unique_ptr を使った例
  {
    std::unique_ptr<T, etude::default_deallocate<T>> p( new T() );
    p->~T();
  }
  {
    std::unique_ptr<T[], etude::default_deallocate<T[]>> p( new T[n] );
    for( int i = n; i --> 0; ) {
      p[i].~T();
    }
  }
}

template<class T>
void bad_test()
{
  T* p = new T();
  p->~T();
  ::operator delete(p); // あっ
  
  int const n = 10;
  
  p = new T[n];
  for( int i = n; i --> 0; ) {
    (p+i)->~T();
  }
  ::operator delete[](p); // …。
}

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
  static int count_arrays() {
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
  // 普通の型に対して
  test<int>();
  
  // operator delete がある型について
  test<hoge>();
  // operator delete は呼ばれてるね。
  BOOST_ASSERT( hoge::count_instance() == 0 );
  BOOST_ASSERT( hoge::count_arrays() == 0 );
  
  // 一応、 operator_delete じゃなければダメな理由を。
  bad_test<hoge>();
  // ::operator delete の場合、確かに T::operator delete は呼ばれない。
  BOOST_ASSERT( hoge::count_instance() == 1 );
  BOOST_ASSERT( hoge::count_arrays() == 1 );
  
  // 念には念を入れ、メモリリークチェック
  int const blocksize = 128 * 1024; // 128kiB のメモリブロックを、
  int const n = 1024 * 1024;  // 1 Mi 回数確保する（つまり累計 128 GiBだけ確保する）
  
  for( int i = 0; i < n; ++i ) {
    // single object の場合
    {
      struct buffer {
        char buf[blocksize];
      };
      // メモリ確保
      buffer* const buf = new buffer;
      // 破棄して解放
      buf->~buffer();
      etude::operator_delete<buffer>(buf);
    }
    // 配列の場合
    {
      // メモリ確保
      char* const buf = new char[blocksize];
      // trivially-destructible なのでデストラクタは呼ばなくていい
      // 解放
      etude::operator_delete<char[]>(buf);
    }
  }
}
