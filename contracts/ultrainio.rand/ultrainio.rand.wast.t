(module
 (type $ii (func (param i32) (result i32)))
 (type $iiv (func (param i32 i32)))
 (type $iii (func (param i32 i32) (result i32)))
 (type $v (func))
 (type $iiiv (func (param i32 i32 i32)))
 (type $iv (func (param i32)))
 (type $iIii (func (param i32 i64 i32) (result i32)))
 (type $Iiv (func (param i64 i32)))
 (type $IIIIv (func (param i64 i64 i64 i64)))
 (type $iIi (func (param i32 i64) (result i32)))
 (type $iIv (func (param i32 i64)))
 (type $iI (func (param i32) (result i64)))
 (type $IIIIi (func (param i64 i64 i64 i64) (result i32)))
 (type $i (func (result i32)))
 (type $I (func (result i64)))
 (type $iIiv (func (param i32 i64 i32)))
 (type $iiii (func (param i32 i32 i32) (result i32)))
 (type $IIIIiii (func (param i64 i64 i64 i64 i32 i32) (result i32)))
 (type $IIii (func (param i64 i64 i32) (result i32)))
 (type $IIIi (func (param i64 i64 i64) (result i32)))
 (type $iiiiiv (func (param i32 i32 i32 i32 i32)))
 (type $FiF (func (param f64 i32) (result f64)))
 (type $Ii (func (param i64) (result i32)))
 (type $iiiiv (func (param i32 i32 i32 i32)))
 (type $IIiiv (func (param i64 i64 i32 i32)))
 (type $iIiiv (func (param i32 i64 i32 i32)))
 (type $iIiii (func (param i32 i64 i32 i32) (result i32)))
 (type $Iiiii (func (param i64 i32 i32 i32) (result i32)))
 (type $iiF (func (param i32 i32) (result f64)))
 (type $FUNCSIG$vii (func (param i32 i32)))
 (type $FUNCSIG$ii (func (param i32) (result i32)))
 (type $FUNCSIG$j (func (result i64)))
 (type $FUNCSIG$ijj (func (param i64 i64) (result i32)))
 (type $FUNCSIG$ijjj (func (param i64 i64 i64) (result i32)))
 (type $FUNCSIG$iii (func (param i32 i32) (result i32)))
 (type $FUNCSIG$ij (func (param i64) (result i32)))
 (type $FUNCSIG$iiii (func (param i32 i32 i32) (result i32)))
 (type $FUNCSIG$dd (func (param f64) (result f64)))
 (type $FUNCSIG$viii (func (param i32 i32 i32)))
 (type $FUNCSIG$iji (func (param i64 i32) (result i32)))
 (type $FUNCSIG$vijii (func (param i32 i64 i32 i32)))
 (type $FUNCSIG$ijjii (func (param i64 i64 i32 i32) (result i32)))
 (type $FUNCSIG$i (func (result i32)))
 (import "env" "abort" (func $~lib/env/abort))
 (import "env" "ultrainio_assert" (func $~lib/env/ultrainio_assert (param i32 i32)))
 (import "env" "ts_log_print_s" (func $~lib/ultrain-ts-lib/src/log/env.ts_log_print_s (param i32)))
 (import "env" "ts_log_print_i" (func $~lib/ultrain-ts-lib/src/log/env.ts_log_print_i (param i64 i32)))
 (import "env" "ts_log_done" (func $~lib/ultrain-ts-lib/src/log/env.ts_log_done))
 (import "env" "db_find_i64" (func $~lib/env/db_find_i64 (param i64 i64 i64 i64) (result i32)))
 (import "env" "head_block_number" (func $~lib/ultrain-ts-lib/lib/headblock/env.head_block_number (result i32)))
 (import "env" "current_sender" (func $~lib/ultrain-ts-lib/internal/action.d/env.current_sender (result i64)))
 (import "env" "current_receiver" (func $~lib/env/current_receiver (result i64)))
 (import "env" "db_store_i64" (func $~lib/env/db_store_i64 (param i64 i64 i64 i64 i32 i32) (result i32)))
 (import "env" "action_data_size" (func $~lib/ultrain-ts-lib/internal/action.d/env.action_data_size (result i32)))
 (import "env" "read_action_data" (func $~lib/ultrain-ts-lib/internal/action.d/env.read_action_data (param i32 i32) (result i32)))
 (import "env" "db_drop_i64" (func $~lib/env/db_drop_i64 (param i64 i64 i64) (result i32)))
 (import "env" "db_remove_i64" (func $~lib/env/db_remove_i64 (param i32)))
 (import "env" "send_inline" (func $~lib/ultrain-ts-lib/internal/action.d/env.send_inline (param i32 i32)))
 (import "env" "db_get_i64" (func $~lib/env/db_get_i64 (param i32 i32 i32) (result i32)))
 (import "env" "ts_public_key_of_account" (func $~lib/ultrain-ts-lib/internal/crypto.d/env.ts_public_key_of_account (param i64 i32 i32 i32) (result i32)))
 (import "env" "ts_verify_with_pk" (func $~lib/ultrain-ts-lib/internal/crypto.d/env.ts_verify_with_pk (param i32 i32 i32) (result i32)))
 (import "env" "ts_sha256" (func $~lib/ultrain-ts-lib/internal/crypto.d/env.ts_sha256 (param i32 i32 i32 i32)))
 (import "env" "db_update_i64" (func $~lib/env/db_update_i64 (param i32 i64 i32 i32)))
 (import "env" "set_result_str" (func $~lib/ultrain-ts-lib/src/return/env.set_result_str (param i32)))
 (memory $0 1)
 (data (i32.const 8) "\01\00\00\00 ")
 (data (i32.const 16) "\01\00\00\00!")
 (data (i32.const 24) "\01\00\00\00\"")
 (data (i32.const 32) "\01\00\00\00#")
 (data (i32.const 40) "\01\00\00\00$")
 (data (i32.const 48) "\01\00\00\00%")
 (data (i32.const 56) "\01\00\00\00&")
 (data (i32.const 64) "\01\00\00\00\'")
 (data (i32.const 72) "\01\00\00\00(")
 (data (i32.const 80) "\01\00\00\00)")
 (data (i32.const 88) "\01\00\00\00*")
 (data (i32.const 96) "\01\00\00\00+")
 (data (i32.const 104) "\01\00\00\00,")
 (data (i32.const 112) "\01\00\00\00-")
 (data (i32.const 120) "\01\00\00\00.")
 (data (i32.const 128) "\01\00\00\00/")
 (data (i32.const 136) "\01\00\00\000")
 (data (i32.const 144) "\01\00\00\001")
 (data (i32.const 152) "\01\00\00\002")
 (data (i32.const 160) "\01\00\00\003")
 (data (i32.const 168) "\01\00\00\004")
 (data (i32.const 176) "\01\00\00\005")
 (data (i32.const 184) "\01\00\00\006")
 (data (i32.const 192) "\01\00\00\007")
 (data (i32.const 200) "\01\00\00\008")
 (data (i32.const 208) "\01\00\00\009")
 (data (i32.const 216) "\01\00\00\00:")
 (data (i32.const 224) "\01\00\00\00;")
 (data (i32.const 232) "\01\00\00\00<")
 (data (i32.const 240) "\01\00\00\00=")
 (data (i32.const 248) "\01\00\00\00>")
 (data (i32.const 256) "\01\00\00\00?")
 (data (i32.const 264) "\01\00\00\00@")
 (data (i32.const 272) "\01\00\00\00A")
 (data (i32.const 280) "\01\00\00\00B")
 (data (i32.const 288) "\01\00\00\00C")
 (data (i32.const 296) "\01\00\00\00D")
 (data (i32.const 304) "\01\00\00\00E")
 (data (i32.const 312) "\01\00\00\00F")
 (data (i32.const 320) "\01\00\00\00G")
 (data (i32.const 328) "\01\00\00\00H")
 (data (i32.const 336) "\01\00\00\00I")
 (data (i32.const 344) "\01\00\00\00J")
 (data (i32.const 352) "\01\00\00\00K")
 (data (i32.const 360) "\01\00\00\00L")
 (data (i32.const 368) "\01\00\00\00M")
 (data (i32.const 376) "\01\00\00\00N")
 (data (i32.const 384) "\01\00\00\00O")
 (data (i32.const 392) "\01\00\00\00P")
 (data (i32.const 400) "\01\00\00\00Q")
 (data (i32.const 408) "\01\00\00\00R")
 (data (i32.const 416) "\01\00\00\00T")
 (data (i32.const 424) "\01\00\00\00U")
 (data (i32.const 432) "\01\00\00\00V")
 (data (i32.const 440) "\01\00\00\00W")
 (data (i32.const 448) "\01\00\00\00X")
 (data (i32.const 456) "\01\00\00\00Y")
 (data (i32.const 464) "\01\00\00\00Z")
 (data (i32.const 472) "\01\00\00\00[")
 (data (i32.const 480) "\01\00\00\00\\")
 (data (i32.const 488) "\01\00\00\00]")
 (data (i32.const 496) "\01\00\00\00^")
 (data (i32.const 504) "\01\00\00\00_")
 (data (i32.const 512) "\01\00\00\00`")
 (data (i32.const 520) "\01\00\00\00a")
 (data (i32.const 528) "\01\00\00\00b")
 (data (i32.const 536) "\01\00\00\00c")
 (data (i32.const 544) "\01\00\00\00d")
 (data (i32.const 552) "\01\00\00\00e")
 (data (i32.const 560) "\01\00\00\00f")
 (data (i32.const 568) "\01\00\00\00g")
 (data (i32.const 576) "\01\00\00\00h")
 (data (i32.const 584) "\01\00\00\00i")
 (data (i32.const 592) "\01\00\00\00j")
 (data (i32.const 600) "\01\00\00\00k")
 (data (i32.const 608) "\01\00\00\00l")
 (data (i32.const 616) "\01\00\00\00m")
 (data (i32.const 624) "\01\00\00\00n")
 (data (i32.const 632) "\01\00\00\00o")
 (data (i32.const 640) "\01\00\00\00p")
 (data (i32.const 648) "\01\00\00\00q")
 (data (i32.const 656) "\01\00\00\00r")
 (data (i32.const 664) "\01\00\00\00s")
 (data (i32.const 672) "\01\00\00\00t")
 (data (i32.const 680) "\01\00\00\00u")
 (data (i32.const 688) "\01\00\00\00v")
 (data (i32.const 696) "\01\00\00\00w")
 (data (i32.const 704) "\01\00\00\00x")
 (data (i32.const 712) "\01\00\00\00y")
 (data (i32.const 720) "\01\00\00\00z")
 (data (i32.const 728) "\01\00\00\00{")
 (data (i32.const 736) "\01\00\00\00|")
 (data (i32.const 744) "\01\00\00\00}")
 (data (i32.const 752) "\01\00\00\00~")
 (data (i32.const 760) "|\01\00\00\00\00\00\00\08\00\00\00\10\00\00\00\18\00\00\00 \00\00\00(\00\00\000\00\00\008\00\00\00@\00\00\00H\00\00\00P\00\00\00X\00\00\00`\00\00\00h\00\00\00p\00\00\00x\00\00\00\80\00\00\00\88\00\00\00\90\00\00\00\98\00\00\00\a0\00\00\00\a8\00\00\00\b0\00\00\00\b8\00\00\00\c0\00\00\00\c8\00\00\00\d0\00\00\00\d8\00\00\00\e0\00\00\00\e8\00\00\00\f0\00\00\00\f8\00\00\00\00\01\00\00\08\01\00\00\10\01\00\00\18\01\00\00 \01\00\00(\01\00\000\01\00\008\01\00\00@\01\00\00H\01\00\00P\01\00\00X\01\00\00`\01\00\00h\01\00\00p\01\00\00x\01\00\00\80\01\00\00\88\01\00\00\90\01\00\00\98\01\00\00\98\01\00\00\a0\01\00\00\a8\01\00\00\b0\01\00\00\b8\01\00\00\c0\01\00\00\c8\01\00\00\d0\01\00\00\d8\01\00\00\e0\01\00\00\e8\01\00\00\f0\01\00\00\f8\01\00\00\00\02\00\00\08\02\00\00\10\02\00\00\18\02\00\00 \02\00\00(\02\00\000\02\00\008\02\00\00@\02\00\00H\02\00\00P\02\00\00X\02\00\00`\02\00\00h\02\00\00p\02\00\00x\02\00\00\80\02\00\00\88\02\00\00\90\02\00\00\98\02\00\00\a0\02\00\00\a8\02\00\00\b0\02\00\00\b8\02\00\00\c0\02\00\00\c8\02\00\00\d0\02\00\00\d8\02\00\00\e0\02\00\00\e8\02\00\00\f0\02")
 (data (i32.const 1272) "\f8\02\00\00_")
 (data (i32.const 1280) "\04\00\00\00U\00G\00A\00S")
 (data (i32.const 1296) "+\00\00\00l\00e\00n\00g\00t\00h\00 \00o\00f\00 \00_\00s\00y\00m\00b\00o\00l\00 \00n\00a\00m\00e\00 \00m\00u\00s\00t\00 \00b\00e\00 \00l\00e\00s\00s\00 \00t\00h\00a\00n\00 \007\00.")
 (data (i32.const 1392) "\0d\00\00\00~\00l\00i\00b\00/\00a\00r\00r\00a\00y\00.\00t\00s")
 (data (i32.const 1424) "\1c\00\00\00~\00l\00i\00b\00/\00i\00n\00t\00e\00r\00n\00a\00l\00/\00a\00r\00r\00a\00y\00b\00u\00f\00f\00e\00r\00.\00t\00s")
 (data (i32.const 1488) "\0e\00\00\00~\00l\00i\00b\00/\00s\00t\00r\00i\00n\00g\00.\00t\00s")
 (data (i32.const 1520) "0\00\00\00s\00t\00r\00i\00n\00g\00_\00t\00o\00_\00_\00s\00y\00m\00b\00o\00l\00 \00f\00a\00i\00l\00e\00d\00 \00f\00o\00r\00 \00n\00o\00t\00 \00s\00u\00p\00o\00o\00r\00t\00 \00c\00o\00d\00e\00 \00:\00 ")
 (data (i32.const 1624) "\10\00\00\000\001\002\003\004\005\006\007\008\009\00a\00b\00c\00d\00e\00f")
 (data (i32.const 1664) "\t\00\00\00c\00a\00n\00d\00i\00d\00a\00t\00e")
 (data (i32.const 1688) "\0b\00\00\00s\00.\00c\00a\00n\00d\00i\00d\00a\00t\00e")
 (data (i32.const 1720) "\04\00\00\00v\00o\00t\00e")
 (data (i32.const 1736) "\06\00\00\00s\00.\00v\00o\00t\00e")
 (data (i32.const 1752) "\08\00\00\00b\00l\00o\00c\00k\00n\00u\00m")
 (data (i32.const 1776) "3\00\00\00c\00a\00n\00 \00n\00o\00t\00 \00c\00r\00e\00a\00t\00e\00 \00o\00b\00j\00e\00c\00t\00s\00 \00i\00n\00 \00t\00a\00b\00l\00e\00 \00o\00f\00 \00a\00n\00o\00t\00h\00e\00r\00 \00c\00o\00n\00t\00r\00a\00c\00t")
 (data (i32.const 1888) "\1b\00\00\00~\00l\00i\00b\00/\00i\00n\00t\00e\00r\00n\00a\00l\00/\00t\00y\00p\00e\00d\00a\00r\00r\00a\00y\00.\00t\00s")
 (data (i32.const 1952) "\04\00\00\00s\00e\00e\00d")
 (data (i32.const 1968) "\04\00\00\00r\00a\00n\00d")
 (data (i32.const 1984) "\08\00\00\00t\00r\00a\00n\00s\00f\00e\00r")
 (data (i32.const 2008) "\0b\00\00\00u\00t\00r\00i\00o\00.\00t\00o\00k\00e\00n")
 (data (i32.const 2040) "\07\00\00\00c\00l\00e\00a\00r\00D\00B")
 (data (i32.const 2064) "\"\00\00\00o\00n\00l\00y\00 \00c\00o\00n\00t\00r\00a\00c\00t\00 \00o\00w\00n\00e\00r\00 \00c\00a\00n\00 \00c\00l\00e\00a\00r\00 \00D\00B\00s\00.")
 (data (i32.const 2144) "\17\00\00\00~\00l\00i\00b\00/\00i\00n\00t\00e\00r\00n\00a\00l\00/\00s\00t\00r\00i\00n\00g\00.\00t\00s")
 (data (i32.const 2200) "\0c\00\00\00a\00s\00 \00c\00a\00n\00d\00i\00d\00a\00t\00e")
 (data (i32.const 2232) "\1e\00\00\00c\00a\00n\00n\00o\00t\00 \00a\00d\00d\00 \00e\00x\00i\00s\00t\00i\00n\00g\00 \00c\00a\00n\00d\00i\00d\00a\00t\00e\00.")
 (data (i32.const 2296) "0\00\00\00d\00e\00p\00o\00s\00i\00t\00 \00m\00o\00n\00e\00y\00 \00i\00s\00 \00n\00o\00t\00 \00a\00c\00c\00u\00r\00a\00t\00e\00,\00 \00r\00e\00q\00u\00i\00r\00e\00 \00d\00e\00p\00o\00s\00i\00t\00:\00 ")
 (data (i32.const 2400) "\04\00\00\00n\00u\00l\00l")
 (data (i32.const 2416) "(\00\00\00\00\00\00\00\01\00\00\00\n\00\00\00d\00\00\00\e8\03\00\00\10\'\00\00\a0\86\01\00@B\0f\00\80\96\98\00\00\e1\f5\05\00\ca\9a;")
 (data (i32.const 2480) "p\t\00\00\n")
 (data (i32.const 2488) "(\00\00\00\00\00\00\00\01\00\00\00\n\00\00\00d\00\00\00\e8\03\00\00\10\'\00\00\a0\86\01\00@B\0f\00\80\96\98\00\00\e1\f5\05\00\ca\9a;")
 (data (i32.const 2552) "\b8\t\00\00\n")
 (data (i32.const 2560) "\0e\00\00\00a\00d\00d\00C\00a\00n\00d\00i\00d\00a\00t\00e\00:\00 ")
 (data (i32.const 2592) " \00\00\00.\001\002\003\004\005\00a\00b\00c\00d\00e\00f\00g\00h\00i\00j\00k\00l\00m\00n\00o\00p\00q\00r\00s\00t\00u\00v\00w\00x\00y\00z")
 (data (i32.const 2664) "\0d\00\00\00\00\00\00\00.............")
 (data (i32.const 2696) "h\n\00\00\0d")
 (data (i32.const 2704) "\0f\00\00\00r\00e\00m\00o\00v\00e\00C\00a\00n\00d\00i\00d\00a\00t\00e")
 (data (i32.const 2744) "%\00\00\00c\00a\00n\00n\00o\00t\00 \00r\00e\00m\00o\00v\00e\00 \00n\00o\00n\00-\00e\00x\00i\00s\00t\00i\00n\00g\00 \00c\00a\00n\00d\00i\00d\00a\00t\00e\00.")
 (data (i32.const 2824) "3\00\00\00c\00a\00n\00 \00n\00o\00t\00 \00e\00r\00a\00s\00e\00 \00o\00b\00j\00e\00c\00t\00s\00 \00i\00n\00 \00t\00a\00b\00l\00e\00 \00o\00f\00 \00a\00n\00o\00t\00h\00e\00r\00 \00c\00o\00n\00t\00r\00a\00c\00t\00.")
 (data (i32.const 2936) "\16\00\00\00r\00e\00t\00u\00r\00n\00 \00d\00e\00p\00o\00s\00i\00t\00e\00d\00 \00m\00o\00n\00e\00y")
 (data (i32.const 2984) "\06\00\00\00a\00c\00t\00i\00v\00e")
 (data (i32.const 3008) "\b8\0b")
 (data (i32.const 3024) "\c8\0b")
 (data (i32.const 3032) "\"\00\00\00y\00o\00u\00 \00s\00h\00o\00u\00l\00d\00 \00b\00e\00 \00a\00 \00c\00a\00n\00d\00i\00d\00a\00t\00e\00 \00f\00i\00r\00s\00t\00l\00y\00.")
 (data (i32.const 3104) "\16\00\00\00y\00o\00u\00 \00h\00a\00s\00 \00a\00l\00r\00e\00a\00d\00y\00 \00v\00o\00t\00e\00d\00.")
 (data (i32.const 3152) "\03\00\00\00h\00e\00x")
 (data (i32.const 3168) "\n\00\00\00m\00e\00s\00s\00a\00g\00e\00 \00:\00 ")
 (data (i32.const 3192) "!\00\00\00p\00l\00e\00a\00s\00e\00 \00p\00r\00o\00v\00i\00d\00e\00 \00a\00 \00v\00a\00l\00i\00d\00 \00V\00R\00F\00 \00p\00r\00o\00o\00f\00.")
 (data (i32.const 3264) "7\00\00\00o\00b\00j\00e\00c\00t\00 \00p\00a\00s\00s\00e\00d\00 \00t\00o\00 \00m\00o\00d\00i\00f\00y\00 \00i\00s\00 \00n\00o\00t\00 \00f\00o\00u\00n\00d\00 \00i\00n\00 \00t\00h\00i\00s\00 \00D\00B\00M\00a\00n\00a\00g\00e\00r\00.")
 (data (i32.const 3384) "4\00\00\00c\00a\00n\00 \00n\00o\00t\00 \00m\00o\00d\00i\00f\00y\00 \00o\00b\00j\00e\00c\00t\00s\00 \00i\00n\00 \00t\00a\00b\00l\00e\00 \00o\00f\00 \00a\00n\00o\00t\00h\00e\00r\00 \00c\00o\00n\00t\00r\00a\00c\00t\00.")
 (data (i32.const 3496) "\0b\00\00\00b\00o\00n\00u\00s\00 \00m\00o\00n\00e\00y")
 (data (i32.const 3528) "\07\00\00\00 \00R\00a\00n\00d\00:\00 ")
 (data (i32.const 3552) "\0f\00\00\00 \00B\00l\00o\00c\00k\00.\00n\00u\00m\00b\00e\00r\00:\00 ")
 (data (i32.const 3592) "\05\00\00\00q\00u\00e\00r\00y")
 (data (i32.const 3608) "\07\00\00\00o\00n\00e\00r\00r\00o\00r")
 (table $0 1 anyfunc)
 (elem (i32.const 0) $null)
 (global $~lib/allocator/arena/startOffset (mut i32) (i32.const 0))
 (global $~lib/allocator/arena/offset (mut i32) (i32.const 0))
 (global $~lib/ultrain-ts-lib/src/log/Log (mut i32) (i32.const 0))
 (global $~lib/ultrain-ts-lib/src/asset/SYS (mut i64) (i64.const 0))
 (global $~lib/ultrain-ts-lib/src/asset/SYS_NAME (mut i64) (i64.const 0))
 (global $contract/MyContract/DEPOSIT_AMOUNT (mut i32) (i32.const 0))
 (export "memory" (memory $0))
 (export "table" (table $0))
 (export "apply" (func $contract/MyContract/apply))
 (start $start)
 (func $~lib/allocator/arena/__memory_allocate (; 21 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  get_local $0
  i32.const 1073741824
  i32.gt_u
  if
   unreachable
  end
  get_global $~lib/allocator/arena/offset
  tee_local $2
  get_local $0
  i32.const 1
  tee_local $1
  get_local $0
  get_local $1
  i32.gt_u
  select
  i32.add
  i32.const 7
  i32.add
  i32.const -8
  i32.and
  tee_local $3
  current_memory
  tee_local $1
  i32.const 16
  i32.shl
  i32.gt_u
  if
   get_local $1
   get_local $3
   get_local $2
   i32.sub
   i32.const 65535
   i32.add
   i32.const -65536
   i32.and
   i32.const 16
   i32.shr_u
   tee_local $0
   tee_local $4
   get_local $1
   get_local $4
   i32.gt_s
   select
   grow_memory
   i32.const 0
   i32.lt_s
   if
    get_local $0
    grow_memory
    i32.const 0
    i32.lt_s
    if
     unreachable
    end
   end
  end
  get_local $3
  set_global $~lib/allocator/arena/offset
  get_local $2
 )
 (func $~lib/internal/arraybuffer/computeSize (; 22 ;) (type $ii) (param $0 i32) (result i32)
  i32.const 1
  i32.const 32
  get_local $0
  i32.const 7
  i32.add
  i32.clz
  i32.sub
  i32.shl
 )
 (func $~lib/internal/arraybuffer/allocateUnsafe (; 23 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  get_local $0
  i32.const 1073741816
  i32.gt_u
  if
   call $~lib/env/abort
   unreachable
  end
  get_local $0
  call $~lib/internal/arraybuffer/computeSize
  call $~lib/allocator/arena/__memory_allocate
  tee_local $1
  get_local $0
  i32.store
  get_local $1
 )
 (func $~lib/internal/memory/memset (; 24 ;) (type $FUNCSIG$vii) (param $0 i32) (param $1 i32)
  (local $2 i32)
  get_local $1
  i32.eqz
  if
   return
  end
  get_local $0
  i32.const 0
  i32.store8
  get_local $0
  get_local $1
  i32.add
  i32.const 1
  i32.sub
  i32.const 0
  i32.store8
  get_local $1
  i32.const 2
  i32.le_u
  if
   return
  end
  get_local $0
  i32.const 1
  i32.add
  i32.const 0
  i32.store8
  get_local $0
  i32.const 2
  i32.add
  i32.const 0
  i32.store8
  get_local $0
  get_local $1
  i32.add
  i32.const 2
  i32.sub
  i32.const 0
  i32.store8
  get_local $0
  get_local $1
  i32.add
  i32.const 3
  i32.sub
  i32.const 0
  i32.store8
  get_local $1
  i32.const 6
  i32.le_u
  if
   return
  end
  get_local $0
  i32.const 3
  i32.add
  i32.const 0
  i32.store8
  get_local $0
  get_local $1
  i32.add
  i32.const 4
  i32.sub
  i32.const 0
  i32.store8
  get_local $1
  i32.const 8
  i32.le_u
  if
   return
  end
  get_local $0
  i32.const 0
  get_local $0
  i32.sub
  i32.const 3
  i32.and
  tee_local $2
  i32.add
  tee_local $0
  i32.const 0
  i32.store
  get_local $0
  get_local $1
  get_local $2
  i32.sub
  i32.const -4
  i32.and
  tee_local $1
  i32.add
  i32.const 4
  i32.sub
  i32.const 0
  i32.store
  get_local $1
  i32.const 8
  i32.le_u
  if
   return
  end
  get_local $0
  i32.const 4
  i32.add
  i32.const 0
  i32.store
  get_local $0
  i32.const 8
  i32.add
  i32.const 0
  i32.store
  get_local $0
  get_local $1
  i32.add
  i32.const 12
  i32.sub
  i32.const 0
  i32.store
  get_local $0
  get_local $1
  i32.add
  i32.const 8
  i32.sub
  i32.const 0
  i32.store
  get_local $1
  i32.const 24
  i32.le_u
  if
   return
  end
  get_local $0
  i32.const 12
  i32.add
  i32.const 0
  i32.store
  get_local $0
  i32.const 16
  i32.add
  i32.const 0
  i32.store
  get_local $0
  i32.const 20
  i32.add
  i32.const 0
  i32.store
  get_local $0
  i32.const 24
  i32.add
  i32.const 0
  i32.store
  get_local $0
  get_local $1
  i32.add
  i32.const 28
  i32.sub
  i32.const 0
  i32.store
  get_local $0
  get_local $1
  i32.add
  i32.const 24
  i32.sub
  i32.const 0
  i32.store
  get_local $0
  get_local $1
  i32.add
  i32.const 20
  i32.sub
  i32.const 0
  i32.store
  get_local $0
  get_local $1
  i32.add
  i32.const 16
  i32.sub
  i32.const 0
  i32.store
  get_local $0
  get_local $0
  i32.const 4
  i32.and
  i32.const 24
  i32.add
  tee_local $2
  i32.add
  set_local $0
  get_local $1
  get_local $2
  i32.sub
  set_local $1
  loop $continue|0
   get_local $1
   i32.const 32
   i32.ge_u
   if
    get_local $0
    i64.const 0
    i64.store
    get_local $0
    i32.const 8
    i32.add
    i64.const 0
    i64.store
    get_local $0
    i32.const 16
    i32.add
    i64.const 0
    i64.store
    get_local $0
    i32.const 24
    i32.add
    i64.const 0
    i64.store
    get_local $1
    i32.const 32
    i32.sub
    set_local $1
    get_local $0
    i32.const 32
    i32.add
    set_local $0
    br $continue|0
   end
  end
 )
 (func $~lib/array/Array<u8>#constructor (; 25 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  get_local $0
  i32.const 1073741816
  i32.gt_u
  if
   call $~lib/env/abort
   unreachable
  end
  get_local $0
  call $~lib/internal/arraybuffer/allocateUnsafe
  set_local $2
  i32.const 8
  call $~lib/allocator/arena/__memory_allocate
  tee_local $1
  i32.const 0
  i32.store
  get_local $1
  i32.const 0
  i32.store offset=4
  get_local $1
  get_local $2
  i32.store
  get_local $1
  get_local $0
  i32.store offset=4
  get_local $2
  i32.const 8
  i32.add
  get_local $0
  call $~lib/internal/memory/memset
  get_local $1
 )
 (func $~lib/string/String#charCodeAt (; 26 ;) (type $iii) (param $0 i32) (param $1 i32) (result i32)
  get_local $0
  i32.eqz
  if
   call $~lib/env/abort
   unreachable
  end
  get_local $1
  get_local $0
  i32.load
  i32.ge_u
  if
   i32.const -1
   return
  end
  get_local $0
  get_local $1
  i32.const 1
  i32.shl
  i32.add
  i32.load16_u offset=4
 )
 (func $~lib/internal/memory/memcpy (; 27 ;) (type $iiiv) (param $0 i32) (param $1 i32) (param $2 i32)
  (local $3 i32)
  (local $4 i32)
  loop $continue|0
   get_local $1
   i32.const 3
   i32.and
   get_local $2
   get_local $2
   select
   if
    get_local $0
    tee_local $3
    i32.const 1
    i32.add
    set_local $0
    get_local $3
    block (result i32)
     get_local $1
     tee_local $3
     i32.const 1
     i32.add
     set_local $1
     get_local $3
     i32.load8_u
    end
    i32.store8
    get_local $2
    i32.const 1
    i32.sub
    set_local $2
    br $continue|0
   end
  end
  get_local $0
  i32.const 3
  i32.and
  i32.eqz
  if
   loop $continue|1
    get_local $2
    i32.const 16
    i32.ge_u
    if
     get_local $0
     get_local $1
     i32.load
     i32.store
     get_local $0
     i32.const 4
     i32.add
     get_local $1
     i32.const 4
     i32.add
     i32.load
     i32.store
     get_local $0
     i32.const 8
     i32.add
     get_local $1
     i32.const 8
     i32.add
     i32.load
     i32.store
     get_local $0
     i32.const 12
     i32.add
     get_local $1
     i32.const 12
     i32.add
     i32.load
     i32.store
     get_local $1
     i32.const 16
     i32.add
     set_local $1
     get_local $0
     i32.const 16
     i32.add
     set_local $0
     get_local $2
     i32.const 16
     i32.sub
     set_local $2
     br $continue|1
    end
   end
   get_local $2
   i32.const 8
   i32.and
   if
    get_local $0
    get_local $1
    i32.load
    i32.store
    get_local $0
    i32.const 4
    i32.add
    get_local $1
    i32.const 4
    i32.add
    i32.load
    i32.store
    get_local $0
    i32.const 8
    i32.add
    set_local $0
    get_local $1
    i32.const 8
    i32.add
    set_local $1
   end
   get_local $2
   i32.const 4
   i32.and
   if
    get_local $0
    get_local $1
    i32.load
    i32.store
    get_local $0
    i32.const 4
    i32.add
    set_local $0
    get_local $1
    i32.const 4
    i32.add
    set_local $1
   end
   get_local $2
   i32.const 2
   i32.and
   if
    get_local $0
    get_local $1
    i32.load16_u
    i32.store16
    get_local $0
    i32.const 2
    i32.add
    set_local $0
    get_local $1
    i32.const 2
    i32.add
    set_local $1
   end
   get_local $2
   i32.const 1
   i32.and
   if
    get_local $0
    set_local $3
    get_local $3
    block (result i32)
     get_local $1
     set_local $3
     get_local $3
     i32.load8_u
    end
    i32.store8
   end
   return
  end
  get_local $2
  i32.const 32
  i32.ge_u
  if
   block $break|2
    block $case2|2
     block $case1|2
      block $case0|2
       get_local $0
       i32.const 3
       i32.and
       i32.const 1
       i32.sub
       br_table $case0|2 $case1|2 $case2|2 $break|2
      end
      get_local $1
      i32.load
      set_local $4
      get_local $0
      tee_local $3
      i32.const 1
      i32.add
      set_local $0
      get_local $3
      block (result i32)
       get_local $1
       tee_local $3
       i32.const 1
       i32.add
       set_local $1
       get_local $3
       i32.load8_u
      end
      i32.store8
      get_local $0
      tee_local $3
      i32.const 1
      i32.add
      set_local $0
      get_local $3
      block (result i32)
       get_local $1
       tee_local $3
       i32.const 1
       i32.add
       set_local $1
       get_local $3
       i32.load8_u
      end
      i32.store8
      get_local $0
      tee_local $3
      i32.const 1
      i32.add
      set_local $0
      get_local $3
      block (result i32)
       get_local $1
       tee_local $3
       i32.const 1
       i32.add
       set_local $1
       get_local $3
       i32.load8_u
      end
      i32.store8
      get_local $2
      i32.const 3
      i32.sub
      set_local $2
      loop $continue|3
       get_local $2
       i32.const 17
       i32.ge_u
       if
        get_local $0
        get_local $4
        i32.const 24
        i32.shr_u
        get_local $1
        i32.const 1
        i32.add
        i32.load
        tee_local $3
        i32.const 8
        i32.shl
        i32.or
        i32.store
        get_local $0
        i32.const 4
        i32.add
        get_local $3
        i32.const 24
        i32.shr_u
        get_local $1
        i32.const 5
        i32.add
        i32.load
        tee_local $4
        i32.const 8
        i32.shl
        i32.or
        i32.store
        get_local $0
        i32.const 8
        i32.add
        get_local $4
        i32.const 24
        i32.shr_u
        get_local $1
        i32.const 9
        i32.add
        i32.load
        tee_local $3
        i32.const 8
        i32.shl
        i32.or
        i32.store
        get_local $0
        i32.const 12
        i32.add
        get_local $3
        i32.const 24
        i32.shr_u
        get_local $1
        i32.const 13
        i32.add
        i32.load
        tee_local $4
        i32.const 8
        i32.shl
        i32.or
        i32.store
        get_local $1
        i32.const 16
        i32.add
        set_local $1
        get_local $0
        i32.const 16
        i32.add
        set_local $0
        get_local $2
        i32.const 16
        i32.sub
        set_local $2
        br $continue|3
       end
      end
      br $break|2
     end
     get_local $1
     i32.load
     set_local $4
     get_local $0
     tee_local $3
     i32.const 1
     i32.add
     set_local $0
     get_local $3
     block (result i32)
      get_local $1
      tee_local $3
      i32.const 1
      i32.add
      set_local $1
      get_local $3
      i32.load8_u
     end
     i32.store8
     get_local $0
     tee_local $3
     i32.const 1
     i32.add
     set_local $0
     get_local $3
     block (result i32)
      get_local $1
      tee_local $3
      i32.const 1
      i32.add
      set_local $1
      get_local $3
      i32.load8_u
     end
     i32.store8
     get_local $2
     i32.const 2
     i32.sub
     set_local $2
     loop $continue|4
      get_local $2
      i32.const 18
      i32.ge_u
      if
       get_local $0
       get_local $4
       i32.const 16
       i32.shr_u
       get_local $1
       i32.const 2
       i32.add
       i32.load
       tee_local $3
       i32.const 16
       i32.shl
       i32.or
       i32.store
       get_local $0
       i32.const 4
       i32.add
       get_local $3
       i32.const 16
       i32.shr_u
       get_local $1
       i32.const 6
       i32.add
       i32.load
       tee_local $4
       i32.const 16
       i32.shl
       i32.or
       i32.store
       get_local $0
       i32.const 8
       i32.add
       get_local $4
       i32.const 16
       i32.shr_u
       get_local $1
       i32.const 10
       i32.add
       i32.load
       tee_local $3
       i32.const 16
       i32.shl
       i32.or
       i32.store
       get_local $0
       i32.const 12
       i32.add
       get_local $3
       i32.const 16
       i32.shr_u
       get_local $1
       i32.const 14
       i32.add
       i32.load
       tee_local $4
       i32.const 16
       i32.shl
       i32.or
       i32.store
       get_local $1
       i32.const 16
       i32.add
       set_local $1
       get_local $0
       i32.const 16
       i32.add
       set_local $0
       get_local $2
       i32.const 16
       i32.sub
       set_local $2
       br $continue|4
      end
     end
     br $break|2
    end
    get_local $1
    i32.load
    set_local $4
    get_local $0
    tee_local $3
    i32.const 1
    i32.add
    set_local $0
    get_local $3
    block (result i32)
     get_local $1
     tee_local $3
     i32.const 1
     i32.add
     set_local $1
     get_local $3
     i32.load8_u
    end
    i32.store8
    get_local $2
    i32.const 1
    i32.sub
    set_local $2
    loop $continue|5
     get_local $2
     i32.const 19
     i32.ge_u
     if
      get_local $0
      get_local $4
      i32.const 8
      i32.shr_u
      get_local $1
      i32.const 3
      i32.add
      i32.load
      tee_local $3
      i32.const 24
      i32.shl
      i32.or
      i32.store
      get_local $0
      i32.const 4
      i32.add
      get_local $3
      i32.const 8
      i32.shr_u
      get_local $1
      i32.const 7
      i32.add
      i32.load
      tee_local $4
      i32.const 24
      i32.shl
      i32.or
      i32.store
      get_local $0
      i32.const 8
      i32.add
      get_local $4
      i32.const 8
      i32.shr_u
      get_local $1
      i32.const 11
      i32.add
      i32.load
      tee_local $3
      i32.const 24
      i32.shl
      i32.or
      i32.store
      get_local $0
      i32.const 12
      i32.add
      get_local $3
      i32.const 8
      i32.shr_u
      get_local $1
      i32.const 15
      i32.add
      i32.load
      tee_local $4
      i32.const 24
      i32.shl
      i32.or
      i32.store
      get_local $1
      i32.const 16
      i32.add
      set_local $1
      get_local $0
      i32.const 16
      i32.add
      set_local $0
      get_local $2
      i32.const 16
      i32.sub
      set_local $2
      br $continue|5
     end
    end
   end
  end
  get_local $2
  i32.const 16
  i32.and
  if
   get_local $0
   tee_local $3
   i32.const 1
   i32.add
   set_local $0
   get_local $3
   block (result i32)
    get_local $1
    tee_local $3
    i32.const 1
    i32.add
    set_local $1
    get_local $3
    i32.load8_u
   end
   i32.store8
   get_local $0
   tee_local $3
   i32.const 1
   i32.add
   set_local $0
   get_local $3
   block (result i32)
    get_local $1
    tee_local $3
    i32.const 1
    i32.add
    set_local $1
    get_local $3
    i32.load8_u
   end
   i32.store8
   get_local $0
   tee_local $3
   i32.const 1
   i32.add
   set_local $0
   get_local $3
   block (result i32)
    get_local $1
    tee_local $3
    i32.const 1
    i32.add
    set_local $1
    get_local $3
    i32.load8_u
   end
   i32.store8
   get_local $0
   tee_local $3
   i32.const 1
   i32.add
   set_local $0
   get_local $3
   block (result i32)
    get_local $1
    tee_local $3
    i32.const 1
    i32.add
    set_local $1
    get_local $3
    i32.load8_u
   end
   i32.store8
   get_local $0
   tee_local $3
   i32.const 1
   i32.add
   set_local $0
   get_local $3
   block (result i32)
    get_local $1
    tee_local $3
    i32.const 1
    i32.add
    set_local $1
    get_local $3
    i32.load8_u
   end
   i32.store8
   get_local $0
   tee_local $3
   i32.const 1
   i32.add
   set_local $0
   get_local $3
   block (result i32)
    get_local $1
    tee_local $3
    i32.const 1
    i32.add
    set_local $1
    get_local $3
    i32.load8_u
   end
   i32.store8
   get_local $0
   tee_local $3
   i32.const 1
   i32.add
   set_local $0
   get_local $3
   block (result i32)
    get_local $1
    tee_local $3
    i32.const 1
    i32.add
    set_local $1
    get_local $3
    i32.load8_u
   end
   i32.store8
   get_local $0
   tee_local $3
   i32.const 1
   i32.add
   set_local $0
   get_local $3
   block (result i32)
    get_local $1
    tee_local $3
    i32.const 1
    i32.add
    set_local $1
    get_local $3
    i32.load8_u
   end
   i32.store8
   get_local $0
   tee_local $3
   i32.const 1
   i32.add
   set_local $0
   get_local $3
   block (result i32)
    get_local $1
    tee_local $3
    i32.const 1
    i32.add
    set_local $1
    get_local $3
    i32.load8_u
   end
   i32.store8
   get_local $0
   tee_local $3
   i32.const 1
   i32.add
   set_local $0
   get_local $3
   block (result i32)
    get_local $1
    tee_local $3
    i32.const 1
    i32.add
    set_local $1
    get_local $3
    i32.load8_u
   end
   i32.store8
   get_local $0
   tee_local $3
   i32.const 1
   i32.add
   set_local $0
   get_local $3
   block (result i32)
    get_local $1
    tee_local $3
    i32.const 1
    i32.add
    set_local $1
    get_local $3
    i32.load8_u
   end
   i32.store8
   get_local $0
   tee_local $3
   i32.const 1
   i32.add
   set_local $0
   get_local $3
   block (result i32)
    get_local $1
    tee_local $3
    i32.const 1
    i32.add
    set_local $1
    get_local $3
    i32.load8_u
   end
   i32.store8
   get_local $0
   tee_local $3
   i32.const 1
   i32.add
   set_local $0
   get_local $3
   block (result i32)
    get_local $1
    tee_local $3
    i32.const 1
    i32.add
    set_local $1
    get_local $3
    i32.load8_u
   end
   i32.store8
   get_local $0
   tee_local $3
   i32.const 1
   i32.add
   set_local $0
   get_local $3
   block (result i32)
    get_local $1
    tee_local $3
    i32.const 1
    i32.add
    set_local $1
    get_local $3
    i32.load8_u
   end
   i32.store8
   get_local $0
   tee_local $3
   i32.const 1
   i32.add
   set_local $0
   get_local $3
   block (result i32)
    get_local $1
    tee_local $3
    i32.const 1
    i32.add
    set_local $1
    get_local $3
    i32.load8_u
   end
   i32.store8
   get_local $0
   tee_local $3
   i32.const 1
   i32.add
   set_local $0
   get_local $3
   block (result i32)
    get_local $1
    tee_local $3
    i32.const 1
    i32.add
    set_local $1
    get_local $3
    i32.load8_u
   end
   i32.store8
  end
  get_local $2
  i32.const 8
  i32.and
  if
   get_local $0
   tee_local $3
   i32.const 1
   i32.add
   set_local $0
   get_local $3
   block (result i32)
    get_local $1
    tee_local $3
    i32.const 1
    i32.add
    set_local $1
    get_local $3
    i32.load8_u
   end
   i32.store8
   get_local $0
   tee_local $3
   i32.const 1
   i32.add
   set_local $0
   get_local $3
   block (result i32)
    get_local $1
    tee_local $3
    i32.const 1
    i32.add
    set_local $1
    get_local $3
    i32.load8_u
   end
   i32.store8
   get_local $0
   tee_local $3
   i32.const 1
   i32.add
   set_local $0
   get_local $3
   block (result i32)
    get_local $1
    tee_local $3
    i32.const 1
    i32.add
    set_local $1
    get_local $3
    i32.load8_u
   end
   i32.store8
   get_local $0
   tee_local $3
   i32.const 1
   i32.add
   set_local $0
   get_local $3
   block (result i32)
    get_local $1
    tee_local $3
    i32.const 1
    i32.add
    set_local $1
    get_local $3
    i32.load8_u
   end
   i32.store8
   get_local $0
   tee_local $3
   i32.const 1
   i32.add
   set_local $0
   get_local $3
   block (result i32)
    get_local $1
    tee_local $3
    i32.const 1
    i32.add
    set_local $1
    get_local $3
    i32.load8_u
   end
   i32.store8
   get_local $0
   tee_local $3
   i32.const 1
   i32.add
   set_local $0
   get_local $3
   block (result i32)
    get_local $1
    tee_local $3
    i32.const 1
    i32.add
    set_local $1
    get_local $3
    i32.load8_u
   end
   i32.store8
   get_local $0
   tee_local $3
   i32.const 1
   i32.add
   set_local $0
   get_local $3
   block (result i32)
    get_local $1
    tee_local $3
    i32.const 1
    i32.add
    set_local $1
    get_local $3
    i32.load8_u
   end
   i32.store8
   get_local $0
   tee_local $3
   i32.const 1
   i32.add
   set_local $0
   get_local $3
   block (result i32)
    get_local $1
    tee_local $3
    i32.const 1
    i32.add
    set_local $1
    get_local $3
    i32.load8_u
   end
   i32.store8
  end
  get_local $2
  i32.const 4
  i32.and
  if
   get_local $0
   tee_local $3
   i32.const 1
   i32.add
   set_local $0
   get_local $3
   block (result i32)
    get_local $1
    tee_local $3
    i32.const 1
    i32.add
    set_local $1
    get_local $3
    i32.load8_u
   end
   i32.store8
   get_local $0
   tee_local $3
   i32.const 1
   i32.add
   set_local $0
   get_local $3
   block (result i32)
    get_local $1
    tee_local $3
    i32.const 1
    i32.add
    set_local $1
    get_local $3
    i32.load8_u
   end
   i32.store8
   get_local $0
   tee_local $3
   i32.const 1
   i32.add
   set_local $0
   get_local $3
   block (result i32)
    get_local $1
    tee_local $3
    i32.const 1
    i32.add
    set_local $1
    get_local $3
    i32.load8_u
   end
   i32.store8
   get_local $0
   tee_local $3
   i32.const 1
   i32.add
   set_local $0
   get_local $3
   block (result i32)
    get_local $1
    tee_local $3
    i32.const 1
    i32.add
    set_local $1
    get_local $3
    i32.load8_u
   end
   i32.store8
  end
  get_local $2
  i32.const 2
  i32.and
  if
   get_local $0
   tee_local $3
   i32.const 1
   i32.add
   set_local $0
   get_local $3
   block (result i32)
    get_local $1
    tee_local $3
    i32.const 1
    i32.add
    set_local $1
    get_local $3
    i32.load8_u
   end
   i32.store8
   get_local $0
   tee_local $3
   i32.const 1
   i32.add
   set_local $0
   get_local $3
   block (result i32)
    get_local $1
    tee_local $3
    i32.const 1
    i32.add
    set_local $1
    get_local $3
    i32.load8_u
   end
   i32.store8
  end
  get_local $2
  i32.const 1
  i32.and
  if
   get_local $0
   set_local $3
   get_local $3
   block (result i32)
    get_local $1
    set_local $3
    get_local $3
    i32.load8_u
   end
   i32.store8
  end
 )
 (func $~lib/internal/memory/memmove (; 28 ;) (type $iiiv) (param $0 i32) (param $1 i32) (param $2 i32)
  (local $3 i32)
  get_local $0
  get_local $1
  i32.eq
  if
   return
  end
  get_local $1
  get_local $2
  i32.add
  get_local $0
  i32.le_u
  tee_local $3
  if (result i32)
   get_local $3
  else   
   get_local $0
   get_local $2
   i32.add
   get_local $1
   i32.le_u
  end
  if
   get_local $0
   get_local $1
   get_local $2
   call $~lib/internal/memory/memcpy
   return
  end
  get_local $0
  get_local $1
  i32.lt_u
  if
   get_local $1
   i32.const 7
   i32.and
   get_local $0
   i32.const 7
   i32.and
   i32.eq
   if
    loop $continue|0
     get_local $0
     i32.const 7
     i32.and
     if
      get_local $2
      i32.eqz
      if
       return
      end
      get_local $2
      i32.const 1
      i32.sub
      set_local $2
      get_local $0
      tee_local $3
      i32.const 1
      i32.add
      set_local $0
      get_local $3
      block (result i32)
       get_local $1
       tee_local $3
       i32.const 1
       i32.add
       set_local $1
       get_local $3
       i32.load8_u
      end
      i32.store8
      br $continue|0
     end
    end
    loop $continue|1
     get_local $2
     i32.const 8
     i32.ge_u
     if
      get_local $0
      get_local $1
      i64.load
      i64.store
      get_local $2
      i32.const 8
      i32.sub
      set_local $2
      get_local $0
      i32.const 8
      i32.add
      set_local $0
      get_local $1
      i32.const 8
      i32.add
      set_local $1
      br $continue|1
     end
    end
   end
   loop $continue|2
    get_local $2
    if
     get_local $0
     tee_local $3
     i32.const 1
     i32.add
     set_local $0
     get_local $3
     block (result i32)
      get_local $1
      tee_local $3
      i32.const 1
      i32.add
      set_local $1
      get_local $3
      i32.load8_u
     end
     i32.store8
     get_local $2
     i32.const 1
     i32.sub
     set_local $2
     br $continue|2
    end
   end
  else   
   get_local $1
   i32.const 7
   i32.and
   get_local $0
   i32.const 7
   i32.and
   i32.eq
   if
    loop $continue|3
     get_local $0
     get_local $2
     i32.add
     i32.const 7
     i32.and
     if
      get_local $2
      i32.eqz
      if
       return
      end
      get_local $0
      get_local $2
      i32.const 1
      i32.sub
      tee_local $2
      i32.add
      get_local $1
      get_local $2
      i32.add
      i32.load8_u
      i32.store8
      br $continue|3
     end
    end
    loop $continue|4
     get_local $2
     i32.const 8
     i32.ge_u
     if
      get_local $0
      get_local $2
      i32.const 8
      i32.sub
      tee_local $2
      i32.add
      get_local $1
      get_local $2
      i32.add
      i64.load
      i64.store
      br $continue|4
     end
    end
   end
   loop $continue|5
    get_local $2
    if
     get_local $0
     get_local $2
     i32.const 1
     i32.sub
     tee_local $2
     i32.add
     get_local $1
     get_local $2
     i32.add
     i32.load8_u
     i32.store8
     br $continue|5
    end
   end
  end
 )
 (func $~lib/internal/arraybuffer/reallocateUnsafe (; 29 ;) (type $iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  get_local $1
  get_local $0
  i32.load
  tee_local $2
  i32.gt_s
  if
   get_local $1
   i32.const 1073741816
   i32.gt_s
   if
    call $~lib/env/abort
    unreachable
   end
   get_local $1
   get_local $2
   call $~lib/internal/arraybuffer/computeSize
   i32.const 8
   i32.sub
   i32.le_s
   if
    get_local $0
    get_local $1
    i32.store
    get_local $0
    i32.const 8
    i32.add
    get_local $2
    i32.add
    get_local $1
    get_local $2
    i32.sub
    call $~lib/internal/memory/memset
   else    
    get_local $1
    call $~lib/internal/arraybuffer/allocateUnsafe
    tee_local $3
    i32.const 8
    i32.add
    get_local $0
    i32.const 8
    i32.add
    get_local $2
    call $~lib/internal/memory/memmove
    get_local $3
    i32.const 8
    i32.add
    get_local $2
    i32.add
    get_local $1
    get_local $2
    i32.sub
    call $~lib/internal/memory/memset
    get_local $3
    return
   end
  else   
   get_local $1
   get_local $2
   i32.lt_s
   if
    get_local $1
    i32.const 0
    i32.lt_s
    if
     call $~lib/env/abort
     unreachable
    end
    get_local $0
    get_local $1
    i32.store
   end
  end
  get_local $0
 )
 (func $~lib/array/Array<u8>#push (; 30 ;) (type $iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  get_local $0
  i32.load offset=4
  tee_local $2
  i32.const 1
  i32.add
  set_local $3
  get_local $2
  get_local $0
  i32.load
  tee_local $4
  i32.load
  i32.ge_u
  if
   get_local $2
   i32.const 1073741816
   i32.ge_u
   if
    call $~lib/env/abort
    unreachable
   end
   get_local $0
   get_local $4
   get_local $3
   call $~lib/internal/arraybuffer/reallocateUnsafe
   tee_local $4
   i32.store
  end
  get_local $0
  get_local $3
  i32.store offset=4
  get_local $4
  get_local $2
  i32.add
  get_local $1
  i32.store8 offset=8
  get_local $3
 )
 (func $~lib/utf8util/toUTF8Array (; 31 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  i32.const 0
  call $~lib/array/Array<u8>#constructor
  set_local $2
  block $break|0
   loop $repeat|0
    get_local $3
    get_local $0
    i32.load
    i32.ge_s
    br_if $break|0
    get_local $0
    get_local $3
    call $~lib/string/String#charCodeAt
    tee_local $1
    i32.const 128
    i32.lt_s
    if
     get_local $2
     get_local $1
     call $~lib/array/Array<u8>#push
     drop
    else     
     get_local $1
     i32.const 2048
     i32.lt_s
     if
      get_local $2
      get_local $1
      i32.const 6
      i32.shr_s
      i32.const 192
      i32.or
      call $~lib/array/Array<u8>#push
      drop
     else      
      get_local $1
      i32.const 55296
      i32.lt_s
      tee_local $4
      if (result i32)
       get_local $4
      else       
       get_local $1
       i32.const 57344
       i32.ge_s
      end
      if
       get_local $2
       get_local $1
       i32.const 12
       i32.shr_s
       i32.const 224
       i32.or
       call $~lib/array/Array<u8>#push
       drop
      else       
       get_local $2
       get_local $1
       i32.const 1023
       i32.and
       i32.const 10
       i32.shl
       get_local $0
       get_local $3
       i32.const 1
       i32.add
       tee_local $3
       call $~lib/string/String#charCodeAt
       i32.const 1023
       i32.and
       i32.or
       i32.const 65536
       i32.add
       tee_local $1
       i32.const 18
       i32.shr_s
       i32.const 240
       i32.or
       call $~lib/array/Array<u8>#push
       drop
       get_local $2
       get_local $1
       i32.const 12
       i32.shr_s
       i32.const 63
       i32.and
       i32.const 128
       i32.or
       call $~lib/array/Array<u8>#push
       drop
      end
      get_local $2
      get_local $1
      i32.const 6
      i32.shr_s
      i32.const 63
      i32.and
      i32.const 128
      i32.or
      call $~lib/array/Array<u8>#push
      drop
     end
     get_local $2
     get_local $1
     i32.const 63
     i32.and
     i32.const 128
     i32.or
     call $~lib/array/Array<u8>#push
     drop
    end
    get_local $3
    i32.const 1
    i32.add
    set_local $3
    br $repeat|0
    unreachable
   end
   unreachable
  end
  get_local $2
  i32.const 0
  call $~lib/array/Array<u8>#push
  drop
  get_local $2
 )
 (func $~lib/utf8util/string2cstr (; 32 ;) (type $ii) (param $0 i32) (result i32)
  get_local $0
  call $~lib/utf8util/toUTF8Array
  i32.load
  i32.const 8
  i32.add
 )
 (func $~lib/env/ultrain_assert (; 33 ;) (type $iiv) (param $0 i32) (param $1 i32)
  get_local $0
  i32.eqz
  if
   i32.const 0
   get_local $1
   call $~lib/utf8util/string2cstr
   call $~lib/env/ultrainio_assert
  end
 )
 (func $~lib/ultrain-ts-lib/src/log/Logger#s (; 34 ;) (type $iii) (param $0 i32) (param $1 i32) (result i32)
  get_local $1
  call $~lib/utf8util/string2cstr
  call $~lib/ultrain-ts-lib/src/log/env.ts_log_print_s
  get_local $0
 )
 (func $~lib/ultrain-ts-lib/src/asset/StringToSymbol (; 35 ;) (type $FUNCSIG$j) (result i64)
  (local $0 i32)
  (local $1 i32)
  (local $2 i32)
  (local $3 i64)
  (local $4 i32)
  i32.const 1280
  i32.load
  tee_local $4
  i32.const 255
  i32.and
  i32.const 7
  i32.le_u
  i32.const 1296
  call $~lib/env/ultrain_assert
  loop $repeat|0
   get_local $0
   get_local $4
   i32.const 255
   i32.and
   i32.lt_u
   if
    i32.const 1280
    get_local $0
    i32.const 255
    i32.and
    call $~lib/string/String#charCodeAt
    i32.const 255
    i32.and
    tee_local $1
    i32.const 65
    i32.lt_u
    tee_local $2
    if (result i32)
     get_local $2
    else     
     get_local $1
     i32.const 90
     i32.gt_u
    end
    if
     get_global $~lib/ultrain-ts-lib/src/log/Log
     i32.const 1520
     call $~lib/ultrain-ts-lib/src/log/Logger#s
     set_local $2
     get_local $1
     i64.extend_u/i32
     i32.const 16
     call $~lib/ultrain-ts-lib/src/log/env.ts_log_print_i
     call $~lib/ultrain-ts-lib/src/log/env.ts_log_done
    else     
     get_local $3
     get_local $1
     i64.extend_u/i32
     get_local $0
     i32.const 1
     i32.add
     i32.const 255
     i32.and
     i64.extend_u/i32
     i64.const 8
     i64.mul
     i64.shl
     i64.or
     set_local $3
    end
    get_local $0
    i32.const 1
    i32.add
    set_local $0
    br $repeat|0
   end
  end
  get_local $3
  i64.const 4
  i64.or
 )
 (func $~lib/ultrain-ts-lib/src/asset/Asset#constructor (; 36 ;) (type $FUNCSIG$ijj) (param $0 i64) (param $1 i64) (result i32)
  (local $2 i32)
  i32.const 16
  call $~lib/allocator/arena/__memory_allocate
  tee_local $2
  i64.const 0
  i64.store
  get_local $2
  i64.const 0
  i64.store offset=8
  get_local $2
  get_local $0
  i64.store
  get_local $2
  get_local $1
  i64.store offset=8
  get_local $2
 )
 (func $~lib/ultrain-ts-lib/lib/name/char_to_symbol (; 37 ;) (type $iI) (param $0 i32) (result i64)
  (local $1 i32)
  get_local $0
  i32.const 255
  i32.and
  i32.const 97
  i32.ge_u
  tee_local $1
  if (result i32)
   get_local $0
   i32.const 255
   i32.and
   i32.const 122
   i32.le_u
  else   
   get_local $1
  end
  if
   get_local $0
   i32.const -91
   i32.add
   i32.const 255
   i32.and
   i64.extend_u/i32
   return
  end
  get_local $0
  i32.const 255
  i32.and
  i32.const 49
  i32.ge_u
  tee_local $1
  if (result i32)
   get_local $0
   i32.const 255
   i32.and
   i32.const 53
   i32.le_u
  else   
   get_local $1
  end
  if
   get_local $0
   i32.const -48
   i32.add
   i32.const 255
   i32.and
   i64.extend_u/i32
   return
  end
  i64.const 0
 )
 (func $~lib/ultrain-ts-lib/lib/name/N (; 38 ;) (type $iI) (param $0 i32) (result i64)
  (local $1 i32)
  (local $2 i64)
  (local $3 i64)
  (local $4 i32)
  (local $5 i32)
  get_local $0
  i32.load
  set_local $4
  block $break|0
   loop $repeat|0
    get_local $1
    i32.const 12
    i32.gt_u
    br_if $break|0
    i64.const 0
    set_local $2
    get_local $1
    get_local $4
    i32.lt_u
    tee_local $5
    if (result i32)
     get_local $1
     i32.const 12
     i32.le_u
    else     
     get_local $5
    end
    if
     get_local $0
     get_local $1
     call $~lib/string/String#charCodeAt
     i32.const 255
     i32.and
     call $~lib/ultrain-ts-lib/lib/name/char_to_symbol
     set_local $2
    end
    get_local $3
    get_local $2
    i64.const 31
    i64.and
    i64.const 64
    get_local $1
    i32.const 1
    i32.add
    i64.extend_u/i32
    i64.const 5
    i64.mul
    i64.sub
    i64.shl
    get_local $2
    i64.const 15
    i64.and
    get_local $1
    i32.const 12
    i32.lt_u
    select
    tee_local $2
    i64.or
    set_local $3
    get_local $1
    i32.const 1
    i32.add
    set_local $1
    br $repeat|0
    unreachable
   end
   unreachable
  end
  get_local $3
 )
 (func $~lib/dbmanager/DBManager<Candidate>#constructor (; 39 ;) (type $FUNCSIG$ijjj) (param $0 i64) (param $1 i64) (param $2 i64) (result i32)
  (local $3 i32)
  i32.const 24
  call $~lib/allocator/arena/__memory_allocate
  tee_local $3
  i64.const 0
  i64.store
  get_local $3
  i64.const 0
  i64.store offset=8
  get_local $3
  i64.const 0
  i64.store offset=16
  get_local $3
  get_local $0
  i64.store
  get_local $3
  get_local $1
  i64.store offset=8
  get_local $3
  get_local $2
  i64.store offset=16
  get_local $3
 )
 (func $~lib/dbmanager/DBManager<Vote>#find (; 40 ;) (type $iIi) (param $0 i32) (param $1 i64) (result i32)
  get_local $0
  i64.load offset=8
  get_local $0
  i64.load offset=16
  get_local $0
  i64.load
  get_local $1
  call $~lib/env/db_find_i64
 )
 (func $~lib/dbmanager/DBManager<Vote>#exists (; 41 ;) (type $iIi) (param $0 i32) (param $1 i64) (result i32)
  i32.const 0
  i32.const 1
  get_local $0
  get_local $1
  call $~lib/dbmanager/DBManager<Vote>#find
  i32.const 0
  i32.lt_s
  select
 )
 (func $~lib/datastream/DataStream#constructor (; 42 ;) (type $FUNCSIG$iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  i32.const 12
  call $~lib/allocator/arena/__memory_allocate
  tee_local $2
  i32.const 0
  i32.store
  get_local $2
  i32.const 0
  i32.store offset=4
  get_local $2
  i32.const 0
  i32.store offset=8
  get_local $2
  get_local $0
  i32.store
  get_local $2
  get_local $1
  i32.store offset=4
  get_local $2
  i32.const 0
  i32.store offset=8
  get_local $2
 )
 (func $~lib/datastream/DataStream#isMeasureMode (; 43 ;) (type $ii) (param $0 i32) (result i32)
  get_local $0
  i32.load
  i32.eqz
 )
 (func $~lib/datastream/DataStream#write<u64> (; 44 ;) (type $iIv) (param $0 i32) (param $1 i64)
  get_local $0
  call $~lib/datastream/DataStream#isMeasureMode
  i32.eqz
  if
   get_local $0
   i32.load
   get_local $0
   i32.load offset=8
   i32.add
   get_local $1
   i64.store
  end
  get_local $0
  get_local $0
  i32.load offset=8
  i32.const 8
  i32.add
  i32.store offset=8
 )
 (func $contract/MyContract/Vote#serialize (; 45 ;) (type $iiv) (param $0 i32) (param $1 i32)
  get_local $1
  get_local $0
  i64.load
  call $~lib/datastream/DataStream#write<u64>
  get_local $1
  get_local $0
  i64.load offset=8
  call $~lib/datastream/DataStream#write<u64>
 )
 (func $~lib/datastream/DataStream.measure<Vote> (; 46 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  get_local $0
  i32.const 0
  i32.const 0
  call $~lib/datastream/DataStream#constructor
  tee_local $1
  call $contract/MyContract/Vote#serialize
  get_local $1
  i32.load offset=8
 )
 (func $~lib/internal/typedarray/TypedArray<u8_u32>#constructor (; 47 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  get_local $0
  i32.const 1073741816
  i32.gt_u
  if
   call $~lib/env/abort
   unreachable
  end
  get_local $0
  tee_local $1
  call $~lib/internal/arraybuffer/allocateUnsafe
  tee_local $2
  i32.const 8
  i32.add
  get_local $0
  call $~lib/internal/memory/memset
  i32.const 12
  call $~lib/allocator/arena/__memory_allocate
  tee_local $0
  i32.const 0
  i32.store
  get_local $0
  i32.const 0
  i32.store offset=4
  get_local $0
  i32.const 0
  i32.store offset=8
  get_local $0
  get_local $2
  i32.store
  get_local $0
  i32.const 0
  i32.store offset=4
  get_local $0
  get_local $1
  i32.store offset=8
  get_local $0
 )
 (func $~lib/dbmanager/DBManager<Vote>#emplace (; 48 ;) (type $iIiv) (param $0 i32) (param $1 i64) (param $2 i32)
  (local $3 i32)
  get_local $0
  i64.load offset=8
  call $~lib/env/current_receiver
  i64.eq
  i32.const 1776
  call $~lib/env/ultrain_assert
  get_local $2
  get_local $2
  call $~lib/datastream/DataStream.measure<Vote>
  tee_local $3
  call $~lib/internal/typedarray/TypedArray<u8_u32>#constructor
  i32.load
  get_local $3
  call $~lib/datastream/DataStream#constructor
  tee_local $3
  call $contract/MyContract/Vote#serialize
  get_local $0
  i64.load offset=16
  get_local $0
  i64.load
  get_local $1
  get_local $2
  i64.load
  get_local $3
  i32.load
  get_local $3
  i32.load offset=8
  call $~lib/env/db_store_i64
  drop
 )
 (func $contract/MyContract/rand#constructor (; 49 ;) (type $FUNCSIG$ij) (param $0 i64) (result i32)
  (local $1 i32)
  (local $2 i32)
  i32.const 20
  call $~lib/allocator/arena/__memory_allocate
  tee_local $1
  i64.const 0
  i64.store
  get_local $1
  i32.const 0
  i32.store offset=8
  get_local $1
  i32.const 0
  i32.store offset=12
  get_local $1
  i32.const 0
  i32.store offset=16
  get_local $1
  tee_local $2
  get_local $0
  i64.store
  get_local $1
  i32.const 1664
  call $~lib/ultrain-ts-lib/lib/name/N
  get_local $1
  i64.load
  i32.const 1688
  call $~lib/ultrain-ts-lib/lib/name/N
  call $~lib/dbmanager/DBManager<Candidate>#constructor
  i32.store offset=12
  get_local $1
  i32.const 1720
  call $~lib/ultrain-ts-lib/lib/name/N
  get_local $1
  i64.load
  i32.const 1736
  call $~lib/ultrain-ts-lib/lib/name/N
  call $~lib/dbmanager/DBManager<Candidate>#constructor
  i32.store offset=16
  get_local $1
  i32.load offset=16
  i32.const 1752
  call $~lib/ultrain-ts-lib/lib/name/N
  call $~lib/dbmanager/DBManager<Vote>#exists
  i32.eqz
  if
   i32.const 16
   call $~lib/allocator/arena/__memory_allocate
   tee_local $1
   i64.const 0
   i64.store
   get_local $1
   i64.const 0
   i64.store offset=8
   get_local $1
   i32.const 1752
   call $~lib/ultrain-ts-lib/lib/name/N
   i64.store
   get_local $1
   call $~lib/ultrain-ts-lib/lib/headblock/env.head_block_number
   i64.extend_u/i32
   i64.store offset=8
   get_local $2
   i32.load offset=16
   call $~lib/ultrain-ts-lib/internal/action.d/env.current_sender
   get_local $1
   call $~lib/dbmanager/DBManager<Vote>#emplace
   get_local $1
   i32.const 1952
   call $~lib/ultrain-ts-lib/lib/name/N
   i64.store
   get_local $1
   i64.const 2346
   i64.store offset=8
   get_local $2
   i32.load offset=16
   call $~lib/ultrain-ts-lib/internal/action.d/env.current_sender
   get_local $1
   call $~lib/dbmanager/DBManager<Vote>#emplace
   get_local $1
   i32.const 1968
   call $~lib/ultrain-ts-lib/lib/name/N
   i64.store
   get_local $1
   i64.const 0
   i64.store offset=8
   get_local $2
   i32.load offset=16
   call $~lib/ultrain-ts-lib/internal/action.d/env.current_sender
   get_local $1
   call $~lib/dbmanager/DBManager<Vote>#emplace
  end
  get_local $2
 )
 (func $~lib/ultrain-ts-lib/lib/name_ex/char_to_symbol_ex (; 50 ;) (type $iI) (param $0 i32) (result i64)
  (local $1 i32)
  get_local $0
  i32.const 255
  i32.and
  i32.const 46
  i32.eq
  if
   i64.const 0
   return
  end
  get_local $0
  i32.const 255
  i32.and
  i32.const 95
  i32.eq
  if
   i64.const 1
   return
  end
  get_local $0
  i32.const 255
  i32.and
  i32.const 48
  i32.ge_u
  tee_local $1
  if (result i32)
   get_local $0
   i32.const 255
   i32.and
   i32.const 57
   i32.le_u
  else   
   get_local $1
  end
  if
   get_local $0
   i32.const -46
   i32.add
   i32.const 255
   i32.and
   i64.extend_u/i32
   return
  end
  get_local $0
  i32.const 255
  i32.and
  i32.const 97
  i32.ge_u
  tee_local $1
  if (result i32)
   get_local $0
   i32.const 255
   i32.and
   i32.const 122
   i32.le_u
  else   
   get_local $1
  end
  if
   get_local $0
   i32.const -85
   i32.add
   i32.const 255
   i32.and
   i64.extend_u/i32
   return
  end
  get_local $0
  i32.const 255
  i32.and
  i32.const 65
  i32.ge_u
  tee_local $1
  if (result i32)
   get_local $0
   i32.const 255
   i32.and
   i32.const 90
   i32.le_u
  else   
   get_local $1
  end
  if
   get_local $0
   i32.const -27
   i32.add
   i32.const 255
   i32.and
   i64.extend_u/i32
   return
  end
  i64.const 255
 )
 (func $~lib/ultrain-ts-lib/lib/name_ex/NEX (; 51 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i64)
  (local $3 i32)
  (local $4 i64)
  (local $5 i32)
  i64.const 0
  i64.const 0
  call $~lib/ultrain-ts-lib/src/asset/Asset#constructor
  set_local $3
  get_local $0
  i32.load
  set_local $5
  block $break|0
   loop $repeat|0
    get_local $1
    get_local $5
    i32.ge_s
    br_if $break|0
    get_local $0
    get_local $1
    call $~lib/string/String#charCodeAt
    call $~lib/ultrain-ts-lib/lib/name_ex/char_to_symbol_ex
    set_local $4
    get_local $1
    i32.const 9
    i32.le_s
    if (result i64)
     get_local $2
     get_local $4
     get_local $1
     i64.extend_s/i32
     i64.const 6
     i64.mul
     i64.shl
     i64.or
    else     
     get_local $1
     i32.const 10
     i32.eq
     if (result i64)
      get_local $3
      get_local $2
      get_local $4
      i64.const 15
      i64.and
      get_local $1
      i64.extend_s/i32
      i64.const 6
      i64.mul
      i64.shl
      i64.or
      i64.store offset=8
      get_local $4
      i64.const 48
      i64.and
      i64.const 4
      i64.shr_u
     else      
      get_local $2
      get_local $4
      get_local $1
      i32.const 11
      i32.sub
      i64.extend_s/i32
      i64.const 6
      i64.mul
      i64.const 2
      i64.add
      i64.shl
      i64.or
     end
    end
    set_local $2
    get_local $5
    i32.const 10
    i32.le_s
    if
     get_local $3
     get_local $2
     i64.store offset=8
    else     
     get_local $3
     get_local $2
     i64.store
    end
    get_local $1
    i32.const 1
    i32.add
    set_local $1
    br $repeat|0
    unreachable
   end
   unreachable
  end
  get_local $3
 )
 (func $~lib/ultrain-ts-lib/lib/name_ex/NameEx._eq (; 52 ;) (type $iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  get_local $0
  i64.load
  get_local $1
  i64.load
  i64.eq
  tee_local $2
  if (result i32)
   get_local $0
   i64.load offset=8
   get_local $1
   i64.load offset=8
   i64.eq
  else   
   get_local $2
  end
 )
 (func $~lib/ultrain-ts-lib/src/contract/Contract.filterAcceptTransferTokenAction (; 53 ;) (type $IIii) (param $0 i64) (param $1 i64) (param $2 i32) (result i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  get_local $1
  get_local $0
  i64.eq
  tee_local $3
  if (result i32)
   i32.const 1984
   call $~lib/ultrain-ts-lib/lib/name_ex/NEX
   set_local $4
   get_local $2
   tee_local $3
   i64.load
   get_local $4
   i64.load
   i64.ne
   tee_local $5
   if (result i32)
    get_local $5
   else    
    get_local $3
    i64.load offset=8
    get_local $4
    i64.load offset=8
    i64.ne
   end
   tee_local $3
  else   
   get_local $3
  end
  if (result i32)
   get_local $3
  else   
   get_local $1
   i32.const 2008
   call $~lib/ultrain-ts-lib/lib/name/N
   i64.eq
   tee_local $3
   if (result i32)
    get_local $2
    i32.const 1984
    call $~lib/ultrain-ts-lib/lib/name_ex/NEX
    call $~lib/ultrain-ts-lib/lib/name_ex/NameEx._eq
   else    
    get_local $3
   end
  end
 )
 (func $~lib/ultrain-ts-lib/src/contract/Contract#isAction (; 54 ;) (type $iii) (param $0 i32) (param $1 i32) (result i32)
  get_local $0
  i32.load offset=8
  get_local $1
  call $~lib/ultrain-ts-lib/lib/name_ex/NEX
  call $~lib/ultrain-ts-lib/lib/name_ex/NameEx._eq
 )
 (func $~lib/dbmanager/DBManager<Candidate>#dropAll (; 55 ;) (type $ii) (param $0 i32) (result i32)
  get_local $0
  i64.load offset=8
  get_local $0
  i64.load offset=16
  get_local $0
  i64.load
  call $~lib/env/db_drop_i64
 )
 (func $~lib/datastream/DataStream#read<u64> (; 56 ;) (type $iI) (param $0 i32) (result i64)
  (local $1 i64)
  get_local $0
  i32.load
  get_local $0
  i32.load offset=8
  i32.add
  i64.load
  set_local $1
  get_local $0
  get_local $0
  i32.load offset=8
  i32.const 8
  i32.add
  i32.store offset=8
  get_local $1
 )
 (func $~lib/ultrain-ts-lib/src/asset/Asset#deserialize (; 57 ;) (type $iiv) (param $0 i32) (param $1 i32)
  get_local $0
  get_local $1
  call $~lib/datastream/DataStream#read<u64>
  i64.store
  get_local $0
  get_local $1
  call $~lib/datastream/DataStream#read<u64>
  i64.store offset=8
 )
 (func $~lib/datastream/DataStream#readVarint32 (; 58 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  loop $continue|0
   get_local $2
   block (result i32)
    get_local $3
    tee_local $2
    i32.const 1
    i32.add
    set_local $3
    get_local $0
    tee_local $1
    i32.load
    get_local $1
    i32.load offset=8
    i32.add
    i32.load8_u
    set_local $4
    get_local $1
    get_local $1
    i32.load offset=8
    i32.const 1
    i32.add
    i32.store offset=8
    get_local $4
    tee_local $1
    i32.const 127
    i32.and
    get_local $2
    i32.const 7
    i32.mul
    i32.shl
   end
   i32.or
   set_local $2
   get_local $1
   i32.const 128
   i32.and
   br_if $continue|0
  end
  get_local $2
 )
 (func $~lib/internal/string/allocateUnsafe (; 59 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  get_local $0
  i32.const 0
  i32.gt_s
  tee_local $1
  if (result i32)
   get_local $0
   i32.const 536870910
   i32.le_s
  else   
   get_local $1
  end
  i32.eqz
  if
   call $~lib/env/abort
   unreachable
  end
  get_local $0
  i32.const 1
  i32.shl
  i32.const 4
  i32.add
  call $~lib/allocator/arena/__memory_allocate
  tee_local $1
  get_local $0
  i32.store
  get_local $1
 )
 (func $~lib/string/String.fromUTF8 (; 60 ;) (type $iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  get_local $1
  i32.const 1
  i32.lt_u
  if
   i32.const 2136
   return
  end
  get_local $1
  i32.const 1
  i32.shl
  call $~lib/allocator/arena/__memory_allocate
  set_local $6
  loop $continue|0
   get_local $2
   get_local $1
   i32.lt_u
   if
    block (result i32)
     get_local $2
     tee_local $4
     i32.const 1
     i32.add
     set_local $2
     get_local $0
     get_local $4
     i32.add
     i32.load8_u
     tee_local $4
     i32.const 128
     i32.lt_u
    end
    if
     get_local $6
     get_local $5
     i32.add
     get_local $4
     i32.store16
    else     
     get_local $4
     i32.const 191
     i32.gt_u
     tee_local $3
     if (result i32)
      get_local $4
      i32.const 224
      i32.lt_u
     else      
      get_local $3
     end
     if
      get_local $2
      i32.const 1
      i32.add
      get_local $1
      i32.gt_u
      if
       call $~lib/env/abort
       unreachable
      end
      get_local $2
      tee_local $3
      i32.const 1
      i32.add
      set_local $2
      get_local $6
      get_local $5
      i32.add
      get_local $4
      i32.const 31
      i32.and
      i32.const 6
      i32.shl
      get_local $0
      get_local $3
      i32.add
      i32.load8_u
      i32.const 63
      i32.and
      i32.or
      i32.store16
     else      
      get_local $4
      i32.const 239
      i32.gt_u
      tee_local $3
      if (result i32)
       get_local $4
       i32.const 365
       i32.lt_u
      else       
       get_local $3
      end
      if
       get_local $2
       i32.const 3
       i32.add
       get_local $1
       i32.gt_u
       if
        call $~lib/env/abort
        unreachable
       end
       get_local $2
       tee_local $3
       i32.const 1
       i32.add
       set_local $2
       get_local $6
       get_local $5
       i32.add
       get_local $4
       i32.const 7
       i32.and
       i32.const 18
       i32.shl
       get_local $0
       get_local $3
       i32.add
       i32.load8_u
       i32.const 63
       i32.and
       i32.const 12
       i32.shl
       i32.or
       block (result i32)
        get_local $2
        tee_local $3
        i32.const 1
        i32.add
        set_local $2
        get_local $0
        get_local $3
        i32.add
        i32.load8_u
        i32.const 63
        i32.and
        i32.const 6
        i32.shl
       end
       i32.or
       block (result i32)
        get_local $2
        tee_local $3
        i32.const 1
        i32.add
        set_local $2
        get_local $0
        get_local $3
        i32.add
        i32.load8_u
        i32.const 63
        i32.and
       end
       i32.or
       i32.const 65536
       i32.sub
       tee_local $4
       i32.const 10
       i32.shr_u
       i32.const 55296
       i32.add
       i32.store16
       get_local $6
       get_local $5
       i32.const 2
       i32.add
       tee_local $5
       i32.add
       get_local $4
       i32.const 1023
       i32.and
       i32.const 56320
       i32.add
       i32.store16
      else       
       get_local $2
       i32.const 2
       i32.add
       get_local $1
       i32.gt_u
       if
        call $~lib/env/abort
        unreachable
       end
       get_local $2
       tee_local $3
       i32.const 1
       i32.add
       set_local $2
       get_local $6
       get_local $5
       i32.add
       get_local $4
       i32.const 15
       i32.and
       i32.const 12
       i32.shl
       get_local $0
       get_local $3
       i32.add
       i32.load8_u
       i32.const 63
       i32.and
       i32.const 6
       i32.shl
       i32.or
       block (result i32)
        get_local $2
        tee_local $3
        i32.const 1
        i32.add
        set_local $2
        get_local $0
        get_local $3
        i32.add
        i32.load8_u
        i32.const 63
        i32.and
       end
       i32.or
       i32.store16
      end
     end
    end
    get_local $5
    i32.const 2
    i32.add
    set_local $5
    br $continue|0
   end
  end
  get_local $2
  get_local $1
  i32.ne
  if
   call $~lib/env/abort
   unreachable
  end
  get_local $5
  i32.const 1
  i32.shr_u
  call $~lib/internal/string/allocateUnsafe
  tee_local $0
  i32.const 4
  i32.add
  get_local $6
  get_local $5
  call $~lib/internal/memory/memmove
  get_local $0
 )
 (func $~lib/datastream/DataStream#readString (; 61 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  get_local $0
  call $~lib/datastream/DataStream#readVarint32
  tee_local $1
  i32.eqz
  if
   i32.const 2136
   return
  end
  get_local $1
  call $~lib/internal/typedarray/TypedArray<u8_u32>#constructor
  tee_local $2
  i32.load
  get_local $0
  i32.load
  get_local $0
  i32.load offset=8
  i32.add
  get_local $1
  call $~lib/internal/memory/memmove
  get_local $0
  get_local $0
  i32.load offset=8
  get_local $1
  i32.add
  i32.store offset=8
  get_local $2
  i32.load
  get_local $1
  call $~lib/string/String.fromUTF8
 )
 (func $~lib/internal/string/compareUnsafe (; 62 ;) (type $FUNCSIG$iiii) (param $0 i32) (param $1 i32) (param $2 i32) (result i32)
  (local $3 i32)
  loop $continue|0
   get_local $2
   if (result i32)
    get_local $0
    i32.load16_u offset=4
    get_local $1
    i32.load16_u offset=4
    i32.sub
    tee_local $3
    i32.eqz
   else    
    get_local $2
   end
   if
    get_local $2
    i32.const 1
    i32.sub
    set_local $2
    get_local $0
    i32.const 2
    i32.add
    set_local $0
    get_local $1
    i32.const 2
    i32.add
    set_local $1
    br $continue|0
   end
  end
  get_local $3
 )
 (func $~lib/string/String.__eq (; 63 ;) (type $iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  get_local $0
  get_local $1
  i32.eq
  if
   i32.const 1
   return
  end
  get_local $0
  i32.eqz
  tee_local $2
  if (result i32)
   get_local $2
  else   
   get_local $1
   i32.eqz
  end
  if
   i32.const 0
   return
  end
  get_local $0
  i32.load
  tee_local $2
  get_local $1
  i32.load
  i32.ne
  if
   i32.const 0
   return
  end
  get_local $0
  get_local $1
  get_local $2
  call $~lib/internal/string/compareUnsafe
  i32.eqz
 )
 (func $~lib/internal/string/copyUnsafe (; 64 ;) (type $iiiiiv) (param $0 i32) (param $1 i32) (param $2 i32) (param $3 i32) (param $4 i32)
  get_local $0
  get_local $1
  i32.const 1
  i32.shl
  i32.add
  i32.const 4
  i32.add
  get_local $2
  get_local $3
  i32.const 1
  i32.shl
  i32.add
  i32.const 4
  i32.add
  get_local $4
  i32.const 1
  i32.shl
  call $~lib/internal/memory/memmove
 )
 (func $~lib/string/String#concat (; 65 ;) (type $iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  get_local $0
  i32.eqz
  if
   call $~lib/env/abort
   unreachable
  end
  get_local $1
  i32.eqz
  if
   i32.const 2400
   set_local $1
  end
  get_local $0
  i32.load
  tee_local $3
  get_local $1
  i32.load
  tee_local $4
  i32.add
  tee_local $2
  i32.eqz
  if
   i32.const 2136
   return
  end
  get_local $2
  call $~lib/internal/string/allocateUnsafe
  tee_local $2
  i32.const 0
  get_local $0
  i32.const 0
  get_local $3
  call $~lib/internal/string/copyUnsafe
  get_local $2
  get_local $3
  get_local $1
  i32.const 0
  get_local $4
  call $~lib/internal/string/copyUnsafe
  get_local $2
 )
 (func $~lib/math/NativeMath.scalbn (; 66 ;) (type $FiF) (param $0 f64) (param $1 i32) (result f64)
  (local $2 i32)
  get_local $1
  i32.const 1023
  i32.gt_s
  if
   get_local $0
   f64.const 8988465674311579538646525e283
   f64.mul
   set_local $0
   get_local $1
   i32.const 1023
   i32.sub
   tee_local $1
   i32.const 1023
   i32.gt_s
   if
    get_local $0
    f64.const 8988465674311579538646525e283
    f64.mul
    set_local $0
    get_local $1
    i32.const 1023
    i32.sub
    tee_local $1
    i32.const 1023
    tee_local $2
    get_local $1
    get_local $2
    i32.lt_s
    select
    set_local $1
   end
  else   
   get_local $1
   i32.const -1022
   i32.lt_s
   if
    get_local $0
    f64.const 2.004168360008973e-292
    f64.mul
    set_local $0
    get_local $1
    i32.const 969
    i32.add
    tee_local $1
    i32.const -1022
    i32.lt_s
    if
     get_local $0
     f64.const 2.004168360008973e-292
     f64.mul
     set_local $0
     get_local $1
     i32.const 969
     i32.add
     tee_local $1
     i32.const -1022
     tee_local $2
     get_local $1
     get_local $2
     i32.gt_s
     select
     set_local $1
    end
   end
  end
  get_local $0
  get_local $1
  i64.extend_s/i32
  i64.const 1023
  i64.add
  i64.const 52
  i64.shl
  f64.reinterpret/i64
  f64.mul
 )
 (func $~lib/math/NativeMath.pow (; 67 ;) (type $FUNCSIG$dd) (param $0 f64) (result f64)
  (local $1 i32)
  (local $2 f64)
  (local $3 f64)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  (local $7 f64)
  (local $8 i64)
  (local $9 f64)
  block $folding-inner2
   block $folding-inner1
    block $folding-inner0
     get_local $0
     i64.reinterpret/f64
     tee_local $8
     i64.const 32
     i64.shr_u
     i32.wrap/i64
     tee_local $1
     i32.const 2147483647
     i32.and
     tee_local $4
     get_local $8
     i32.wrap/i64
     tee_local $5
     i32.or
     i32.eqz
     if
      f64.const 1
      return
     end
     get_local $4
     i32.const 2146435072
     i32.gt_s
     tee_local $6
     if (result i32)
      get_local $6
     else      
      get_local $4
      i32.const 2146435072
      i32.eq
      tee_local $6
      if (result i32)
       get_local $5
      else       
       get_local $6
      end
     end
     if
      f64.const 10
      get_local $0
      f64.add
      return
     end
     get_local $5
     i32.eqz
     if
      get_local $4
      i32.const 2146435072
      i32.eq
      if
       get_local $0
       f64.const 0
       get_local $1
       i32.const 0
       i32.ge_s
       select
       return
      end
      get_local $4
      i32.const 1072693248
      i32.eq
      if
       get_local $1
       i32.const 0
       i32.ge_s
       if
        f64.const 10
        return
       end
       f64.const 0.1
       return
      end
      get_local $1
      i32.const 1073741824
      i32.eq
      if
       f64.const 100
       return
      end
      get_local $1
      i32.const 1071644672
      i32.eq
      if
       f64.const 3.1622776601683795
       return
      end
     end
     f64.const 10
     set_local $2
     get_local $4
     i32.const 1105199104
     i32.gt_s
     if (result f64)
      br $folding-inner2
     else      
      f64.const 3.321928024291992
      set_local $3
      f64.const 7.05953701603694e-08
     end
     set_local $2
     get_local $0
     get_local $0
     i64.reinterpret/f64
     i64.const -4294967296
     i64.and
     f64.reinterpret/i64
     tee_local $7
     f64.sub
     get_local $3
     f64.mul
     get_local $0
     get_local $2
     f64.mul
     f64.add
     tee_local $0
     get_local $7
     get_local $3
     f64.mul
     tee_local $7
     f64.add
     tee_local $2
     i64.reinterpret/f64
     tee_local $8
     i64.const 32
     i64.shr_u
     i32.wrap/i64
     set_local $1
     get_local $8
     i32.wrap/i64
     set_local $6
     get_local $1
     i32.const 1083179008
     i32.ge_s
     if
      get_local $1
      i32.const 1083179008
      i32.sub
      get_local $6
      i32.or
      get_local $0
      f64.const 8.008566259537294e-17
      f64.add
      get_local $2
      get_local $7
      f64.sub
      f64.gt
      i32.or
      br_if $folding-inner1
     else      
      get_local $1
      i32.const 2147483647
      i32.and
      i32.const 1083231232
      i32.ge_s
      if
       get_local $1
       i32.const -1064252416
       i32.sub
       get_local $6
       i32.or
       get_local $0
       get_local $2
       get_local $7
       f64.sub
       f64.le
       i32.or
       br_if $folding-inner0
      end
     end
     get_local $1
     i32.const 2147483647
     i32.and
     tee_local $6
     i32.const 20
     i32.shr_s
     i32.const 1023
     i32.sub
     set_local $4
     i32.const 0
     set_local $5
     get_local $6
     i32.const 1071644672
     i32.gt_s
     if
      get_local $1
      i32.const 1048576
      get_local $4
      i32.const 1
      i32.add
      i32.shr_s
      i32.add
      tee_local $5
      i32.const 2147483647
      i32.and
      i32.const 20
      i32.shr_s
      i32.const 1023
      i32.sub
      set_local $4
      get_local $5
      i32.const 1048575
      get_local $4
      i32.shr_s
      i32.const -1
      i32.xor
      i32.and
      i64.extend_s/i32
      i64.const 32
      i64.shl
      f64.reinterpret/i64
      set_local $3
      get_local $5
      i32.const 1048575
      i32.and
      i32.const 1048576
      i32.or
      i32.const 20
      get_local $4
      i32.sub
      i32.shr_s
      set_local $5
      get_local $1
      i32.const 0
      i32.lt_s
      if
       i32.const 0
       get_local $5
       i32.sub
       set_local $5
      end
      get_local $7
      get_local $3
      f64.sub
      set_local $7
     end
     f64.const 1
     f64.const 1
     get_local $0
     get_local $7
     f64.add
     i64.reinterpret/f64
     i64.const -4294967296
     i64.and
     f64.reinterpret/i64
     tee_local $3
     f64.const 0.6931471824645996
     f64.mul
     tee_local $9
     get_local $0
     get_local $3
     get_local $7
     f64.sub
     f64.sub
     f64.const 0.6931471805599453
     f64.mul
     get_local $3
     f64.const -1.904654299957768e-09
     f64.mul
     f64.add
     tee_local $0
     f64.add
     tee_local $2
     get_local $2
     get_local $2
     get_local $2
     f64.mul
     tee_local $3
     f64.const 0.16666666666666602
     get_local $3
     f64.const -2.7777777777015593e-03
     get_local $3
     f64.const 6.613756321437934e-05
     get_local $3
     f64.const -1.6533902205465252e-06
     get_local $3
     f64.const 4.1381367970572385e-08
     f64.mul
     f64.add
     f64.mul
     f64.add
     f64.mul
     f64.add
     f64.mul
     f64.add
     f64.mul
     f64.sub
     tee_local $3
     f64.mul
     get_local $3
     f64.const 2
     f64.sub
     f64.div
     get_local $0
     get_local $2
     get_local $9
     f64.sub
     f64.sub
     tee_local $7
     get_local $2
     get_local $7
     f64.mul
     f64.add
     f64.sub
     get_local $2
     f64.sub
     f64.sub
     tee_local $2
     i64.reinterpret/f64
     i64.const 32
     i64.shr_u
     i32.wrap/i64
     get_local $5
     i32.const 20
     i32.shl
     i32.add
     tee_local $1
     i32.const 20
     i32.shr_s
     i32.const 0
     i32.le_s
     if (result f64)
      get_local $2
      get_local $5
      call $~lib/math/NativeMath.scalbn
     else      
      get_local $2
      i64.reinterpret/f64
      i64.const 4294967295
      i64.and
      get_local $1
      i64.extend_s/i32
      i64.const 32
      i64.shl
      i64.or
      f64.reinterpret/i64
     end
     f64.mul
     return
    end
    f64.const 0
    return
   end
   f64.const inf
   return
  end
  f64.const inf
  f64.const 0
  get_local $1
  i32.const 0
  i32.gt_s
  select
 )
 (func $~lib/internal/number/decimalCount32 (; 68 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  i32.const 32
  get_local $0
  i32.clz
  i32.sub
  i32.const 1233
  i32.mul
  i32.const 12
  i32.shr_u
  tee_local $1
  get_local $0
  i32.const 2480
  i32.load
  get_local $1
  i32.const 2
  i32.shl
  i32.add
  i32.load offset=8
  i32.lt_u
  i32.sub
  i32.const 1
  i32.add
 )
 (func $~lib/internal/number/utoa_simple<u32> (; 69 ;) (type $iiiv) (param $0 i32) (param $1 i32) (param $2 i32)
  (local $3 i32)
  loop $continue|0
   get_local $1
   i32.const 10
   i32.rem_u
   set_local $3
   get_local $1
   i32.const 10
   i32.div_u
   set_local $1
   get_local $0
   get_local $2
   i32.const 1
   i32.sub
   tee_local $2
   i32.const 1
   i32.shl
   i32.add
   get_local $3
   i32.const 48
   i32.add
   i32.store16 offset=4
   get_local $1
   br_if $continue|0
  end
 )
 (func $~lib/internal/number/decimalCount64 (; 70 ;) (type $Ii) (param $0 i64) (result i32)
  (local $1 i32)
  i32.const 64
  get_local $0
  i64.clz
  i32.wrap/i64
  i32.sub
  i32.const 1233
  i32.mul
  i32.const 12
  i32.shr_u
  tee_local $1
  get_local $0
  i32.const 2552
  i32.load
  get_local $1
  i32.const 10
  i32.sub
  i32.const 2
  i32.shl
  i32.add
  i64.load32_u offset=8
  i64.const 10000000000
  i64.mul
  i64.lt_u
  i32.sub
  i32.const 1
  i32.add
 )
 (func $~lib/internal/number/utoa_simple<u64> (; 71 ;) (type $iIiv) (param $0 i32) (param $1 i64) (param $2 i32)
  (local $3 i32)
  loop $continue|0
   get_local $1
   i64.const 10
   i64.rem_u
   i32.wrap/i64
   set_local $3
   get_local $1
   i64.const 10
   i64.div_u
   set_local $1
   get_local $0
   get_local $2
   i32.const 1
   i32.sub
   tee_local $2
   i32.const 1
   i32.shl
   i32.add
   get_local $3
   i32.const 48
   i32.add
   i32.store16 offset=4
   get_local $1
   i64.const 0
   i64.ne
   br_if $continue|0
  end
 )
 (func $~lib/internal/number/itoa64 (; 72 ;) (type $Ii) (param $0 i64) (result i32)
  (local $1 i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  get_local $0
  i64.eqz
  if
   i32.const 136
   return
  end
  get_local $0
  i64.const 0
  i64.lt_s
  tee_local $1
  if
   i64.const 0
   get_local $0
   i64.sub
   set_local $0
  end
  get_local $0
  i64.const 4294967295
  i64.le_u
  if
   get_local $0
   i32.wrap/i64
   tee_local $4
   call $~lib/internal/number/decimalCount32
   get_local $1
   i32.add
   tee_local $2
   call $~lib/internal/string/allocateUnsafe
   tee_local $3
   get_local $4
   get_local $2
   call $~lib/internal/number/utoa_simple<u32>
  else   
   get_local $0
   call $~lib/internal/number/decimalCount64
   get_local $1
   i32.add
   tee_local $2
   call $~lib/internal/string/allocateUnsafe
   tee_local $3
   get_local $0
   get_local $2
   call $~lib/internal/number/utoa_simple<u64>
  end
  get_local $1
  if
   get_local $3
   i32.const 45
   i32.store16 offset=4
  end
  get_local $3
 )
 (func $~lib/internal/string/repeatUnsafe (; 73 ;) (type $FUNCSIG$viii) (param $0 i32) (param $1 i32) (param $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  (local $7 i64)
  block $break|0
   block $case5|0
    block $case4|0
     block $case3|0
      block $case2|0
       block $case1|0
        get_local $1
        i32.load
        tee_local $6
        br_table $break|0 $case1|0 $case2|0 $case3|0 $case4|0 $case5|0
       end
       get_local $1
       i32.load16_u offset=4
       set_local $5
       get_local $0
       set_local $3
       block $break|1
        i32.const 0
        set_local $0
        loop $repeat|1
         get_local $0
         get_local $2
         i32.ge_s
         br_if $break|1
         get_local $3
         get_local $0
         i32.const 1
         i32.shl
         i32.add
         get_local $5
         i32.store16 offset=4
         get_local $0
         i32.const 1
         i32.add
         set_local $0
         br $repeat|1
         unreachable
        end
        unreachable
       end
       br $break|0
      end
      get_local $1
      i32.load offset=4
      set_local $3
      get_local $0
      set_local $5
      block $break|2
       i32.const 0
       set_local $0
       loop $repeat|2
        get_local $0
        get_local $2
        i32.ge_s
        br_if $break|2
        get_local $5
        get_local $0
        i32.const 2
        i32.shl
        i32.add
        get_local $3
        i32.store offset=4
        get_local $0
        i32.const 1
        i32.add
        set_local $0
        br $repeat|2
        unreachable
       end
       unreachable
      end
      br $break|0
     end
     get_local $1
     i32.load offset=4
     set_local $5
     get_local $1
     i32.load16_u offset=8
     set_local $3
     block $break|3
      loop $repeat|3
       get_local $4
       get_local $2
       i32.ge_s
       br_if $break|3
       get_local $0
       get_local $4
       i32.const 2
       i32.shl
       i32.add
       get_local $5
       i32.store offset=4
       get_local $0
       get_local $4
       i32.const 1
       i32.shl
       i32.add
       get_local $3
       i32.store16 offset=8
       get_local $4
       i32.const 1
       i32.add
       set_local $4
       br $repeat|3
       unreachable
      end
      unreachable
     end
     br $break|0
    end
    get_local $1
    i64.load offset=4
    set_local $7
    loop $repeat|4
     get_local $3
     get_local $2
     i32.ge_s
     i32.eqz
     if
      get_local $0
      get_local $3
      i32.const 3
      i32.shl
      i32.add
      get_local $7
      i64.store offset=4
      get_local $3
      i32.const 1
      i32.add
      set_local $3
      br $repeat|4
     end
    end
    br $break|0
   end
   get_local $0
   i32.const 4
   i32.add
   set_local $3
   get_local $1
   i32.const 4
   i32.add
   set_local $5
   block $break|5
    get_local $6
    i32.const 1
    i32.shl
    tee_local $0
    get_local $2
    i32.mul
    set_local $1
    loop $repeat|5
     get_local $4
     get_local $1
     i32.ge_s
     br_if $break|5
     get_local $3
     get_local $4
     i32.add
     get_local $5
     get_local $0
     call $~lib/internal/memory/memmove
     get_local $4
     get_local $0
     i32.add
     set_local $4
     br $repeat|5
     unreachable
    end
    unreachable
   end
  end
 )
 (func $~lib/string/String#repeat (; 74 ;) (type $iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  get_local $0
  i32.eqz
  if
   call $~lib/env/abort
   unreachable
  end
  get_local $0
  i32.load
  set_local $2
  get_local $1
  i32.const 0
  i32.lt_s
  tee_local $3
  if (result i32)
   get_local $3
  else   
   get_local $2
   get_local $1
   i32.mul
   i32.const 268435456
   i32.gt_s
  end
  if
   call $~lib/env/abort
   unreachable
  end
  get_local $1
  i32.eqz
  tee_local $3
  if (result i32)
   get_local $3
  else   
   get_local $2
   i32.eqz
  end
  if
   i32.const 2136
   return
  end
  get_local $1
  i32.const 1
  i32.eq
  if
   get_local $0
   return
  end
  get_local $2
  get_local $1
  i32.mul
  call $~lib/internal/string/allocateUnsafe
  tee_local $2
  get_local $0
  get_local $1
  call $~lib/internal/string/repeatUnsafe
  get_local $2
 )
 (func $~lib/ultrain-ts-lib/src/asset/Asset#formatAmount (; 75 ;) (type $FUNCSIG$iji) (param $0 i64) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i64)
  get_local $0
  get_local $1
  i32.const 255
  i32.and
  f64.convert_s/i32
  call $~lib/math/NativeMath.pow
  i64.trunc_u/f64
  tee_local $4
  i64.div_u
  call $~lib/internal/number/itoa64
  set_local $2
  get_local $1
  i32.const 255
  i32.and
  if
   get_local $0
   get_local $4
   i64.rem_u
   call $~lib/internal/number/itoa64
   tee_local $3
   i32.load
   get_local $1
   i32.const 255
   i32.and
   i32.ne
   if
    get_local $2
    i32.const 120
    call $~lib/string/String#concat
    i32.const 136
    get_local $1
    i32.const 255
    i32.and
    get_local $3
    i32.load
    i32.sub
    call $~lib/string/String#repeat
    call $~lib/string/String#concat
    get_local $3
    call $~lib/string/String#concat
    return
   else    
    get_local $2
    i32.const 120
    call $~lib/string/String#concat
    get_local $3
    call $~lib/string/String#concat
    return
   end
   unreachable
  end
  get_local $2
 )
 (func $~lib/ultrain-ts-lib/src/asset/Asset#toString (; 76 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i64)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  get_local $0
  i64.load offset=8
  tee_local $2
  i64.const 255
  i64.and
  i32.wrap/i64
  set_local $5
  i32.const 2136
  set_local $1
  loop $repeat|0
   get_local $3
   i32.const 7
   i32.lt_s
   if
    get_local $2
    i64.const 8
    i64.shr_u
    tee_local $2
    i64.const 255
    i64.and
    i32.wrap/i64
    tee_local $4
    i32.const 255
    i32.and
    i32.const 65
    i32.ge_u
    tee_local $6
    if (result i32)
     get_local $4
     i32.const 255
     i32.and
     i32.const 90
     i32.le_u
    else     
     get_local $6
    end
    if
     get_local $1
     block (result i32)
      i32.const 1
      call $~lib/internal/string/allocateUnsafe
      tee_local $1
      get_local $4
      i32.const 255
      i32.and
      i32.store16 offset=4
      get_local $1
     end
     call $~lib/string/String#concat
     set_local $1
    end
    get_local $3
    i32.const 1
    i32.add
    set_local $3
    br $repeat|0
   end
  end
  get_local $0
  i64.load
  get_local $5
  call $~lib/ultrain-ts-lib/src/asset/Asset#formatAmount
  i32.const 8
  call $~lib/string/String#concat
  get_local $1
  call $~lib/string/String#concat
 )
 (func $~lib/string/String.__concat (; 77 ;) (type $iii) (param $0 i32) (param $1 i32) (result i32)
  get_local $0
  i32.eqz
  if
   i32.const 2400
   set_local $0
  end
  get_local $0
  get_local $1
  call $~lib/string/String#concat
 )
 (func $~lib/array/Array<u8>#__set (; 78 ;) (type $iiiv) (param $0 i32) (param $1 i32) (param $2 i32)
  (local $3 i32)
  get_local $1
  get_local $0
  i32.load
  tee_local $3
  i32.load
  i32.ge_u
  if
   get_local $1
   i32.const 1073741816
   i32.ge_u
   if
    call $~lib/env/abort
    unreachable
   end
   get_local $0
   get_local $3
   get_local $1
   i32.const 1
   i32.add
   call $~lib/internal/arraybuffer/reallocateUnsafe
   tee_local $3
   i32.store
   get_local $0
   get_local $1
   i32.const 1
   i32.add
   i32.store offset=4
  end
  get_local $3
  get_local $1
  i32.add
  get_local $2
  i32.store8 offset=8
 )
 (func $~lib/array/Array<u8>#__get (; 79 ;) (type $iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  get_local $1
  get_local $0
  i32.load
  tee_local $2
  i32.load
  i32.lt_u
  if (result i32)
   get_local $2
   get_local $1
   i32.add
   i32.load8_u offset=8
  else   
   unreachable
  end
 )
 (func $~lib/array/Array<String>#__get (; 80 ;) (type $iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  get_local $1
  get_local $0
  i32.load
  tee_local $2
  i32.load
  i32.const 2
  i32.shr_u
  i32.lt_u
  if (result i32)
   get_local $2
   get_local $1
   i32.const 2
   i32.shl
   i32.add
   i32.load offset=8
  else   
   unreachable
  end
 )
 (func $~lib/ultrain-ts-lib/lib/name/RN (; 81 ;) (type $Ii) (param $0 i64) (result i32)
  (local $1 i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  get_local $0
  i64.const 0
  i64.eq
  if
   i32.const 136
   return
  end
  i32.const 2696
  set_local $3
  block $break|0
   loop $repeat|0
    get_local $1
    i32.const 12
    i32.gt_u
    br_if $break|0
    get_local $3
    i32.const 12
    get_local $1
    i32.sub
    i32.const 2592
    get_local $0
    i64.const 31
    i64.const 15
    get_local $1
    select
    i64.and
    i32.wrap/i64
    call $~lib/string/String#charCodeAt
    i32.const 255
    i32.and
    call $~lib/array/Array<u8>#__set
    get_local $0
    i64.const 5
    i64.const 4
    get_local $1
    select
    i64.shr_u
    set_local $0
    get_local $1
    i32.const 1
    i32.add
    set_local $1
    br $repeat|0
    unreachable
   end
   unreachable
  end
  i32.const 2136
  set_local $1
  i32.const 1
  set_local $4
  block $break|1
   i32.const 12
   set_local $2
   loop $repeat|1
    get_local $2
    i32.const 0
    i32.lt_s
    br_if $break|1
    get_local $3
    get_local $2
    call $~lib/array/Array<u8>#__get
    i32.const 255
    i32.and
    i32.const 46
    i32.eq
    tee_local $5
    if (result i32)
     get_local $4
    else     
     get_local $5
    end
    i32.eqz
    if
     i32.const 0
     set_local $4
     i32.const 1272
     get_local $3
     get_local $2
     call $~lib/array/Array<u8>#__get
     i32.const 32
     i32.sub
     i32.const 255
     i32.and
     call $~lib/array/Array<String>#__get
     get_local $1
     call $~lib/string/String.__concat
     set_local $1
    end
    get_local $2
    i32.const 1
    i32.sub
    set_local $2
    br $repeat|1
    unreachable
   end
   unreachable
  end
  get_local $1
 )
 (func $~lib/string/String#get:lengthUTF8 (; 82 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  i32.const 1
  set_local $1
  get_local $0
  i32.load
  set_local $4
  loop $continue|0
   get_local $2
   get_local $4
   i32.lt_u
   if
    get_local $0
    get_local $2
    i32.const 1
    i32.shl
    i32.add
    i32.load16_u offset=4
    tee_local $3
    i32.const 128
    i32.lt_u
    if (result i32)
     get_local $1
     i32.const 1
     i32.add
     set_local $1
     get_local $2
     i32.const 1
     i32.add
    else     
     get_local $3
     i32.const 2048
     i32.lt_u
     if (result i32)
      get_local $1
      i32.const 2
      i32.add
      set_local $1
      get_local $2
      i32.const 1
      i32.add
     else      
      get_local $3
      i32.const 64512
      i32.and
      i32.const 55296
      i32.eq
      tee_local $3
      if (result i32)
       get_local $2
       i32.const 1
       i32.add
       get_local $4
       i32.lt_u
       tee_local $3
      else       
       get_local $3
      end
      if (result i32)
       get_local $0
       get_local $2
       i32.const 1
       i32.add
       i32.const 1
       i32.shl
       i32.add
       i32.load16_u offset=4
       i32.const 64512
       i32.and
       i32.const 56320
       i32.eq
      else       
       get_local $3
      end
      if (result i32)
       get_local $1
       i32.const 4
       i32.add
       set_local $1
       get_local $2
       i32.const 2
       i32.add
      else       
       get_local $1
       i32.const 3
       i32.add
       set_local $1
       get_local $2
       i32.const 1
       i32.add
      end
     end
    end
    set_local $2
    br $continue|0
   end
  end
  get_local $1
 )
 (func $~lib/datastream/DataStream#write<u8> (; 83 ;) (type $iiv) (param $0 i32) (param $1 i32)
  get_local $0
  call $~lib/datastream/DataStream#isMeasureMode
  i32.eqz
  if
   get_local $0
   i32.load
   get_local $0
   i32.load offset=8
   i32.add
   get_local $1
   i32.store8
  end
  get_local $0
  get_local $0
  i32.load offset=8
  i32.const 1
  i32.add
  i32.store offset=8
 )
 (func $~lib/datastream/DataStream#writeVarint32 (; 84 ;) (type $iiv) (param $0 i32) (param $1 i32)
  loop $continue|0
   get_local $0
   get_local $1
   i32.const 127
   i32.and
   i32.const 1
   i32.const 0
   get_local $1
   i32.const 7
   i32.shr_u
   tee_local $1
   i32.const 0
   i32.gt_u
   select
   i32.const 7
   i32.shl
   i32.or
   call $~lib/datastream/DataStream#write<u8>
   get_local $1
   br_if $continue|0
  end
 )
 (func $~lib/string/String#toUTF8 (; 85 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  (local $7 i32)
  get_local $0
  call $~lib/string/String#get:lengthUTF8
  call $~lib/allocator/arena/__memory_allocate
  set_local $5
  get_local $0
  i32.load
  set_local $7
  loop $continue|0
   get_local $3
   get_local $7
   i32.lt_u
   if
    get_local $0
    get_local $3
    i32.const 1
    i32.shl
    i32.add
    i32.load16_u offset=4
    tee_local $1
    i32.const 128
    i32.lt_u
    if
     get_local $5
     get_local $2
     i32.add
     get_local $1
     i32.store8
     get_local $2
     i32.const 1
     i32.add
     set_local $2
    else     
     get_local $1
     i32.const 2048
     i32.lt_u
     if
      get_local $5
      get_local $2
      i32.add
      tee_local $4
      get_local $1
      i32.const 6
      i32.shr_u
      i32.const 192
      i32.or
      i32.store8
      get_local $4
      get_local $1
      i32.const 63
      i32.and
      i32.const 128
      i32.or
      i32.store8 offset=1
      get_local $2
      i32.const 2
      i32.add
      set_local $2
     else      
      get_local $5
      get_local $2
      i32.add
      set_local $4
      get_local $1
      i32.const 64512
      i32.and
      i32.const 55296
      i32.eq
      tee_local $6
      if (result i32)
       get_local $3
       i32.const 1
       i32.add
       get_local $7
       i32.lt_u
      else       
       get_local $6
      end
      if
       get_local $0
       get_local $3
       i32.const 1
       i32.add
       i32.const 1
       i32.shl
       i32.add
       i32.load16_u offset=4
       tee_local $6
       i32.const 64512
       i32.and
       i32.const 56320
       i32.eq
       if
        get_local $4
        get_local $1
        i32.const 1023
        i32.and
        i32.const 10
        i32.shl
        i32.const 65536
        i32.add
        get_local $6
        i32.const 1023
        i32.and
        i32.add
        tee_local $1
        i32.const 18
        i32.shr_u
        i32.const 240
        i32.or
        i32.store8
        get_local $4
        get_local $1
        i32.const 12
        i32.shr_u
        i32.const 63
        i32.and
        i32.const 128
        i32.or
        i32.store8 offset=1
        get_local $4
        get_local $1
        i32.const 6
        i32.shr_u
        i32.const 63
        i32.and
        i32.const 128
        i32.or
        i32.store8 offset=2
        get_local $4
        get_local $1
        i32.const 63
        i32.and
        i32.const 128
        i32.or
        i32.store8 offset=3
        get_local $2
        i32.const 4
        i32.add
        set_local $2
        get_local $3
        i32.const 2
        i32.add
        set_local $3
        br $continue|0
       end
      end
      get_local $4
      get_local $1
      i32.const 12
      i32.shr_u
      i32.const 224
      i32.or
      i32.store8
      get_local $4
      get_local $1
      i32.const 6
      i32.shr_u
      i32.const 63
      i32.and
      i32.const 128
      i32.or
      i32.store8 offset=1
      get_local $4
      get_local $1
      i32.const 63
      i32.and
      i32.const 128
      i32.or
      i32.store8 offset=2
      get_local $2
      i32.const 3
      i32.add
      set_local $2
     end
    end
    get_local $3
    i32.const 1
    i32.add
    set_local $3
    br $continue|0
   end
  end
  get_local $5
  get_local $2
  i32.add
  i32.const 0
  i32.store8
  get_local $5
 )
 (func $~lib/datastream/DataStream#writeString (; 86 ;) (type $iiv) (param $0 i32) (param $1 i32)
  (local $2 i32)
  get_local $0
  get_local $1
  call $~lib/string/String#get:lengthUTF8
  i32.const 1
  i32.sub
  tee_local $2
  call $~lib/datastream/DataStream#writeVarint32
  get_local $2
  i32.eqz
  if
   return
  end
  get_local $0
  call $~lib/datastream/DataStream#isMeasureMode
  i32.eqz
  if
   get_local $1
   call $~lib/string/String#toUTF8
   set_local $1
   get_local $0
   i32.load
   get_local $0
   i32.load offset=8
   i32.add
   get_local $1
   get_local $2
   call $~lib/internal/memory/memmove
  end
  get_local $0
  get_local $0
  i32.load offset=8
  get_local $2
  i32.add
  i32.store offset=8
 )
 (func $~lib/datastream/DataStream#write<u32> (; 87 ;) (type $iiv) (param $0 i32) (param $1 i32)
  get_local $0
  call $~lib/datastream/DataStream#isMeasureMode
  i32.eqz
  if
   get_local $0
   i32.load
   get_local $0
   i32.load offset=8
   i32.add
   get_local $1
   i32.store
  end
  get_local $0
  get_local $0
  i32.load offset=8
  i32.const 4
  i32.add
  i32.store offset=8
 )
 (func $contract/MyContract/Candidate#serialize (; 88 ;) (type $iiv) (param $0 i32) (param $1 i32)
  get_local $1
  get_local $0
  i64.load
  call $~lib/datastream/DataStream#write<u64>
  get_local $1
  get_local $0
  i32.load offset=8
  call $~lib/datastream/DataStream#writeString
  get_local $1
  get_local $0
  i32.load offset=12
  call $~lib/datastream/DataStream#write<u32>
 )
 (func $~lib/dbmanager/DBManager<Candidate>#emplace (; 89 ;) (type $iIiv) (param $0 i32) (param $1 i64) (param $2 i32)
  (local $3 i32)
  get_local $0
  i64.load offset=8
  call $~lib/env/current_receiver
  i64.eq
  i32.const 1776
  call $~lib/env/ultrain_assert
  get_local $2
  block (result i32)
   get_local $2
   i32.const 0
   i32.const 0
   call $~lib/datastream/DataStream#constructor
   tee_local $3
   call $contract/MyContract/Candidate#serialize
   get_local $3
   i32.load offset=8
   tee_local $3
  end
  call $~lib/internal/typedarray/TypedArray<u8_u32>#constructor
  i32.load
  get_local $3
  call $~lib/datastream/DataStream#constructor
  tee_local $3
  call $contract/MyContract/Candidate#serialize
  get_local $0
  i64.load offset=16
  get_local $0
  i64.load
  get_local $1
  get_local $2
  i64.load
  get_local $3
  i32.load
  get_local $3
  i32.load offset=8
  call $~lib/env/db_store_i64
  drop
 )
 (func $contract/MyContract/rand#transfer (; 90 ;) (type $FUNCSIG$vijii) (param $0 i32) (param $1 i64) (param $2 i32) (param $3 i32)
  (local $4 i32)
  get_local $1
  get_local $0
  i64.load
  i64.eq
  if
   return
  end
  get_local $3
  i32.const 2200
  call $~lib/string/String.__eq
  i32.eqz
  if
   return
  end
  get_local $0
  i32.load offset=12
  get_local $1
  call $~lib/dbmanager/DBManager<Vote>#exists
  i32.eqz
  i32.const 2232
  call $~lib/env/ultrain_assert
  get_local $2
  i64.load offset=8
  get_global $contract/MyContract/DEPOSIT_AMOUNT
  tee_local $3
  i64.load offset=8
  i64.eq
  tee_local $4
  if (result i32)
   get_local $2
   i64.load
   get_local $3
   i64.load
   i64.eq
  else   
   get_local $4
  end
  i32.const 2296
  get_global $contract/MyContract/DEPOSIT_AMOUNT
  call $~lib/ultrain-ts-lib/src/asset/Asset#toString
  call $~lib/string/String.__concat
  call $~lib/env/ultrain_assert
  get_global $~lib/ultrain-ts-lib/src/log/Log
  i32.const 2560
  call $~lib/ultrain-ts-lib/src/log/Logger#s
  get_local $1
  call $~lib/ultrain-ts-lib/lib/name/RN
  call $~lib/ultrain-ts-lib/src/log/Logger#s
  drop
  call $~lib/ultrain-ts-lib/src/log/env.ts_log_done
  i32.const 16
  call $~lib/allocator/arena/__memory_allocate
  tee_local $2
  i64.const 0
  i64.store
  get_local $2
  i32.const 2136
  i32.store offset=8
  get_local $2
  i32.const 0
  i32.store offset=12
  get_local $2
  get_local $1
  i64.store
  get_local $2
  call $~lib/ultrain-ts-lib/lib/headblock/env.head_block_number
  i32.store offset=12
  get_local $0
  i32.load offset=12
  get_local $0
  i64.load
  get_local $2
  call $~lib/dbmanager/DBManager<Candidate>#emplace
 )
 (func $~lib/dbmanager/DBManager<Candidate>#erase (; 91 ;) (type $iIv) (param $0 i32) (param $1 i64)
  (local $2 i32)
  get_local $0
  i64.load offset=8
  call $~lib/env/current_receiver
  i64.eq
  i32.const 2824
  call $~lib/env/ultrain_assert
  get_local $0
  get_local $1
  call $~lib/dbmanager/DBManager<Vote>#find
  tee_local $2
  i32.const 0
  i32.ge_s
  if
   get_local $2
   call $~lib/env/db_remove_i64
  end
 )
 (func $~lib/ultrain-ts-lib/src/action/TransferParams#constructor (; 92 ;) (type $FUNCSIG$ijjii) (param $0 i64) (param $1 i64) (param $2 i32) (param $3 i32) (result i32)
  (local $4 i32)
  i32.const 24
  call $~lib/allocator/arena/__memory_allocate
  tee_local $4
  i64.const 0
  i64.store
  get_local $4
  i64.const 0
  i64.store offset=8
  get_local $4
  i32.const 0
  i32.store offset=16
  get_local $4
  i32.const 0
  i32.store offset=20
  get_local $4
  get_local $0
  i64.store
  get_local $4
  get_local $1
  i64.store offset=8
  get_local $4
  get_local $2
  i32.store offset=16
  get_local $4
  get_local $3
  i32.store offset=20
  get_local $4
 )
 (func $~lib/array/Array<PermissionLevel>#constructor (; 93 ;) (type $FUNCSIG$i) (result i32)
  (local $0 i32)
  (local $1 i32)
  i32.const 4
  call $~lib/internal/arraybuffer/allocateUnsafe
  set_local $1
  i32.const 8
  call $~lib/allocator/arena/__memory_allocate
  tee_local $0
  i32.const 0
  i32.store
  get_local $0
  i32.const 0
  i32.store offset=4
  get_local $0
  get_local $1
  i32.store
  get_local $0
  i32.const 1
  i32.store offset=4
  get_local $1
  i32.const 8
  i32.add
  i32.const 4
  call $~lib/internal/memory/memset
  get_local $0
 )
 (func $~lib/ultrain-ts-lib/src/action/ActionImpl#constructor (; 94 ;) (type $FUNCSIG$i) (result i32)
  (local $0 i32)
  i32.const 20
  call $~lib/allocator/arena/__memory_allocate
  tee_local $0
  i64.const 0
  i64.store
  get_local $0
  i32.const 0
  i32.store offset=8
  get_local $0
  i32.const 0
  i32.store offset=12
  get_local $0
  i32.const 0
  i32.store offset=16
  get_local $0
  i64.const 0
  i64.store
  get_local $0
  i64.const 0
  i64.const 0
  call $~lib/ultrain-ts-lib/src/asset/Asset#constructor
  i32.store offset=8
  get_local $0
  i32.const 3008
  i32.store offset=12
  get_local $0
  i32.const 3024
  i32.store offset=16
  get_local $0
 )
 (func $~lib/ultrain-ts-lib/src/action/TransferParams#serialize (; 95 ;) (type $iiv) (param $0 i32) (param $1 i32)
  get_local $1
  get_local $0
  i64.load
  call $~lib/datastream/DataStream#write<u64>
  get_local $1
  get_local $0
  i64.load offset=8
  call $~lib/datastream/DataStream#write<u64>
  get_local $0
  i32.load offset=16
  get_local $1
  call $contract/MyContract/Vote#serialize
  get_local $1
  get_local $0
  i32.load offset=20
  call $~lib/datastream/DataStream#writeString
 )
 (func $~lib/datastream/DataStream#toArray<u8> (; 96 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  get_local $0
  i32.load offset=4
  i32.eqz
  if
   i32.const 0
   call $~lib/array/Array<u8>#constructor
   return
  end
  get_local $0
  i32.load offset=4
  tee_local $4
  call $~lib/array/Array<u8>#constructor
  set_local $3
  block $break|0
   loop $repeat|0
    get_local $1
    get_local $4
    i32.ge_u
    br_if $break|0
    get_local $3
    get_local $1
    get_local $0
    i32.load
    get_local $2
    i32.add
    i32.load8_u
    call $~lib/array/Array<u8>#__set
    get_local $2
    i32.const 1
    i32.add
    set_local $2
    get_local $1
    i32.const 1
    i32.add
    set_local $1
    br $repeat|0
    unreachable
   end
   unreachable
  end
  get_local $3
 )
 (func $~lib/ultrain-ts-lib/src/action/SerializableToArray<TransferParams> (; 97 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  get_local $0
  block (result i32)
   get_local $0
   i32.const 0
   i32.const 0
   call $~lib/datastream/DataStream#constructor
   tee_local $1
   call $~lib/ultrain-ts-lib/src/action/TransferParams#serialize
   get_local $1
   i32.load offset=8
   tee_local $0
  end
  call $~lib/internal/typedarray/TypedArray<u8_u32>#constructor
  i32.load
  get_local $0
  call $~lib/datastream/DataStream#constructor
  tee_local $0
  call $~lib/ultrain-ts-lib/src/action/TransferParams#serialize
  get_local $0
  call $~lib/datastream/DataStream#toArray<u8>
 )
 (func $~lib/datastream/DataStream#writeComplexVector<PermissionLevel> (; 98 ;) (type $iiv) (param $0 i32) (param $1 i32)
  (local $2 i32)
  (local $3 i32)
  get_local $0
  get_local $1
  i32.load offset=4
  tee_local $3
  call $~lib/datastream/DataStream#writeVarint32
  block $break|0
   loop $repeat|0
    get_local $2
    get_local $3
    i32.ge_u
    br_if $break|0
    get_local $1
    get_local $2
    call $~lib/array/Array<String>#__get
    get_local $0
    call $contract/MyContract/Vote#serialize
    get_local $2
    i32.const 1
    i32.add
    set_local $2
    br $repeat|0
    unreachable
   end
   unreachable
  end
 )
 (func $~lib/datastream/DataStream#writeVector<u8> (; 99 ;) (type $iiv) (param $0 i32) (param $1 i32)
  (local $2 i32)
  (local $3 i32)
  get_local $0
  get_local $1
  i32.load offset=4
  tee_local $3
  call $~lib/datastream/DataStream#writeVarint32
  block $break|0
   loop $repeat|0
    get_local $2
    get_local $3
    i32.ge_u
    br_if $break|0
    get_local $0
    get_local $1
    get_local $2
    call $~lib/array/Array<u8>#__get
    call $~lib/datastream/DataStream#write<u8>
    get_local $2
    i32.const 1
    i32.add
    set_local $2
    br $repeat|0
    unreachable
   end
   unreachable
  end
 )
 (func $~lib/ultrain-ts-lib/src/action/ActionImpl#serialize (; 100 ;) (type $iiv) (param $0 i32) (param $1 i32)
  get_local $1
  get_local $0
  i64.load
  call $~lib/datastream/DataStream#write<u64>
  get_local $0
  i32.load offset=8
  get_local $1
  call $contract/MyContract/Vote#serialize
  get_local $1
  get_local $0
  i32.load offset=12
  call $~lib/datastream/DataStream#writeComplexVector<PermissionLevel>
  get_local $1
  get_local $0
  i32.load offset=16
  call $~lib/datastream/DataStream#writeVector<u8>
 )
 (func $~lib/ultrain-ts-lib/src/action/composeActionData<TransferParams> (; 101 ;) (type $iIiii) (param $0 i32) (param $1 i64) (param $2 i32) (param $3 i32) (result i32)
  (local $4 i32)
  call $~lib/ultrain-ts-lib/src/action/ActionImpl#constructor
  tee_local $4
  get_local $0
  i32.store offset=12
  get_local $4
  get_local $1
  i64.store
  get_local $4
  get_local $2
  i32.store offset=8
  get_local $4
  get_local $3
  call $~lib/ultrain-ts-lib/src/action/SerializableToArray<TransferParams>
  i32.store offset=16
  get_local $4
  block (result i32)
   get_local $4
   i32.const 0
   i32.const 0
   call $~lib/datastream/DataStream#constructor
   tee_local $0
   call $~lib/ultrain-ts-lib/src/action/ActionImpl#serialize
   get_local $0
   i32.load offset=8
   tee_local $0
  end
  call $~lib/internal/typedarray/TypedArray<u8_u32>#constructor
  i32.load
  get_local $0
  call $~lib/datastream/DataStream#constructor
  tee_local $0
  call $~lib/ultrain-ts-lib/src/action/ActionImpl#serialize
  get_local $0
 )
 (func $~lib/ultrain-ts-lib/src/asset/Asset.transfer (; 102 ;) (type $IIiiv) (param $0 i64) (param $1 i64) (param $2 i32) (param $3 i32)
  (local $4 i32)
  i64.const 0
  i64.const 0
  call $~lib/ultrain-ts-lib/src/asset/Asset#constructor
  tee_local $4
  get_local $0
  i64.store
  get_local $4
  i32.const 2984
  call $~lib/ultrain-ts-lib/lib/name/N
  i64.store offset=8
  get_local $0
  get_local $1
  get_local $2
  get_local $3
  call $~lib/ultrain-ts-lib/src/action/TransferParams#constructor
  set_local $2
  block (result i32)
   call $~lib/array/Array<PermissionLevel>#constructor
   tee_local $3
   i32.load
   get_local $4
   i32.store offset=8
   get_local $3
  end
  i32.const 2008
  call $~lib/ultrain-ts-lib/lib/name/N
  block (result i32)
   i32.const 1984
   call $~lib/ultrain-ts-lib/lib/name_ex/NEX
   tee_local $3
   i64.load
   set_local $0
   get_local $3
   i64.load offset=8
   set_local $1
   i32.const 4
   call $~lib/allocator/arena/__memory_allocate
   tee_local $3
   i32.const 0
   i32.store
   get_local $3
   get_local $0
   get_local $1
   call $~lib/ultrain-ts-lib/src/asset/Asset#constructor
   i32.store
   get_local $3
   i32.load
  end
  get_local $2
  call $~lib/ultrain-ts-lib/src/action/composeActionData<TransferParams>
  tee_local $2
  i32.load
  get_local $2
  i32.load offset=8
  call $~lib/ultrain-ts-lib/internal/action.d/env.send_inline
 )
 (func $contract/MyContract/rand#removeCandidate (; 103 ;) (type $iv) (param $0 i32)
  get_local $0
  i32.load offset=12
  call $~lib/ultrain-ts-lib/internal/action.d/env.current_sender
  call $~lib/dbmanager/DBManager<Vote>#exists
  i32.const 2744
  call $~lib/env/ultrain_assert
  get_local $0
  i32.load offset=12
  call $~lib/ultrain-ts-lib/internal/action.d/env.current_sender
  call $~lib/dbmanager/DBManager<Candidate>#erase
  get_local $0
  i32.load offset=16
  call $~lib/ultrain-ts-lib/internal/action.d/env.current_sender
  call $~lib/dbmanager/DBManager<Vote>#exists
  if
   get_local $0
   i32.load offset=16
   call $~lib/ultrain-ts-lib/internal/action.d/env.current_sender
   call $~lib/dbmanager/DBManager<Candidate>#erase
  end
  get_local $0
  i64.load
  call $~lib/ultrain-ts-lib/internal/action.d/env.current_sender
  get_global $contract/MyContract/DEPOSIT_AMOUNT
  i32.const 2936
  call $~lib/ultrain-ts-lib/src/asset/Asset.transfer
 )
 (func $~lib/dbmanager/DBManager<Vote>#loadObjectByPrimaryIterator (; 104 ;) (type $FUNCSIG$vii) (param $0 i32) (param $1 i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  get_local $0
  i32.const 0
  i32.const 0
  call $~lib/env/db_get_i64
  tee_local $2
  call $~lib/internal/typedarray/TypedArray<u8_u32>#constructor
  tee_local $3
  i32.load
  get_local $2
  call $~lib/datastream/DataStream#constructor
  set_local $4
  get_local $0
  get_local $3
  i32.load
  get_local $2
  call $~lib/env/db_get_i64
  drop
  get_local $1
  get_local $4
  call $~lib/ultrain-ts-lib/src/asset/Asset#deserialize
 )
 (func $~lib/dbmanager/DBManager<Vote>#get (; 105 ;) (type $iIii) (param $0 i32) (param $1 i64) (param $2 i32) (result i32)
  (local $3 i32)
  get_local $0
  i64.load offset=8
  get_local $0
  i64.load offset=16
  get_local $0
  i64.load
  get_local $1
  call $~lib/env/db_find_i64
  tee_local $3
  i32.const 0
  i32.lt_s
  if
   i32.const 0
   return
  end
  get_local $3
  get_local $2
  call $~lib/dbmanager/DBManager<Vote>#loadObjectByPrimaryIterator
  i32.const 1
 )
 (func $contract/MyContract/Candidate#deserialize (; 106 ;) (type $iiv) (param $0 i32) (param $1 i32)
  (local $2 i32)
  get_local $0
  get_local $1
  call $~lib/datastream/DataStream#read<u64>
  i64.store
  get_local $0
  get_local $1
  call $~lib/datastream/DataStream#readString
  i32.store offset=8
  get_local $1
  i32.load
  get_local $1
  i32.load offset=8
  i32.add
  i32.load
  set_local $2
  get_local $1
  get_local $1
  i32.load offset=8
  i32.const 4
  i32.add
  i32.store offset=8
  get_local $0
  get_local $2
  i32.store offset=12
 )
 (func $~lib/dbmanager/DBManager<Candidate>#loadObjectByPrimaryIterator (; 107 ;) (type $FUNCSIG$vii) (param $0 i32) (param $1 i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  get_local $0
  i32.const 0
  i32.const 0
  call $~lib/env/db_get_i64
  tee_local $2
  call $~lib/internal/typedarray/TypedArray<u8_u32>#constructor
  tee_local $3
  i32.load
  get_local $2
  call $~lib/datastream/DataStream#constructor
  set_local $4
  get_local $0
  get_local $3
  i32.load
  get_local $2
  call $~lib/env/db_get_i64
  drop
  block
   get_local $1
   get_local $4
   call $contract/MyContract/Candidate#deserialize
  end
 )
 (func $~lib/dbmanager/DBManager<Candidate>#get (; 108 ;) (type $iIii) (param $0 i32) (param $1 i64) (param $2 i32) (result i32)
  (local $3 i32)
  get_local $0
  i64.load offset=8
  get_local $0
  i64.load offset=16
  get_local $0
  i64.load
  get_local $1
  call $~lib/env/db_find_i64
  tee_local $3
  i32.const 0
  i32.lt_s
  if
   i32.const 0
   return
  end
  get_local $3
  get_local $2
  call $~lib/dbmanager/DBManager<Candidate>#loadObjectByPrimaryIterator
  i32.const 1
 )
 (func $~lib/ultrain-ts-lib/src/utils/intToString (; 109 ;) (type $FUNCSIG$ij) (param $0 i64) (result i32)
  (local $1 i64)
  (local $2 i32)
  (local $3 i32)
  get_local $0
  i64.const 10
  i64.div_u
  set_local $1
  i32.const 1272
  get_local $0
  i64.const 10
  i64.rem_u
  i32.wrap/i64
  i32.const 16
  i32.add
  call $~lib/array/Array<String>#__get
  set_local $2
  loop $continue|0
   get_local $1
   i64.const 0
   i64.ne
   if
    get_local $1
    i64.const 10
    i64.rem_u
    i32.wrap/i64
    set_local $3
    get_local $1
    i64.const 10
    i64.div_u
    set_local $1
    i32.const 1272
    get_local $3
    i32.const 16
    i32.add
    call $~lib/array/Array<String>#__get
    get_local $2
    call $~lib/string/String.__concat
    set_local $2
    br $continue|0
   end
  end
  get_local $2
 )
 (func $~lib/ultrain-ts-lib/src/account/Account.publicKeyOf (; 110 ;) (type $FUNCSIG$ij) (param $0 i64) (result i32)
  (local $1 i32)
  (local $2 i32)
  get_local $0
  i32.const 128
  call $~lib/internal/typedarray/TypedArray<u8_u32>#constructor
  tee_local $1
  i32.load
  get_local $1
  i32.load offset=8
  i32.const 3152
  call $~lib/utf8util/string2cstr
  call $~lib/ultrain-ts-lib/internal/crypto.d/env.ts_public_key_of_account
  tee_local $2
  i32.const 0
  i32.gt_s
  if (result i32)
   get_local $1
   i32.load
   get_local $2
   call $~lib/string/String.fromUTF8
  else   
   i32.const 2136
  end
 )
 (func $~lib/string/String#substring (; 111 ;) (type $FUNCSIG$iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  get_local $0
  i32.eqz
  if
   call $~lib/env/abort
   unreachable
  end
  i32.const 0
  get_local $0
  i32.load
  tee_local $4
  tee_local $3
  i32.const 0
  get_local $3
  i32.lt_s
  select
  set_local $5
  get_local $1
  tee_local $2
  i32.const 0
  get_local $2
  i32.const 0
  i32.gt_s
  select
  tee_local $2
  get_local $4
  tee_local $3
  get_local $2
  get_local $3
  i32.lt_s
  select
  set_local $3
  get_local $5
  tee_local $2
  get_local $3
  get_local $2
  get_local $3
  i32.lt_s
  select
  set_local $1
  get_local $2
  get_local $3
  get_local $2
  get_local $3
  i32.gt_s
  select
  tee_local $3
  get_local $1
  i32.sub
  tee_local $4
  i32.eqz
  if
   i32.const 2136
   return
  end
  get_local $1
  i32.eqz
  tee_local $2
  if (result i32)
   get_local $3
   get_local $0
   i32.load
   i32.eq
  else   
   get_local $2
  end
  if
   get_local $0
   return
  end
  get_local $4
  call $~lib/internal/string/allocateUnsafe
  tee_local $2
  i32.const 0
  get_local $0
  get_local $1
  get_local $4
  call $~lib/internal/string/copyUnsafe
  get_local $2
 )
 (func $~lib/ultrain-ts-lib/src/crypto/Crypto#get:buffer (; 112 ;) (type $ii) (param $0 i32) (result i32)
  get_local $0
  i32.load
  i32.load
 )
 (func $~lib/ultrain-ts-lib/src/crypto/Crypto#get:bufferSize (; 113 ;) (type $ii) (param $0 i32) (result i32)
  get_local $0
  i32.load
  i32.load offset=8
 )
 (func $~lib/string/String#charAt (; 114 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  (local $1 i32)
  get_local $0
  i32.const 1624
  i32.load
  i32.ge_u
  if
   i32.const 2136
   return
  end
  i32.const 1
  call $~lib/internal/string/allocateUnsafe
  tee_local $1
  get_local $0
  i32.const 1
  i32.shl
  i32.const 1624
  i32.add
  i32.load16_u offset=4
  i32.store16 offset=4
  get_local $1
 )
 (func $~lib/ultrain-ts-lib/src/crypto/to_hex (; 115 ;) (type $iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  i32.const 2136
  set_local $2
  block $break|0
   loop $repeat|0
    get_local $3
    get_local $1
    i32.ge_u
    br_if $break|0
    get_local $2
    get_local $0
    get_local $3
    i32.add
    i32.load8_u
    tee_local $2
    i32.const 4
    i32.shr_u
    call $~lib/string/String#charAt
    call $~lib/string/String.__concat
    get_local $2
    i32.const 15
    i32.and
    call $~lib/string/String#charAt
    call $~lib/string/String.__concat
    set_local $2
    get_local $3
    i32.const 1
    i32.add
    set_local $3
    br $repeat|0
    unreachable
   end
   unreachable
  end
  get_local $2
 )
 (func $~lib/internal/string/parse<f64> (; 116 ;) (type $iiF) (param $0 i32) (param $1 i32) (result f64)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 f64)
  (local $6 f64)
  get_local $0
  i32.load
  tee_local $4
  i32.eqz
  if
   f64.const nan:0x8000000000000
   return
  end
  get_local $0
  tee_local $3
  i32.load16_u offset=4
  tee_local $2
  i32.const 45
  i32.eq
  if (result f64)
   get_local $4
   i32.const 1
   i32.sub
   tee_local $4
   i32.eqz
   if
    f64.const nan:0x8000000000000
    return
   end
   get_local $3
   i32.const 2
   i32.add
   tee_local $3
   i32.load16_u offset=4
   set_local $2
   f64.const -1
  else   
   get_local $2
   i32.const 43
   i32.eq
   if
    get_local $4
    i32.const 1
    i32.sub
    tee_local $4
    i32.eqz
    if
     f64.const nan:0x8000000000000
     return
    end
    get_local $3
    i32.const 2
    i32.add
    tee_local $3
    i32.load16_u offset=4
    set_local $2
   end
   f64.const 1
  end
  set_local $6
  get_local $1
  if
   get_local $1
   i32.const 2
   i32.lt_s
   tee_local $0
   if (result i32)
    get_local $0
   else    
    get_local $1
    i32.const 36
    i32.gt_s
   end
   if
    f64.const nan:0x8000000000000
    return
   end
  else   
   get_local $2
   i32.const 48
   i32.eq
   tee_local $0
   if (result i32)
    get_local $4
    i32.const 2
    i32.gt_s
   else    
    get_local $0
   end
   if (result i32)
    block $break|0 (result i32)
     block $case6|0
      block $case5|0
       block $case3|0
        get_local $3
        i32.const 2
        i32.add
        i32.load16_u offset=4
        tee_local $0
        i32.const 66
        i32.eq
        get_local $0
        i32.const 98
        i32.eq
        i32.or
        i32.eqz
        if
         get_local $0
         i32.const 79
         i32.eq
         get_local $0
         i32.const 111
         i32.eq
         i32.or
         br_if $case3|0
         get_local $0
         i32.const 88
         i32.eq
         get_local $0
         i32.const 120
         i32.eq
         i32.or
         br_if $case5|0
         br $case6|0
        end
        get_local $3
        i32.const 4
        i32.add
        set_local $3
        get_local $4
        i32.const 2
        i32.sub
        set_local $4
        i32.const 2
        br $break|0
       end
       get_local $3
       i32.const 4
       i32.add
       set_local $3
       get_local $4
       i32.const 2
       i32.sub
       set_local $4
       i32.const 8
       br $break|0
      end
      get_local $3
      i32.const 4
      i32.add
      set_local $3
      get_local $4
      i32.const 2
      i32.sub
      set_local $4
      i32.const 16
      br $break|0
     end
     i32.const 10
    end
   else    
    i32.const 10
   end
   set_local $1
  end
  loop $continue|1
   block (result i32)
    get_local $4
    tee_local $0
    i32.const 1
    i32.sub
    set_local $4
    get_local $0
   end
   if
    block $break|1
     get_local $3
     i32.load16_u offset=4
     tee_local $2
     i32.const 48
     i32.ge_s
     tee_local $0
     if (result i32)
      get_local $2
      i32.const 57
      i32.le_s
     else      
      get_local $0
     end
     if (result i32)
      get_local $2
      i32.const 48
      i32.sub
     else      
      get_local $2
      i32.const 65
      i32.ge_s
      tee_local $0
      if (result i32)
       get_local $2
       i32.const 90
       i32.le_s
      else       
       get_local $0
      end
      if (result i32)
       get_local $2
       i32.const 55
       i32.sub
      else       
       get_local $2
       i32.const 97
       i32.ge_s
       tee_local $0
       if (result i32)
        get_local $2
        i32.const 122
        i32.le_s
       else        
        get_local $0
       end
       if (result i32)
        get_local $2
        i32.const 87
        i32.sub
       else        
        br $break|1
       end
      end
     end
     tee_local $2
     get_local $1
     i32.ge_s
     br_if $break|1
     get_local $5
     get_local $1
     f64.convert_s/i32
     f64.mul
     get_local $2
     f64.convert_s/i32
     f64.add
     set_local $5
     get_local $3
     i32.const 2
     i32.add
     set_local $3
     br $continue|1
    end
   end
  end
  get_local $6
  get_local $5
  f64.mul
 )
 (func $~lib/dbmanager/DBManager<Vote>#modify (; 117 ;) (type $iIiv) (param $0 i32) (param $1 i64) (param $2 i32)
  (local $3 i32)
  get_local $0
  get_local $2
  i64.load
  call $~lib/dbmanager/DBManager<Vote>#find
  tee_local $3
  i32.const 0
  i32.ge_s
  i32.const 3264
  call $~lib/env/ultrain_assert
  get_local $0
  i64.load offset=8
  call $~lib/env/current_receiver
  i64.eq
  i32.const 3384
  call $~lib/env/ultrain_assert
  get_local $2
  get_local $2
  call $~lib/datastream/DataStream.measure<Vote>
  tee_local $0
  call $~lib/internal/typedarray/TypedArray<u8_u32>#constructor
  i32.load
  get_local $0
  call $~lib/datastream/DataStream#constructor
  tee_local $0
  call $contract/MyContract/Vote#serialize
  get_local $3
  get_local $1
  get_local $0
  i32.load
  get_local $0
  i32.load offset=8
  call $~lib/env/db_update_i64
 )
 (func $~lib/ultrain-ts-lib/src/return/Return<String> (; 118 ;) (type $iv) (param $0 i32)
  get_local $0
  call $~lib/utf8util/string2cstr
  call $~lib/ultrain-ts-lib/src/return/env.set_result_str
 )
 (func $contract/MyContract/rand#vote (; 119 ;) (type $iiv) (param $0 i32) (param $1 i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i64)
  (local $5 i64)
  (local $6 i32)
  get_local $0
  i32.load offset=12
  call $~lib/ultrain-ts-lib/internal/action.d/env.current_sender
  call $~lib/dbmanager/DBManager<Vote>#exists
  i32.const 3032
  call $~lib/env/ultrain_assert
  i32.const 16
  call $~lib/allocator/arena/__memory_allocate
  tee_local $2
  i64.const 0
  i64.store
  get_local $2
  i64.const 0
  i64.store offset=8
  get_local $0
  i32.load offset=16
  i32.const 1752
  call $~lib/ultrain-ts-lib/lib/name/N
  get_local $2
  call $~lib/dbmanager/DBManager<Vote>#get
  drop
  call $~lib/ultrain-ts-lib/lib/headblock/env.head_block_number
  i64.extend_u/i32
  get_local $2
  i64.load offset=8
  i64.sub
  i64.const 3
  i64.ge_u
  if
   get_local $2
   i64.load offset=8
   set_local $4
   loop $continue|0
    get_local $4
    i64.const 3
    i64.add
    call $~lib/ultrain-ts-lib/lib/headblock/env.head_block_number
    i64.extend_u/i32
    i64.lt_u
    if
     get_local $4
     i64.const 3
     i64.add
     set_local $4
     br $continue|0
    end
   end
   get_local $0
   i32.load offset=16
   i32.const 1968
   call $~lib/ultrain-ts-lib/lib/name/N
   get_local $2
   call $~lib/dbmanager/DBManager<Vote>#get
   drop
   get_local $2
   i64.load offset=8
   set_local $5
   get_local $0
   i32.load offset=16
   call $~lib/dbmanager/DBManager<Candidate>#dropAll
   drop
   get_local $2
   i32.const 1752
   call $~lib/ultrain-ts-lib/lib/name/N
   i64.store
   get_local $2
   get_local $4
   i64.store offset=8
   get_local $0
   i32.load offset=16
   call $~lib/ultrain-ts-lib/internal/action.d/env.current_sender
   get_local $2
   call $~lib/dbmanager/DBManager<Vote>#emplace
   get_local $2
   i32.const 1952
   call $~lib/ultrain-ts-lib/lib/name/N
   i64.store
   get_local $2
   get_local $5
   i64.store offset=8
   get_local $0
   i32.load offset=16
   call $~lib/ultrain-ts-lib/internal/action.d/env.current_sender
   get_local $2
   call $~lib/dbmanager/DBManager<Vote>#emplace
   get_local $2
   i32.const 1968
   call $~lib/ultrain-ts-lib/lib/name/N
   i64.store
   get_local $2
   i64.const 0
   i64.store offset=8
   get_local $0
   i32.load offset=16
   call $~lib/ultrain-ts-lib/internal/action.d/env.current_sender
   get_local $2
   call $~lib/dbmanager/DBManager<Vote>#emplace
  end
  get_local $0
  i32.load offset=16
  call $~lib/ultrain-ts-lib/internal/action.d/env.current_sender
  call $~lib/dbmanager/DBManager<Vote>#exists
  i32.eqz
  i32.const 3104
  call $~lib/env/ultrain_assert
  i32.const 16
  call $~lib/allocator/arena/__memory_allocate
  tee_local $3
  i64.const 0
  i64.store
  get_local $3
  i32.const 2136
  i32.store offset=8
  get_local $3
  i32.const 0
  i32.store offset=12
  get_local $0
  i32.load offset=12
  call $~lib/ultrain-ts-lib/internal/action.d/env.current_sender
  get_local $3
  call $~lib/dbmanager/DBManager<Candidate>#get
  drop
  get_local $0
  i32.load offset=16
  i32.const 1952
  call $~lib/ultrain-ts-lib/lib/name/N
  get_local $2
  call $~lib/dbmanager/DBManager<Vote>#get
  drop
  i32.const 136
  i32.const 64
  get_local $2
  i64.load offset=8
  call $~lib/ultrain-ts-lib/src/utils/intToString
  tee_local $3
  i32.load
  i32.sub
  call $~lib/string/String#repeat
  set_local $6
  get_local $3
  get_local $6
  call $~lib/string/String#concat
  set_local $3
  call $~lib/ultrain-ts-lib/internal/action.d/env.current_sender
  call $~lib/ultrain-ts-lib/src/account/Account.publicKeyOf
  set_local $6
  get_global $~lib/ultrain-ts-lib/src/log/Log
  i32.const 3168
  call $~lib/ultrain-ts-lib/src/log/Logger#s
  get_local $3
  call $~lib/ultrain-ts-lib/src/log/Logger#s
  drop
  call $~lib/ultrain-ts-lib/src/log/env.ts_log_done
  i32.const 1
  i32.const 0
  get_local $6
  call $~lib/utf8util/string2cstr
  get_local $1
  call $~lib/utf8util/string2cstr
  get_local $3
  call $~lib/utf8util/string2cstr
  call $~lib/ultrain-ts-lib/internal/crypto.d/env.ts_verify_with_pk
  i32.const 1
  i32.eq
  select
  i32.const 3192
  get_local $6
  call $~lib/string/String.__concat
  i32.const 8
  call $~lib/string/String.__concat
  get_local $1
  call $~lib/string/String.__concat
  i32.const 8
  call $~lib/string/String.__concat
  get_local $3
  call $~lib/string/String.__concat
  call $~lib/env/ultrain_assert
  block (result i32)
   i32.const 4
   call $~lib/allocator/arena/__memory_allocate
   tee_local $3
   i32.const 0
   i32.store
   get_local $3
   i32.const 32
   call $~lib/internal/typedarray/TypedArray<u8_u32>#constructor
   i32.store
   get_local $1
   i32.const 66
   call $~lib/string/String#substring
   tee_local $1
   call $~lib/utf8util/string2cstr
   get_local $1
   i32.load
   get_local $3
   call $~lib/ultrain-ts-lib/src/crypto/Crypto#get:buffer
   get_local $3
   call $~lib/ultrain-ts-lib/src/crypto/Crypto#get:bufferSize
   call $~lib/ultrain-ts-lib/internal/crypto.d/env.ts_sha256
   get_local $3
   call $~lib/ultrain-ts-lib/src/crypto/Crypto#get:buffer
   get_local $3
   call $~lib/ultrain-ts-lib/src/crypto/Crypto#get:bufferSize
   call $~lib/ultrain-ts-lib/src/crypto/to_hex
  end
  i32.const 14
  call $~lib/string/String#substring
  i32.const 16
  call $~lib/internal/string/parse<f64>
  i64.trunc_u/f64
  set_local $5
  get_local $0
  i32.load offset=16
  i32.const 1968
  call $~lib/ultrain-ts-lib/lib/name/N
  get_local $2
  call $~lib/dbmanager/DBManager<Vote>#get
  drop
  get_local $2
  get_local $2
  i64.load offset=8
  get_local $5
  i64.xor
  i64.store offset=8
  get_local $2
  i64.load offset=8
  set_local $4
  get_local $0
  i32.load offset=16
  get_local $0
  i64.load
  get_local $2
  call $~lib/dbmanager/DBManager<Vote>#modify
  get_local $2
  call $~lib/ultrain-ts-lib/internal/action.d/env.current_sender
  i64.store
  get_local $2
  get_local $5
  i64.store offset=8
  get_local $0
  i32.load offset=16
  call $~lib/ultrain-ts-lib/internal/action.d/env.current_sender
  get_local $2
  call $~lib/dbmanager/DBManager<Vote>#emplace
  get_local $0
  i64.load
  call $~lib/ultrain-ts-lib/internal/action.d/env.current_sender
  i64.const 100
  i64.const 357577479428
  call $~lib/ultrain-ts-lib/src/asset/Asset#constructor
  i32.const 3496
  call $~lib/ultrain-ts-lib/src/asset/Asset.transfer
  i32.const 3528
  get_local $4
  call $~lib/ultrain-ts-lib/src/utils/intToString
  call $~lib/string/String.__concat
  i32.const 3552
  call $~lib/string/String.__concat
  call $~lib/ultrain-ts-lib/lib/headblock/env.head_block_number
  i64.extend_u/i32
  call $~lib/ultrain-ts-lib/src/utils/intToString
  call $~lib/string/String.__concat
  call $~lib/ultrain-ts-lib/src/return/Return<String>
 )
 (func $contract/MyContract/rand#query (; 120 ;) (type $iv) (param $0 i32)
  (local $1 i32)
  i32.const 16
  call $~lib/allocator/arena/__memory_allocate
  tee_local $1
  i64.const 0
  i64.store
  get_local $1
  i64.const 0
  i64.store offset=8
  get_local $0
  i32.load offset=16
  i32.const 1752
  call $~lib/ultrain-ts-lib/lib/name/N
  get_local $1
  call $~lib/dbmanager/DBManager<Vote>#get
  drop
  call $~lib/ultrain-ts-lib/lib/headblock/env.head_block_number
  i64.extend_u/i32
  get_local $1
  i64.load offset=8
  i64.sub
  i64.const 3
  i64.ge_u
  if (result i32)
   get_local $0
   i32.load offset=16
   i32.const 1968
   call $~lib/ultrain-ts-lib/lib/name/N
   get_local $1
   call $~lib/dbmanager/DBManager<Vote>#get
  else   
   get_local $0
   i32.load offset=16
   i32.const 1952
   call $~lib/ultrain-ts-lib/lib/name/N
   get_local $1
   call $~lib/dbmanager/DBManager<Vote>#get
  end
  drop
  i32.const 3528
  get_local $1
  i64.load offset=8
  call $~lib/ultrain-ts-lib/src/utils/intToString
  call $~lib/string/String.__concat
  i32.const 3552
  call $~lib/string/String.__concat
  call $~lib/ultrain-ts-lib/lib/headblock/env.head_block_number
  i64.extend_u/i32
  call $~lib/ultrain-ts-lib/src/utils/intToString
  call $~lib/string/String.__concat
  call $~lib/ultrain-ts-lib/src/return/Return<String>
 )
 (func $contract/MyContract/apply (; 121 ;) (type $IIIIv) (param $0 i64) (param $1 i64) (param $2 i64) (param $3 i64)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  get_local $0
  call $contract/MyContract/rand#constructor
  tee_local $4
  get_local $2
  get_local $3
  call $~lib/ultrain-ts-lib/src/asset/Asset#constructor
  i32.store offset=8
  get_local $4
  i64.load
  get_local $1
  get_local $4
  i32.load offset=8
  call $~lib/ultrain-ts-lib/src/contract/Contract.filterAcceptTransferTokenAction
  if
   call $~lib/ultrain-ts-lib/internal/action.d/env.action_data_size
   tee_local $5
   call $~lib/internal/typedarray/TypedArray<u8_u32>#constructor
   tee_local $6
   i32.load
   get_local $5
   call $~lib/ultrain-ts-lib/internal/action.d/env.read_action_data
   drop
   get_local $6
   i32.load
   get_local $5
   call $~lib/datastream/DataStream#constructor
   set_local $5
   get_local $4
   i32.const 2040
   call $~lib/ultrain-ts-lib/src/contract/Contract#isAction
   if
    call $~lib/ultrain-ts-lib/internal/action.d/env.current_sender
    get_local $4
    i64.load
    i64.eq
    i32.const 2064
    call $~lib/env/ultrain_assert
    get_local $4
    i32.load offset=12
    call $~lib/dbmanager/DBManager<Candidate>#dropAll
    drop
    get_local $4
    i32.load offset=16
    call $~lib/dbmanager/DBManager<Candidate>#dropAll
    drop
   end
   get_local $4
   i32.const 1984
   call $~lib/ultrain-ts-lib/src/contract/Contract#isAction
   if
    get_local $5
    call $~lib/datastream/DataStream#read<u64>
    set_local $0
    get_local $5
    call $~lib/datastream/DataStream#read<u64>
    drop
    i64.const 0
    i64.const 357577479428
    call $~lib/ultrain-ts-lib/src/asset/Asset#constructor
    tee_local $6
    get_local $5
    call $~lib/ultrain-ts-lib/src/asset/Asset#deserialize
    get_local $4
    get_local $0
    get_local $6
    get_local $5
    call $~lib/datastream/DataStream#readString
    call $contract/MyContract/rand#transfer
   end
   get_local $4
   i32.const 2704
   call $~lib/ultrain-ts-lib/src/contract/Contract#isAction
   if
    get_local $4
    call $contract/MyContract/rand#removeCandidate
   end
   get_local $4
   i32.const 1720
   call $~lib/ultrain-ts-lib/src/contract/Contract#isAction
   if
    get_local $4
    get_local $5
    call $~lib/datastream/DataStream#readString
    call $contract/MyContract/rand#vote
   end
   get_local $4
   i32.const 3592
   call $~lib/ultrain-ts-lib/src/contract/Contract#isAction
   if
    get_local $4
    call $contract/MyContract/rand#query
   end
   get_local $4
   i32.const 3608
   call $~lib/ultrain-ts-lib/src/contract/Contract#isAction
   drop
  end
 )
 (func $start (; 122 ;) (type $v)
  i32.const 3632
  set_global $~lib/allocator/arena/startOffset
  get_global $~lib/allocator/arena/startOffset
  set_global $~lib/allocator/arena/offset
  i32.const 0
  call $~lib/allocator/arena/__memory_allocate
  set_global $~lib/ultrain-ts-lib/src/log/Log
  call $~lib/ultrain-ts-lib/src/asset/StringToSymbol
  set_global $~lib/ultrain-ts-lib/src/asset/SYS
  get_global $~lib/ultrain-ts-lib/src/asset/SYS
  i64.const 8
  i64.shr_u
  set_global $~lib/ultrain-ts-lib/src/asset/SYS_NAME
  i64.const 20000
  i64.const 357577479428
  call $~lib/ultrain-ts-lib/src/asset/Asset#constructor
  set_global $contract/MyContract/DEPOSIT_AMOUNT
 )
 (func $null (; 123 ;) (type $v)
  nop
 )
)
