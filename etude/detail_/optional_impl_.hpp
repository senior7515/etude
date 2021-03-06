//
//  optional_impl_:
//    etude::optional の実装用クラス
// 
//    実装用ヘッダ。
//    etude::optional はこのクラスを内部に保持します。
//  
//  Copyright (C) 2011  Takaya Saito (SubaruG)
//    Distributed under the Boost Software License, Version 1.0.
//    http://www.boost.org/LICENSE_1_0.txt
//
#ifndef ETUDE_DETAIL_INCLUDED_OPTIONAL_IMPL_HPP_
#define ETUDE_DETAIL_INCLUDED_OPTIONAL_IMPL_HPP_

#include <utility>
#include <type_traits>
#include <boost/utility/addressof.hpp>

#include "../memory/storage.hpp"
#include "../utility/compressed_pair.hpp"
#include "../types/is_copy_constructible.hpp"
#include "../types/is_move_constructible.hpp"
#include "../types/is_assignable.hpp"
#include "../types/is_trivially_destructible.hpp"
#include "../memory/apply_in_place.hpp"

#include "../noncopyable.hpp"
#include "../immovable.hpp"

namespace etude {

  // 前方宣言
  template<class T>
  class optional;


  // 実装用基底クラス
  template<class T, class = void>
  struct optional_impl_base_
  {
    // construct
    optional_impl_base_()
      : impl_() {}  // impl_ の ctor で first は初期化されて false になる
    
    // copy/move
    // 派生先で実装すると T の copy/move 可能性に応じた delete が不能になるので
    // 基底クラスの方で実装する
    // copy/move ctor
    optional_impl_base_( optional_impl_base_ const& src )
      : impl_()
    {
      if( auto const p = src.get() ) {
        construct( *p );
      }
    }
    optional_impl_base_( optional_impl_base_ && src )
      : impl_()
    {
      if( auto const p = src.get() ) {
        construct( std::forward<T>(*p) );
      }
    }
    // copy/move assign
    optional_impl_base_& operator=( optional_impl_base_ const& src )
    {
      if( auto const p = src.get() ) {
        this->assign( *p );
      }
      else {
        this->dispose();
      }
      return *this;
    }
    optional_impl_base_& operator=( optional_impl_base_ && src )
    {
      if( auto const p = src.get() ) {
        this->assign( std::forward<T>(*p) );
      }
      else {
        this->dispose();
      }
      return *this;
    }
    
    // dtor は本来 dispose する必要があるが、今のところは trivial に
    // 派生先で T の dtor が non-trivial な場合には、そこで dispose() を呼ぶ。
    // ~optional_impl_base_() = default;
    
    
    // observers
    bool is_initialized() const { return impl_.first(); }
    
    void*       address()       { return impl_.second().address(); }
    void const* address() const { return impl_.second().address(); }
    
    T* get() {
      return is_initialized() ? static_cast<T*>( address() ) : 0;
    }
    T const* get() const {
      return is_initialized() ? static_cast<T const*>( address() ) : 0;
    }
    
    
    // modifiers
    void dispose()
    {
      if( T* p = this->get() ) {
        p->~T();
        impl_.first() = false;
      }
    }
    
    template<class... Args>
    void construct( Args&&... args )
    {
      dispose();
      
      ::new( address() ) T( std::forward<Args>(args)... );
      impl_.first() = true;
    }
    
    template<class InPlace>
    void in_place_construct( InPlace && in_place )
    {
      dispose();
      
      apply_in_place<T>( std::forward<InPlace>(in_place), address() );
      impl_.first() = true;
    }
    
    template<class U>
    void assign( U && x )
    {
      return assign_( std::forward<U>(x), 0 );
    }
    
    // swap
    void swap( optional_impl_base_& other )
    {
      if( auto const p1 = this->get() ) {
        if( auto const p2 = other.get() ) {
          using std::swap;
          swap( *p1, *p2 );
        }
        else {
          other.construct( std::forward<T>(*p2) );
          this->dispose();
        }
      }
      else {
        if( auto const p2 = other.get() ) {
          this->construct( std::forward<T>(*p2) );
          other.dispose();
        }
      }
    }
    
   private:
    typedef etude::storage<T> storage_type;
    etude::compressed_pair<bool, storage_type> impl_;
    
    // assign の実装
    // assignable ならば代入を使う
    template< class U,
      class = typename std::enable_if<
        etude::is_assignable<T, U>::value
      >::type
    >
    void assign_( U && x, int )
    {
      if( auto const p = this->get() ) {
        *p = std::forward<U>(x);
      }
      else {
        construct( std::forward<U>(x) );
      }
    }
    // それ以外は単純に construct する
    template<class U>
    void assign_( U && x_, ... )
    {
      T x = std::forward<U>(x_);  // 自己代入回避
      construct( std::forward<T>(x) );
    }
    
  };
  
  // 参照の場合
  template<class T>
  struct optional_impl_base_< T,
    typename std::enable_if<std::is_reference<T>::value>::type
  >
  {
    typedef typename std::add_pointer<T>::type pointer;
    
    optional_impl_base_()
      : p_() {}
    // dtor は trivial
    
    bool is_initialized() const { return p_; }
    pointer get() const { return p_; }
    
    // construct は非テンプレートでおｋ
    void construct( T && x ) {
      p_ = boost::addressof(x);
    }
    
    // in_place は構築不能
    template<class InPlace>
    void in_place_construct( InPlace && in_place ) = delete;
    
    // assign も非テンプレート
    void assign( T && x ) {
      p_ = boost::addressof(x);
    }
    
    void dispose() {
      p_ = 0;
    }
    
    void swap( optional_impl_base_& other ) {
      std::swap( p_, other.p_ );
    }
    
   private:
    pointer p_;
  
  };


  // メタ関数 optional_impl_select_base_
  // T の copy/move の有無に合わせて copy/move の有無を変える
  // T が U&& の場合には copy 不能にする必要あるので参照の場合も select する
  template<class T>
  struct optional_impl_select_base_
  {
    typedef optional_impl_base_<T> base;
    
    typedef typename std::conditional<
      !etude::is_move_constructible<T>::value,
      etude::immovable<base>, typename std::conditional<
        !etude::is_copy_constructible<T>::value,
        etude::noncopyable<base>, base
      >::type
    >::type type;
  
  };
  

  // etude::optional の中で保持されるクラス optional_impl_
  // T の dtor が trivial か否かに応じて切り分ける
  
  // T の dtor が non-trivial の場合
  template<class T, class = void>
  struct optional_impl_
    : optional_impl_select_base_<T>::type
  {
    typedef optional_impl_base_<T> base_;
    
    optional_impl_() = default;
    ~optional_impl_(){ base_::dispose(); }
    
  };
  
  // T の dtor が trivial の場合（参照含む）
  template<class T>
  struct optional_impl_< T,
    typename std::enable_if<
      etude::is_trivially_destructible<T>::value
    >::type
  >
    : optional_impl_select_base_<T>::type
  {
    typedef optional_impl_base_<T> base_;
    
    optional_impl_() = default;
    // dtor は trivial でいい
    // ~optional_impl_() = default;
    
  };
  
  

} // namespace etude

#endif  // #ifndef ETUDE_DETAIL_INCLUDED_OPTIONAL_IMPL_HPP_
