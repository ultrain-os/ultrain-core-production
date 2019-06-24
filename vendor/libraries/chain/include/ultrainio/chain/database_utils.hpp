/**
 *  @file
 *  @copyright defined in ultrainio/LICENSE.txt
 */
#pragma once

#include <ultrainio/chain/types.hpp>
#include <fc/io/raw.hpp>
#include <softfloat.hpp>

namespace ultrainio { namespace chain {

   template<typename ...Indices>
   class index_set;

   template<typename Index>
   class index_utils {
      public:
         using index_t = Index;

         template<typename F>
         static void walk( const chainbase::database& db, F function ) {
            auto const& index = db.get_index<Index>().backup_indices();
            const auto& first = index.begin();
            const auto& last = index.end();
            for (auto itr = first; itr != last; ++itr) {
               function(*itr);
            }
         }

         template<typename Secondary, typename Key, typename F>
         static void walk_range( const chainbase::database& db, const Key& begin_key, const Key& end_key, F function ) {
            const auto& idx = db.get_index<Index>().backup_indices().template get<Secondary>();
            auto begin_itr = idx.lower_bound(begin_key);
            auto end_itr = idx.lower_bound(end_key);
            for (auto itr = begin_itr; itr != end_itr; ++itr) {
               function(*itr);
            }
         }

         template<typename Secondary, typename Key>
         static size_t size_range( const chainbase::database& db, const Key& begin_key, const Key& end_key ) {
            const auto& idx = db.get_index<Index>().backup_indices().template get<Secondary>();
            auto begin_itr = idx.lower_bound(begin_key);
            auto end_itr = idx.lower_bound(end_key);
            size_t res = 0;
            while (begin_itr != end_itr) {
               res++; ++begin_itr;
            }
            return res;
         }

         template<typename F>
         static void create( chainbase::database& db, F cons ,bool ws=false) {
            if (ws)
                db.backup_create<typename index_t::value_type>(cons);
            else
                db.create<typename index_t::value_type>(cons);
         }
   };

   template<typename Index>
   class index_set<Index> {
   public:
      static void add_indices( chainbase::database& db ) {
         db.add_index<Index>();
      }

      template<typename F>
      static void walk_indices( F function ) {
         function( index_utils<Index>() );
      }
   };

   template<typename FirstIndex, typename ...RemainingIndices>
   class index_set<FirstIndex, RemainingIndices...> {
   public:
      static void add_indices( chainbase::database& db ) {
         index_set<FirstIndex>::add_indices(db);
         index_set<RemainingIndices...>::add_indices(db);
      }

      template<typename F>
      static void walk_indices( F function ) {
         index_set<FirstIndex>::walk_indices(function);
         index_set<RemainingIndices...>::walk_indices(function);
      }
   };

} }

namespace fc {

   // overloads for to/from_variant
   template<typename OidType>
   void to_variant( const chainbase::oid<OidType>& oid, variant& v ) {
      v = variant(oid._id);
   }

   template<typename OidType>
   void from_variant( const variant& v, chainbase::oid<OidType>& oid ) {
      from_variant(v, oid._id);
   }

   inline
   void to_variant( const float64_t& f, variant& v ) {
      v = variant(*reinterpret_cast<const double*>(&f));
   }

   inline
   void from_variant( const variant& v, float64_t& f ) {
      from_variant(v, *reinterpret_cast<double*>(&f));
   }

   inline
   void to_variant( const float128_t& f, variant& v ) {
      v = variant(*reinterpret_cast<const uint128_t*>(&f));
   }

   inline
   void from_variant( const variant& v, float128_t& f ) {
      from_variant(v, *reinterpret_cast<uint128_t*>(&f));
   }

   inline
   void to_variant( const ultrainio::chain::shared_string& s, variant& v ) {
      v = variant(std::string(s.begin(), s.end()));
   }

   inline
   void from_variant( const variant& v, ultrainio::chain::shared_string& s ) {
      string _s;
      from_variant(v, _s);
      s = ultrainio::chain::shared_string(_s.begin(), _s.end(), s.get_allocator());
   }

   inline
   void to_variant( const blob& b, variant& v ) {
      v = variant(base64_encode(b.data.data(), b.data.size()));
   }

   inline
   void from_variant( const variant& v, blob& b ) {
      string _s = base64_decode(v.as_string());
      b.data = std::vector<char>(_s.begin(), _s.end());
   }

   template<typename T>
   void to_variant( const ultrainio::chain::shared_vector<T>& sv, variant& v ) {
      to_variant(std::vector<T>(sv.begin(), sv.end()), v);
   }

   template<typename T>
   void from_variant( const variant& v, ultrainio::chain::shared_vector<T>& sv ) {
      std::vector<T> _v;
      from_variant(v, _v);
      sv = ultrainio::chain::shared_vector<T>(_v.begin(), _v.end(), sv.get_allocator());
   }

   template<typename T>
   void to_variant( const ultrainio::chain::shared_set<T>& s, variant& v ) {
      to_variant(std::vector<T>(s.begin(), s.end()), v);
   }

   template<typename T>
   void from_variant( const variant& v, ultrainio::chain::shared_set<T>& s ) {
      std::vector<T> _v;
      from_variant(v, _v);
      s = ultrainio::chain::shared_set<T>(_v.begin(), _v.end(), s.get_allocator());
   }

     //shared_set
   template<typename Stream, typename T>
   inline void pack( Stream& s, const ultrainio::chain::shared_set<T>& value ) {
      FC_ASSERT( value.size() <= MAX_NUM_ARRAY_ELEMENTS );
      fc::raw::pack( s, unsigned_int((uint32_t)value.size()) );
      auto itr = value.begin();
      auto end = value.end();
      while( itr != end ) {
        fc::raw::pack( s, *itr );
        ++itr;
      }
    }

   template<typename Stream, typename T>
   inline void unpack( Stream& s, ultrainio::chain::shared_set<T>& value ) {
      unsigned_int size; fc::raw::unpack( s, size );
      FC_ASSERT( size.value <= MAX_NUM_ARRAY_ELEMENTS );
      for( uint64_t i = 0; i < size.value; ++i )
      {
        T tmp(value.get_allocator());
        fc::raw::unpack( s, tmp );
        value.insert( std::move(tmp) );
      }
    }
}

namespace chainbase {
   // overloads for OID packing
   template<typename DataStream, typename OidType>
   DataStream& operator << ( DataStream& ds, const oid<OidType>& oid ) {
      fc::raw::pack(ds, oid._id);
      return ds;
   }

   template<typename DataStream, typename OidType>
   DataStream& operator >> ( DataStream& ds, oid<OidType>& oid ) {
      fc::raw::unpack(ds, oid._id);
      return ds;
   }
}

// overloads for softfloat packing
template<typename DataStream>
DataStream& operator << ( DataStream& ds, const float64_t& v ) {
   fc::raw::pack(ds, *reinterpret_cast<const double *>(&v));
   return ds;
}

template<typename DataStream>
DataStream& operator >> ( DataStream& ds, float64_t& v ) {
   fc::raw::unpack(ds, *reinterpret_cast<double *>(&v));
   return ds;
}

template<typename DataStream>
DataStream& operator << ( DataStream& ds, const float128_t& v ) {
   fc::raw::pack(ds, *reinterpret_cast<const ultrainio::chain::uint128_t*>(&v));
   return ds;
}

template<typename DataStream>
DataStream& operator >> ( DataStream& ds, float128_t& v ) {
   fc::raw::unpack(ds, *reinterpret_cast<ultrainio::chain::uint128_t*>(&v));
   return ds;
}

