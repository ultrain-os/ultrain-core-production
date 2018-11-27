#pragma once
#include <ultrainio/chain/config.hpp>

#include <stdint.h>
#include <fc/time.hpp>
#include <fc/variant.hpp>
#include <fc/string.hpp>
#include <fc/optional.hpp>
#include <fc/exception/exception.hpp>

namespace ultrainio { namespace chain {

   /**
   * This class is used in the block headers to represent the block time
   **/
   class block_timestamp {
      public:
         explicit block_timestamp( uint32_t s=0 ) :abstime(s){}

         block_timestamp(const fc::time_point& t) {
            set_time_point(t);
         }

         block_timestamp(const fc::time_point_sec& t) {
            set_time_point(t);
         }

         static block_timestamp maximum() { return block_timestamp( 0xffff ); }
         static block_timestamp min() { return block_timestamp(0); }

         block_timestamp next() const {
            ULTRAIN_ASSERT( std::numeric_limits<uint32_t>::max() - abstime > (config::block_interval_ms / 1000),
                            fc::overflow_exception,
                            "block timestamp overflow" );
            auto result = block_timestamp(*this);
            result.abstime += config::block_interval_ms / 1000;
            return result;
         }

         fc::time_point to_time_point() const {
            return (fc::time_point)(*this);
         }

         operator fc::time_point() const {
            int64_t msec = abstime * 1000ll;
            msec += config::block_timestamp_epoch;
            return fc::time_point(fc::milliseconds(msec));
         }

         void operator = (const fc::time_point& t ) {
            set_time_point(t);
         }

         bool   operator > ( const block_timestamp& t )const   { return abstime >  t.abstime; }
         bool   operator >=( const block_timestamp& t )const   { return abstime >= t.abstime; }
         bool   operator < ( const block_timestamp& t )const   { return abstime <  t.abstime; }
         bool   operator <=( const block_timestamp& t )const   { return abstime <= t.abstime; }
         bool   operator ==( const block_timestamp& t )const   { return abstime == t.abstime; }
         bool   operator !=( const block_timestamp& t )const   { return abstime != t.abstime; }
         uint32_t abstime;

      private:
      void set_time_point(const fc::time_point& t) {
         auto micro_since_epoch = t.time_since_epoch();
         auto msec_since_epoch  = micro_since_epoch.count() / 1000;
         abstime = ( msec_since_epoch - config::block_timestamp_epoch ) / 1000;
      }

      void set_time_point(const fc::time_point_sec& t) {
         uint64_t  sec_since_epoch = t.sec_since_epoch();
         abstime = (sec_since_epoch * 1000 - config::block_timestamp_epoch) / 1000 ;
      }
   }; // block_timestamp

  typedef block_timestamp block_timestamp_type;
} } /// ultrainio::chain


#include <fc/reflect/reflect.hpp>
FC_REFLECT(ultrainio::chain::block_timestamp_type, (abstime))

namespace fc {
  void to_variant(const ultrainio::chain::block_timestamp& t, fc::variant& v);
  void from_variant(const fc::variant& v, ultrainio::chain::block_timestamp& t);
}

#ifdef _MSC_VER
  #pragma warning (pop)
#endif /// #ifdef _MSC_VER
