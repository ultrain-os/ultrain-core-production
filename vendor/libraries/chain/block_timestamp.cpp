#include <ultrainio/chain/block_timestamp.hpp>
#include <fc/variant.hpp>






namespace fc{
  void to_variant(const ultrainio::chain::block_timestamp&t,fc::variant&v)
  {
	to_variant((fc::time_point)t,v);
  }
  void from_variant(const fc::variant&v,ultrainio::chain::block_timestamp&t)
  {
	t=v.as<fc::time_point>();
  }
}
