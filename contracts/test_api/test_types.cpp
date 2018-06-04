/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#include <ultrainiolib/ultrainio.hpp>

#include "test_api.hpp"

void test_types::types_size() {

   ultrainio_assert( sizeof(int64_t) == 8, "int64_t size != 8");
   ultrainio_assert( sizeof(uint64_t) ==  8, "uint64_t size != 8");
   ultrainio_assert( sizeof(uint32_t) ==  4, "uint32_t size != 4");
   ultrainio_assert( sizeof(int32_t) ==  4, "int32_t size != 4");
   ultrainio_assert( sizeof(uint128_t) == 16, "uint128_t size != 16");
   ultrainio_assert( sizeof(int128_t) == 16, "int128_t size != 16");
   ultrainio_assert( sizeof(uint8_t) ==  1, "uint8_t size != 1");

   ultrainio_assert( sizeof(account_name) ==  8, "account_name size !=  8");
   ultrainio_assert( sizeof(table_name) ==  8, "table_name size !=  8");
   ultrainio_assert( sizeof(time) ==  4, "time size !=  4");
   ultrainio_assert( sizeof(ultrainio::key256) == 32, "key256 size != 32" );
}

void test_types::char_to_symbol() {

   ultrainio_assert( ultrainio::char_to_symbol('1') ==  1, "ultrainio::char_to_symbol('1') !=  1");
   ultrainio_assert( ultrainio::char_to_symbol('2') ==  2, "ultrainio::char_to_symbol('2') !=  2");
   ultrainio_assert( ultrainio::char_to_symbol('3') ==  3, "ultrainio::char_to_symbol('3') !=  3");
   ultrainio_assert( ultrainio::char_to_symbol('4') ==  4, "ultrainio::char_to_symbol('4') !=  4");
   ultrainio_assert( ultrainio::char_to_symbol('5') ==  5, "ultrainio::char_to_symbol('5') !=  5");
   ultrainio_assert( ultrainio::char_to_symbol('a') ==  6, "ultrainio::char_to_symbol('a') !=  6");
   ultrainio_assert( ultrainio::char_to_symbol('b') ==  7, "ultrainio::char_to_symbol('b') !=  7");
   ultrainio_assert( ultrainio::char_to_symbol('c') ==  8, "ultrainio::char_to_symbol('c') !=  8");
   ultrainio_assert( ultrainio::char_to_symbol('d') ==  9, "ultrainio::char_to_symbol('d') !=  9");
   ultrainio_assert( ultrainio::char_to_symbol('e') == 10, "ultrainio::char_to_symbol('e') != 10");
   ultrainio_assert( ultrainio::char_to_symbol('f') == 11, "ultrainio::char_to_symbol('f') != 11");
   ultrainio_assert( ultrainio::char_to_symbol('g') == 12, "ultrainio::char_to_symbol('g') != 12");
   ultrainio_assert( ultrainio::char_to_symbol('h') == 13, "ultrainio::char_to_symbol('h') != 13");
   ultrainio_assert( ultrainio::char_to_symbol('i') == 14, "ultrainio::char_to_symbol('i') != 14");
   ultrainio_assert( ultrainio::char_to_symbol('j') == 15, "ultrainio::char_to_symbol('j') != 15");
   ultrainio_assert( ultrainio::char_to_symbol('k') == 16, "ultrainio::char_to_symbol('k') != 16");
   ultrainio_assert( ultrainio::char_to_symbol('l') == 17, "ultrainio::char_to_symbol('l') != 17");
   ultrainio_assert( ultrainio::char_to_symbol('m') == 18, "ultrainio::char_to_symbol('m') != 18");
   ultrainio_assert( ultrainio::char_to_symbol('n') == 19, "ultrainio::char_to_symbol('n') != 19");
   ultrainio_assert( ultrainio::char_to_symbol('o') == 20, "ultrainio::char_to_symbol('o') != 20");
   ultrainio_assert( ultrainio::char_to_symbol('p') == 21, "ultrainio::char_to_symbol('p') != 21");
   ultrainio_assert( ultrainio::char_to_symbol('q') == 22, "ultrainio::char_to_symbol('q') != 22");
   ultrainio_assert( ultrainio::char_to_symbol('r') == 23, "ultrainio::char_to_symbol('r') != 23");
   ultrainio_assert( ultrainio::char_to_symbol('s') == 24, "ultrainio::char_to_symbol('s') != 24");
   ultrainio_assert( ultrainio::char_to_symbol('t') == 25, "ultrainio::char_to_symbol('t') != 25");
   ultrainio_assert( ultrainio::char_to_symbol('u') == 26, "ultrainio::char_to_symbol('u') != 26");
   ultrainio_assert( ultrainio::char_to_symbol('v') == 27, "ultrainio::char_to_symbol('v') != 27");
   ultrainio_assert( ultrainio::char_to_symbol('w') == 28, "ultrainio::char_to_symbol('w') != 28");
   ultrainio_assert( ultrainio::char_to_symbol('x') == 29, "ultrainio::char_to_symbol('x') != 29");
   ultrainio_assert( ultrainio::char_to_symbol('y') == 30, "ultrainio::char_to_symbol('y') != 30");
   ultrainio_assert( ultrainio::char_to_symbol('z') == 31, "ultrainio::char_to_symbol('z') != 31");

   for(unsigned char i = 0; i<255; i++) {
      if((i >= 'a' && i <= 'z') || (i >= '1' || i <= '5')) continue;
      ultrainio_assert( ultrainio::char_to_symbol((char)i) == 0, "ultrainio::char_to_symbol() != 0");
   }
}

