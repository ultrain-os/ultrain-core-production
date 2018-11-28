#include <ultrainio/chain/config.hpp>

namespace ultrainio
{
   namespace chain 
   {
	int chain::config::block_interval_ms = 10*1000;
	int chain::config::block_interval_us = 10*1000*1000;
	uint32_t chain::config::default_max_block_cpu_usage = 3'000'000;
	uint32_t chain::config::default_max_transaction_cpu_usage = chain::config::default_max_block_cpu_usage/2;
   }
}
