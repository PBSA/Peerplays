#pragma once

#include <mutex>
#include <fc/optional.hpp>

namespace sidechain {

template<class T1>
class thread_safe_index {

public:

   using iterator = typename T1::iterator;

   std::pair<iterator,bool> insert( const typename T1::value_type& value ) {
      std::lock_guard<std::recursive_mutex> locker( lock );
      return data.insert( value );
   }

   template<class T2, typename Key>
   void modify( const Key _key, const std::function<void( typename T1::value_type& e)>& func ) {
      std::lock_guard<std::recursive_mutex> locker( lock );
      const auto& obj = *find_iterator<T2>( _key );
      data.modify( data.iterator_to( obj ), [&func]( typename T1::value_type& obj ) { func( obj ); } );
   }

   template<class T2, typename Key>
   void remove( const Key _key ) {
      std::lock_guard<std::recursive_mutex> locker( lock );
      const auto& obj = *find_iterator<T2>( _key );
      data.erase( data.iterator_to( obj ) );
   }

   size_t size() {
      std::lock_guard<std::recursive_mutex> locker( lock );
      return data.size();
   }

   template<class T2, typename Key>
   fc::optional<typename T1::value_type> find( const Key _key ) {
      std::lock_guard<std::recursive_mutex> locker( lock );
      auto& index = data.template get<T2>();
      auto it = index.find( _key );
      if( it != index.end() ) {
         return fc::optional<typename T1::value_type>(*it);
      }
      return fc::optional<typename T1::value_type>();
   }

   template<class T2>
   void safe_for(std::function<void(typename T1::template index<T2>::type::iterator itr1, 
                                    typename T1::template index<T2>::type::iterator itr2)> func) {
       std::lock_guard<std::recursive_mutex> locker( lock );
       auto& index = data.template get<T2>();
       func(index.begin(), index.end());
   }

private:

   template<class T2, typename Key>
   typename T1::template index<T2>::type::iterator find_iterator( const Key _key ) {
      auto& index = data.template get<T2>();
      auto it = index.find( _key );
      return it;
   }


   std::recursive_mutex lock;
   
   T1 data;

};

}
