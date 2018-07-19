#pragma once

#include <cstdint>
#include <initializer_list>
#include <map>
#include "contrib.hpp"

namespace CryptoNote {
namespace parameters {

const uint64_t CRYPTONOTE_MAX_BLOCK_NUMBER                   = 500000000;
const size_t   CRYPTONOTE_MAX_BLOCK_BLOB_SIZE                = 500000000;
const size_t   CRYPTONOTE_MAX_TX_SIZE                        = 1000000000;
const uint64_t CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX       = 0xe3d9f55;  // ckbzz //9524; // addresses start with "Xa"
const size_t   CRYPTONOTE_MINED_MONEY_UNLOCK_WINDOW          = 10; 
const uint64_t CRYPTONOTE_DEFAULT_TX_SPENDABLE_AGE           = 10;

const size_t   BLOCKCHAIN_TIMESTAMP_CHECK_WINDOW             = 30;
const size_t   BLOCKCHAIN_TIMESTAMP_CHECK_WINDOW_V1          = 11;  //jagerman's patch 

const uint64_t MONEY_SUPPLY                                  = UINT64_C(2100000000000000000);

const size_t   CRYPTONOTE_COIN_VERSION                       = 1;
const size_t   MIN_MIXIN                          	     = 0;

const size_t   CRYPTONOTE_REWARD_BLOCKS_WINDOW               = 100;
const size_t   CRYPTONOTE_BLOCK_GRANTED_FULL_REWARD_ZONE     = 1000000; //size of block (bytes) after which reward for block calculated using block size
const size_t   CRYPTONOTE_BLOCK_GRANTED_FULL_REWARD_ZONE_V2      = 1000000;
const size_t   CRYPTONOTE_BLOCK_GRANTED_FULL_REWARD_ZONE_V1      = 100000;
const size_t   CRYPTONOTE_BLOCK_GRANTED_FULL_REWARD_ZONE_CURRENT = CRYPTONOTE_BLOCK_GRANTED_FULL_REWARD_ZONE;
const size_t   CRYPTONOTE_COINBASE_BLOB_RESERVED_SIZE            = 600;
const size_t   CRYPTONOTE_DISPLAY_DECIMAL_POINT                  = 8;

// COIN - number of smallest units in one coin
const uint64_t POINT                                         = UINT64_C(1000); // pow(10, 3)
const uint64_t COIN                                          = UINT64_C(100000000); // pow(10, 6)
const uint64_t TAIL_EMISSION_REWARD                          = UINT64_C(10000000000);
const uint64_t PRE_BLOCK_REWARD	                             = UINT64_C(84100000000000000); // premine first block
const uint64_t MINIMUM_FEE                                   = UINT64_C(1000000); // pow(10, 5)
const uint64_t DEFAULT_DUST_THRESHOLD                        = UINT64_C(1000000); // pow(10, 5)
const unsigned EMISSION_SPEED_FACTOR 			     = 20;
static_assert(EMISSION_SPEED_FACTOR <= 8 * sizeof(uint64_t), "Bad EMISSION_SPEED_FACTOR");

const uint64_t DIFFICULTY_TARGET                             = 120; // LWMA-2
const uint64_t CRYPTONOTE_BLOCK_FUTURE_TIME_LIMIT            = 60 * 60 * 2;
const uint64_t CRYPTONOTE_BLOCK_FUTURE_TIME_LIMIT_V1         = 3 * DIFFICULTY_TARGET;  //LWMA
const uint64_t EXPECTED_NUMBER_OF_BLOCKS_PER_DAY             = 24 * 60 * 60 / DIFFICULTY_TARGET;
const size_t   DIFFICULTY_WINDOW                             = EXPECTED_NUMBER_OF_BLOCKS_PER_DAY; // blocks
const size_t   DIFFICULTY_WINDOW_V2                          = 17;  // blocks
const size_t   DIFFICULTY_WINDOW_V3                          = 60 + 1;  // blocks
const size_t   DIFFICULTY_CUT                                = 30;  // timestamps to cut after sorting

const uint64_t DEPOSIT_MIN_AMOUNT                            = 1 * COIN;
const uint32_t DEPOSIT_MIN_TERM                              = 22000; // ~1 month [this block of 1 mo]
const uint32_t DEPOSIT_MAX_TERM                              = 1 * 12 * 22000; // ~1 year
const uint64_t DEPOSIT_MIN_TOTAL_RATE_FACTOR                 = 0; // rate is constant
const uint64_t DEPOSIT_MAX_TOTAL_RATE                        = 31; // Apr % per year
static_assert(DEPOSIT_MIN_TERM > 0, "Bad DEPOSIT_MIN_TERM");
static_assert(DEPOSIT_MIN_TERM <= DEPOSIT_MAX_TERM, "Bad DEPOSIT_MAX_TERM");
static_assert(DEPOSIT_MIN_TERM * DEPOSIT_MAX_TOTAL_RATE > DEPOSIT_MIN_TOTAL_RATE_FACTOR, "Bad DEPOSIT_MIN_TOTAL_RATE_FACTOR or DEPOSIT_MAX_TOTAL_RATE");

const size_t   MAX_BLOCK_SIZE_INITIAL                        = 1000000;
const uint64_t MAX_BLOCK_SIZE_GROWTH_SPEED_NUMERATOR         = 100 * 1024;
const uint64_t MAX_BLOCK_SIZE_GROWTH_SPEED_DENOMINATOR       = 365 * 24 * 60 * 60 / DIFFICULTY_TARGET;

const uint64_t CRYPTONOTE_LOCKED_TX_ALLOWED_DELTA_BLOCKS     = 1;
const uint64_t CRYPTONOTE_LOCKED_TX_ALLOWED_DELTA_SECONDS    = DIFFICULTY_TARGET * CRYPTONOTE_LOCKED_TX_ALLOWED_DELTA_BLOCKS;

const size_t   CRYPTONOTE_MAX_TX_SIZE_LIMIT                  = (CRYPTONOTE_BLOCK_GRANTED_FULL_REWARD_ZONE / 4) - CRYPTONOTE_COINBASE_BLOB_RESERVED_SIZE;

const uint64_t CRYPTONOTE_MEMPOOL_TX_LIVETIME                = (60 * 60 * 14); // seconds, 14 hours
const uint64_t CRYPTONOTE_MEMPOOL_TX_FROM_ALT_BLOCK_LIVETIME = (60 * 60 * 24); // seconds, one day
const uint64_t CRYPTONOTE_NUMBER_OF_PERIODS_TO_FORGET_TX_DELETED_FROM_POOL = 7; // CRYPTONOTE_NUMBER_OF_PERIODS_TO_FORGET_TX_DELETED_FROM_POOL * CRYPTONOTE_MEMPOOL_TX_LIVETIME = time to forget tx

const size_t   FUSION_TX_MAX_SIZE                            = CRYPTONOTE_BLOCK_GRANTED_FULL_REWARD_ZONE_CURRENT * 30 / 100;
const size_t   FUSION_TX_MIN_INPUT_COUNT                     = 12;
const size_t   FUSION_TX_MIN_IN_OUT_COUNT_RATIO              = 4;

const uint32_t UPGRADE_HEIGHT_V1                             = 136212;
const uint32_t UPGRADE_HEIGHT_V2                             = 317950;
const uint32_t UPGRADE_HEIGHT_V3                             = 338000;
const uint32_t UPGRADE_HEIGHT_V4                             = 668946;  //June 25 12AM UTC
const unsigned UPGRADE_VOTING_THRESHOLD                      = 90; // percent
const uint32_t UPGRADE_VOTING_WINDOW                         = EXPECTED_NUMBER_OF_BLOCKS_PER_DAY; // blocks
const uint32_t UPGRADE_WINDOW                                = EXPECTED_NUMBER_OF_BLOCKS_PER_DAY; // blocks
static_assert(0 < UPGRADE_VOTING_THRESHOLD && UPGRADE_VOTING_THRESHOLD <= 100, "Bad UPGRADE_VOTING_THRESHOLD");
static_assert(UPGRADE_VOTING_WINDOW > 1, "Bad UPGRADE_VOTING_WINDOW");

const char     CRYPTONOTE_BLOCKS_FILENAME[]                  = "blocks.dat";
const char     CRYPTONOTE_BLOCKINDEXES_FILENAME[]            = "blockindexes.dat";
const char     CRYPTONOTE_BLOCKSCACHE_FILENAME[]             = "blockscache.dat";
const char     CRYPTONOTE_POOLDATA_FILENAME[]                = "poolstate.bin";
const char     P2P_NET_DATA_FILENAME[]                       = "p2pstate.bin";
const char     CRYPTONOTE_BLOCKCHAIN_INDICES_FILENAME[]      = "blockchainindices.dat";
const char     MINER_CONFIG_FILE_NAME[]                      = "miner_conf.json";

} // parameters

const char     CRYPTONOTE_NAME[]                             = "kribbz";
const char     GENESIS_COINBASE_TX_HEX[]                     =
//"010a01ff0001c5f5b7d9a43a029b2e4c0281c0b02e7c53291a94d1d0cbff8883f8024f5142ee494ffbbd0880712101c8b3eb861829c338eded4298e4ed66a1e41cbcc884a779c177e46f480014e08d";
"010a01ff0001c5f5b7d9a43a029b2e4c0281c0b02e7c53291a94d1d0cbff8883f8024f5142ee494ffbbd08807121012d8363178452b4bda8b01f0cb5babd1279bf1b2eb8df78432fb9976e12513875";

const uint32_t GENESIS_NONCE                                 = 70;
const uint64_t GENESIS_TIMESTAMP                             = 1529201466;

const uint8_t  CURRENT_TRANSACTION_VERSION                   = 1;
const uint8_t  CURRENT_BLOCK_MAJOR                           = 1; //if put current 2 then next is 3 and soon....
const uint8_t  NEXT_BLOCK_MAJOR_LIMIT                        = 5; //this Maximum layer block
const uint8_t  BLOCK_MINOR_VERSION_0                         = 0;
const uint8_t  BLOCK_MINOR_VERSION_1                         = 1;

const size_t   BLOCKS_IDS_SYNCHRONIZING_DEFAULT_COUNT        = 10000; // by default, blocks ids count in synchronizing
const size_t   BLOCKS_SYNCHRONIZING_DEFAULT_COUNT            = 128; // by default, blocks count in blocks downloading
const size_t   COMMAND_RPC_GET_BLOCKS_FAST_MAX_COUNT         = 1000;

const int      P2P_DEFAULT_PORT                              = 23925;//5290;
const int      RPC_DEFAULT_PORT                              = 23926;//5295;

const size_t   P2P_LOCAL_WHITE_PEERLIST_LIMIT                = 1000;
const size_t   P2P_LOCAL_GRAY_PEERLIST_LIMIT                 = 5000;

const size_t   P2P_CONNECTION_MAX_WRITE_BUFFER_SIZE          = 64 * 1024 * 1024; // 64 MB
const uint32_t P2P_DEFAULT_CONNECTIONS_COUNT                 = 8;
const size_t   P2P_DEFAULT_WHITELIST_CONNECTIONS_PERCENT     = 70;
const uint32_t P2P_DEFAULT_HANDSHAKE_INTERVAL                = 60; // seconds
const uint32_t P2P_DEFAULT_PACKET_MAX_SIZE                   = 50000000; // 50000000 bytes maximum packet size
const uint32_t P2P_DEFAULT_PEERS_IN_HANDSHAKE                = 250;
const uint32_t P2P_DEFAULT_CONNECTION_TIMEOUT                = 5000; // 5 seconds
const uint32_t P2P_DEFAULT_PING_CONNECTION_TIMEOUT           = 2000; // 2 seconds
const uint64_t P2P_DEFAULT_INVOKE_TIMEOUT                    = 60 * 2 * 1000; // 2 minutes
const size_t   P2P_DEFAULT_HANDSHAKE_INVOKE_TIMEOUT          = 5000; // 5 seconds
//const char     P2P_STAT_TRUSTED_PUB_KEY[]                    = "FF9507CA55455F37A3B783EE2C5123B8B6A34A0C5CAAE050922C6254161480C1";
const char     P2P_STAT_TRUSTED_PUB_KEY[]                    = "0000000000000000000000000000000000000000000000000000000000000000";

const std::initializer_list<const char*> SEED_NODES = {
//  "seed1.catalyst.cash:5290",
//  "seed2.catalyst.cash:5290",
//"kribbzseed1.com:23925",F
//"kribbzseed2.com:23925",

  "54.70.55.32:23925",
  "172.31.15.56:23925",
  "52.13.195.226:23925",
  "172.31.20.249:23925",

};

struct CheckpointData {
  uint32_t height;
  const char* blockId;
};

#ifdef __GNUC__
__attribute__((unused))
#endif

// You may add here other checkpoints using the following format:
// {<block height>, "<block hash>"},
const std::initializer_list<CheckpointData> CHECKPOINTS = {
     {1,	"adc114785cca62b5cbdeba8d3ed71aa9cd4c4426c434ce2e412e97f6ba5594f2" },

//  {1000,	"3818c26bbb67d523ef131939ce8adf98b1c5266f1da535728624701997aedee8" },
//  {2000,	"0b988d37f053ab50a25ebc67eb951dc6af52cf5cf452921332026ef50026c3d8" },
//  {3000,	"e950923633648b5d4fa2166fdde572accf03d9b5ab068c1c7b8ac914cddc61d3" },
//  {4000,	"d8be2bea155b1d0e255d2d12d6492a368f0a32a1d4617dbd7a9471253851dd17" },
//  {5730,	"7a8e192ca2334cc6d369575acb7731b6d8ee75d6ea1d286c8550a147fa8bcb2a" },
//  {5820,	"1dd963fa6f02f4115a63419e0d64e005ca046a985c5303a4d9d4d0c8dda45dd3" },
//  {6149,	"ba6e50247e73f02ea9961aa2b91ecaab63c50d0fd8f2c5e2c007fb978ee2a649" },
//  {6431,	"57bd7d6c9921d934086c098f74c4ea410471e9b4729fe2e3d408a16a55f0ea2b" },
//  {6567,	"4c5fc21d61afd9ebdfa331e5367be36ff61ddb95ccd7746160856655e0c86533" },
//  {7045,	"9415d1b923b4c0ea8a987891f69e596460d1af42b97e1e1acb3860b627f8cfec" },
};

const std::map<const uint32_t, const uint8_t> Version = {
	// {BlockIndex , Version}
	{ 2500000, 1 }
};

} // CryptoNote

#define ALLOW_DEBUG_COMMANDS

