//
//  lifetime_check: 解放漏れやらなんやらをチェックするクラス
//    テスト用ユーティリティの一つです。
//
//  Copyright (C) 2010  Takaya Saito (SubaruG)
//    Distributed under the Boost Software License, Version 1.0.
//    http://www.boost.org/LICENSE_1_0.txt
//
#ifndef ETUDE_TEST_MEMORY_INCLUDED_LIFETIME_CHECK_HPP_
#define ETUDE_TEST_MEMORY_INCLUDED_LIFETIME_CHECK_HPP_

#include <boost/assert.hpp>
#include <set>

// 寿命チェック用オブジェクト
template<class Derived>
struct lifetime_check
{
  static bool is_initialized( void const* p ) {
    if( p == 0 ) {
      return false;
    }
    return get_object_list_().find(p) != get_object_list_().end();
  }
  
  static bool count_existing_instance() {
    return get_object_list_().size();
  }
  
 protected:
  lifetime_check() {
    register_instance_( static_cast<Derived*>(this) );
  }
  lifetime_check( lifetime_check const& ) {
    register_instance_( static_cast<Derived*>(this) );
  }
  ~lifetime_check() {
    unregister_instance_( static_cast<Derived*>(this) );
  }
  
 private:
  typedef std::set<void const*> object_list_t;
  
  static object_list_t& get_object_list_() {
    static object_list_t object_list;
    return object_list;
  }
  
  static void register_instance_( void const* p ){
    bool const succeeded = get_object_list_().insert(p).second;
    BOOST_ASSERT( succeeded );
  }
  static void unregister_instance_( void const* p ){
    std::size_t erased = get_object_list_().erase(p);
    BOOST_ASSERT( erased == 1 );
  }
  
};

#endif  // #ifndef ETUDE_TEST_MEMORY_INCLUDED_LIFETIME_CHECK_HPP_
