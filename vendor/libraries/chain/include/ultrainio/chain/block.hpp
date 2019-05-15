#pragma once
#include <ultrainio/chain/block_header.hpp>
#include <ultrainio/chain/transaction.hpp>

namespace ultrainio { namespace chain {

   /**
    * When a transaction is referenced by a block it could imply one of several outcomes which
    * describe the state-transition undertaken by the block producer.
    */

   struct transaction_receipt_header {
      enum status_enum {
         executed  = 0, ///< succeed, no error handler executed
         soft_fail = 1, ///< objectively failed (not executed), error handler executed
         hard_fail = 2, ///< objectively failed and error handler objectively failed thus no state change
         delayed   = 3, ///< transaction delayed/deferred/scheduled for future execution
         expired   = 4  ///< transaction expired and storage space refuned to user
      };

      transaction_receipt_header():status(hard_fail){}
      transaction_receipt_header( status_enum s ):status(s){}

      friend inline bool operator ==( const transaction_receipt_header& lhs, const transaction_receipt_header& rhs ) {
         return std::tie(lhs.status, lhs.cpu_usage_us, lhs.net_usage_words) == std::tie(rhs.status, rhs.cpu_usage_us, rhs.net_usage_words);
      }

      fc::enum_type<uint8_t,status_enum>   status;
      uint32_t                             cpu_usage_us; ///< total billed CPU usage (microseconds)
      fc::unsigned_int                     net_usage_words; ///<  total billed NET usage, so we can reconstruct resource state when skipping context free data... hard failures...
   };

   struct transaction_receipt : public transaction_receipt_header {

      transaction_receipt():transaction_receipt_header(){}
      transaction_receipt( transaction_id_type tid ):transaction_receipt_header(executed),trx(tid){}
      transaction_receipt( packed_transaction ptrx ):transaction_receipt_header(executed),trx(ptrx){}
      transaction_receipt( packed_generated_transaction pgtrx ):transaction_receipt_header(executed),trx(pgtrx){}

      fc::static_variant<transaction_id_type, packed_transaction, packed_generated_transaction> trx;

      digest_type digest()const {
         digest_type::encoder enc;
         fc::raw::pack( enc, status );
         fc::raw::pack( enc, cpu_usage_us );
         fc::raw::pack( enc, net_usage_words );
         if( trx.contains<transaction_id_type>() )
            fc::raw::pack( enc, trx.get<transaction_id_type>() );
         else if(trx.contains<packed_generated_transaction>()) {
            packed_transaction temp_ptrx(trx.get<packed_generated_transaction>().get_transaction());
            fc::raw::pack( enc, temp_ptrx.packed_digest() );
         }
         else
            fc::raw::pack( enc, trx.get<packed_transaction>().packed_digest() );
         return enc.result();
      }
   };


   /**
    */
   struct signed_block : public signed_block_header {
      using signed_block_header::signed_block_header;
      signed_block() = default;
      signed_block( const signed_block_header& h ):signed_block_header(h){}

      vector<transaction_receipt>   transactions; /// new or generated transactions
      extensions_type               block_extensions;
   };
   using signed_block_ptr = std::shared_ptr<signed_block>;

   // block record struct in database
   struct block_db_record : public signed_block_header {
      using signed_block_header::signed_block_header;
      block_db_record(const signed_block& block):signed_block_header(block),
                                              block_extensions(block.block_extensions){
         for (auto& t : block.transactions) {
            if (t.trx.contains<transaction_id_type>()) {
               trx_ids.emplace_back(t.trx.get<transaction_id_type>());
            } else if(t.trx.contains<packed_generated_transaction>()) {
               trx_ids.emplace_back(t.trx.get<packed_generated_transaction>().id());
            } else {
               trx_ids.emplace_back(t.trx.get<packed_transaction>().id());
            }
         }
      }

      vector<transaction_id_type>   trx_ids;
      extensions_type               block_extensions;
   };

   struct producer_confirmation {
      block_id_type   block_id;
      digest_type     block_digest;
      account_name    producer;
      signature_type  sig;
   };

} } /// ultrainio::chain

FC_REFLECT_ENUM( ultrainio::chain::transaction_receipt::status_enum,
                 (executed)(soft_fail)(hard_fail)(delayed)(expired) )

FC_REFLECT(ultrainio::chain::transaction_receipt_header, (status)(cpu_usage_us)(net_usage_words) )
FC_REFLECT_DERIVED(ultrainio::chain::transaction_receipt, (ultrainio::chain::transaction_receipt_header), (trx) )
FC_REFLECT_DERIVED(ultrainio::chain::signed_block, (ultrainio::chain::signed_block_header), (transactions)(block_extensions) )
FC_REFLECT_DERIVED(ultrainio::chain::block_db_record, (ultrainio::chain::signed_block_header), (trx_ids)(block_extensions) )