void test_types::string_to_name() {

   ultrainio_assert( ultrainio::string_to_name("a") == N(a) , "ultrainio::string_to_name(a)" );
   ultrainio_assert( ultrainio::string_to_name("ba") == N(ba) , "ultrainio::string_to_name(ba)" );
   ultrainio_assert( ultrainio::string_to_name("cba") == N(cba) , "ultrainio::string_to_name(cba)" );
   ultrainio_assert( ultrainio::string_to_name("dcba") == N(dcba) , "ultrainio::string_to_name(dcba)" );
   ultrainio_assert( ultrainio::string_to_name("edcba") == N(edcba) , "ultrainio::string_to_name(edcba)" );
   ultrainio_assert( ultrainio::string_to_name("fedcba") == N(fedcba) , "ultrainio::string_to_name(fedcba)" );
   ultrainio_assert( ultrainio::string_to_name("gfedcba") == N(gfedcba) , "ultrainio::string_to_name(gfedcba)" );
   ultrainio_assert( ultrainio::string_to_name("hgfedcba") == N(hgfedcba) , "ultrainio::string_to_name(hgfedcba)" );
   ultrainio_assert( ultrainio::string_to_name("ihgfedcba") == N(ihgfedcba) , "ultrainio::string_to_name(ihgfedcba)" );
   ultrainio_assert( ultrainio::string_to_name("jihgfedcba") == N(jihgfedcba) , "ultrainio::string_to_name(jihgfedcba)" );
   ultrainio_assert( ultrainio::string_to_name("kjihgfedcba") == N(kjihgfedcba) , "ultrainio::string_to_name(kjihgfedcba)" );
   ultrainio_assert( ultrainio::string_to_name("lkjihgfedcba") == N(lkjihgfedcba) , "ultrainio::string_to_name(lkjihgfedcba)" );
   ultrainio_assert( ultrainio::string_to_name("mlkjihgfedcba") == N(mlkjihgfedcba) , "ultrainio::string_to_name(mlkjihgfedcba)" );
   ultrainio_assert( ultrainio::string_to_name("mlkjihgfedcba1") == N(mlkjihgfedcba2) , "ultrainio::string_to_name(mlkjihgfedcba2)" );
   ultrainio_assert( ultrainio::string_to_name("mlkjihgfedcba55") == N(mlkjihgfedcba14) , "ultrainio::string_to_name(mlkjihgfedcba14)" );

   ultrainio_assert( ultrainio::string_to_name("azAA34") == N(azBB34) , "ultrainio::string_to_name N(azBB34)" );
   ultrainio_assert( ultrainio::string_to_name("AZaz12Bc34") == N(AZaz12Bc34) , "ultrainio::string_to_name AZaz12Bc34" );
   ultrainio_assert( ultrainio::string_to_name("AAAAAAAAAAAAAAA") == ultrainio::string_to_name("BBBBBBBBBBBBBDDDDDFFFGG") , "ultrainio::string_to_name BBBBBBBBBBBBBDDDDDFFFGG" );
}

void test_types::name_class() {

   ultrainio_assert( ultrainio::name{ultrainio::string_to_name("azAA34")}.value == N(azAA34), "ultrainio::name != N(azAA34)" );
   ultrainio_assert( ultrainio::name{ultrainio::string_to_name("AABBCC")}.value == 0, "ultrainio::name != N(0)" );
   ultrainio_assert( ultrainio::name{ultrainio::string_to_name("AA11")}.value == N(AA11), "ultrainio::name != N(AA11)" );
   ultrainio_assert( ultrainio::name{ultrainio::string_to_name("11AA")}.value == N(11), "ultrainio::name != N(11)" );
   ultrainio_assert( ultrainio::name{ultrainio::string_to_name("22BBCCXXAA")}.value == N(22), "ultrainio::name != N(22)" );
   ultrainio_assert( ultrainio::name{ultrainio::string_to_name("AAAbbcccdd")} == ultrainio::name{ultrainio::string_to_name("AAAbbcccdd")}, "ultrainio::name == ultrainio::name" );

   uint64_t tmp = ultrainio::name{ultrainio::string_to_name("11bbcccdd")};
   ultrainio_assert(N(11bbcccdd) == tmp, "N(11bbcccdd) == tmp");
}
