(module
 (type $ii (func (param i32) (result i32)))
 (type $iiI (func (param i32 i32) (result i64)))
 (type $iiv (func (param i32 i32)))
 (type $iii (func (param i32 i32) (result i32)))
 (type $v (func))
 (type $iiiv (func (param i32 i32 i32)))
 (type $iv (func (param i32)))
 (type $iIii (func (param i32 i64 i32) (result i32)))
 (type $Iiv (func (param i64 i32)))
 (type $iI (func (param i32) (result i64)))
 (type $iIIi (func (param i32 i64 i64) (result i32)))
 (type $IIIIv (func (param i64 i64 i64 i64)))
 (type $iIi (func (param i32 i64) (result i32)))
 (type $iIv (func (param i32 i64)))
 (type $iIIIi (func (param i32 i64 i64 i64) (result i32)))
 (type $IIIIi (func (param i64 i64 i64 i64) (result i32)))
 (type $I (func (result i64)))
 (type $iIiv (func (param i32 i64 i32)))
 (type $iiii (func (param i32 i32 i32) (result i32)))
 (type $IIIIiii (func (param i64 i64 i64 i64 i32 i32) (result i32)))
 (type $i (func (result i32)))
 (type $iII (func (param i32 i64) (result i64)))
 (type $Iii (func (param i64 i32) (result i32)))
 (type $iiiiiv (func (param i32 i32 i32 i32 i32)))
 (type $iiiiv (func (param i32 i32 i32 i32)))
 (type $iiF (func (param i32 i32) (result f64)))
 (type $iIIv (func (param i32 i64 i64)))
 (type $IIii (func (param i64 i64 i32) (result i32)))
 (type $iIIiiv (func (param i32 i64 i64 i32 i32)))
 (type $Ii (func (param i64) (result i32)))
 (type $iIiiv (func (param i32 i64 i32 i32)))
 (type $iiiiii (func (param i32 i32 i32 i32 i32) (result i32)))
 (type $iiIv (func (param i32 i32 i64)))
 (type $IIiiv (func (param i64 i64 i32 i32)))
 (type $iIIiii (func (param i32 i64 i64 i32 i32) (result i32)))
 (type $iIiii (func (param i32 i64 i32 i32) (result i32)))
 (type $Iiii (func (param i64 i32 i32) (result i32)))
 (type $iIIiv (func (param i32 i64 i64 i32)))
 (type $iIiIv (func (param i32 i64 i32 i64)))
 (type $iiII (func (param i32 i32 i64) (result i64)))
 (type $Iiiii (func (param i64 i32 i32 i32) (result i32)))
 (type $iIIIv (func (param i32 i64 i64 i64)))
 (import "env" "abort" (func $~lib/env/abort))
 (import "env" "ultrainio_assert" (func $~lib/env/ultrainio_assert (param i32 i32)))
 (import "env" "ts_log_print_s" (func $~lib/ultrain-ts-lib/src/log/env.ts_log_print_s (param i32)))
 (import "env" "ts_log_print_i" (func $~lib/ultrain-ts-lib/src/log/env.ts_log_print_i (param i64 i32)))
 (import "env" "ts_log_done" (func $~lib/ultrain-ts-lib/src/log/env.ts_log_done))
 (import "env" "db_find_i64" (func $~lib/env/db_find_i64 (param i64 i64 i64 i64) (result i32)))
 (import "env" "current_sender" (func $~lib/ultrain-ts-lib/internal/action.d/env.current_sender (result i64)))
 (import "env" "current_receiver" (func $~lib/env/current_receiver (result i64)))
 (import "env" "db_store_i64" (func $~lib/env/db_store_i64 (param i64 i64 i64 i64 i32 i32) (result i32)))
 (import "env" "head_block_number" (func $~lib/ultrain-ts-lib/lib/headblock/env.head_block_number (result i32)))
 (import "env" "ts_sha256" (func $~lib/ultrain-ts-lib/internal/crypto.d/env.ts_sha256 (param i32 i32 i32 i32)))
 (import "env" "action_data_size" (func $~lib/ultrain-ts-lib/internal/action.d/env.action_data_size (result i32)))
 (import "env" "read_action_data" (func $~lib/ultrain-ts-lib/internal/action.d/env.read_action_data (param i32 i32) (result i32)))
 (import "env" "db_get_i64" (func $~lib/env/db_get_i64 (param i32 i32 i32) (result i32)))
 (import "env" "db_update_i64" (func $~lib/env/db_update_i64 (param i32 i64 i32 i32)))
 (import "env" "send_inline" (func $~lib/ultrain-ts-lib/internal/action.d/env.send_inline (param i32 i32)))
 (import "env" "db_remove_i64" (func $~lib/env/db_remove_i64 (param i32)))
 (import "env" "ts_public_key_of_account" (func $~lib/ultrain-ts-lib/internal/crypto.d/env.ts_public_key_of_account (param i64 i32 i32 i32) (result i32)))
 (import "env" "ts_verify_with_pk" (func $~lib/ultrain-ts-lib/internal/crypto.d/env.ts_verify_with_pk (param i32 i32 i32) (result i32)))
 (import "env" "set_result_str" (func $~lib/ultrain-ts-lib/src/return/env.set_result_str (param i32)))
 (memory $0 1)
 (data (i32.const 8) "\01\00\00\00 \00")
 (data (i32.const 16) "\01\00\00\00!\00")
 (data (i32.const 24) "\01\00\00\00\"\00")
 (data (i32.const 32) "\01\00\00\00#\00")
 (data (i32.const 40) "\01\00\00\00$\00")
 (data (i32.const 48) "\01\00\00\00%\00")
 (data (i32.const 56) "\01\00\00\00&\00")
 (data (i32.const 64) "\01\00\00\00\'\00")
 (data (i32.const 72) "\01\00\00\00(\00")
 (data (i32.const 80) "\01\00\00\00)\00")
 (data (i32.const 88) "\01\00\00\00*\00")
 (data (i32.const 96) "\01\00\00\00+\00")
 (data (i32.const 104) "\01\00\00\00,\00")
 (data (i32.const 112) "\01\00\00\00-\00")
 (data (i32.const 120) "\01\00\00\00.\00")
 (data (i32.const 128) "\01\00\00\00/\00")
 (data (i32.const 136) "\01\00\00\000\00")
 (data (i32.const 144) "\01\00\00\001\00")
 (data (i32.const 152) "\01\00\00\002\00")
 (data (i32.const 160) "\01\00\00\003\00")
 (data (i32.const 168) "\01\00\00\004\00")
 (data (i32.const 176) "\01\00\00\005\00")
 (data (i32.const 184) "\01\00\00\006\00")
 (data (i32.const 192) "\01\00\00\007\00")
 (data (i32.const 200) "\01\00\00\008\00")
 (data (i32.const 208) "\01\00\00\009\00")
 (data (i32.const 216) "\01\00\00\00:\00")
 (data (i32.const 224) "\01\00\00\00;\00")
 (data (i32.const 232) "\01\00\00\00<\00")
 (data (i32.const 240) "\01\00\00\00=\00")
 (data (i32.const 248) "\01\00\00\00>\00")
 (data (i32.const 256) "\01\00\00\00?\00")
 (data (i32.const 264) "\01\00\00\00@\00")
 (data (i32.const 272) "\01\00\00\00A\00")
 (data (i32.const 280) "\01\00\00\00B\00")
 (data (i32.const 288) "\01\00\00\00C\00")
 (data (i32.const 296) "\01\00\00\00D\00")
 (data (i32.const 304) "\01\00\00\00E\00")
 (data (i32.const 312) "\01\00\00\00F\00")
 (data (i32.const 320) "\01\00\00\00G\00")
 (data (i32.const 328) "\01\00\00\00H\00")
 (data (i32.const 336) "\01\00\00\00I\00")
 (data (i32.const 344) "\01\00\00\00J\00")
 (data (i32.const 352) "\01\00\00\00K\00")
 (data (i32.const 360) "\01\00\00\00L\00")
 (data (i32.const 368) "\01\00\00\00M\00")
 (data (i32.const 376) "\01\00\00\00N\00")
 (data (i32.const 384) "\01\00\00\00O\00")
 (data (i32.const 392) "\01\00\00\00P\00")
 (data (i32.const 400) "\01\00\00\00Q\00")
 (data (i32.const 408) "\01\00\00\00R\00")
 (data (i32.const 416) "\01\00\00\00T\00")
 (data (i32.const 424) "\01\00\00\00U\00")
 (data (i32.const 432) "\01\00\00\00V\00")
 (data (i32.const 440) "\01\00\00\00W\00")
 (data (i32.const 448) "\01\00\00\00X\00")
 (data (i32.const 456) "\01\00\00\00Y\00")
 (data (i32.const 464) "\01\00\00\00Z\00")
 (data (i32.const 472) "\01\00\00\00[\00")
 (data (i32.const 480) "\01\00\00\00\\\00")
 (data (i32.const 488) "\01\00\00\00]\00")
 (data (i32.const 496) "\01\00\00\00^\00")
 (data (i32.const 504) "\01\00\00\00_\00")
 (data (i32.const 512) "\01\00\00\00`\00")
 (data (i32.const 520) "\01\00\00\00a\00")
 (data (i32.const 528) "\01\00\00\00b\00")
 (data (i32.const 536) "\01\00\00\00c\00")
 (data (i32.const 544) "\01\00\00\00d\00")
 (data (i32.const 552) "\01\00\00\00e\00")
 (data (i32.const 560) "\01\00\00\00f\00")
 (data (i32.const 568) "\01\00\00\00g\00")
 (data (i32.const 576) "\01\00\00\00h\00")
 (data (i32.const 584) "\01\00\00\00i\00")
 (data (i32.const 592) "\01\00\00\00j\00")
 (data (i32.const 600) "\01\00\00\00k\00")
 (data (i32.const 608) "\01\00\00\00l\00")
 (data (i32.const 616) "\01\00\00\00m\00")
 (data (i32.const 624) "\01\00\00\00n\00")
 (data (i32.const 632) "\01\00\00\00o\00")
 (data (i32.const 640) "\01\00\00\00p\00")
 (data (i32.const 648) "\01\00\00\00q\00")
 (data (i32.const 656) "\01\00\00\00r\00")
 (data (i32.const 664) "\01\00\00\00s\00")
 (data (i32.const 672) "\01\00\00\00t\00")
 (data (i32.const 680) "\01\00\00\00u\00")
 (data (i32.const 688) "\01\00\00\00v\00")
 (data (i32.const 696) "\01\00\00\00w\00")
 (data (i32.const 704) "\01\00\00\00x\00")
 (data (i32.const 712) "\01\00\00\00y\00")
 (data (i32.const 720) "\01\00\00\00z\00")
 (data (i32.const 728) "\01\00\00\00{\00")
 (data (i32.const 736) "\01\00\00\00|\00")
 (data (i32.const 744) "\01\00\00\00}\00")
 (data (i32.const 752) "\01\00\00\00~\00")
 (data (i32.const 760) "|\01\00\00\00\00\00\00\08\00\00\00\10\00\00\00\18\00\00\00 \00\00\00(\00\00\000\00\00\008\00\00\00@\00\00\00H\00\00\00P\00\00\00X\00\00\00`\00\00\00h\00\00\00p\00\00\00x\00\00\00\80\00\00\00\88\00\00\00\90\00\00\00\98\00\00\00\a0\00\00\00\a8\00\00\00\b0\00\00\00\b8\00\00\00\c0\00\00\00\c8\00\00\00\d0\00\00\00\d8\00\00\00\e0\00\00\00\e8\00\00\00\f0\00\00\00\f8\00\00\00\00\01\00\00\08\01\00\00\10\01\00\00\18\01\00\00 \01\00\00(\01\00\000\01\00\008\01\00\00@\01\00\00H\01\00\00P\01\00\00X\01\00\00`\01\00\00h\01\00\00p\01\00\00x\01\00\00\80\01\00\00\88\01\00\00\90\01\00\00\98\01\00\00\98\01\00\00\a0\01\00\00\a8\01\00\00\b0\01\00\00\b8\01\00\00\c0\01\00\00\c8\01\00\00\d0\01\00\00\d8\01\00\00\e0\01\00\00\e8\01\00\00\f0\01\00\00\f8\01\00\00\00\02\00\00\08\02\00\00\10\02\00\00\18\02\00\00 \02\00\00(\02\00\000\02\00\008\02\00\00@\02\00\00H\02\00\00P\02\00\00X\02\00\00`\02\00\00h\02\00\00p\02\00\00x\02\00\00\80\02\00\00\88\02\00\00\90\02\00\00\98\02\00\00\a0\02\00\00\a8\02\00\00\b0\02\00\00\b8\02\00\00\c0\02\00\00\c8\02\00\00\d0\02\00\00\d8\02\00\00\e0\02\00\00\e8\02\00\00\f0\02\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00")
 (data (i32.const 1272) "\f8\02\00\00_\00\00\00")
 (data (i32.const 1280) "\04\00\00\00U\00G\00A\00S\00")
 (data (i32.const 1296) "+\00\00\00l\00e\00n\00g\00t\00h\00 \00o\00f\00 \00_\00s\00y\00m\00b\00o\00l\00 \00n\00a\00m\00e\00 \00m\00u\00s\00t\00 \00b\00e\00 \00l\00e\00s\00s\00 \00t\00h\00a\00n\00 \007\00.\00")
 (data (i32.const 1392) "\0d\00\00\00~\00l\00i\00b\00/\00a\00r\00r\00a\00y\00.\00t\00s\00")
 (data (i32.const 1424) "\1c\00\00\00~\00l\00i\00b\00/\00i\00n\00t\00e\00r\00n\00a\00l\00/\00a\00r\00r\00a\00y\00b\00u\00f\00f\00e\00r\00.\00t\00s\00")
 (data (i32.const 1488) "\0e\00\00\00~\00l\00i\00b\00/\00s\00t\00r\00i\00n\00g\00.\00t\00s\00")
 (data (i32.const 1520) "0\00\00\00s\00t\00r\00i\00n\00g\00_\00t\00o\00_\00_\00s\00y\00m\00b\00o\00l\00 \00f\00a\00i\00l\00e\00d\00 \00f\00o\00r\00 \00n\00o\00t\00 \00s\00u\00p\00o\00o\00r\00t\00 \00c\00o\00d\00e\00 \00:\00 \00")
 (data (i32.const 1624) "\10\00\00\000\001\002\003\004\005\006\007\008\009\00a\00b\00c\00d\00e\00f\00")
 (data (i32.const 1664) "\05\00\00\00v\00o\00t\00e\00r\00")
 (data (i32.const 1680) "\06\00\00\00w\00a\00i\00t\00e\00r\00")
 (data (i32.const 1696) "\04\00\00\00r\00a\00n\00d\00")
 (data (i32.const 1712) "\n\00\00\00u\00t\00r\00i\00o\00.\00r\00a\00n\00d\00")
 (data (i32.const 1736) "\07\00\00\00m\00a\00i\00n\00n\00u\00m\00")
 (data (i32.const 1760) "3\00\00\00c\00a\00n\00 \00n\00o\00t\00 \00c\00r\00e\00a\00t\00e\00 \00o\00b\00j\00e\00c\00t\00s\00 \00i\00n\00 \00t\00a\00b\00l\00e\00 \00o\00f\00 \00a\00n\00o\00t\00h\00e\00r\00 \00c\00o\00n\00t\00r\00a\00c\00t\00")
 (data (i32.const 1872) "\1b\00\00\00~\00l\00i\00b\00/\00i\00n\00t\00e\00r\00n\00a\00l\00/\00t\00y\00p\00e\00d\00a\00r\00r\00a\00y\00.\00t\00s\00")
 (data (i32.const 1936) "\04\00\00\00n\00u\00l\00l\00")
 (data (i32.const 1952) "\00\00\00\00")
 (data (i32.const 1960) "\17\00\00\00~\00l\00i\00b\00/\00i\00n\00t\00e\00r\00n\00a\00l\00/\00s\00t\00r\00i\00n\00g\00.\00t\00s\00")
 (data (i32.const 2016) "\08\00\00\00t\00r\00a\00n\00s\00f\00e\00r\00")
 (data (i32.const 2040) "\0b\00\00\00u\00t\00r\00i\00o\00.\00t\00o\00k\00e\00n\00")
 (data (i32.const 2072) "\0c\00\00\00A\00 \00c\00a\00n\00d\00i\00d\00a\00t\00e\00 \00")
 (data (i32.const 2104) " \00\00\00.\001\002\003\004\005\00a\00b\00c\00d\00e\00f\00g\00h\00i\00j\00k\00l\00m\00n\00o\00p\00q\00r\00s\00t\00u\00v\00w\00x\00y\00z\00")
 (data (i32.const 2176) "\0d\00\00\00\00\00\00\00.............\00\00\00\00\00\00\00\00\00\00\00")
 (data (i32.const 2208) "\80\08\00\00\0d\00\00\00")
 (data (i32.const 2216) "\0e\00\00\00 \00r\00e\00g\00i\00s\00t\00e\00r\00 \00t\00o\00:\00 \00")
 (data (i32.const 2248) "\0c\00\00\00a\00s\00 \00c\00a\00n\00d\00i\00d\00a\00t\00e\00")
 (data (i32.const 2280) "\1e\00\00\00c\00a\00n\00n\00o\00t\00 \00a\00d\00d\00 \00e\00x\00i\00s\00t\00i\00n\00g\00 \00c\00a\00n\00d\00i\00d\00a\00t\00e\00.\00")
 (data (i32.const 2344) "&\00\00\00T\00h\00e\00 \00t\00o\00t\00a\00l\00 \00o\00f\00 \00m\00a\00i\00n\00 \00s\00h\00o\00u\00l\00d\00 \00b\00e\00 \00l\00e\00s\00s\00 \00t\00h\00a\00n\00 \00")
 (data (i32.const 2424) "7\00\00\00o\00b\00j\00e\00c\00t\00 \00p\00a\00s\00s\00e\00d\00 \00t\00o\00 \00m\00o\00d\00i\00f\00y\00 \00i\00s\00 \00n\00o\00t\00 \00f\00o\00u\00n\00d\00 \00i\00n\00 \00t\00h\00i\00s\00 \00D\00B\00M\00a\00n\00a\00g\00e\00r\00.\00")
 (data (i32.const 2544) "4\00\00\00c\00a\00n\00 \00n\00o\00t\00 \00m\00o\00d\00i\00f\00y\00 \00o\00b\00j\00e\00c\00t\00s\00 \00i\00n\00 \00t\00a\00b\00l\00e\00 \00o\00f\00 \00a\00n\00o\00t\00h\00e\00r\00 \00c\00o\00n\00t\00r\00a\00c\00t\00.\00")
 (data (i32.const 2656) ")\00\00\00T\00h\00e\00 \00t\00o\00t\00a\00l\00 \00o\00f\00 \00w\00a\00i\00t\00e\00r\00s\00 \00s\00h\00o\00u\00l\00d\00 \00b\00e\00 \00l\00e\00s\00s\00 \00t\00h\00a\00n\00 \00")
 (data (i32.const 2744) "\06\00\00\00U\00s\00e\00r\00:\00 \00")
 (data (i32.const 2760) "\1a\00\00\00 \00d\00e\00p\00o\00s\00i\00t\00e\00d\00 \00m\00o\00n\00e\00y\00 \00a\00c\00c\00u\00r\00a\00t\00e\00.\00")
 (data (i32.const 2816) "I\00\00\00d\00e\00p\00o\00s\00i\00t\00 \00m\00o\00n\00e\00y\00 \00i\00s\00 \00n\00o\00t\00 \00a\00c\00c\00u\00r\00a\00t\00e\00,\00 \00r\00e\00q\00u\00i\00r\00e\00 \00d\00e\00p\00o\00s\00i\00t\00 \002\00.\000\000\000\000\00 \00U\00G\00A\00S\00 \00o\00r\00 \000\00.\002\000\000\000\00 \00U\00G\00A\00S\00")
 (data (i32.const 2968) "\n\00\00\00u\00n\00r\00e\00g\00i\00s\00t\00e\00r\00")
 (data (i32.const 2992) "%\00\00\00c\00a\00n\00n\00o\00t\00 \00r\00e\00m\00o\00v\00e\00 \00n\00o\00n\00-\00e\00x\00i\00s\00t\00i\00n\00g\00 \00c\00a\00n\00d\00i\00d\00a\00t\00e\00.\00")
 (data (i32.const 3072) "\16\00\00\00r\00e\00t\00u\00r\00n\00 \00d\00e\00p\00o\00s\00i\00t\00e\00d\00 \00m\00o\00n\00e\00y\00")
 (data (i32.const 3120) "\06\00\00\00a\00c\00t\00i\00v\00e\00")
 (data (i32.const 3136) "\00\00\00\00\00\00\00\00")
 (data (i32.const 3144) "@\0c\00\00\00\00\00\00")
 (data (i32.const 3152) "\00\00\00\00\00\00\00\00")
 (data (i32.const 3160) "P\0c\00\00\00\00\00\00")
 (data (i32.const 3168) "\06\00\00\00r\00e\00d\00e\00e\00m\00")
 (data (i32.const 3184) "3\00\00\00c\00a\00n\00 \00n\00o\00t\00 \00e\00r\00a\00s\00e\00 \00o\00b\00j\00e\00c\00t\00s\00 \00i\00n\00 \00t\00a\00b\00l\00e\00 \00o\00f\00 \00a\00n\00o\00t\00h\00e\00r\00 \00c\00o\00n\00t\00r\00a\00c\00t\00.\00")
 (data (i32.const 3296) "<\00\00\00c\00a\00n\00n\00o\00t\00 \00r\00e\00d\00e\00e\00m\00 \00d\00e\00p\00o\00s\00i\00t\00,\00 \00y\00o\00u\00 \00m\00u\00s\00t\00 \00w\00a\00i\00t\00 \00f\00o\00r\00 \001\000\00 \00b\00l\00o\00c\00k\00 \00g\00e\00n\00e\00r\00a\00t\00e\00d\00.\00")
 (data (i32.const 3424) "\04\00\00\00v\00o\00t\00e\00")
 (data (i32.const 3440) " \00\00\00Y\00o\00u\00 \00s\00h\00o\00u\00l\00d\00 \00b\00e\00 \00a\00 \00c\00a\00n\00d\00i\00d\00a\00t\00e\00 \00f\00i\00r\00s\00t\00.\00")
 (data (i32.const 3512) "\1c\00\00\00T\00h\00e\00 \00c\00u\00r\00r\00e\00n\00t\00 \00b\00l\00o\00c\00k\00 \00n\00u\00m\00b\00e\00r\00 \00i\00s\00 \00")
 (data (i32.const 3576) "\1e\00\00\00.\00 \00T\00h\00e\00 \00v\00o\00t\00e\00 \00s\00e\00e\00d\00 \00b\00l\00o\00c\00k\00 \00n\00u\00m\00b\00e\00r\00:\00 \00")
 (data (i32.const 3640) "\0d\00\00\00 \00h\00a\00s\00 \00e\00x\00p\00i\00r\00e\00d\00.\00")
 (data (i32.const 3672) "-\00\00\00T\00h\00e\00 \00r\00a\00n\00d\00o\00m\00 \00o\00f\00 \00b\00l\00o\00c\00k\00 \00h\00e\00i\00g\00h\00t\00 \00h\00a\00s\00 \00n\00o\00t\00 \00g\00e\00n\00e\00r\00a\00t\00e\00d\00.\00")
 (data (i32.const 3768) "\03\00\00\00h\00e\00x\00")
 (data (i32.const 3784) "!\00\00\00p\00l\00e\00a\00s\00e\00 \00p\00r\00o\00v\00i\00d\00e\00 \00a\00 \00v\00a\00l\00i\00d\00 \00V\00R\00F\00 \00p\00r\00o\00o\00f\00.\00")
 (data (i32.const 3856) "#\00\00\00T\00h\00e\00 \00i\00n\00d\00e\00x\00 \00s\00h\00o\00u\00l\00d\00 \00b\00e\00 \00b\00e\00t\00w\00e\00e\00n\00 \000\00 \00a\00n\00d\00 \002\00")
 (data (i32.const 3936) "G\00\00\00Y\00o\00u\00 \00h\00a\00d\00 \00a\00l\00r\00e\00a\00d\00y\00 \00s\00u\00b\00m\00i\00t\00t\00e\00d\00 \00t\00h\00e\00 \00v\00o\00t\00e\00,\00 \00y\00o\00u\00 \00s\00h\00o\00u\00l\00d\00 \00w\00a\00i\00t\00 \00f\00o\00r\00 \00t\00h\00e\00 \00n\00e\00x\00t\00 \00r\00o\00u\00n\00d\00.\00")
 (data (i32.const 4088) "\16\00\00\00Y\00o\00u\00r\00 \00v\00o\00t\00e\00 \00w\00a\00s\00 \00e\00x\00p\00i\00r\00e\00d\00.\00")
 (data (i32.const 4136) "\0b\00\00\00b\00o\00n\00u\00s\00 \00m\00o\00n\00e\00y\00")
 (data (i32.const 4168) "C\00\00\00T\00h\00e\00 \00w\00a\00i\00t\00e\00r\00 \00s\00h\00o\00u\00l\00d\00 \00v\00o\00t\00e\00 \00t\00h\00e\00 \00n\00e\00x\00t\00 \00t\00w\00o\00 \00b\00l\00o\00c\00k\00 \00p\00e\00r\00i\00o\00d\00 \00a\00f\00t\00e\00r\00 \00t\00h\00e\00 \00b\00l\00o\00c\00k\00N\00u\00m\00")
 (data (i32.const 4312) "\05\00\00\00q\00u\00e\00r\00y\00")
 (data (i32.const 4328) "\08\00\00\00q\00u\00e\00r\00y\00B\00c\00k\00")
 (data (i32.const 4352) "9\00\00\00C\00u\00r\00r\00e\00n\00t\00l\00y\00 \00t\00h\00e\00 \00b\00l\00o\00c\00k\00 \00n\00u\00m\00b\00e\00r\00 \00o\00f\00 \00t\00h\00e\00 \00r\00a\00n\00d\00 \00s\00h\00o\00u\00l\00d\00 \00b\00e\00 \00b\00e\00t\00w\00e\00e\00n\00 \00")
 (data (i32.const 4472) "\05\00\00\00 \00a\00n\00d\00 \00")
 (data (i32.const 4488) "\07\00\00\00o\00n\00e\00r\00r\00o\00r\00")
 (table $0 2 anyfunc)
 (elem (i32.const 0) $null $contract/ultrainio.rand/RandContract#unregister~anonymous|1)
 (global $~lib/internal/allocator/AL_BITS i32 (i32.const 3))
 (global $~lib/internal/allocator/AL_SIZE i32 (i32.const 8))
 (global $~lib/internal/allocator/AL_MASK i32 (i32.const 7))
 (global $~lib/internal/allocator/MAX_SIZE_32 i32 (i32.const 1073741824))
 (global $~lib/allocator/arena/startOffset (mut i32) (i32.const 0))
 (global $~lib/allocator/arena/offset (mut i32) (i32.const 0))
 (global $~lib/ultrain-ts-lib/src/utils/PrintableChar i32 (i32.const 1272))
 (global $~lib/ultrain-ts-lib/src/log/Log (mut i32) (i32.const 0))
 (global $~lib/internal/arraybuffer/HEADER_SIZE i32 (i32.const 8))
 (global $~lib/internal/arraybuffer/MAX_BLENGTH i32 (i32.const 1073741816))
 (global $~lib/internal/string/HEADER_SIZE i32 (i32.const 4))
 (global $~lib/internal/string/MAX_LENGTH i32 (i32.const 536870910))
 (global $~lib/internal/number/MAX_DOUBLE_LENGTH i32 (i32.const 28))
 (global $~lib/internal/number/_K (mut i32) (i32.const 0))
 (global $~lib/internal/number/_frc (mut i64) (i64.const 0))
 (global $~lib/internal/number/_exp (mut i32) (i32.const 0))
 (global $~lib/internal/number/_frc_minus (mut i64) (i64.const 0))
 (global $~lib/internal/number/_frc_plus (mut i64) (i64.const 0))
 (global $~lib/internal/number/_frc_pow (mut i64) (i64.const 0))
 (global $~lib/internal/number/_exp_pow (mut i32) (i32.const 0))
 (global $~lib/ultrain-ts-lib/src/asset/CHAR_A i32 (i32.const 65))
 (global $~lib/ultrain-ts-lib/src/asset/CHAR_Z i32 (i32.const 90))
 (global $~lib/ultrain-ts-lib/src/asset/SYS (mut i64) (i64.const 0))
 (global $~lib/ultrain-ts-lib/src/asset/SYS_NAME (mut i64) (i64.const 0))
 (global $~lib/ultrain-ts-lib/src/asset/MAX_AMOUNT i64 (i64.const 4611686018427387903))
 (global $~lib/ultrain-ts-lib/src/crypto/HexDigital i32 (i32.const 1624))
 (global $~lib/ultrain-ts-lib/src/crypto/CHAR0 i32 (i32.const 48))
 (global $~lib/ultrain-ts-lib/src/crypto/CHAR9 i32 (i32.const 57))
 (global $~lib/ultrain-ts-lib/src/crypto/CHARa i32 (i32.const 97))
 (global $~lib/ultrain-ts-lib/src/crypto/CHARf i32 (i32.const 102))
 (global $~lib/ultrain-ts-lib/src/crypto/CHARA i32 (i32.const 65))
 (global $~lib/ultrain-ts-lib/src/crypto/CHARF i32 (i32.const 70))
 (global $contract/lib/random.lib/VOTER_TABLE i32 (i32.const 1664))
 (global $contract/lib/random.lib/WAITER_TABLE i32 (i32.const 1680))
 (global $contract/lib/random.lib/RAND_TABLE i32 (i32.const 1696))
 (global $contract/lib/random.lib/CONT_NAME i32 (i32.const 1712))
 (global $contract/lib/random.lib/EPOCH i64 (i64.const 3))
 (global $contract/lib/random.lib/RAND_KEY (mut i64) (i64.const 0))
 (global $contract/lib/random.lib/CACHED_RAND_COUNT i64 (i64.const 999))
 (global $contract/ultrainio.rand/MAIN_COUNT_KEY (mut i64) (i64.const 0))
 (global $contract/ultrainio.rand/MAIN_DEPOSIT_AMOUNT (mut i32) (i32.const 0))
 (global $contract/ultrainio.rand/WAITER_DEPOSIT_AMOUNT (mut i32) (i32.const 0))
 (global $contract/ultrainio.rand/MAIN_BONUS i64 (i64.const 100))
 (global $contract/ultrainio.rand/WAITER_BONUS i64 (i64.const 8))
 (global $contract/ultrainio.rand/WAITER_NUM i64 (i64.const 100))
 (global $contract/ultrainio.rand/MAIN_NUM i64 (i64.const 20))
 (global $ASC_SHRINK_LEVEL i32 (i32.const 0))
 (global $NaN f64 (f64.const nan:0x8000000000000))
 (global $~lib/internal/string/CharCode.PLUS i32 (i32.const 43))
 (global $~lib/internal/string/CharCode.MINUS i32 (i32.const 45))
 (global $~lib/internal/string/CharCode.DOT i32 (i32.const 46))
 (global $~lib/internal/string/CharCode._0 i32 (i32.const 48))
 (global $~lib/internal/string/CharCode._1 i32 (i32.const 49))
 (global $~lib/internal/string/CharCode._2 i32 (i32.const 50))
 (global $~lib/internal/string/CharCode._3 i32 (i32.const 51))
 (global $~lib/internal/string/CharCode._4 i32 (i32.const 52))
 (global $~lib/internal/string/CharCode._5 i32 (i32.const 53))
 (global $~lib/internal/string/CharCode._6 i32 (i32.const 54))
 (global $~lib/internal/string/CharCode._7 i32 (i32.const 55))
 (global $~lib/internal/string/CharCode._8 i32 (i32.const 56))
 (global $~lib/internal/string/CharCode._9 i32 (i32.const 57))
 (global $~lib/internal/string/CharCode.A i32 (i32.const 65))
 (global $~lib/internal/string/CharCode.B i32 (i32.const 66))
 (global $~lib/internal/string/CharCode.E i32 (i32.const 69))
 (global $~lib/internal/string/CharCode.N i32 (i32.const 78))
 (global $~lib/internal/string/CharCode.O i32 (i32.const 79))
 (global $~lib/internal/string/CharCode.X i32 (i32.const 88))
 (global $~lib/internal/string/CharCode.Z i32 (i32.const 90))
 (global $~lib/internal/string/CharCode.a i32 (i32.const 97))
 (global $~lib/internal/string/CharCode.b i32 (i32.const 98))
 (global $~lib/internal/string/CharCode.e i32 (i32.const 101))
 (global $~lib/internal/string/CharCode.n i32 (i32.const 110))
 (global $~lib/internal/string/CharCode.o i32 (i32.const 111))
 (global $~lib/internal/string/CharCode.x i32 (i32.const 120))
 (global $~lib/internal/string/CharCode.z i32 (i32.const 122))
 (global $~argc (mut i32) (i32.const 0))
 (global $HEAP_BASE i32 (i32.const 4508))
 (export "memory" (memory $0))
 (export "table" (table $0))
 (export "apply" (func $contract/ultrainio.rand/apply))
 (start $start)
 (func $~lib/allocator/arena/__memory_allocate (; 20 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  get_local $0
  get_global $~lib/internal/allocator/MAX_SIZE_32
  i32.gt_u
  if
   unreachable
  end
  get_global $~lib/allocator/arena/offset
  set_local $1
  get_local $1
  get_local $0
  tee_local $2
  i32.const 1
  tee_local $3
  get_local $2
  get_local $3
  i32.gt_u
  select
  i32.add
  get_global $~lib/internal/allocator/AL_MASK
  i32.add
  get_global $~lib/internal/allocator/AL_MASK
  i32.const -1
  i32.xor
  i32.and
  set_local $4
  current_memory
  set_local $5
  get_local $4
  get_local $5
  i32.const 16
  i32.shl
  i32.gt_u
  if
   get_local $4
   get_local $1
   i32.sub
   i32.const 65535
   i32.add
   i32.const 65535
   i32.const -1
   i32.xor
   i32.and
   i32.const 16
   i32.shr_u
   set_local $2
   get_local $5
   tee_local $3
   get_local $2
   tee_local $6
   get_local $3
   get_local $6
   i32.gt_s
   select
   set_local $3
   get_local $3
   grow_memory
   i32.const 0
   i32.lt_s
   if
    get_local $2
    grow_memory
    i32.const 0
    i32.lt_s
    if
     unreachable
    end
   end
  end
  get_local $4
  set_global $~lib/allocator/arena/offset
  get_local $1
 )
 (func $~lib/memory/memory.allocate (; 21 ;) (type $ii) (param $0 i32) (result i32)
  get_local $0
  call $~lib/allocator/arena/__memory_allocate
  return
 )
 (func $~lib/internal/arraybuffer/computeSize (; 22 ;) (type $ii) (param $0 i32) (result i32)
  i32.const 1
  i32.const 32
  get_local $0
  get_global $~lib/internal/arraybuffer/HEADER_SIZE
  i32.add
  i32.const 1
  i32.sub
  i32.clz
  i32.sub
  i32.shl
 )
 (func $~lib/internal/arraybuffer/allocateUnsafe (; 23 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  get_local $0
  get_global $~lib/internal/arraybuffer/MAX_BLENGTH
  i32.le_u
  i32.eqz
  if
   call $~lib/env/abort
   unreachable
  end
  block $~lib/memory/memory.allocate|inlined.0 (result i32)
   get_local $0
   call $~lib/internal/arraybuffer/computeSize
   set_local $2
   get_local $2
   call $~lib/allocator/arena/__memory_allocate
   br $~lib/memory/memory.allocate|inlined.0
  end
  set_local $1
  get_local $1
  get_local $0
  i32.store
  get_local $1
 )
 (func $~lib/internal/memory/memset (; 24 ;) (type $iiiv) (param $0 i32) (param $1 i32) (param $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i64)
  get_local $2
  i32.eqz
  if
   return
  end
  get_local $0
  get_local $1
  i32.store8
  get_local $0
  get_local $2
  i32.add
  i32.const 1
  i32.sub
  get_local $1
  i32.store8
  get_local $2
  i32.const 2
  i32.le_u
  if
   return
  end
  get_local $0
  i32.const 1
  i32.add
  get_local $1
  i32.store8
  get_local $0
  i32.const 2
  i32.add
  get_local $1
  i32.store8
  get_local $0
  get_local $2
  i32.add
  i32.const 2
  i32.sub
  get_local $1
  i32.store8
  get_local $0
  get_local $2
  i32.add
  i32.const 3
  i32.sub
  get_local $1
  i32.store8
  get_local $2
  i32.const 6
  i32.le_u
  if
   return
  end
  get_local $0
  i32.const 3
  i32.add
  get_local $1
  i32.store8
  get_local $0
  get_local $2
  i32.add
  i32.const 4
  i32.sub
  get_local $1
  i32.store8
  get_local $2
  i32.const 8
  i32.le_u
  if
   return
  end
  i32.const 0
  get_local $0
  i32.sub
  i32.const 3
  i32.and
  set_local $3
  get_local $0
  get_local $3
  i32.add
  set_local $0
  get_local $2
  get_local $3
  i32.sub
  set_local $2
  get_local $2
  i32.const -4
  i32.and
  set_local $2
  i32.const -1
  i32.const 255
  i32.div_u
  get_local $1
  i32.const 255
  i32.and
  i32.mul
  set_local $4
  get_local $0
  get_local $4
  i32.store
  get_local $0
  get_local $2
  i32.add
  i32.const 4
  i32.sub
  get_local $4
  i32.store
  get_local $2
  i32.const 8
  i32.le_u
  if
   return
  end
  get_local $0
  i32.const 4
  i32.add
  get_local $4
  i32.store
  get_local $0
  i32.const 8
  i32.add
  get_local $4
  i32.store
  get_local $0
  get_local $2
  i32.add
  i32.const 12
  i32.sub
  get_local $4
  i32.store
  get_local $0
  get_local $2
  i32.add
  i32.const 8
  i32.sub
  get_local $4
  i32.store
  get_local $2
  i32.const 24
  i32.le_u
  if
   return
  end
  get_local $0
  i32.const 12
  i32.add
  get_local $4
  i32.store
  get_local $0
  i32.const 16
  i32.add
  get_local $4
  i32.store
  get_local $0
  i32.const 20
  i32.add
  get_local $4
  i32.store
  get_local $0
  i32.const 24
  i32.add
  get_local $4
  i32.store
  get_local $0
  get_local $2
  i32.add
  i32.const 28
  i32.sub
  get_local $4
  i32.store
  get_local $0
  get_local $2
  i32.add
  i32.const 24
  i32.sub
  get_local $4
  i32.store
  get_local $0
  get_local $2
  i32.add
  i32.const 20
  i32.sub
  get_local $4
  i32.store
  get_local $0
  get_local $2
  i32.add
  i32.const 16
  i32.sub
  get_local $4
  i32.store
  i32.const 24
  get_local $0
  i32.const 4
  i32.and
  i32.add
  set_local $3
  get_local $0
  get_local $3
  i32.add
  set_local $0
  get_local $2
  get_local $3
  i32.sub
  set_local $2
  get_local $4
  i64.extend_u/i32
  get_local $4
  i64.extend_u/i32
  i64.const 32
  i64.shl
  i64.or
  set_local $5
  block $break|0
   loop $continue|0
    get_local $2
    i32.const 32
    i32.ge_u
    if
     block
      get_local $0
      get_local $5
      i64.store
      get_local $0
      i32.const 8
      i32.add
      get_local $5
      i64.store
      get_local $0
      i32.const 16
      i32.add
      get_local $5
      i64.store
      get_local $0
      i32.const 24
      i32.add
      get_local $5
      i64.store
      get_local $2
      i32.const 32
      i32.sub
      set_local $2
      get_local $0
      i32.const 32
      i32.add
      set_local $0
     end
     br $continue|0
    end
   end
  end
 )
 (func $~lib/array/Array<u8>#constructor (; 25 ;) (type $iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  get_local $1
  i32.const 1073741816
  i32.gt_u
  if
   call $~lib/env/abort
   unreachable
  end
  get_local $1
  i32.const 0
  i32.shl
  set_local $2
  get_local $2
  call $~lib/internal/arraybuffer/allocateUnsafe
  set_local $3
  get_local $0
  if (result i32)
   get_local $0
  else   
   block (result i32)
    i32.const 8
    call $~lib/memory/memory.allocate
    set_local $4
    get_local $4
    i32.const 0
    i32.store
    get_local $4
    i32.const 0
    i32.store offset=4
    get_local $4
   end
   tee_local $0
  end
  tee_local $0
  get_local $3
  i32.store
  get_local $0
  get_local $1
  i32.store offset=4
  get_local $3
  get_global $~lib/internal/arraybuffer/HEADER_SIZE
  i32.add
  set_local $4
  i32.const 0
  set_local $5
  get_local $4
  get_local $5
  get_local $2
  call $~lib/internal/memory/memset
  get_local $0
 )
 (func $~lib/string/String#charCodeAt (; 26 ;) (type $iii) (param $0 i32) (param $1 i32) (result i32)
  get_local $0
  i32.const 0
  i32.ne
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
  (local $5 i32)
  block $break|0
   loop $continue|0
    get_local $2
    if (result i32)
     get_local $1
     i32.const 3
     i32.and
    else     
     get_local $2
    end
    if
     block
      block (result i32)
       get_local $0
       tee_local $5
       i32.const 1
       i32.add
       set_local $0
       get_local $5
      end
      block (result i32)
       get_local $1
       tee_local $5
       i32.const 1
       i32.add
       set_local $1
       get_local $5
      end
      i32.load8_u
      i32.store8
      get_local $2
      i32.const 1
      i32.sub
      set_local $2
     end
     br $continue|0
    end
   end
  end
  get_local $0
  i32.const 3
  i32.and
  i32.const 0
  i32.eq
  if
   block $break|1
    loop $continue|1
     get_local $2
     i32.const 16
     i32.ge_u
     if
      block
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
      end
      br $continue|1
     end
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
    block (result i32)
     get_local $0
     tee_local $5
     i32.const 1
     i32.add
     set_local $0
     get_local $5
    end
    block (result i32)
     get_local $1
     tee_local $5
     i32.const 1
     i32.add
     set_local $1
     get_local $5
    end
    i32.load8_u
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
       set_local $5
       get_local $5
       i32.const 1
       i32.eq
       br_if $case0|2
       get_local $5
       i32.const 2
       i32.eq
       br_if $case1|2
       get_local $5
       i32.const 3
       i32.eq
       br_if $case2|2
       br $break|2
      end
      block
       get_local $1
       i32.load
       set_local $3
       block (result i32)
        get_local $0
        tee_local $5
        i32.const 1
        i32.add
        set_local $0
        get_local $5
       end
       block (result i32)
        get_local $1
        tee_local $5
        i32.const 1
        i32.add
        set_local $1
        get_local $5
       end
       i32.load8_u
       i32.store8
       block (result i32)
        get_local $0
        tee_local $5
        i32.const 1
        i32.add
        set_local $0
        get_local $5
       end
       block (result i32)
        get_local $1
        tee_local $5
        i32.const 1
        i32.add
        set_local $1
        get_local $5
       end
       i32.load8_u
       i32.store8
       block (result i32)
        get_local $0
        tee_local $5
        i32.const 1
        i32.add
        set_local $0
        get_local $5
       end
       block (result i32)
        get_local $1
        tee_local $5
        i32.const 1
        i32.add
        set_local $1
        get_local $5
       end
       i32.load8_u
       i32.store8
       get_local $2
       i32.const 3
       i32.sub
       set_local $2
       block $break|3
        loop $continue|3
         get_local $2
         i32.const 17
         i32.ge_u
         if
          block
           get_local $1
           i32.const 1
           i32.add
           i32.load
           set_local $4
           get_local $0
           get_local $3
           i32.const 24
           i32.shr_u
           get_local $4
           i32.const 8
           i32.shl
           i32.or
           i32.store
           get_local $1
           i32.const 5
           i32.add
           i32.load
           set_local $3
           get_local $0
           i32.const 4
           i32.add
           get_local $4
           i32.const 24
           i32.shr_u
           get_local $3
           i32.const 8
           i32.shl
           i32.or
           i32.store
           get_local $1
           i32.const 9
           i32.add
           i32.load
           set_local $4
           get_local $0
           i32.const 8
           i32.add
           get_local $3
           i32.const 24
           i32.shr_u
           get_local $4
           i32.const 8
           i32.shl
           i32.or
           i32.store
           get_local $1
           i32.const 13
           i32.add
           i32.load
           set_local $3
           get_local $0
           i32.const 12
           i32.add
           get_local $4
           i32.const 24
           i32.shr_u
           get_local $3
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
          end
          br $continue|3
         end
        end
       end
       br $break|2
       unreachable
      end
      unreachable
     end
     block
      get_local $1
      i32.load
      set_local $3
      block (result i32)
       get_local $0
       tee_local $5
       i32.const 1
       i32.add
       set_local $0
       get_local $5
      end
      block (result i32)
       get_local $1
       tee_local $5
       i32.const 1
       i32.add
       set_local $1
       get_local $5
      end
      i32.load8_u
      i32.store8
      block (result i32)
       get_local $0
       tee_local $5
       i32.const 1
       i32.add
       set_local $0
       get_local $5
      end
      block (result i32)
       get_local $1
       tee_local $5
       i32.const 1
       i32.add
       set_local $1
       get_local $5
      end
      i32.load8_u
      i32.store8
      get_local $2
      i32.const 2
      i32.sub
      set_local $2
      block $break|4
       loop $continue|4
        get_local $2
        i32.const 18
        i32.ge_u
        if
         block
          get_local $1
          i32.const 2
          i32.add
          i32.load
          set_local $4
          get_local $0
          get_local $3
          i32.const 16
          i32.shr_u
          get_local $4
          i32.const 16
          i32.shl
          i32.or
          i32.store
          get_local $1
          i32.const 6
          i32.add
          i32.load
          set_local $3
          get_local $0
          i32.const 4
          i32.add
          get_local $4
          i32.const 16
          i32.shr_u
          get_local $3
          i32.const 16
          i32.shl
          i32.or
          i32.store
          get_local $1
          i32.const 10
          i32.add
          i32.load
          set_local $4
          get_local $0
          i32.const 8
          i32.add
          get_local $3
          i32.const 16
          i32.shr_u
          get_local $4
          i32.const 16
          i32.shl
          i32.or
          i32.store
          get_local $1
          i32.const 14
          i32.add
          i32.load
          set_local $3
          get_local $0
          i32.const 12
          i32.add
          get_local $4
          i32.const 16
          i32.shr_u
          get_local $3
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
         end
         br $continue|4
        end
       end
      end
      br $break|2
      unreachable
     end
     unreachable
    end
    block
     get_local $1
     i32.load
     set_local $3
     block (result i32)
      get_local $0
      tee_local $5
      i32.const 1
      i32.add
      set_local $0
      get_local $5
     end
     block (result i32)
      get_local $1
      tee_local $5
      i32.const 1
      i32.add
      set_local $1
      get_local $5
     end
     i32.load8_u
     i32.store8
     get_local $2
     i32.const 1
     i32.sub
     set_local $2
     block $break|5
      loop $continue|5
       get_local $2
       i32.const 19
       i32.ge_u
       if
        block
         get_local $1
         i32.const 3
         i32.add
         i32.load
         set_local $4
         get_local $0
         get_local $3
         i32.const 8
         i32.shr_u
         get_local $4
         i32.const 24
         i32.shl
         i32.or
         i32.store
         get_local $1
         i32.const 7
         i32.add
         i32.load
         set_local $3
         get_local $0
         i32.const 4
         i32.add
         get_local $4
         i32.const 8
         i32.shr_u
         get_local $3
         i32.const 24
         i32.shl
         i32.or
         i32.store
         get_local $1
         i32.const 11
         i32.add
         i32.load
         set_local $4
         get_local $0
         i32.const 8
         i32.add
         get_local $3
         i32.const 8
         i32.shr_u
         get_local $4
         i32.const 24
         i32.shl
         i32.or
         i32.store
         get_local $1
         i32.const 15
         i32.add
         i32.load
         set_local $3
         get_local $0
         i32.const 12
         i32.add
         get_local $4
         i32.const 8
         i32.shr_u
         get_local $3
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
        end
        br $continue|5
       end
      end
     end
     br $break|2
     unreachable
    end
    unreachable
   end
  end
  get_local $2
  i32.const 16
  i32.and
  if
   block (result i32)
    get_local $0
    tee_local $5
    i32.const 1
    i32.add
    set_local $0
    get_local $5
   end
   block (result i32)
    get_local $1
    tee_local $5
    i32.const 1
    i32.add
    set_local $1
    get_local $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    get_local $0
    tee_local $5
    i32.const 1
    i32.add
    set_local $0
    get_local $5
   end
   block (result i32)
    get_local $1
    tee_local $5
    i32.const 1
    i32.add
    set_local $1
    get_local $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    get_local $0
    tee_local $5
    i32.const 1
    i32.add
    set_local $0
    get_local $5
   end
   block (result i32)
    get_local $1
    tee_local $5
    i32.const 1
    i32.add
    set_local $1
    get_local $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    get_local $0
    tee_local $5
    i32.const 1
    i32.add
    set_local $0
    get_local $5
   end
   block (result i32)
    get_local $1
    tee_local $5
    i32.const 1
    i32.add
    set_local $1
    get_local $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    get_local $0
    tee_local $5
    i32.const 1
    i32.add
    set_local $0
    get_local $5
   end
   block (result i32)
    get_local $1
    tee_local $5
    i32.const 1
    i32.add
    set_local $1
    get_local $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    get_local $0
    tee_local $5
    i32.const 1
    i32.add
    set_local $0
    get_local $5
   end
   block (result i32)
    get_local $1
    tee_local $5
    i32.const 1
    i32.add
    set_local $1
    get_local $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    get_local $0
    tee_local $5
    i32.const 1
    i32.add
    set_local $0
    get_local $5
   end
   block (result i32)
    get_local $1
    tee_local $5
    i32.const 1
    i32.add
    set_local $1
    get_local $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    get_local $0
    tee_local $5
    i32.const 1
    i32.add
    set_local $0
    get_local $5
   end
   block (result i32)
    get_local $1
    tee_local $5
    i32.const 1
    i32.add
    set_local $1
    get_local $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    get_local $0
    tee_local $5
    i32.const 1
    i32.add
    set_local $0
    get_local $5
   end
   block (result i32)
    get_local $1
    tee_local $5
    i32.const 1
    i32.add
    set_local $1
    get_local $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    get_local $0
    tee_local $5
    i32.const 1
    i32.add
    set_local $0
    get_local $5
   end
   block (result i32)
    get_local $1
    tee_local $5
    i32.const 1
    i32.add
    set_local $1
    get_local $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    get_local $0
    tee_local $5
    i32.const 1
    i32.add
    set_local $0
    get_local $5
   end
   block (result i32)
    get_local $1
    tee_local $5
    i32.const 1
    i32.add
    set_local $1
    get_local $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    get_local $0
    tee_local $5
    i32.const 1
    i32.add
    set_local $0
    get_local $5
   end
   block (result i32)
    get_local $1
    tee_local $5
    i32.const 1
    i32.add
    set_local $1
    get_local $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    get_local $0
    tee_local $5
    i32.const 1
    i32.add
    set_local $0
    get_local $5
   end
   block (result i32)
    get_local $1
    tee_local $5
    i32.const 1
    i32.add
    set_local $1
    get_local $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    get_local $0
    tee_local $5
    i32.const 1
    i32.add
    set_local $0
    get_local $5
   end
   block (result i32)
    get_local $1
    tee_local $5
    i32.const 1
    i32.add
    set_local $1
    get_local $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    get_local $0
    tee_local $5
    i32.const 1
    i32.add
    set_local $0
    get_local $5
   end
   block (result i32)
    get_local $1
    tee_local $5
    i32.const 1
    i32.add
    set_local $1
    get_local $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    get_local $0
    tee_local $5
    i32.const 1
    i32.add
    set_local $0
    get_local $5
   end
   block (result i32)
    get_local $1
    tee_local $5
    i32.const 1
    i32.add
    set_local $1
    get_local $5
   end
   i32.load8_u
   i32.store8
  end
  get_local $2
  i32.const 8
  i32.and
  if
   block (result i32)
    get_local $0
    tee_local $5
    i32.const 1
    i32.add
    set_local $0
    get_local $5
   end
   block (result i32)
    get_local $1
    tee_local $5
    i32.const 1
    i32.add
    set_local $1
    get_local $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    get_local $0
    tee_local $5
    i32.const 1
    i32.add
    set_local $0
    get_local $5
   end
   block (result i32)
    get_local $1
    tee_local $5
    i32.const 1
    i32.add
    set_local $1
    get_local $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    get_local $0
    tee_local $5
    i32.const 1
    i32.add
    set_local $0
    get_local $5
   end
   block (result i32)
    get_local $1
    tee_local $5
    i32.const 1
    i32.add
    set_local $1
    get_local $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    get_local $0
    tee_local $5
    i32.const 1
    i32.add
    set_local $0
    get_local $5
   end
   block (result i32)
    get_local $1
    tee_local $5
    i32.const 1
    i32.add
    set_local $1
    get_local $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    get_local $0
    tee_local $5
    i32.const 1
    i32.add
    set_local $0
    get_local $5
   end
   block (result i32)
    get_local $1
    tee_local $5
    i32.const 1
    i32.add
    set_local $1
    get_local $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    get_local $0
    tee_local $5
    i32.const 1
    i32.add
    set_local $0
    get_local $5
   end
   block (result i32)
    get_local $1
    tee_local $5
    i32.const 1
    i32.add
    set_local $1
    get_local $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    get_local $0
    tee_local $5
    i32.const 1
    i32.add
    set_local $0
    get_local $5
   end
   block (result i32)
    get_local $1
    tee_local $5
    i32.const 1
    i32.add
    set_local $1
    get_local $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    get_local $0
    tee_local $5
    i32.const 1
    i32.add
    set_local $0
    get_local $5
   end
   block (result i32)
    get_local $1
    tee_local $5
    i32.const 1
    i32.add
    set_local $1
    get_local $5
   end
   i32.load8_u
   i32.store8
  end
  get_local $2
  i32.const 4
  i32.and
  if
   block (result i32)
    get_local $0
    tee_local $5
    i32.const 1
    i32.add
    set_local $0
    get_local $5
   end
   block (result i32)
    get_local $1
    tee_local $5
    i32.const 1
    i32.add
    set_local $1
    get_local $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    get_local $0
    tee_local $5
    i32.const 1
    i32.add
    set_local $0
    get_local $5
   end
   block (result i32)
    get_local $1
    tee_local $5
    i32.const 1
    i32.add
    set_local $1
    get_local $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    get_local $0
    tee_local $5
    i32.const 1
    i32.add
    set_local $0
    get_local $5
   end
   block (result i32)
    get_local $1
    tee_local $5
    i32.const 1
    i32.add
    set_local $1
    get_local $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    get_local $0
    tee_local $5
    i32.const 1
    i32.add
    set_local $0
    get_local $5
   end
   block (result i32)
    get_local $1
    tee_local $5
    i32.const 1
    i32.add
    set_local $1
    get_local $5
   end
   i32.load8_u
   i32.store8
  end
  get_local $2
  i32.const 2
  i32.and
  if
   block (result i32)
    get_local $0
    tee_local $5
    i32.const 1
    i32.add
    set_local $0
    get_local $5
   end
   block (result i32)
    get_local $1
    tee_local $5
    i32.const 1
    i32.add
    set_local $1
    get_local $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    get_local $0
    tee_local $5
    i32.const 1
    i32.add
    set_local $0
    get_local $5
   end
   block (result i32)
    get_local $1
    tee_local $5
    i32.const 1
    i32.add
    set_local $1
    get_local $5
   end
   i32.load8_u
   i32.store8
  end
  get_local $2
  i32.const 1
  i32.and
  if
   block (result i32)
    get_local $0
    tee_local $5
    i32.const 1
    i32.add
    set_local $0
    get_local $5
   end
   block (result i32)
    get_local $1
    tee_local $5
    i32.const 1
    i32.add
    set_local $1
    get_local $5
   end
   i32.load8_u
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
    block $break|0
     loop $continue|0
      get_local $0
      i32.const 7
      i32.and
      if
       block
        get_local $2
        i32.eqz
        if
         return
        end
        get_local $2
        i32.const 1
        i32.sub
        set_local $2
        block (result i32)
         get_local $0
         tee_local $3
         i32.const 1
         i32.add
         set_local $0
         get_local $3
        end
        block (result i32)
         get_local $1
         tee_local $3
         i32.const 1
         i32.add
         set_local $1
         get_local $3
        end
        i32.load8_u
        i32.store8
       end
       br $continue|0
      end
     end
    end
    block $break|1
     loop $continue|1
      get_local $2
      i32.const 8
      i32.ge_u
      if
       block
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
       end
       br $continue|1
      end
     end
    end
   end
   block $break|2
    loop $continue|2
     get_local $2
     if
      block
       block (result i32)
        get_local $0
        tee_local $3
        i32.const 1
        i32.add
        set_local $0
        get_local $3
       end
       block (result i32)
        get_local $1
        tee_local $3
        i32.const 1
        i32.add
        set_local $1
        get_local $3
       end
       i32.load8_u
       i32.store8
       get_local $2
       i32.const 1
       i32.sub
       set_local $2
      end
      br $continue|2
     end
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
    block $break|3
     loop $continue|3
      get_local $0
      get_local $2
      i32.add
      i32.const 7
      i32.and
      if
       block
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
       end
       br $continue|3
      end
     end
    end
    block $break|4
     loop $continue|4
      get_local $2
      i32.const 8
      i32.ge_u
      if
       block
        get_local $2
        i32.const 8
        i32.sub
        set_local $2
        get_local $0
        get_local $2
        i32.add
        get_local $1
        get_local $2
        i32.add
        i64.load
        i64.store
       end
       br $continue|4
      end
     end
    end
   end
   block $break|5
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
  end
 )
 (func $~lib/internal/arraybuffer/reallocateUnsafe (; 29 ;) (type $iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  get_local $0
  i32.load
  set_local $2
  get_local $1
  get_local $2
  i32.gt_s
  if
   get_local $1
   get_global $~lib/internal/arraybuffer/MAX_BLENGTH
   i32.le_s
   i32.eqz
   if
    call $~lib/env/abort
    unreachable
   end
   get_local $1
   get_local $2
   call $~lib/internal/arraybuffer/computeSize
   get_global $~lib/internal/arraybuffer/HEADER_SIZE
   i32.sub
   i32.le_s
   if
    get_local $0
    get_local $1
    i32.store
    get_local $0
    get_global $~lib/internal/arraybuffer/HEADER_SIZE
    i32.add
    get_local $2
    i32.add
    set_local $3
    i32.const 0
    set_local $4
    get_local $1
    get_local $2
    i32.sub
    set_local $5
    get_local $3
    get_local $4
    get_local $5
    call $~lib/internal/memory/memset
   else    
    get_local $1
    call $~lib/internal/arraybuffer/allocateUnsafe
    set_local $5
    get_local $5
    get_global $~lib/internal/arraybuffer/HEADER_SIZE
    i32.add
    set_local $4
    get_local $0
    get_global $~lib/internal/arraybuffer/HEADER_SIZE
    i32.add
    set_local $3
    get_local $4
    get_local $3
    get_local $2
    call $~lib/internal/memory/memmove
    get_local $5
    get_global $~lib/internal/arraybuffer/HEADER_SIZE
    i32.add
    get_local $2
    i32.add
    set_local $3
    i32.const 0
    set_local $4
    get_local $1
    get_local $2
    i32.sub
    set_local $6
    get_local $3
    get_local $4
    get_local $6
    call $~lib/internal/memory/memset
    get_local $5
    return
   end
  else   
   get_local $1
   get_local $2
   i32.lt_s
   if
    get_local $1
    i32.const 0
    i32.ge_s
    i32.eqz
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
  (local $5 i32)
  get_local $0
  i32.load offset=4
  set_local $2
  get_local $0
  i32.load
  set_local $3
  get_local $3
  i32.load
  i32.const 0
  i32.shr_u
  set_local $4
  get_local $2
  i32.const 1
  i32.add
  set_local $5
  get_local $2
  get_local $4
  i32.ge_u
  if
   get_local $2
   i32.const 1073741816
   i32.ge_u
   if
    call $~lib/env/abort
    unreachable
   end
   get_local $3
   get_local $5
   i32.const 0
   i32.shl
   call $~lib/internal/arraybuffer/reallocateUnsafe
   set_local $3
   get_local $0
   get_local $3
   i32.store
  end
  get_local $0
  get_local $5
  i32.store offset=4
  get_local $3
  get_local $2
  i32.const 0
  i32.shl
  i32.add
  get_local $1
  i32.store8 offset=8
  get_local $5
 )
 (func $~lib/utf8util/toUTF8Array (; 31 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  i32.const 0
  i32.const 0
  call $~lib/array/Array<u8>#constructor
  set_local $1
  block $break|0
   i32.const 0
   set_local $2
   loop $repeat|0
    get_local $2
    get_local $0
    i32.load
    i32.lt_s
    i32.eqz
    br_if $break|0
    block
     get_local $0
     get_local $2
     call $~lib/string/String#charCodeAt
     set_local $3
     get_local $3
     i32.const 128
     i32.lt_s
     if
      get_local $1
      get_local $3
      call $~lib/array/Array<u8>#push
      drop
     else      
      get_local $3
      i32.const 2048
      i32.lt_s
      if
       get_local $1
       i32.const 192
       get_local $3
       i32.const 6
       i32.shr_s
       i32.or
       call $~lib/array/Array<u8>#push
       drop
       get_local $1
       i32.const 128
       get_local $3
       i32.const 63
       i32.and
       i32.or
       call $~lib/array/Array<u8>#push
       drop
      else       
       get_local $3
       i32.const 55296
       i32.lt_s
       tee_local $4
       if (result i32)
        get_local $4
       else        
        get_local $3
        i32.const 57344
        i32.ge_s
       end
       if
        get_local $1
        i32.const 224
        get_local $3
        i32.const 12
        i32.shr_s
        i32.or
        call $~lib/array/Array<u8>#push
        drop
        get_local $1
        i32.const 128
        get_local $3
        i32.const 6
        i32.shr_s
        i32.const 63
        i32.and
        i32.or
        call $~lib/array/Array<u8>#push
        drop
        get_local $1
        i32.const 128
        get_local $3
        i32.const 63
        i32.and
        i32.or
        call $~lib/array/Array<u8>#push
        drop
       else        
        get_local $2
        i32.const 1
        i32.add
        set_local $2
        i32.const 65536
        get_local $3
        i32.const 1023
        i32.and
        i32.const 10
        i32.shl
        get_local $0
        get_local $2
        call $~lib/string/String#charCodeAt
        i32.const 1023
        i32.and
        i32.or
        i32.add
        set_local $3
        get_local $1
        i32.const 240
        get_local $3
        i32.const 18
        i32.shr_s
        i32.or
        call $~lib/array/Array<u8>#push
        drop
        get_local $1
        i32.const 128
        get_local $3
        i32.const 12
        i32.shr_s
        i32.const 63
        i32.and
        i32.or
        call $~lib/array/Array<u8>#push
        drop
        get_local $1
        i32.const 128
        get_local $3
        i32.const 6
        i32.shr_s
        i32.const 63
        i32.and
        i32.or
        call $~lib/array/Array<u8>#push
        drop
        get_local $1
        i32.const 128
        get_local $3
        i32.const 63
        i32.and
        i32.or
        call $~lib/array/Array<u8>#push
        drop
       end
      end
     end
    end
    get_local $2
    i32.const 1
    i32.add
    set_local $2
    br $repeat|0
    unreachable
   end
   unreachable
  end
  get_local $1
  i32.const 0
  call $~lib/array/Array<u8>#push
  drop
  get_local $1
 )
 (func $~lib/utf8util/string2cstr (; 32 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  get_local $0
  call $~lib/utf8util/toUTF8Array
  set_local $1
  get_local $1
  i32.load
  set_local $2
  get_local $2
  i32.const 8
  i32.add
 )
 (func $~lib/env/ultrain_assert (; 33 ;) (type $iiv) (param $0 i32) (param $1 i32)
  get_local $0
  i32.const 0
  i32.ne
  i32.const 0
  i32.eq
  if
   i32.const 0
   get_local $1
   call $~lib/utf8util/string2cstr
   call $~lib/env/ultrainio_assert
  end
 )
 (func $~lib/ultrain-ts-lib/src/utils/toUTF8Array (; 34 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  i32.const 0
  i32.const 0
  call $~lib/array/Array<u8>#constructor
  set_local $1
  block $break|0
   i32.const 0
   set_local $2
   loop $repeat|0
    get_local $2
    get_local $0
    i32.load
    i32.lt_s
    i32.eqz
    br_if $break|0
    block
     get_local $0
     get_local $2
     call $~lib/string/String#charCodeAt
     set_local $3
     get_local $3
     i32.const 128
     i32.lt_s
     if
      get_local $1
      get_local $3
      call $~lib/array/Array<u8>#push
      drop
     else      
      get_local $3
      i32.const 2048
      i32.lt_s
      if
       get_local $1
       i32.const 192
       get_local $3
       i32.const 6
       i32.shr_s
       i32.or
       call $~lib/array/Array<u8>#push
       drop
       get_local $1
       i32.const 128
       get_local $3
       i32.const 63
       i32.and
       i32.or
       call $~lib/array/Array<u8>#push
       drop
      else       
       get_local $3
       i32.const 55296
       i32.lt_s
       tee_local $4
       if (result i32)
        get_local $4
       else        
        get_local $3
        i32.const 57344
        i32.ge_s
       end
       if
        get_local $1
        i32.const 224
        get_local $3
        i32.const 12
        i32.shr_s
        i32.or
        call $~lib/array/Array<u8>#push
        drop
        get_local $1
        i32.const 128
        get_local $3
        i32.const 6
        i32.shr_s
        i32.const 63
        i32.and
        i32.or
        call $~lib/array/Array<u8>#push
        drop
        get_local $1
        i32.const 128
        get_local $3
        i32.const 63
        i32.and
        i32.or
        call $~lib/array/Array<u8>#push
        drop
       else        
        get_local $2
        i32.const 1
        i32.add
        set_local $2
        i32.const 65536
        get_local $3
        i32.const 1023
        i32.and
        i32.const 10
        i32.shl
        get_local $0
        get_local $2
        call $~lib/string/String#charCodeAt
        i32.const 1023
        i32.and
        i32.or
        i32.add
        set_local $3
        get_local $1
        i32.const 240
        get_local $3
        i32.const 18
        i32.shr_s
        i32.or
        call $~lib/array/Array<u8>#push
        drop
        get_local $1
        i32.const 128
        get_local $3
        i32.const 12
        i32.shr_s
        i32.const 63
        i32.and
        i32.or
        call $~lib/array/Array<u8>#push
        drop
        get_local $1
        i32.const 128
        get_local $3
        i32.const 6
        i32.shr_s
        i32.const 63
        i32.and
        i32.or
        call $~lib/array/Array<u8>#push
        drop
        get_local $1
        i32.const 128
        get_local $3
        i32.const 63
        i32.and
        i32.or
        call $~lib/array/Array<u8>#push
        drop
       end
      end
     end
    end
    get_local $2
    i32.const 1
    i32.add
    set_local $2
    br $repeat|0
    unreachable
   end
   unreachable
  end
  get_local $1
  i32.const 0
  call $~lib/array/Array<u8>#push
  drop
  get_local $1
 )
 (func $~lib/ultrain-ts-lib/src/utils/string2cstr (; 35 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  get_local $0
  call $~lib/ultrain-ts-lib/src/utils/toUTF8Array
  set_local $1
  get_local $1
  i32.load
  set_local $2
  get_local $2
  i32.const 8
  i32.add
 )
 (func $~lib/ultrain-ts-lib/src/log/Logger#s (; 36 ;) (type $iii) (param $0 i32) (param $1 i32) (result i32)
  get_local $1
  call $~lib/ultrain-ts-lib/src/utils/string2cstr
  call $~lib/ultrain-ts-lib/src/log/env.ts_log_print_s
  get_local $0
 )
 (func $~lib/ultrain-ts-lib/src/log/Logger#i (; 37 ;) (type $iIii) (param $0 i32) (param $1 i64) (param $2 i32) (result i32)
  get_local $1
  get_local $2
  call $~lib/ultrain-ts-lib/src/log/env.ts_log_print_i
  get_local $0
 )
 (func $~lib/ultrain-ts-lib/src/log/Logger#flush (; 38 ;) (type $iv) (param $0 i32)
  call $~lib/ultrain-ts-lib/src/log/env.ts_log_done
 )
 (func $~lib/ultrain-ts-lib/src/asset/StringToSymbol (; 39 ;) (type $iiI) (param $0 i32) (param $1 i32) (result i64)
  (local $2 i32)
  (local $3 i64)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  get_local $1
  i32.load
  set_local $2
  get_local $2
  i32.const 255
  i32.and
  i32.const 7
  i32.le_u
  i32.const 1296
  call $~lib/env/ultrain_assert
  i64.const 0
  set_local $3
  block $break|0
   i32.const 0
   set_local $4
   loop $repeat|0
    get_local $4
    get_local $2
    i32.const 255
    i32.and
    i32.lt_u
    i32.eqz
    br_if $break|0
    block
     get_local $1
     get_local $4
     i32.const 255
     i32.and
     call $~lib/string/String#charCodeAt
     i32.const 255
     i32.and
     set_local $5
     get_local $5
     get_global $~lib/ultrain-ts-lib/src/asset/CHAR_A
     i32.lt_u
     tee_local $6
     if (result i32)
      get_local $6
     else      
      get_local $5
      get_global $~lib/ultrain-ts-lib/src/asset/CHAR_Z
      i32.gt_u
     end
     if
      get_global $~lib/ultrain-ts-lib/src/log/Log
      i32.const 1520
      call $~lib/ultrain-ts-lib/src/log/Logger#s
      get_local $5
      i64.extend_u/i32
      i32.const 16
      call $~lib/ultrain-ts-lib/src/log/Logger#i
      call $~lib/ultrain-ts-lib/src/log/Logger#flush
     else      
      get_local $3
      get_local $5
      i64.extend_u/i32
      i64.const 8
      get_local $4
      i32.const 1
      i32.add
      i32.const 255
      i32.and
      i64.extend_u/i32
      i64.mul
      i64.shl
      i64.or
      set_local $3
     end
    end
    get_local $4
    i32.const 1
    i32.add
    set_local $4
    br $repeat|0
    unreachable
   end
   unreachable
  end
  get_local $3
  get_local $0
  i32.const 255
  i32.and
  i64.extend_u/i32
  i64.or
  set_local $3
  get_local $3
 )
 (func $~lib/ultrain-ts-lib/lib/name/char_to_symbol (; 40 ;) (type $iI) (param $0 i32) (result i64)
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
   i32.const 97
   i32.sub
   i32.const 6
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
   i32.const 49
   i32.sub
   i32.const 1
   i32.add
   i32.const 255
   i32.and
   i64.extend_u/i32
   return
  end
  i64.const 0
 )
 (func $~lib/ultrain-ts-lib/lib/name/N (; 41 ;) (type $iI) (param $0 i32) (result i64)
  (local $1 i32)
  (local $2 i64)
  (local $3 i32)
  (local $4 i64)
  (local $5 i32)
  get_local $0
  i32.load
  set_local $1
  i64.const 0
  set_local $2
  block $break|0
   i32.const 0
   set_local $3
   loop $repeat|0
    get_local $3
    i32.const 12
    i32.le_u
    i32.eqz
    br_if $break|0
    block
     i64.const 0
     set_local $4
     get_local $3
     get_local $1
     i32.lt_u
     tee_local $5
     if (result i32)
      get_local $3
      i32.const 12
      i32.le_u
     else      
      get_local $5
     end
     if
      get_local $0
      get_local $3
      call $~lib/string/String#charCodeAt
      i32.const 255
      i32.and
      call $~lib/ultrain-ts-lib/lib/name/char_to_symbol
      set_local $4
     end
     get_local $3
     i32.const 12
     i32.lt_u
     if
      get_local $4
      i64.const 31
      i64.and
      set_local $4
      get_local $4
      i64.const 64
      i64.const 5
      get_local $3
      i32.const 1
      i32.add
      i64.extend_u/i32
      i64.mul
      i64.sub
      i64.shl
      set_local $4
     else      
      get_local $4
      i64.const 15
      i64.and
      set_local $4
     end
     get_local $2
     get_local $4
     i64.or
     set_local $2
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
 )
 (func $~lib/ultrain-ts-lib/src/account/NAME (; 42 ;) (type $iI) (param $0 i32) (result i64)
  get_local $0
  call $~lib/ultrain-ts-lib/lib/name/N
 )
 (func $~lib/ultrain-ts-lib/src/asset/Asset#constructor (; 43 ;) (type $iIIi) (param $0 i32) (param $1 i64) (param $2 i64) (result i32)
  (local $3 i32)
  get_local $0
  if (result i32)
   get_local $0
  else   
   block (result i32)
    i32.const 16
    call $~lib/memory/memory.allocate
    set_local $3
    get_local $3
    i64.const 0
    i64.store
    get_local $3
    i64.const 0
    i64.store offset=8
    get_local $3
   end
   tee_local $0
  end
  tee_local $0
  get_local $1
  i64.store
  get_local $0
  get_local $2
  i64.store offset=8
  get_local $0
 )
 (func $~lib/ultrain-ts-lib/src/contract/Contract#_Contract_super (; 44 ;) (type $iIv) (param $0 i32) (param $1 i64)
  get_local $0
  get_local $1
  i64.store
 )
 (func $~lib/dbmanager/DBManager<Voter>#constructor (; 45 ;) (type $iIIIi) (param $0 i32) (param $1 i64) (param $2 i64) (param $3 i64) (result i32)
  (local $4 i32)
  get_local $0
  if (result i32)
   get_local $0
  else   
   block (result i32)
    i32.const 24
    call $~lib/memory/memory.allocate
    set_local $4
    get_local $4
    i64.const 0
    i64.store
    get_local $4
    i64.const 0
    i64.store offset=8
    get_local $4
    i64.const 0
    i64.store offset=16
    get_local $4
   end
   tee_local $0
  end
  tee_local $0
  get_local $1
  i64.store
  get_local $0
  get_local $2
  i64.store offset=8
  get_local $0
  get_local $3
  i64.store offset=16
  get_local $0
 )
 (func $~lib/dbmanager/DBManager<Waiter>#constructor (; 46 ;) (type $iIIIi) (param $0 i32) (param $1 i64) (param $2 i64) (param $3 i64) (result i32)
  (local $4 i32)
  get_local $0
  if (result i32)
   get_local $0
  else   
   block (result i32)
    i32.const 24
    call $~lib/memory/memory.allocate
    set_local $4
    get_local $4
    i64.const 0
    i64.store
    get_local $4
    i64.const 0
    i64.store offset=8
    get_local $4
    i64.const 0
    i64.store offset=16
    get_local $4
   end
   tee_local $0
  end
  tee_local $0
  get_local $1
  i64.store
  get_local $0
  get_local $2
  i64.store offset=8
  get_local $0
  get_local $3
  i64.store offset=16
  get_local $0
 )
 (func $~lib/dbmanager/DBManager<RandRecord>#constructor (; 47 ;) (type $iIIIi) (param $0 i32) (param $1 i64) (param $2 i64) (param $3 i64) (result i32)
  (local $4 i32)
  get_local $0
  if (result i32)
   get_local $0
  else   
   block (result i32)
    i32.const 24
    call $~lib/memory/memory.allocate
    set_local $4
    get_local $4
    i64.const 0
    i64.store
    get_local $4
    i64.const 0
    i64.store offset=8
    get_local $4
    i64.const 0
    i64.store offset=16
    get_local $4
   end
   tee_local $0
  end
  tee_local $0
  get_local $1
  i64.store
  get_local $0
  get_local $2
  i64.store offset=8
  get_local $0
  get_local $3
  i64.store offset=16
  get_local $0
 )
 (func $contract/lib/random.lib/Random#constructor (; 48 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  get_local $0
  if (result i32)
   get_local $0
  else   
   block (result i32)
    i32.const 12
    call $~lib/memory/memory.allocate
    set_local $1
    get_local $1
    i32.const 0
    i32.store
    get_local $1
    i32.const 0
    i32.store offset=4
    get_local $1
    i32.const 0
    i32.store offset=8
    get_local $1
   end
   tee_local $0
  end
  tee_local $0
  i32.const 0
  get_global $contract/lib/random.lib/VOTER_TABLE
  call $~lib/ultrain-ts-lib/src/account/NAME
  get_global $contract/lib/random.lib/CONT_NAME
  call $~lib/ultrain-ts-lib/src/account/NAME
  get_global $contract/lib/random.lib/VOTER_TABLE
  call $~lib/ultrain-ts-lib/src/account/NAME
  call $~lib/dbmanager/DBManager<Voter>#constructor
  i32.store
  get_local $0
  i32.const 0
  get_global $contract/lib/random.lib/WAITER_TABLE
  call $~lib/ultrain-ts-lib/src/account/NAME
  get_global $contract/lib/random.lib/CONT_NAME
  call $~lib/ultrain-ts-lib/src/account/NAME
  get_global $contract/lib/random.lib/WAITER_TABLE
  call $~lib/ultrain-ts-lib/src/account/NAME
  call $~lib/dbmanager/DBManager<Waiter>#constructor
  i32.store offset=4
  get_local $0
  i32.const 0
  get_global $contract/lib/random.lib/RAND_TABLE
  call $~lib/ultrain-ts-lib/src/account/NAME
  get_global $contract/lib/random.lib/CONT_NAME
  call $~lib/ultrain-ts-lib/src/account/NAME
  get_global $contract/lib/random.lib/RAND_TABLE
  call $~lib/ultrain-ts-lib/src/account/NAME
  call $~lib/dbmanager/DBManager<RandRecord>#constructor
  i32.store offset=8
  get_local $0
 )
 (func $~lib/dbmanager/DBManager<Voter>#find (; 49 ;) (type $iIi) (param $0 i32) (param $1 i64) (result i32)
  (local $2 i32)
  get_local $0
  i64.load offset=8
  get_local $0
  i64.load offset=16
  get_local $0
  i64.load
  get_local $1
  call $~lib/env/db_find_i64
  set_local $2
  get_local $2
 )
 (func $~lib/dbmanager/DBManager<Voter>#exists (; 50 ;) (type $iIi) (param $0 i32) (param $1 i64) (result i32)
  (local $2 i32)
  get_local $0
  get_local $1
  call $~lib/dbmanager/DBManager<Voter>#find
  set_local $2
  get_local $2
  i32.const 0
  i32.lt_s
  if (result i32)
   i32.const 0
  else   
   i32.const 1
  end
 )
 (func $~lib/array/Array<u64>#constructor (; 51 ;) (type $iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  get_local $1
  i32.const 134217727
  i32.gt_u
  if
   call $~lib/env/abort
   unreachable
  end
  get_local $1
  i32.const 3
  i32.shl
  set_local $2
  get_local $2
  call $~lib/internal/arraybuffer/allocateUnsafe
  set_local $3
  get_local $0
  if (result i32)
   get_local $0
  else   
   block (result i32)
    i32.const 8
    call $~lib/memory/memory.allocate
    set_local $4
    get_local $4
    i32.const 0
    i32.store
    get_local $4
    i32.const 0
    i32.store offset=4
    get_local $4
   end
   tee_local $0
  end
  tee_local $0
  get_local $3
  i32.store
  get_local $0
  get_local $1
  i32.store offset=4
  get_local $3
  get_global $~lib/internal/arraybuffer/HEADER_SIZE
  i32.add
  set_local $4
  i32.const 0
  set_local $5
  get_local $4
  get_local $5
  get_local $2
  call $~lib/internal/memory/memset
  get_local $0
 )
 (func $contract/lib/random.lib/Voter#constructor (; 52 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  get_local $0
  if (result i32)
   get_local $0
  else   
   block (result i32)
    i32.const 44
    call $~lib/memory/memory.allocate
    set_local $1
    get_local $1
    i64.const 0
    i64.store
    get_local $1
    i32.const 0
    i32.store offset=8
    get_local $1
    i32.const 0
    i32.store offset=12
    get_local $1
    i64.const 0
    i64.store offset=16
    get_local $1
    i64.const 0
    i64.store offset=24
    get_local $1
    i64.const 0
    i64.store offset=32
    get_local $1
    i32.const 0
    i32.store offset=40
    get_local $1
   end
   tee_local $0
  end
  tee_local $0
  i32.const 0
  get_global $contract/lib/random.lib/EPOCH
  i32.wrap/i64
  call $~lib/array/Array<u64>#constructor
  i32.store offset=8
  get_local $0
  i32.const 0
  get_global $contract/lib/random.lib/EPOCH
  i32.wrap/i64
  call $~lib/array/Array<u64>#constructor
  i32.store offset=12
  get_local $0
 )
 (func $~lib/ultrain-ts-lib/src/action/Action.get:sender (; 53 ;) (type $I) (result i64)
  call $~lib/ultrain-ts-lib/internal/action.d/env.current_sender
 )
 (func $~lib/datastream/DataStream#constructor (; 54 ;) (type $iiii) (param $0 i32) (param $1 i32) (param $2 i32) (result i32)
  (local $3 i32)
  get_local $0
  if (result i32)
   get_local $0
  else   
   block (result i32)
    i32.const 12
    call $~lib/memory/memory.allocate
    set_local $3
    get_local $3
    i32.const 0
    i32.store
    get_local $3
    i32.const 0
    i32.store offset=4
    get_local $3
    i32.const 0
    i32.store offset=8
    get_local $3
   end
   tee_local $0
  end
  tee_local $0
  get_local $1
  i32.store
  get_local $0
  get_local $2
  i32.store offset=4
  get_local $0
  i32.const 0
  i32.store offset=8
  get_local $0
 )
 (func $~lib/datastream/DataStream#isMeasureMode (; 55 ;) (type $ii) (param $0 i32) (result i32)
  get_local $0
  i32.load
  i32.const 0
  i32.eq
 )
 (func $~lib/datastream/DataStream#write<u64> (; 56 ;) (type $iIv) (param $0 i32) (param $1 i64)
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
 (func $~lib/datastream/DataStream#write<u8> (; 57 ;) (type $iiv) (param $0 i32) (param $1 i32)
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
 (func $~lib/datastream/DataStream#writeVarint32 (; 58 ;) (type $iiv) (param $0 i32) (param $1 i32)
  (local $2 i32)
  loop $continue|0
   block
    get_local $1
    i32.const 127
    i32.and
    set_local $2
    get_local $1
    i32.const 7
    i32.shr_u
    set_local $1
    get_local $2
    get_local $1
    i32.const 0
    i32.gt_u
    if (result i32)
     i32.const 1
    else     
     i32.const 0
    end
    i32.const 7
    i32.shl
    i32.or
    set_local $2
    get_local $0
    get_local $2
    call $~lib/datastream/DataStream#write<u8>
   end
   get_local $1
   br_if $continue|0
  end
 )
 (func $~lib/array/Array<u64>#__get (; 59 ;) (type $iiI) (param $0 i32) (param $1 i32) (result i64)
  (local $2 i32)
  get_local $0
  i32.load
  set_local $2
  get_local $1
  get_local $2
  i32.load
  i32.const 3
  i32.shr_u
  i32.lt_u
  if (result i64)
   get_local $2
   get_local $1
   i32.const 3
   i32.shl
   i32.add
   i64.load offset=8
  else   
   unreachable
  end
 )
 (func $~lib/datastream/DataStream#writeVector<u64> (; 60 ;) (type $iiv) (param $0 i32) (param $1 i32)
  (local $2 i32)
  (local $3 i32)
  block $~lib/array/Array<u64>#get:length|inlined.0 (result i32)
   get_local $1
   i32.load offset=4
  end
  set_local $2
  get_local $0
  get_local $2
  call $~lib/datastream/DataStream#writeVarint32
  block $break|0
   i32.const 0
   set_local $3
   loop $repeat|0
    get_local $3
    get_local $2
    i32.lt_u
    i32.eqz
    br_if $break|0
    get_local $0
    get_local $1
    get_local $3
    call $~lib/array/Array<u64>#__get
    call $~lib/datastream/DataStream#write<u64>
    get_local $3
    i32.const 1
    i32.add
    set_local $3
    br $repeat|0
    unreachable
   end
   unreachable
  end
 )
 (func $~lib/datastream/DataStream#write<i32> (; 61 ;) (type $iiv) (param $0 i32) (param $1 i32)
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
 (func $contract/lib/random.lib/Voter#serialize (; 62 ;) (type $iiv) (param $0 i32) (param $1 i32)
  get_local $1
  get_local $0
  i64.load
  call $~lib/datastream/DataStream#write<u64>
  get_local $1
  get_local $0
  i32.load offset=8
  call $~lib/datastream/DataStream#writeVector<u64>
  get_local $1
  get_local $0
  i32.load offset=12
  call $~lib/datastream/DataStream#writeVector<u64>
  get_local $1
  get_local $0
  i64.load offset=16
  call $~lib/datastream/DataStream#write<u64>
  get_local $1
  get_local $0
  i64.load offset=24
  call $~lib/datastream/DataStream#write<u64>
  get_local $1
  get_local $0
  i64.load offset=32
  call $~lib/datastream/DataStream#write<u64>
  get_local $1
  get_local $0
  i32.load offset=40
  call $~lib/datastream/DataStream#write<i32>
 )
 (func $~lib/datastream/DataStream.measure<Voter> (; 63 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  i32.const 0
  i32.const 0
  i32.const 0
  call $~lib/datastream/DataStream#constructor
  set_local $1
  get_local $0
  get_local $1
  call $contract/lib/random.lib/Voter#serialize
  get_local $1
  i32.load offset=8
 )
 (func $~lib/internal/typedarray/TypedArray<u8_u32>#constructor (; 64 ;) (type $iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  get_local $1
  i32.const 1073741816
  i32.gt_u
  if
   call $~lib/env/abort
   unreachable
  end
  get_local $1
  i32.const 0
  i32.shl
  set_local $2
  get_local $2
  call $~lib/internal/arraybuffer/allocateUnsafe
  set_local $3
  get_local $3
  get_global $~lib/internal/arraybuffer/HEADER_SIZE
  i32.add
  set_local $4
  i32.const 0
  set_local $5
  get_local $4
  get_local $5
  get_local $2
  call $~lib/internal/memory/memset
  get_local $0
  if (result i32)
   get_local $0
  else   
   block (result i32)
    i32.const 12
    call $~lib/memory/memory.allocate
    set_local $5
    get_local $5
    i32.const 0
    i32.store
    get_local $5
    i32.const 0
    i32.store offset=4
    get_local $5
    i32.const 0
    i32.store offset=8
    get_local $5
   end
   tee_local $0
  end
  tee_local $0
  get_local $3
  i32.store
  get_local $0
  i32.const 0
  i32.store offset=4
  get_local $0
  get_local $2
  i32.store offset=8
  get_local $0
 )
 (func $contract/lib/random.lib/Voter#primaryKey (; 65 ;) (type $iI) (param $0 i32) (result i64)
  get_local $0
  i64.load
 )
 (func $~lib/dbmanager/DBManager<Voter>#emplace (; 66 ;) (type $iIiv) (param $0 i32) (param $1 i64) (param $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i64)
  get_local $0
  i64.load offset=8
  call $~lib/env/current_receiver
  i64.eq
  i32.const 1760
  call $~lib/env/ultrain_assert
  get_local $2
  call $~lib/datastream/DataStream.measure<Voter>
  set_local $3
  i32.const 0
  get_local $3
  call $~lib/internal/typedarray/TypedArray<u8_u32>#constructor
  set_local $4
  i32.const 0
  get_local $4
  i32.load
  get_local $3
  call $~lib/datastream/DataStream#constructor
  set_local $5
  get_local $2
  get_local $5
  call $contract/lib/random.lib/Voter#serialize
  get_local $2
  call $contract/lib/random.lib/Voter#primaryKey
  set_local $6
  get_local $0
  i64.load offset=16
  get_local $0
  i64.load
  get_local $1
  get_local $6
  get_local $5
  i32.load
  get_local $5
  i32.load offset=8
  call $~lib/env/db_store_i64
  drop
 )
 (func $~lib/dbmanager/DBManager<Waiter>#find (; 67 ;) (type $iIi) (param $0 i32) (param $1 i64) (result i32)
  (local $2 i32)
  get_local $0
  i64.load offset=8
  get_local $0
  i64.load offset=16
  get_local $0
  i64.load
  get_local $1
  call $~lib/env/db_find_i64
  set_local $2
  get_local $2
 )
 (func $~lib/dbmanager/DBManager<Waiter>#exists (; 68 ;) (type $iIi) (param $0 i32) (param $1 i64) (result i32)
  (local $2 i32)
  get_local $0
  get_local $1
  call $~lib/dbmanager/DBManager<Waiter>#find
  set_local $2
  get_local $2
  i32.const 0
  i32.lt_s
  if (result i32)
   i32.const 0
  else   
   i32.const 1
  end
 )
 (func $contract/lib/random.lib/Waiter#serialize (; 69 ;) (type $iiv) (param $0 i32) (param $1 i32)
  get_local $1
  get_local $0
  i32.load
  call $~lib/datastream/DataStream#writeVector<u64>
 )
 (func $~lib/datastream/DataStream.measure<Waiter> (; 70 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  i32.const 0
  i32.const 0
  i32.const 0
  call $~lib/datastream/DataStream#constructor
  set_local $1
  get_local $0
  get_local $1
  call $contract/lib/random.lib/Waiter#serialize
  get_local $1
  i32.load offset=8
 )
 (func $contract/lib/random.lib/Waiter#primaryKey (; 71 ;) (type $iI) (param $0 i32) (result i64)
  i64.const 0
 )
 (func $~lib/dbmanager/DBManager<Waiter>#emplace (; 72 ;) (type $iIiv) (param $0 i32) (param $1 i64) (param $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i64)
  get_local $0
  i64.load offset=8
  call $~lib/env/current_receiver
  i64.eq
  i32.const 1760
  call $~lib/env/ultrain_assert
  get_local $2
  call $~lib/datastream/DataStream.measure<Waiter>
  set_local $3
  i32.const 0
  get_local $3
  call $~lib/internal/typedarray/TypedArray<u8_u32>#constructor
  set_local $4
  i32.const 0
  get_local $4
  i32.load
  get_local $3
  call $~lib/datastream/DataStream#constructor
  set_local $5
  get_local $2
  get_local $5
  call $contract/lib/random.lib/Waiter#serialize
  get_local $2
  call $contract/lib/random.lib/Waiter#primaryKey
  set_local $6
  get_local $0
  i64.load offset=16
  get_local $0
  i64.load
  get_local $1
  get_local $6
  get_local $5
  i32.load
  get_local $5
  i32.load offset=8
  call $~lib/env/db_store_i64
  drop
 )
 (func $~lib/dbmanager/DBManager<RandRecord>#find (; 73 ;) (type $iIi) (param $0 i32) (param $1 i64) (result i32)
  (local $2 i32)
  get_local $0
  i64.load offset=8
  get_local $0
  i64.load offset=16
  get_local $0
  i64.load
  get_local $1
  call $~lib/env/db_find_i64
  set_local $2
  get_local $2
 )
 (func $~lib/dbmanager/DBManager<RandRecord>#exists (; 74 ;) (type $iIi) (param $0 i32) (param $1 i64) (result i32)
  (local $2 i32)
  get_local $0
  get_local $1
  call $~lib/dbmanager/DBManager<RandRecord>#find
  set_local $2
  get_local $2
  i32.const 0
  i32.lt_s
  if (result i32)
   i32.const 0
  else   
   i32.const 1
  end
 )
 (func $~lib/ultrain-ts-lib/lib/headblock/HeadBlock.get:number (; 75 ;) (type $i) (result i32)
  call $~lib/ultrain-ts-lib/lib/headblock/env.head_block_number
 )
 (func $~lib/ultrain-ts-lib/src/block/Block.get:number (; 76 ;) (type $i) (result i32)
  call $~lib/ultrain-ts-lib/lib/headblock/HeadBlock.get:number
 )
 (func $contract/lib/random.lib/RandRecord#serialize (; 77 ;) (type $iiv) (param $0 i32) (param $1 i32)
  get_local $1
  get_local $0
  i64.load
  call $~lib/datastream/DataStream#write<u64>
  get_local $1
  get_local $0
  i64.load offset=8
  call $~lib/datastream/DataStream#write<u64>
  get_local $1
  get_local $0
  i32.load offset=16
  call $~lib/datastream/DataStream#write<i32>
 )
 (func $~lib/datastream/DataStream.measure<RandRecord> (; 78 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  i32.const 0
  i32.const 0
  i32.const 0
  call $~lib/datastream/DataStream#constructor
  set_local $1
  get_local $0
  get_local $1
  call $contract/lib/random.lib/RandRecord#serialize
  get_local $1
  i32.load offset=8
 )
 (func $contract/lib/random.lib/RandRecord#primaryKey (; 79 ;) (type $iI) (param $0 i32) (result i64)
  get_local $0
  i64.load
 )
 (func $~lib/dbmanager/DBManager<RandRecord>#emplace (; 80 ;) (type $iIiv) (param $0 i32) (param $1 i64) (param $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i64)
  get_local $0
  i64.load offset=8
  call $~lib/env/current_receiver
  i64.eq
  i32.const 1760
  call $~lib/env/ultrain_assert
  get_local $2
  call $~lib/datastream/DataStream.measure<RandRecord>
  set_local $3
  i32.const 0
  get_local $3
  call $~lib/internal/typedarray/TypedArray<u8_u32>#constructor
  set_local $4
  i32.const 0
  get_local $4
  i32.load
  get_local $3
  call $~lib/datastream/DataStream#constructor
  set_local $5
  get_local $2
  get_local $5
  call $contract/lib/random.lib/RandRecord#serialize
  get_local $2
  call $contract/lib/random.lib/RandRecord#primaryKey
  set_local $6
  get_local $0
  i64.load offset=16
  get_local $0
  i64.load
  get_local $1
  get_local $6
  get_local $5
  i32.load
  get_local $5
  i32.load offset=8
  call $~lib/env/db_store_i64
  drop
 )
 (func $~lib/ultrain-ts-lib/src/crypto/Crypto#_Crypto_super (; 81 ;) (type $iiv) (param $0 i32) (param $1 i32)
  get_local $0
  i32.const 0
  get_local $1
  call $~lib/internal/typedarray/TypedArray<u8_u32>#constructor
  i32.store
 )
 (func $~lib/ultrain-ts-lib/src/crypto/SHA256#constructor (; 82 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  get_local $0
  drop
  i32.const 32
  drop
  get_local $0
  if (result i32)
   get_local $0
  else   
   block (result i32)
    i32.const 4
    call $~lib/memory/memory.allocate
    set_local $1
    get_local $1
    i32.const 0
    i32.store
    get_local $1
   end
   tee_local $0
  end
  tee_local $0
  i32.const 32
  call $~lib/ultrain-ts-lib/src/crypto/Crypto#_Crypto_super
  get_local $0
 )
 (func $~lib/array/Array<String>#__get (; 83 ;) (type $iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  get_local $0
  i32.load
  set_local $2
  get_local $1
  get_local $2
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
 (func $~lib/internal/string/allocateUnsafe (; 84 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  get_local $0
  i32.const 0
  i32.gt_s
  tee_local $1
  if (result i32)
   get_local $0
   get_global $~lib/internal/string/MAX_LENGTH
   i32.le_s
  else   
   get_local $1
  end
  i32.eqz
  if
   call $~lib/env/abort
   unreachable
  end
  block $~lib/memory/memory.allocate|inlined.1 (result i32)
   get_global $~lib/internal/string/HEADER_SIZE
   get_local $0
   i32.const 1
   i32.shl
   i32.add
   set_local $1
   get_local $1
   call $~lib/allocator/arena/__memory_allocate
   br $~lib/memory/memory.allocate|inlined.1
  end
  set_local $2
  get_local $2
  get_local $0
  i32.store
  get_local $2
 )
 (func $~lib/internal/string/copyUnsafe (; 85 ;) (type $iiiiiv) (param $0 i32) (param $1 i32) (param $2 i32) (param $3 i32) (param $4 i32)
  (local $5 i32)
  (local $6 i32)
  (local $7 i32)
  get_local $0
  get_local $1
  i32.const 1
  i32.shl
  i32.add
  get_global $~lib/internal/string/HEADER_SIZE
  i32.add
  set_local $5
  get_local $2
  get_local $3
  i32.const 1
  i32.shl
  i32.add
  get_global $~lib/internal/string/HEADER_SIZE
  i32.add
  set_local $6
  get_local $4
  i32.const 1
  i32.shl
  set_local $7
  get_local $5
  get_local $6
  get_local $7
  call $~lib/internal/memory/memmove
 )
 (func $~lib/string/String#concat (; 86 ;) (type $iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  get_local $0
  i32.const 0
  i32.ne
  i32.eqz
  if
   call $~lib/env/abort
   unreachable
  end
  get_local $1
  i32.const 0
  i32.eq
  if
   i32.const 1936
   set_local $1
  end
  get_local $0
  i32.load
  set_local $2
  get_local $1
  i32.load
  set_local $3
  get_local $2
  get_local $3
  i32.add
  set_local $4
  get_local $4
  i32.const 0
  i32.eq
  if
   i32.const 1952
   return
  end
  get_local $4
  call $~lib/internal/string/allocateUnsafe
  set_local $5
  get_local $5
  i32.const 0
  get_local $0
  i32.const 0
  get_local $2
  call $~lib/internal/string/copyUnsafe
  get_local $5
  get_local $2
  get_local $1
  i32.const 0
  get_local $3
  call $~lib/internal/string/copyUnsafe
  get_local $5
 )
 (func $~lib/string/String.__concat (; 87 ;) (type $iii) (param $0 i32) (param $1 i32) (result i32)
  get_local $0
  i32.eqz
  if
   i32.const 1936
   set_local $0
  end
  get_local $0
  get_local $1
  call $~lib/string/String#concat
 )
 (func $~lib/internal/string/repeatUnsafe (; 88 ;) (type $iiiiv) (param $0 i32) (param $1 i32) (param $2 i32) (param $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  (local $7 i32)
  (local $8 i32)
  (local $9 i64)
  (local $10 i32)
  (local $11 i32)
  get_local $2
  i32.load
  set_local $4
  block $break|0
   block $case5|0
    block $case4|0
     block $case3|0
      block $case2|0
       block $case1|0
        block $case0|0
         get_local $4
         set_local $5
         get_local $5
         i32.const 0
         i32.eq
         br_if $case0|0
         get_local $5
         i32.const 1
         i32.eq
         br_if $case1|0
         get_local $5
         i32.const 2
         i32.eq
         br_if $case2|0
         get_local $5
         i32.const 3
         i32.eq
         br_if $case3|0
         get_local $5
         i32.const 4
         i32.eq
         br_if $case4|0
         br $case5|0
        end
        br $break|0
       end
       block
        get_local $2
        i32.load16_u offset=4
        set_local $5
        get_local $0
        get_local $1
        i32.const 1
        i32.shl
        i32.add
        set_local $6
        block $break|1
         i32.const 0
         set_local $7
         loop $repeat|1
          get_local $7
          get_local $3
          i32.lt_s
          i32.eqz
          br_if $break|1
          get_local $6
          get_local $7
          i32.const 1
          i32.shl
          i32.add
          get_local $5
          i32.store16 offset=4
          get_local $7
          i32.const 1
          i32.add
          set_local $7
          br $repeat|1
          unreachable
         end
         unreachable
        end
        br $break|0
        unreachable
       end
       unreachable
      end
      block
       get_local $2
       i32.load offset=4
       set_local $6
       get_local $0
       get_local $1
       i32.const 1
       i32.shl
       i32.add
       set_local $5
       block $break|2
        i32.const 0
        set_local $7
        loop $repeat|2
         get_local $7
         get_local $3
         i32.lt_s
         i32.eqz
         br_if $break|2
         get_local $5
         get_local $7
         i32.const 2
         i32.shl
         i32.add
         get_local $6
         i32.store offset=4
         get_local $7
         i32.const 1
         i32.add
         set_local $7
         br $repeat|2
         unreachable
        end
        unreachable
       end
       br $break|0
       unreachable
      end
      unreachable
     end
     block
      get_local $2
      i32.load offset=4
      set_local $5
      get_local $2
      i32.load16_u offset=8
      set_local $6
      get_local $0
      get_local $1
      i32.const 1
      i32.shl
      i32.add
      set_local $7
      block $break|3
       i32.const 0
       set_local $8
       loop $repeat|3
        get_local $8
        get_local $3
        i32.lt_s
        i32.eqz
        br_if $break|3
        block
         get_local $7
         get_local $8
         i32.const 2
         i32.shl
         i32.add
         get_local $5
         i32.store offset=4
         get_local $7
         get_local $8
         i32.const 1
         i32.shl
         i32.add
         get_local $6
         i32.store16 offset=8
        end
        get_local $8
        i32.const 1
        i32.add
        set_local $8
        br $repeat|3
        unreachable
       end
       unreachable
      end
      br $break|0
      unreachable
     end
     unreachable
    end
    block
     get_local $2
     i64.load offset=4
     set_local $9
     get_local $0
     get_local $1
     i32.const 1
     i32.shl
     i32.add
     set_local $7
     block $break|4
      i32.const 0
      set_local $6
      loop $repeat|4
       get_local $6
       get_local $3
       i32.lt_s
       i32.eqz
       br_if $break|4
       get_local $7
       get_local $6
       i32.const 3
       i32.shl
       i32.add
       get_local $9
       i64.store offset=4
       get_local $6
       i32.const 1
       i32.add
       set_local $6
       br $repeat|4
       unreachable
      end
      unreachable
     end
     br $break|0
     unreachable
    end
    unreachable
   end
   block
    get_local $4
    i32.const 1
    i32.shl
    set_local $7
    get_local $0
    get_global $~lib/internal/string/HEADER_SIZE
    i32.add
    get_local $1
    i32.const 1
    i32.shl
    i32.add
    set_local $6
    get_local $2
    get_global $~lib/internal/string/HEADER_SIZE
    i32.add
    set_local $5
    block $break|5
     block
      i32.const 0
      set_local $8
      get_local $7
      get_local $3
      i32.mul
      set_local $10
     end
     loop $repeat|5
      get_local $8
      get_local $10
      i32.lt_s
      i32.eqz
      br_if $break|5
      block
       get_local $6
       get_local $8
       i32.add
       set_local $11
       get_local $11
       get_local $5
       get_local $7
       call $~lib/internal/memory/memmove
      end
      get_local $8
      get_local $7
      i32.add
      set_local $8
      br $repeat|5
      unreachable
     end
     unreachable
    end
    br $break|0
    unreachable
   end
   unreachable
  end
 )
 (func $~lib/string/String#repeat (; 89 ;) (type $iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  get_local $0
  i32.const 0
  i32.ne
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
   i32.const 1
   i32.const 28
   i32.shl
   i32.gt_s
  end
  if
   call $~lib/env/abort
   unreachable
  end
  get_local $1
  i32.const 0
  i32.eq
  tee_local $3
  if (result i32)
   get_local $3
  else   
   get_local $2
   i32.eqz
  end
  if
   i32.const 1952
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
  set_local $4
  get_local $4
  i32.const 0
  get_local $0
  get_local $1
  call $~lib/internal/string/repeatUnsafe
  get_local $4
 )
 (func $~lib/ultrain-ts-lib/src/utils/intToString (; 90 ;) (type $Iii) (param $0 i64) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i64)
  (local $4 i32)
  (local $5 i32)
  get_local $0
  i64.const 10
  i64.rem_u
  i32.wrap/i64
  set_local $2
  get_local $0
  i64.const 10
  i64.div_u
  set_local $3
  get_global $~lib/ultrain-ts-lib/src/utils/PrintableChar
  i32.const 16
  get_local $2
  i32.add
  call $~lib/array/Array<String>#__get
  set_local $4
  block $break|0
   loop $continue|0
    get_local $3
    i64.const 0
    i64.ne
    if
     block
      get_local $3
      i64.const 10
      i64.rem_u
      i32.wrap/i64
      set_local $2
      get_local $3
      i64.const 10
      i64.div_u
      set_local $3
      get_global $~lib/ultrain-ts-lib/src/utils/PrintableChar
      i32.const 16
      get_local $2
      i32.add
      call $~lib/array/Array<String>#__get
      get_local $4
      call $~lib/string/String.__concat
      set_local $4
     end
     br $continue|0
    end
   end
  end
  get_local $1
  i32.const 0
  i32.ne
  tee_local $5
  if (result i32)
   get_local $4
   i32.load
   get_local $1
   i32.lt_s
  else   
   get_local $5
  end
  if
   i32.const 136
   set_local $5
   get_local $5
   get_local $1
   get_local $4
   i32.load
   i32.sub
   call $~lib/string/String#repeat
   drop
   get_local $5
   get_local $4
   call $~lib/string/String.__concat
   set_local $4
  end
  get_local $4
 )
 (func $~lib/ultrain-ts-lib/src/crypto/Crypto#get:buffer (; 91 ;) (type $ii) (param $0 i32) (result i32)
  get_local $0
  i32.load
  i32.load
 )
 (func $~lib/ultrain-ts-lib/src/crypto/Crypto#get:bufferSize (; 92 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  get_local $0
  i32.load
  set_local $1
  get_local $1
  i32.load offset=8
  i32.const 0
  i32.shr_u
 )
 (func $~lib/string/String#charAt (; 93 ;) (type $iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  get_local $0
  i32.const 0
  i32.ne
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
   i32.const 1952
   return
  end
  i32.const 1
  call $~lib/internal/string/allocateUnsafe
  set_local $2
  get_local $2
  get_local $0
  get_local $1
  i32.const 1
  i32.shl
  i32.add
  i32.load16_u offset=4
  i32.store16 offset=4
  get_local $2
 )
 (func $~lib/ultrain-ts-lib/src/crypto/to_hex (; 94 ;) (type $iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  i32.const 1952
  set_local $2
  block $break|0
   i32.const 0
   set_local $3
   loop $repeat|0
    get_local $3
    get_local $1
    i32.lt_u
    i32.eqz
    br_if $break|0
    block
     get_local $0
     get_local $3
     i32.add
     i32.load8_u
     set_local $4
     get_local $4
     i32.const 255
     i32.and
     set_local $5
     get_local $2
     get_global $~lib/ultrain-ts-lib/src/crypto/HexDigital
     get_local $5
     i32.const 4
     i32.shr_u
     call $~lib/string/String#charAt
     call $~lib/string/String.__concat
     get_global $~lib/ultrain-ts-lib/src/crypto/HexDigital
     get_local $5
     i32.const 15
     i32.and
     call $~lib/string/String#charAt
     call $~lib/string/String.__concat
     set_local $2
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
 )
 (func $~lib/ultrain-ts-lib/src/crypto/Crypto#toString (; 95 ;) (type $ii) (param $0 i32) (result i32)
  get_local $0
  call $~lib/ultrain-ts-lib/src/crypto/Crypto#get:buffer
  get_local $0
  call $~lib/ultrain-ts-lib/src/crypto/Crypto#get:bufferSize
  call $~lib/ultrain-ts-lib/src/crypto/to_hex
 )
 (func $~lib/ultrain-ts-lib/src/crypto/SHA256#hash (; 96 ;) (type $iii) (param $0 i32) (param $1 i32) (result i32)
  get_local $1
  call $~lib/ultrain-ts-lib/src/utils/string2cstr
  get_local $1
  i32.load
  get_local $0
  call $~lib/ultrain-ts-lib/src/crypto/Crypto#get:buffer
  get_local $0
  call $~lib/ultrain-ts-lib/src/crypto/Crypto#get:bufferSize
  call $~lib/ultrain-ts-lib/internal/crypto.d/env.ts_sha256
  get_local $0
  call $~lib/ultrain-ts-lib/src/crypto/Crypto#toString
 )
 (func $~lib/string/String#substring (; 97 ;) (type $iiii) (param $0 i32) (param $1 i32) (param $2 i32) (result i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  (local $7 i32)
  (local $8 i32)
  (local $9 i32)
  (local $10 i32)
  get_local $0
  i32.const 0
  i32.ne
  i32.eqz
  if
   call $~lib/env/abort
   unreachable
  end
  get_local $0
  i32.load
  set_local $3
  get_local $1
  tee_local $4
  i32.const 0
  tee_local $5
  get_local $4
  get_local $5
  i32.gt_s
  select
  tee_local $4
  get_local $3
  tee_local $5
  get_local $4
  get_local $5
  i32.lt_s
  select
  set_local $6
  get_local $2
  tee_local $4
  i32.const 0
  tee_local $5
  get_local $4
  get_local $5
  i32.gt_s
  select
  tee_local $4
  get_local $3
  tee_local $5
  get_local $4
  get_local $5
  i32.lt_s
  select
  set_local $7
  get_local $6
  tee_local $4
  get_local $7
  tee_local $5
  get_local $4
  get_local $5
  i32.lt_s
  select
  set_local $8
  get_local $6
  tee_local $4
  get_local $7
  tee_local $5
  get_local $4
  get_local $5
  i32.gt_s
  select
  set_local $9
  get_local $9
  get_local $8
  i32.sub
  set_local $3
  get_local $3
  i32.eqz
  if
   i32.const 1952
   return
  end
  get_local $8
  i32.eqz
  tee_local $4
  if (result i32)
   get_local $9
   get_local $0
   i32.load
   i32.eq
  else   
   get_local $4
  end
  if
   get_local $0
   return
  end
  get_local $3
  call $~lib/internal/string/allocateUnsafe
  set_local $10
  get_local $10
  i32.const 0
  get_local $0
  get_local $8
  get_local $3
  call $~lib/internal/string/copyUnsafe
  get_local $10
 )
 (func $~lib/internal/string/parse<f64> (; 98 ;) (type $iiF) (param $0 i32) (param $1 i32) (result f64)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 f64)
  (local $6 i32)
  (local $7 f64)
  get_local $0
  i32.load
  set_local $2
  get_local $2
  i32.eqz
  if
   f64.const nan:0x8000000000000
   return
  end
  get_local $0
  set_local $3
  get_local $3
  i32.load16_u offset=4
  set_local $4
  get_local $4
  get_global $~lib/internal/string/CharCode.MINUS
  i32.eq
  if
   get_local $2
   i32.const 1
   i32.sub
   tee_local $2
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
   set_local $4
   f64.const -1
   set_local $5
  else   
   get_local $4
   get_global $~lib/internal/string/CharCode.PLUS
   i32.eq
   if
    get_local $2
    i32.const 1
    i32.sub
    tee_local $2
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
    set_local $4
    f64.const 1
    set_local $5
   else    
    f64.const 1
    set_local $5
   end
  end
  get_local $1
  i32.eqz
  if
   get_local $4
   get_global $~lib/internal/string/CharCode._0
   i32.eq
   tee_local $6
   if (result i32)
    get_local $2
    i32.const 2
    i32.gt_s
   else    
    get_local $6
   end
   if
    block $break|0
     block $case6|0
      block $case5|0
       block $case4|0
        block $case3|0
         block $case2|0
          block $case1|0
           block $case0|0
            get_local $3
            i32.const 2
            i32.add
            i32.load16_u offset=4
            set_local $6
            get_local $6
            get_global $~lib/internal/string/CharCode.B
            i32.eq
            br_if $case0|0
            get_local $6
            get_global $~lib/internal/string/CharCode.b
            i32.eq
            br_if $case1|0
            get_local $6
            get_global $~lib/internal/string/CharCode.O
            i32.eq
            br_if $case2|0
            get_local $6
            get_global $~lib/internal/string/CharCode.o
            i32.eq
            br_if $case3|0
            get_local $6
            get_global $~lib/internal/string/CharCode.X
            i32.eq
            br_if $case4|0
            get_local $6
            get_global $~lib/internal/string/CharCode.x
            i32.eq
            br_if $case5|0
            br $case6|0
           end
          end
          block
           get_local $3
           i32.const 4
           i32.add
           set_local $3
           get_local $2
           i32.const 2
           i32.sub
           set_local $2
           i32.const 2
           set_local $1
           br $break|0
           unreachable
          end
          unreachable
         end
        end
        block
         get_local $3
         i32.const 4
         i32.add
         set_local $3
         get_local $2
         i32.const 2
         i32.sub
         set_local $2
         i32.const 8
         set_local $1
         br $break|0
         unreachable
        end
        unreachable
       end
      end
      block
       get_local $3
       i32.const 4
       i32.add
       set_local $3
       get_local $2
       i32.const 2
       i32.sub
       set_local $2
       i32.const 16
       set_local $1
       br $break|0
       unreachable
      end
      unreachable
     end
     i32.const 10
     set_local $1
    end
   else    
    i32.const 10
    set_local $1
   end
  else   
   get_local $1
   i32.const 2
   i32.lt_s
   tee_local $6
   if (result i32)
    get_local $6
   else    
    get_local $1
    i32.const 36
    i32.gt_s
   end
   if
    f64.const nan:0x8000000000000
    return
   end
  end
  f64.const 0
  set_local $7
  block $break|1
   loop $continue|1
    block (result i32)
     get_local $2
     tee_local $6
     i32.const 1
     i32.sub
     set_local $2
     get_local $6
    end
    if
     block
      get_local $3
      i32.load16_u offset=4
      set_local $4
      get_local $4
      get_global $~lib/internal/string/CharCode._0
      i32.ge_s
      tee_local $6
      if (result i32)
       get_local $4
       get_global $~lib/internal/string/CharCode._9
       i32.le_s
      else       
       get_local $6
      end
      if
       get_local $4
       get_global $~lib/internal/string/CharCode._0
       i32.sub
       set_local $4
      else       
       get_local $4
       get_global $~lib/internal/string/CharCode.A
       i32.ge_s
       tee_local $6
       if (result i32)
        get_local $4
        get_global $~lib/internal/string/CharCode.Z
        i32.le_s
       else        
        get_local $6
       end
       if
        get_local $4
        get_global $~lib/internal/string/CharCode.A
        i32.const 10
        i32.sub
        i32.sub
        set_local $4
       else        
        get_local $4
        get_global $~lib/internal/string/CharCode.a
        i32.ge_s
        tee_local $6
        if (result i32)
         get_local $4
         get_global $~lib/internal/string/CharCode.z
         i32.le_s
        else         
         get_local $6
        end
        if
         get_local $4
         get_global $~lib/internal/string/CharCode.a
         i32.const 10
         i32.sub
         i32.sub
         set_local $4
        else         
         br $break|1
        end
       end
      end
      get_local $4
      get_local $1
      i32.ge_s
      if
       br $break|1
      end
      get_local $7
      get_local $1
      f64.convert_s/i32
      f64.mul
      get_local $4
      f64.convert_s/i32
      f64.add
      set_local $7
      get_local $3
      i32.const 2
      i32.add
      set_local $3
     end
     br $continue|1
    end
   end
  end
  get_local $5
  get_local $7
  f64.mul
 )
 (func $~lib/string/parseInt (; 99 ;) (type $iiF) (param $0 i32) (param $1 i32) (result f64)
  get_local $0
  get_local $1
  call $~lib/internal/string/parse<f64>
 )
 (func $contract/ultrainio.rand/RandContract#hash (; 100 ;) (type $iII) (param $0 i32) (param $1 i64) (result i64)
  (local $2 i32)
  (local $3 i32)
  i32.const 0
  call $~lib/ultrain-ts-lib/src/crypto/SHA256#constructor
  set_local $2
  get_local $2
  get_local $1
  i32.const 0
  call $~lib/ultrain-ts-lib/src/utils/intToString
  call $~lib/ultrain-ts-lib/src/crypto/SHA256#hash
  set_local $3
  get_local $3
  i32.const 0
  i32.const 14
  call $~lib/string/String#substring
  i32.const 16
  call $~lib/string/parseInt
  i64.trunc_u/f64
 )
 (func $contract/ultrainio.rand/RandContract#constructor (; 101 ;) (type $iIi) (param $0 i32) (param $1 i64) (result i32)
  (local $2 i32)
  (local $3 i32)
  get_local $0
  drop
  get_local $1
  drop
  get_local $0
  if (result i32)
   get_local $0
  else   
   block (result i32)
    i32.const 28
    call $~lib/memory/memory.allocate
    set_local $2
    get_local $2
    i64.const 0
    i64.store
    get_local $2
    i32.const 0
    i32.store offset=8
    get_local $2
    i32.const 0
    i32.store offset=12
    get_local $2
    i32.const 0
    i32.store offset=16
    get_local $2
    i32.const 0
    i32.store offset=20
    get_local $2
    i32.const 0
    i32.store offset=24
    get_local $2
   end
   tee_local $0
  end
  tee_local $0
  get_local $1
  call $~lib/ultrain-ts-lib/src/contract/Contract#_Contract_super
  get_local $0
  i32.const 0
  call $contract/lib/random.lib/Random#constructor
  i32.store offset=24
  get_local $0
  get_local $0
  i32.load offset=24
  i32.load
  i32.store offset=12
  get_local $0
  get_local $0
  i32.load offset=24
  i32.load offset=4
  i32.store offset=16
  get_local $0
  get_local $0
  i32.load offset=24
  i32.load offset=8
  i32.store offset=20
  get_local $0
  i32.load offset=12
  get_global $contract/lib/random.lib/RAND_KEY
  call $~lib/dbmanager/DBManager<Voter>#exists
  i32.eqz
  if
   i32.const 0
   call $contract/lib/random.lib/Voter#constructor
   set_local $2
   get_local $2
   get_global $contract/lib/random.lib/RAND_KEY
   i64.store
   get_local $0
   i32.load offset=12
   call $~lib/ultrain-ts-lib/src/action/Action.get:sender
   get_local $2
   call $~lib/dbmanager/DBManager<Voter>#emplace
  end
  get_local $0
  i32.load offset=12
  get_global $contract/ultrainio.rand/MAIN_COUNT_KEY
  call $~lib/dbmanager/DBManager<Voter>#exists
  i32.eqz
  if
   i32.const 0
   call $contract/lib/random.lib/Voter#constructor
   set_local $2
   get_local $2
   get_global $contract/ultrainio.rand/MAIN_COUNT_KEY
   i64.store
   get_local $0
   i32.load offset=12
   call $~lib/ultrain-ts-lib/src/action/Action.get:sender
   get_local $2
   call $~lib/dbmanager/DBManager<Voter>#emplace
  end
  get_local $0
  i32.load offset=16
  i64.const 0
  call $~lib/dbmanager/DBManager<Waiter>#exists
  i32.eqz
  if
   block (result i32)
    i32.const 4
    call $~lib/memory/memory.allocate
    set_local $2
    get_local $2
    i32.const 0
    i32.const 0
    call $~lib/array/Array<u64>#constructor
    i32.store
    get_local $2
   end
   set_local $2
   get_local $0
   i32.load offset=16
   call $~lib/ultrain-ts-lib/src/action/Action.get:sender
   get_local $2
   call $~lib/dbmanager/DBManager<Waiter>#emplace
  end
  get_local $0
  i32.load offset=20
  i64.const 0
  call $~lib/dbmanager/DBManager<RandRecord>#exists
  i32.eqz
  if
   block (result i32)
    i32.const 20
    call $~lib/memory/memory.allocate
    set_local $2
    get_local $2
    i64.const 0
    i64.store
    get_local $2
    i64.const 0
    i64.store offset=8
    get_local $2
    i32.const 0
    i32.store offset=16
    get_local $2
   end
   set_local $2
   call $~lib/ultrain-ts-lib/src/block/Block.get:number
   i32.const 2
   i32.lt_u
   if (result i32)
    i32.const 2
   else    
    call $~lib/ultrain-ts-lib/src/block/Block.get:number
   end
   set_local $3
   get_local $2
   i64.const 0
   i64.store
   get_local $2
   get_local $3
   i64.extend_s/i32
   i64.store offset=8
   get_local $0
   i32.load offset=20
   call $~lib/ultrain-ts-lib/src/action/Action.get:sender
   get_local $2
   call $~lib/dbmanager/DBManager<RandRecord>#emplace
   get_local $2
   i64.const 1
   i64.store
   get_local $2
   get_local $3
   i64.extend_s/i32
   i64.store offset=8
   get_local $0
   i32.load offset=20
   call $~lib/ultrain-ts-lib/src/action/Action.get:sender
   get_local $2
   call $~lib/dbmanager/DBManager<RandRecord>#emplace
   get_local $2
   get_local $3
   i64.extend_s/i32
   i64.store
   get_local $2
   get_local $0
   get_local $3
   i64.extend_s/i32
   call $contract/ultrainio.rand/RandContract#hash
   i64.store offset=8
   get_local $2
   i32.const 7
   i32.store offset=16
   get_local $0
   i32.load offset=20
   call $~lib/ultrain-ts-lib/src/action/Action.get:sender
   get_local $2
   call $~lib/dbmanager/DBManager<RandRecord>#emplace
  end
  get_local $0
 )
 (func $~lib/ultrain-ts-lib/lib/name_ex/NameEx#constructor (; 102 ;) (type $iIIi) (param $0 i32) (param $1 i64) (param $2 i64) (result i32)
  (local $3 i32)
  get_local $0
  if (result i32)
   get_local $0
  else   
   block (result i32)
    i32.const 16
    call $~lib/memory/memory.allocate
    set_local $3
    get_local $3
    i64.const 0
    i64.store
    get_local $3
    i64.const 0
    i64.store offset=8
    get_local $3
   end
   tee_local $0
  end
  tee_local $0
  get_local $1
  i64.store
  get_local $0
  get_local $2
  i64.store offset=8
  get_local $0
 )
 (func $~lib/ultrain-ts-lib/src/contract/Contract#setActionName (; 103 ;) (type $iIIv) (param $0 i32) (param $1 i64) (param $2 i64)
  get_local $0
  i32.const 0
  get_local $1
  get_local $2
  call $~lib/ultrain-ts-lib/lib/name_ex/NameEx#constructor
  i32.store offset=8
 )
 (func $~lib/ultrain-ts-lib/src/contract/Contract#get:receiver (; 104 ;) (type $iI) (param $0 i32) (result i64)
  get_local $0
  i64.load
 )
 (func $~lib/ultrain-ts-lib/src/contract/Contract#get:action (; 105 ;) (type $ii) (param $0 i32) (result i32)
  get_local $0
  i32.load offset=8
 )
 (func $~lib/ultrain-ts-lib/lib/name_ex/char_to_symbol_ex (; 106 ;) (type $iI) (param $0 i32) (result i64)
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
   i32.const 48
   i32.sub
   i32.const 2
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
   i32.const 97
   i32.sub
   i32.const 12
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
   i32.const 65
   i32.sub
   i32.const 38
   i32.add
   i32.const 255
   i32.and
   i64.extend_u/i32
   return
  end
  i64.const 255
 )
 (func $~lib/ultrain-ts-lib/lib/name_ex/NEX (; 107 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i64)
  (local $3 i32)
  (local $4 i32)
  (local $5 i64)
  (local $6 i64)
  (local $7 i64)
  i32.const 0
  i64.const 0
  i64.const 0
  call $~lib/ultrain-ts-lib/lib/name_ex/NameEx#constructor
  set_local $1
  i64.const 0
  set_local $2
  get_local $0
  i32.load
  set_local $3
  block $break|0
   i32.const 0
   set_local $4
   loop $repeat|0
    get_local $4
    get_local $3
    i32.lt_s
    i32.eqz
    br_if $break|0
    block
     get_local $0
     get_local $4
     call $~lib/string/String#charCodeAt
     call $~lib/ultrain-ts-lib/lib/name_ex/char_to_symbol_ex
     set_local $5
     get_local $4
     i32.const 9
     i32.le_s
     if
      get_local $2
      get_local $5
      i64.const 6
      get_local $4
      i64.extend_s/i32
      i64.mul
      i64.shl
      i64.or
      set_local $2
     else      
      get_local $4
      i32.const 10
      i32.eq
      if
       get_local $5
       i64.const 15
       i64.and
       set_local $6
       get_local $2
       get_local $6
       i64.const 6
       get_local $4
       i64.extend_s/i32
       i64.mul
       i64.shl
       i64.or
       set_local $2
       get_local $1
       get_local $2
       i64.store offset=8
       get_local $5
       i64.const 48
       i64.and
       i64.const 4
       i64.shr_u
       set_local $7
       get_local $7
       set_local $2
      else       
       get_local $2
       get_local $5
       i64.const 6
       get_local $4
       i32.const 11
       i32.sub
       i64.extend_s/i32
       i64.mul
       i64.const 2
       i64.add
       i64.shl
       i64.or
       set_local $2
      end
     end
     get_local $3
     i32.const 10
     i32.le_s
     if
      get_local $1
      get_local $2
      i64.store offset=8
     else      
      get_local $1
      get_local $2
      i64.store
     end
    end
    get_local $4
    i32.const 1
    i32.add
    set_local $4
    br $repeat|0
    unreachable
   end
   unreachable
  end
  get_local $1
 )
 (func $~lib/ultrain-ts-lib/lib/name_ex/NameEx._neq (; 108 ;) (type $iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  get_local $0
  i64.load
  get_local $1
  i64.load
  i64.ne
  tee_local $2
  if (result i32)
   get_local $2
  else   
   get_local $0
   i64.load offset=8
   get_local $1
   i64.load offset=8
   i64.ne
  end
 )
 (func $~lib/ultrain-ts-lib/lib/name_ex/NameEx._eq (; 109 ;) (type $iii) (param $0 i32) (param $1 i32) (result i32)
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
 (func $~lib/ultrain-ts-lib/src/contract/Contract.filterAcceptTransferTokenAction (; 110 ;) (type $IIii) (param $0 i64) (param $1 i64) (param $2 i32) (result i32)
  (local $3 i32)
  get_local $1
  get_local $0
  i64.eq
  tee_local $3
  if (result i32)
   get_local $2
   i32.const 2016
   call $~lib/ultrain-ts-lib/lib/name_ex/NEX
   call $~lib/ultrain-ts-lib/lib/name_ex/NameEx._neq
  else   
   get_local $3
  end
  tee_local $3
  if (result i32)
   get_local $3
  else   
   get_local $1
   i32.const 2040
   call $~lib/ultrain-ts-lib/src/account/NAME
   i64.eq
   tee_local $3
   if (result i32)
    get_local $2
    i32.const 2016
    call $~lib/ultrain-ts-lib/lib/name_ex/NEX
    call $~lib/ultrain-ts-lib/lib/name_ex/NameEx._eq
   else    
    get_local $3
   end
  end
 )
 (func $contract/ultrainio.rand/RandContract#filterAction (; 111 ;) (type $iIi) (param $0 i32) (param $1 i64) (result i32)
  get_local $0
  call $~lib/ultrain-ts-lib/src/contract/Contract#get:receiver
  get_local $1
  get_local $0
  call $~lib/ultrain-ts-lib/src/contract/Contract#get:action
  call $~lib/ultrain-ts-lib/src/contract/Contract.filterAcceptTransferTokenAction
 )
 (func $~lib/ultrain-ts-lib/src/contract/Contract#onInit (; 112 ;) (type $iv) (param $0 i32)
  nop
 )
 (func $~lib/ultrain-ts-lib/src/contract/DataStreamFromCurrentAction (; 113 ;) (type $i) (result i32)
  (local $0 i32)
  (local $1 i32)
  (local $2 i32)
  call $~lib/ultrain-ts-lib/internal/action.d/env.action_data_size
  set_local $0
  i32.const 0
  get_local $0
  call $~lib/internal/typedarray/TypedArray<u8_u32>#constructor
  set_local $1
  get_local $1
  i32.load
  get_local $0
  call $~lib/ultrain-ts-lib/internal/action.d/env.read_action_data
  drop
  i32.const 0
  get_local $1
  i32.load
  get_local $0
  call $~lib/datastream/DataStream#constructor
  set_local $2
  get_local $2
 )
 (func $~lib/ultrain-ts-lib/src/contract/Contract#getDataStream (; 114 ;) (type $ii) (param $0 i32) (result i32)
  call $~lib/ultrain-ts-lib/src/contract/DataStreamFromCurrentAction
 )
 (func $~lib/ultrain-ts-lib/src/contract/Contract#isAction (; 115 ;) (type $iii) (param $0 i32) (param $1 i32) (result i32)
  get_local $0
  i32.load offset=8
  get_local $1
  call $~lib/ultrain-ts-lib/lib/name_ex/NEX
  call $~lib/ultrain-ts-lib/lib/name_ex/NameEx._eq
 )
 (func $~lib/datastream/DataStream#read<u64> (; 116 ;) (type $iI) (param $0 i32) (result i64)
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
 (func $~lib/ultrain-ts-lib/src/asset/Asset#deserialize (; 117 ;) (type $iiv) (param $0 i32) (param $1 i32)
  get_local $0
  get_local $1
  call $~lib/datastream/DataStream#read<u64>
  i64.store
  get_local $0
  get_local $1
  call $~lib/datastream/DataStream#read<u64>
  i64.store offset=8
 )
 (func $~lib/datastream/DataStream#read<u8> (; 118 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  get_local $0
  i32.load
  get_local $0
  i32.load offset=8
  i32.add
  i32.load8_u
  set_local $1
  get_local $0
  get_local $0
  i32.load offset=8
  i32.const 1
  i32.add
  i32.store offset=8
  get_local $1
 )
 (func $~lib/datastream/DataStream#readVarint32 (; 119 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  i32.const 0
  set_local $1
  i32.const 0
  set_local $2
  block $break|0
   loop $continue|0
    block
     get_local $0
     call $~lib/datastream/DataStream#read<u8>
     set_local $3
     get_local $1
     get_local $3
     i32.const 127
     i32.and
     i32.const 7
     block (result i32)
      get_local $2
      tee_local $4
      i32.const 1
      i32.add
      set_local $2
      get_local $4
     end
     i32.mul
     i32.shl
     i32.or
     set_local $1
    end
    get_local $3
    i32.const 128
    i32.and
    br_if $continue|0
   end
  end
  get_local $1
 )
 (func $~lib/allocator/arena/__memory_free (; 120 ;) (type $iv) (param $0 i32)
  nop
 )
 (func $~lib/string/String.fromUTF8 (; 121 ;) (type $iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  (local $7 i32)
  get_local $1
  i32.const 1
  i32.lt_u
  if
   i32.const 1952
   return
  end
  i32.const 0
  set_local $2
  block $~lib/memory/memory.allocate|inlined.2 (result i32)
   get_local $1
   i32.const 1
   i32.shl
   set_local $3
   get_local $3
   call $~lib/allocator/arena/__memory_allocate
   br $~lib/memory/memory.allocate|inlined.2
  end
  set_local $4
  i32.const 0
  set_local $5
  block $break|0
   loop $continue|0
    get_local $2
    get_local $1
    i32.lt_u
    if
     block
      get_local $0
      block (result i32)
       get_local $2
       tee_local $3
       i32.const 1
       i32.add
       set_local $2
       get_local $3
      end
      i32.add
      i32.load8_u
      set_local $3
      get_local $3
      i32.const 128
      i32.lt_u
      if
       get_local $4
       get_local $5
       i32.add
       get_local $3
       i32.store16
       get_local $5
       i32.const 2
       i32.add
       set_local $5
      else       
       get_local $3
       i32.const 191
       i32.gt_u
       tee_local $6
       if (result i32)
        get_local $3
        i32.const 224
        i32.lt_u
       else        
        get_local $6
       end
       if
        get_local $2
        i32.const 1
        i32.add
        get_local $1
        i32.le_u
        i32.eqz
        if
         call $~lib/env/abort
         unreachable
        end
        get_local $4
        get_local $5
        i32.add
        get_local $3
        i32.const 31
        i32.and
        i32.const 6
        i32.shl
        get_local $0
        block (result i32)
         get_local $2
         tee_local $6
         i32.const 1
         i32.add
         set_local $2
         get_local $6
        end
        i32.add
        i32.load8_u
        i32.const 63
        i32.and
        i32.or
        i32.store16
        get_local $5
        i32.const 2
        i32.add
        set_local $5
       else        
        get_local $3
        i32.const 239
        i32.gt_u
        tee_local $6
        if (result i32)
         get_local $3
         i32.const 365
         i32.lt_u
        else         
         get_local $6
        end
        if
         get_local $2
         i32.const 3
         i32.add
         get_local $1
         i32.le_u
         i32.eqz
         if
          call $~lib/env/abort
          unreachable
         end
         get_local $3
         i32.const 7
         i32.and
         i32.const 18
         i32.shl
         get_local $0
         block (result i32)
          get_local $2
          tee_local $6
          i32.const 1
          i32.add
          set_local $2
          get_local $6
         end
         i32.add
         i32.load8_u
         i32.const 63
         i32.and
         i32.const 12
         i32.shl
         i32.or
         get_local $0
         block (result i32)
          get_local $2
          tee_local $6
          i32.const 1
          i32.add
          set_local $2
          get_local $6
         end
         i32.add
         i32.load8_u
         i32.const 63
         i32.and
         i32.const 6
         i32.shl
         i32.or
         get_local $0
         block (result i32)
          get_local $2
          tee_local $6
          i32.const 1
          i32.add
          set_local $2
          get_local $6
         end
         i32.add
         i32.load8_u
         i32.const 63
         i32.and
         i32.or
         i32.const 65536
         i32.sub
         set_local $3
         get_local $4
         get_local $5
         i32.add
         i32.const 55296
         get_local $3
         i32.const 10
         i32.shr_u
         i32.add
         i32.store16
         get_local $5
         i32.const 2
         i32.add
         set_local $5
         get_local $4
         get_local $5
         i32.add
         i32.const 56320
         get_local $3
         i32.const 1023
         i32.and
         i32.add
         i32.store16
         get_local $5
         i32.const 2
         i32.add
         set_local $5
        else         
         get_local $2
         i32.const 2
         i32.add
         get_local $1
         i32.le_u
         i32.eqz
         if
          call $~lib/env/abort
          unreachable
         end
         get_local $4
         get_local $5
         i32.add
         get_local $3
         i32.const 15
         i32.and
         i32.const 12
         i32.shl
         get_local $0
         block (result i32)
          get_local $2
          tee_local $6
          i32.const 1
          i32.add
          set_local $2
          get_local $6
         end
         i32.add
         i32.load8_u
         i32.const 63
         i32.and
         i32.const 6
         i32.shl
         i32.or
         get_local $0
         block (result i32)
          get_local $2
          tee_local $6
          i32.const 1
          i32.add
          set_local $2
          get_local $6
         end
         i32.add
         i32.load8_u
         i32.const 63
         i32.and
         i32.or
         i32.store16
         get_local $5
         i32.const 2
         i32.add
         set_local $5
        end
       end
      end
     end
     br $continue|0
    end
   end
  end
  get_local $2
  get_local $1
  i32.eq
  i32.eqz
  if
   call $~lib/env/abort
   unreachable
  end
  get_local $5
  i32.const 1
  i32.shr_u
  call $~lib/internal/string/allocateUnsafe
  set_local $7
  get_local $7
  get_global $~lib/internal/string/HEADER_SIZE
  i32.add
  set_local $3
  get_local $3
  get_local $4
  get_local $5
  call $~lib/internal/memory/memmove
  block $~lib/memory/memory.free|inlined.0
   block
    get_local $4
    call $~lib/allocator/arena/__memory_free
    br $~lib/memory/memory.free|inlined.0
    unreachable
   end
   unreachable
  end
  get_local $7
 )
 (func $~lib/datastream/DataStream#readString (; 122 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  get_local $0
  call $~lib/datastream/DataStream#readVarint32
  set_local $1
  get_local $1
  i32.const 0
  i32.eq
  if
   i32.const 1952
   return
  end
  i32.const 0
  get_local $1
  call $~lib/internal/typedarray/TypedArray<u8_u32>#constructor
  set_local $2
  get_local $2
  i32.load
  set_local $3
  get_local $0
  i32.load
  get_local $0
  i32.load offset=8
  i32.add
  set_local $4
  get_local $3
  get_local $4
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
 (func $~lib/array/Array<u8>#__set (; 123 ;) (type $iiiv) (param $0 i32) (param $1 i32) (param $2 i32)
  (local $3 i32)
  (local $4 i32)
  get_local $0
  i32.load
  set_local $3
  get_local $3
  i32.load
  i32.const 0
  i32.shr_u
  set_local $4
  get_local $1
  get_local $4
  i32.ge_u
  if
   get_local $1
   i32.const 1073741816
   i32.ge_u
   if
    call $~lib/env/abort
    unreachable
   end
   get_local $3
   get_local $1
   i32.const 1
   i32.add
   i32.const 0
   i32.shl
   call $~lib/internal/arraybuffer/reallocateUnsafe
   set_local $3
   get_local $0
   get_local $3
   i32.store
   get_local $0
   get_local $1
   i32.const 1
   i32.add
   i32.store offset=4
  end
  get_local $3
  get_local $1
  i32.const 0
  i32.shl
  i32.add
  get_local $2
  i32.store8 offset=8
 )
 (func $~lib/array/Array<u8>#__get (; 124 ;) (type $iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  get_local $0
  i32.load
  set_local $2
  get_local $1
  get_local $2
  i32.load
  i32.const 0
  i32.shr_u
  i32.lt_u
  if (result i32)
   get_local $2
   get_local $1
   i32.const 0
   i32.shl
   i32.add
   i32.load8_u offset=8
  else   
   unreachable
  end
 )
 (func $~lib/ultrain-ts-lib/lib/name/RN (; 125 ;) (type $Ii) (param $0 i64) (result i32)
  (local $1 i32)
  (local $2 i64)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  get_local $0
  i64.const 0
  i64.eq
  if
   i32.const 136
   return
  end
  i32.const 2208
  set_local $1
  get_local $0
  set_local $2
  block $break|0
   i32.const 0
   set_local $3
   loop $repeat|0
    get_local $3
    i32.const 12
    i32.le_u
    i32.eqz
    br_if $break|0
    block
     get_local $2
     get_local $3
     i32.const 0
     i32.eq
     if (result i64)
      i64.const 15
     else      
      i64.const 31
     end
     i64.and
     i32.wrap/i64
     set_local $4
     i32.const 2104
     get_local $4
     call $~lib/string/String#charCodeAt
     i32.const 255
     i32.and
     set_local $5
     get_local $1
     i32.const 12
     get_local $3
     i32.sub
     get_local $5
     call $~lib/array/Array<u8>#__set
     get_local $2
     get_local $3
     i32.const 0
     i32.eq
     if (result i64)
      i64.const 4
     else      
      i64.const 5
     end
     i64.shr_u
     set_local $2
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
  i32.const 1952
  set_local $3
  i32.const 1
  set_local $5
  block $break|1
   i32.const 12
   set_local $4
   loop $repeat|1
    get_local $4
    i32.const 0
    i32.ge_s
    i32.eqz
    br_if $break|1
    get_local $1
    get_local $4
    call $~lib/array/Array<u8>#__get
    i32.const 255
    i32.and
    i32.const 46
    i32.eq
    tee_local $6
    if (result i32)
     get_local $5
    else     
     get_local $6
    end
    if
     nop
    else     
     i32.const 0
     set_local $5
     get_local $1
     get_local $4
     call $~lib/array/Array<u8>#__get
     i32.const 32
     i32.sub
     set_local $6
     get_global $~lib/ultrain-ts-lib/src/utils/PrintableChar
     get_local $6
     i32.const 255
     i32.and
     call $~lib/array/Array<String>#__get
     get_local $3
     call $~lib/string/String.__concat
     set_local $3
    end
    get_local $4
    i32.const 1
    i32.sub
    set_local $4
    br $repeat|1
    unreachable
   end
   unreachable
  end
  get_local $3
 )
 (func $~lib/ultrain-ts-lib/src/account/RNAME (; 126 ;) (type $Ii) (param $0 i64) (result i32)
  get_local $0
  call $~lib/ultrain-ts-lib/lib/name/RN
 )
 (func $~lib/internal/string/compareUnsafe (; 127 ;) (type $iiiiii) (param $0 i32) (param $1 i32) (param $2 i32) (param $3 i32) (param $4 i32) (result i32)
  (local $5 i32)
  (local $6 i32)
  (local $7 i32)
  i32.const 0
  set_local $5
  get_local $0
  get_local $1
  i32.const 1
  i32.shl
  i32.add
  set_local $6
  get_local $2
  get_local $3
  i32.const 1
  i32.shl
  i32.add
  set_local $7
  block $break|0
   loop $continue|0
    get_local $4
    if (result i32)
     get_local $6
     i32.load16_u offset=4
     get_local $7
     i32.load16_u offset=4
     i32.sub
     tee_local $5
     i32.eqz
    else     
     get_local $4
    end
    if
     block
      get_local $4
      i32.const 1
      i32.sub
      set_local $4
      get_local $6
      i32.const 2
      i32.add
      set_local $6
      get_local $7
      i32.const 2
      i32.add
      set_local $7
     end
     br $continue|0
    end
   end
  end
  get_local $5
 )
 (func $~lib/string/String.__eq (; 128 ;) (type $iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  get_local $0
  get_local $1
  i32.eq
  if
   i32.const 1
   return
  end
  get_local $0
  i32.const 0
  i32.eq
  tee_local $2
  if (result i32)
   get_local $2
  else   
   get_local $1
   i32.const 0
   i32.eq
  end
  if
   i32.const 0
   return
  end
  get_local $0
  i32.load
  set_local $3
  get_local $3
  get_local $1
  i32.load
  i32.ne
  if
   i32.const 0
   return
  end
  get_local $0
  i32.const 0
  get_local $1
  i32.const 0
  get_local $3
  call $~lib/internal/string/compareUnsafe
  i32.eqz
 )
 (func $~lib/string/String.__ne (; 129 ;) (type $iii) (param $0 i32) (param $1 i32) (result i32)
  get_local $0
  get_local $1
  call $~lib/string/String.__eq
  i32.eqz
 )
 (func $~lib/ultrain-ts-lib/src/asset/Asset#eq (; 130 ;) (type $iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  get_local $0
  i64.load offset=8
  get_local $1
  i64.load offset=8
  i64.eq
  tee_local $2
  if (result i32)
   get_local $0
   i64.load
   get_local $1
   i64.load
   i64.eq
  else   
   get_local $2
  end
 )
 (func $contract/lib/random.lib/Voter#setMainVoter (; 131 ;) (type $iv) (param $0 i32)
  get_local $0
  i32.const 1
  i32.store offset=40
 )
 (func $~lib/array/Array<u64>#__set (; 132 ;) (type $iiIv) (param $0 i32) (param $1 i32) (param $2 i64)
  (local $3 i32)
  (local $4 i32)
  get_local $0
  i32.load
  set_local $3
  get_local $3
  i32.load
  i32.const 3
  i32.shr_u
  set_local $4
  get_local $1
  get_local $4
  i32.ge_u
  if
   get_local $1
   i32.const 134217727
   i32.ge_u
   if
    call $~lib/env/abort
    unreachable
   end
   get_local $3
   get_local $1
   i32.const 1
   i32.add
   i32.const 3
   i32.shl
   call $~lib/internal/arraybuffer/reallocateUnsafe
   set_local $3
   get_local $0
   get_local $3
   i32.store
   get_local $0
   get_local $1
   i32.const 1
   i32.add
   i32.store offset=4
  end
  get_local $3
  get_local $1
  i32.const 3
  i32.shl
  i32.add
  get_local $2
  i64.store offset=8
 )
 (func $~lib/datastream/DataStream#readVector<u64> (; 133 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  (local $3 i32)
  get_local $0
  call $~lib/datastream/DataStream#readVarint32
  set_local $1
  get_local $1
  i32.const 0
  i32.eq
  if
   i32.const 0
   i32.const 0
   call $~lib/array/Array<u64>#constructor
   return
  end
  i32.const 0
  get_local $1
  call $~lib/array/Array<u64>#constructor
  set_local $2
  block $break|0
   i32.const 0
   set_local $3
   loop $repeat|0
    get_local $3
    get_local $1
    i32.lt_u
    i32.eqz
    br_if $break|0
    get_local $2
    get_local $3
    get_local $0
    call $~lib/datastream/DataStream#read<u64>
    call $~lib/array/Array<u64>#__set
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
 (func $~lib/datastream/DataStream#read<i32> (; 134 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  get_local $0
  i32.load
  get_local $0
  i32.load offset=8
  i32.add
  i32.load
  set_local $1
  get_local $0
  get_local $0
  i32.load offset=8
  i32.const 4
  i32.add
  i32.store offset=8
  get_local $1
 )
 (func $contract/lib/random.lib/Voter#deserialize (; 135 ;) (type $iiv) (param $0 i32) (param $1 i32)
  get_local $0
  get_local $1
  call $~lib/datastream/DataStream#read<u64>
  i64.store
  get_local $0
  get_local $1
  call $~lib/datastream/DataStream#readVector<u64>
  i32.store offset=8
  get_local $0
  get_local $1
  call $~lib/datastream/DataStream#readVector<u64>
  i32.store offset=12
  get_local $0
  get_local $1
  call $~lib/datastream/DataStream#read<u64>
  i64.store offset=16
  get_local $0
  get_local $1
  call $~lib/datastream/DataStream#read<u64>
  i64.store offset=24
  get_local $0
  get_local $1
  call $~lib/datastream/DataStream#read<u64>
  i64.store offset=32
  get_local $0
  get_local $1
  call $~lib/datastream/DataStream#read<i32>
  i32.store offset=40
 )
 (func $~lib/dbmanager/DBManager<Voter>#loadObjectByPrimaryIterator (; 136 ;) (type $iiiv) (param $0 i32) (param $1 i32) (param $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  get_local $1
  i32.const 0
  i32.const 0
  call $~lib/env/db_get_i64
  set_local $3
  i32.const 0
  get_local $3
  call $~lib/internal/typedarray/TypedArray<u8_u32>#constructor
  set_local $4
  i32.const 0
  get_local $4
  i32.load
  get_local $3
  call $~lib/datastream/DataStream#constructor
  set_local $5
  get_local $1
  get_local $4
  i32.load
  get_local $3
  call $~lib/env/db_get_i64
  drop
  get_local $2
  get_local $5
  call $contract/lib/random.lib/Voter#deserialize
 )
 (func $~lib/dbmanager/DBManager<Voter>#get (; 137 ;) (type $iIii) (param $0 i32) (param $1 i64) (param $2 i32) (result i32)
  (local $3 i32)
  get_local $0
  i64.load offset=8
  get_local $0
  i64.load offset=16
  get_local $0
  i64.load
  get_local $1
  call $~lib/env/db_find_i64
  set_local $3
  get_local $3
  i32.const 0
  i32.lt_s
  if
   i32.const 0
   return
  end
  get_local $0
  get_local $3
  get_local $2
  call $~lib/dbmanager/DBManager<Voter>#loadObjectByPrimaryIterator
  i32.const 1
 )
 (func $contract/ultrainio.rand/RandContract#countMainVoter (; 138 ;) (type $iI) (param $0 i32) (result i64)
  (local $1 i32)
  i32.const 0
  call $contract/lib/random.lib/Voter#constructor
  set_local $1
  get_local $0
  i32.load offset=12
  get_global $contract/ultrainio.rand/MAIN_COUNT_KEY
  get_local $1
  call $~lib/dbmanager/DBManager<Voter>#get
  drop
  get_local $1
  i32.load offset=12
  i32.const 0
  call $~lib/array/Array<u64>#__get
 )
 (func $~lib/dbmanager/DBManager<Voter>#modify (; 139 ;) (type $iIiv) (param $0 i32) (param $1 i64) (param $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  get_local $0
  get_local $2
  call $contract/lib/random.lib/Voter#primaryKey
  call $~lib/dbmanager/DBManager<Voter>#find
  set_local $3
  get_local $3
  i32.const 0
  i32.ge_s
  i32.const 2424
  call $~lib/env/ultrain_assert
  get_local $0
  i64.load offset=8
  call $~lib/env/current_receiver
  i64.eq
  i32.const 2544
  call $~lib/env/ultrain_assert
  get_local $2
  call $~lib/datastream/DataStream.measure<Voter>
  set_local $4
  i32.const 0
  get_local $4
  call $~lib/internal/typedarray/TypedArray<u8_u32>#constructor
  set_local $5
  i32.const 0
  get_local $5
  i32.load
  get_local $4
  call $~lib/datastream/DataStream#constructor
  set_local $6
  get_local $2
  get_local $6
  call $contract/lib/random.lib/Voter#serialize
  get_local $3
  get_local $1
  get_local $6
  i32.load
  get_local $6
  i32.load offset=8
  call $~lib/env/db_update_i64
 )
 (func $contract/ultrainio.rand/RandContract#updateCountMainVoter (; 140 ;) (type $iIv) (param $0 i32) (param $1 i64)
  (local $2 i32)
  i32.const 0
  call $contract/lib/random.lib/Voter#constructor
  set_local $2
  get_local $0
  i32.load offset=12
  get_global $contract/ultrainio.rand/MAIN_COUNT_KEY
  get_local $2
  call $~lib/dbmanager/DBManager<Voter>#get
  drop
  get_local $2
  i32.load offset=12
  i32.const 0
  get_local $2
  i32.load offset=12
  i32.const 0
  call $~lib/array/Array<u64>#__get
  get_local $1
  i64.add
  call $~lib/array/Array<u64>#__set
  get_local $0
  i32.load offset=12
  call $~lib/ultrain-ts-lib/src/action/Action.get:sender
  get_local $2
  call $~lib/dbmanager/DBManager<Voter>#modify
 )
 (func $contract/lib/random.lib/Voter#setWaiterVoter (; 141 ;) (type $iv) (param $0 i32)
  get_local $0
  i32.const 2
  i32.store offset=40
 )
 (func $contract/lib/random.lib/Waiter#deserialize (; 142 ;) (type $iiv) (param $0 i32) (param $1 i32)
  get_local $0
  get_local $1
  call $~lib/datastream/DataStream#readVector<u64>
  i32.store
 )
 (func $~lib/dbmanager/DBManager<Waiter>#loadObjectByPrimaryIterator (; 143 ;) (type $iiiv) (param $0 i32) (param $1 i32) (param $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  get_local $1
  i32.const 0
  i32.const 0
  call $~lib/env/db_get_i64
  set_local $3
  i32.const 0
  get_local $3
  call $~lib/internal/typedarray/TypedArray<u8_u32>#constructor
  set_local $4
  i32.const 0
  get_local $4
  i32.load
  get_local $3
  call $~lib/datastream/DataStream#constructor
  set_local $5
  get_local $1
  get_local $4
  i32.load
  get_local $3
  call $~lib/env/db_get_i64
  drop
  get_local $2
  get_local $5
  call $contract/lib/random.lib/Waiter#deserialize
 )
 (func $~lib/dbmanager/DBManager<Waiter>#get (; 144 ;) (type $iIii) (param $0 i32) (param $1 i64) (param $2 i32) (result i32)
  (local $3 i32)
  get_local $0
  i64.load offset=8
  get_local $0
  i64.load offset=16
  get_local $0
  i64.load
  get_local $1
  call $~lib/env/db_find_i64
  set_local $3
  get_local $3
  i32.const 0
  i32.lt_s
  if
   i32.const 0
   return
  end
  get_local $0
  get_local $3
  get_local $2
  call $~lib/dbmanager/DBManager<Waiter>#loadObjectByPrimaryIterator
  i32.const 1
 )
 (func $~lib/array/Array<u64>#push (; 145 ;) (type $iIi) (param $0 i32) (param $1 i64) (result i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  get_local $0
  i32.load offset=4
  set_local $2
  get_local $0
  i32.load
  set_local $3
  get_local $3
  i32.load
  i32.const 3
  i32.shr_u
  set_local $4
  get_local $2
  i32.const 1
  i32.add
  set_local $5
  get_local $2
  get_local $4
  i32.ge_u
  if
   get_local $2
   i32.const 134217727
   i32.ge_u
   if
    call $~lib/env/abort
    unreachable
   end
   get_local $3
   get_local $5
   i32.const 3
   i32.shl
   call $~lib/internal/arraybuffer/reallocateUnsafe
   set_local $3
   get_local $0
   get_local $3
   i32.store
  end
  get_local $0
  get_local $5
  i32.store offset=4
  get_local $3
  get_local $2
  i32.const 3
  i32.shl
  i32.add
  get_local $1
  i64.store offset=8
  get_local $5
 )
 (func $~lib/dbmanager/DBManager<Waiter>#modify (; 146 ;) (type $iIiv) (param $0 i32) (param $1 i64) (param $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  get_local $0
  get_local $2
  call $contract/lib/random.lib/Waiter#primaryKey
  call $~lib/dbmanager/DBManager<Waiter>#find
  set_local $3
  get_local $3
  i32.const 0
  i32.ge_s
  i32.const 2424
  call $~lib/env/ultrain_assert
  get_local $0
  i64.load offset=8
  call $~lib/env/current_receiver
  i64.eq
  i32.const 2544
  call $~lib/env/ultrain_assert
  get_local $2
  call $~lib/datastream/DataStream.measure<Waiter>
  set_local $4
  i32.const 0
  get_local $4
  call $~lib/internal/typedarray/TypedArray<u8_u32>#constructor
  set_local $5
  i32.const 0
  get_local $5
  i32.load
  get_local $4
  call $~lib/datastream/DataStream#constructor
  set_local $6
  get_local $2
  get_local $6
  call $contract/lib/random.lib/Waiter#serialize
  get_local $3
  get_local $1
  get_local $6
  i32.load
  get_local $6
  i32.load offset=8
  call $~lib/env/db_update_i64
 )
 (func $~lib/ultrain-ts-lib/src/asset/Asset#getAmount (; 147 ;) (type $iI) (param $0 i32) (result i64)
  get_local $0
  i64.load
 )
 (func $contract/ultrainio.rand/RandContract#curtBckNum (; 148 ;) (type $iI) (param $0 i32) (result i64)
  call $~lib/ultrain-ts-lib/src/block/Block.get:number
  i32.const 1
  i32.add
  i64.extend_u/i32
 )
 (func $contract/ultrainio.rand/RandContract#register (; 149 ;) (type $iIiiv) (param $0 i32) (param $1 i64) (param $2 i32) (param $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  get_local $1
  get_local $0
  call $~lib/ultrain-ts-lib/src/contract/Contract#get:receiver
  i64.eq
  if
   return
  end
  get_local $3
  i32.const 2248
  call $~lib/string/String.__ne
  if
   return
  end
  get_local $0
  i32.load offset=12
  get_local $1
  call $~lib/dbmanager/DBManager<Voter>#exists
  i32.eqz
  i32.const 2280
  call $~lib/env/ultrain_assert
  i32.const 0
  call $contract/lib/random.lib/Voter#constructor
  set_local $4
  get_local $4
  get_local $1
  i64.store
  get_local $2
  get_global $contract/ultrainio.rand/MAIN_DEPOSIT_AMOUNT
  call $~lib/ultrain-ts-lib/src/asset/Asset#eq
  if
   get_local $4
   call $contract/lib/random.lib/Voter#setMainVoter
   get_local $0
   call $contract/ultrainio.rand/RandContract#countMainVoter
   get_global $contract/ultrainio.rand/MAIN_NUM
   i64.le_u
   i32.const 2344
   get_global $contract/ultrainio.rand/MAIN_NUM
   i32.const 0
   call $~lib/ultrain-ts-lib/src/utils/intToString
   call $~lib/string/String.__concat
   call $~lib/env/ultrain_assert
   get_local $0
   i64.const 1
   call $contract/ultrainio.rand/RandContract#updateCountMainVoter
  else   
   get_local $2
   get_global $contract/ultrainio.rand/WAITER_DEPOSIT_AMOUNT
   call $~lib/ultrain-ts-lib/src/asset/Asset#eq
   if
    get_local $4
    call $contract/lib/random.lib/Voter#setWaiterVoter
    block (result i32)
     i32.const 4
     call $~lib/memory/memory.allocate
     set_local $5
     get_local $5
     i32.const 0
     i32.const 0
     call $~lib/array/Array<u64>#constructor
     i32.store
     get_local $5
    end
    set_local $5
    get_local $0
    i32.load offset=16
    i64.const 0
    get_local $5
    call $~lib/dbmanager/DBManager<Waiter>#get
    drop
    block $~lib/array/Array<u64>#get:length|inlined.1 (result i32)
     get_local $5
     i32.load
     set_local $6
     get_local $6
     i32.load offset=4
    end
    get_global $contract/ultrainio.rand/WAITER_NUM
    i32.wrap/i64
    i32.le_s
    i32.const 2656
    get_global $contract/ultrainio.rand/WAITER_NUM
    i32.const 0
    call $~lib/ultrain-ts-lib/src/utils/intToString
    call $~lib/string/String.__concat
    call $~lib/env/ultrain_assert
    get_local $5
    i32.load
    get_local $1
    call $~lib/array/Array<u64>#push
    drop
    get_local $0
    i32.load offset=16
    get_local $1
    get_local $5
    call $~lib/dbmanager/DBManager<Waiter>#modify
   else    
    get_global $~lib/ultrain-ts-lib/src/log/Log
    i32.const 2744
    call $~lib/ultrain-ts-lib/src/log/Logger#s
    get_local $1
    call $~lib/ultrain-ts-lib/src/account/RNAME
    call $~lib/ultrain-ts-lib/src/log/Logger#s
    i32.const 2760
    call $~lib/ultrain-ts-lib/src/log/Logger#s
    call $~lib/ultrain-ts-lib/src/log/Logger#flush
    i32.const 0
    i32.const 2816
    call $~lib/env/ultrain_assert
   end
  end
  get_local $4
  get_local $2
  call $~lib/ultrain-ts-lib/src/asset/Asset#getAmount
  i64.store offset=16
  get_local $4
  get_local $0
  call $contract/ultrainio.rand/RandContract#curtBckNum
  i64.store offset=24
  get_local $4
  i64.const 0
  i64.store offset=32
  get_local $0
  i32.load offset=12
  get_local $0
  call $~lib/ultrain-ts-lib/src/contract/Contract#get:receiver
  get_local $4
  call $~lib/dbmanager/DBManager<Voter>#emplace
 )
 (func $contract/ultrainio.rand/RandContract#transfer (; 150 ;) (type $iIIiiv) (param $0 i32) (param $1 i64) (param $2 i64) (param $3 i32) (param $4 i32)
  get_global $~lib/ultrain-ts-lib/src/log/Log
  i32.const 2072
  call $~lib/ultrain-ts-lib/src/log/Logger#s
  get_local $1
  call $~lib/ultrain-ts-lib/src/account/RNAME
  call $~lib/ultrain-ts-lib/src/log/Logger#s
  i32.const 2216
  call $~lib/ultrain-ts-lib/src/log/Logger#s
  get_local $2
  call $~lib/ultrain-ts-lib/src/account/RNAME
  call $~lib/ultrain-ts-lib/src/log/Logger#s
  call $~lib/ultrain-ts-lib/src/log/Logger#flush
  get_local $0
  get_local $1
  get_local $3
  get_local $4
  call $contract/ultrainio.rand/RandContract#register
 )
 (func $contract/lib/random.lib/Voter#isMainVoter (; 151 ;) (type $ii) (param $0 i32) (result i32)
  get_local $0
  i32.load offset=40
  i32.const 1
  i32.eq
 )
 (func $~lib/ultrain-ts-lib/src/permission-level/PermissionLevel#constructor (; 152 ;) (type $iIIi) (param $0 i32) (param $1 i64) (param $2 i64) (result i32)
  (local $3 i32)
  get_local $0
  if (result i32)
   get_local $0
  else   
   block (result i32)
    i32.const 16
    call $~lib/memory/memory.allocate
    set_local $3
    get_local $3
    i64.const 0
    i64.store
    get_local $3
    i64.const 0
    i64.store offset=8
    get_local $3
   end
   tee_local $0
  end
  tee_local $0
  get_local $1
  i64.store
  get_local $0
  get_local $2
  i64.store offset=8
  get_local $0
 )
 (func $~lib/ultrain-ts-lib/src/action/TransferParams#constructor (; 153 ;) (type $iIIiii) (param $0 i32) (param $1 i64) (param $2 i64) (param $3 i32) (param $4 i32) (result i32)
  (local $5 i32)
  get_local $0
  if (result i32)
   get_local $0
  else   
   block (result i32)
    i32.const 24
    call $~lib/memory/memory.allocate
    set_local $5
    get_local $5
    i64.const 0
    i64.store
    get_local $5
    i64.const 0
    i64.store offset=8
    get_local $5
    i32.const 0
    i32.store offset=16
    get_local $5
    i32.const 0
    i32.store offset=20
    get_local $5
   end
   tee_local $0
  end
  tee_local $0
  get_local $1
  i64.store
  get_local $0
  get_local $2
  i64.store offset=8
  get_local $0
  get_local $3
  i32.store offset=16
  get_local $0
  get_local $4
  i32.store offset=20
  get_local $0
 )
 (func $~lib/array/Array<PermissionLevel>#constructor (; 154 ;) (type $iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  get_local $1
  i32.const 268435454
  i32.gt_u
  if
   call $~lib/env/abort
   unreachable
  end
  get_local $1
  i32.const 2
  i32.shl
  set_local $2
  get_local $2
  call $~lib/internal/arraybuffer/allocateUnsafe
  set_local $3
  get_local $0
  if (result i32)
   get_local $0
  else   
   block (result i32)
    i32.const 8
    call $~lib/memory/memory.allocate
    set_local $4
    get_local $4
    i32.const 0
    i32.store
    get_local $4
    i32.const 0
    i32.store offset=4
    get_local $4
   end
   tee_local $0
  end
  tee_local $0
  get_local $3
  i32.store
  get_local $0
  get_local $1
  i32.store offset=4
  get_local $3
  get_global $~lib/internal/arraybuffer/HEADER_SIZE
  i32.add
  set_local $4
  i32.const 0
  set_local $5
  get_local $4
  get_local $5
  get_local $2
  call $~lib/internal/memory/memset
  get_local $0
 )
 (func $~lib/array/Array<PermissionLevel>#__unchecked_set (; 155 ;) (type $iiiv) (param $0 i32) (param $1 i32) (param $2 i32)
  (local $3 i32)
  get_local $0
  i32.load
  set_local $3
  get_local $3
  get_local $1
  i32.const 2
  i32.shl
  i32.add
  get_local $2
  i32.store offset=8
 )
 (func $~lib/ultrain-ts-lib/src/action/Action#constructor (; 156 ;) (type $iIIi) (param $0 i32) (param $1 i64) (param $2 i64) (result i32)
  (local $3 i32)
  get_local $0
  if (result i32)
   get_local $0
  else   
   block (result i32)
    i32.const 4
    call $~lib/memory/memory.allocate
    set_local $3
    get_local $3
    i32.const 0
    i32.store
    get_local $3
   end
   tee_local $0
  end
  tee_local $0
  i32.const 0
  get_local $1
  get_local $2
  call $~lib/ultrain-ts-lib/lib/name_ex/NameEx#constructor
  i32.store
  get_local $0
 )
 (func $~lib/ultrain-ts-lib/src/action/ACTION (; 157 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  get_local $0
  call $~lib/ultrain-ts-lib/lib/name_ex/NEX
  set_local $1
  i32.const 0
  get_local $1
  i64.load
  get_local $1
  i64.load offset=8
  call $~lib/ultrain-ts-lib/src/action/Action#constructor
 )
 (func $~lib/ultrain-ts-lib/src/action/Action#get:code (; 158 ;) (type $ii) (param $0 i32) (result i32)
  get_local $0
  i32.load
 )
 (func $~lib/ultrain-ts-lib/src/action/ActionImpl#constructor (; 159 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  get_local $0
  if (result i32)
   get_local $0
  else   
   block (result i32)
    i32.const 20
    call $~lib/memory/memory.allocate
    set_local $1
    get_local $1
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
   end
   tee_local $0
  end
  tee_local $0
  i64.const 0
  i64.store
  get_local $0
  i32.const 0
  i64.const 0
  i64.const 0
  call $~lib/ultrain-ts-lib/lib/name_ex/NameEx#constructor
  i32.store offset=8
  get_local $0
  i32.const 3144
  i32.store offset=12
  get_local $0
  i32.const 3160
  i32.store offset=16
  get_local $0
 )
 (func $~lib/ultrain-ts-lib/src/asset/Asset#serialize (; 160 ;) (type $iiv) (param $0 i32) (param $1 i32)
  get_local $1
  get_local $0
  i64.load
  call $~lib/datastream/DataStream#write<u64>
  get_local $1
  get_local $0
  i64.load offset=8
  call $~lib/datastream/DataStream#write<u64>
 )
 (func $~lib/string/String#get:lengthUTF8 (; 161 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  i32.const 1
  set_local $1
  i32.const 0
  set_local $2
  get_local $0
  i32.load
  set_local $3
  block $break|0
   loop $continue|0
    get_local $2
    get_local $3
    i32.lt_u
    if
     block
      get_local $0
      get_local $2
      i32.const 1
      i32.shl
      i32.add
      i32.load16_u offset=4
      set_local $4
      get_local $4
      i32.const 128
      i32.lt_u
      if
       get_local $1
       i32.const 1
       i32.add
       set_local $1
       get_local $2
       i32.const 1
       i32.add
       set_local $2
      else       
       get_local $4
       i32.const 2048
       i32.lt_u
       if
        get_local $1
        i32.const 2
        i32.add
        set_local $1
        get_local $2
        i32.const 1
        i32.add
        set_local $2
       else        
        get_local $4
        i32.const 64512
        i32.and
        i32.const 55296
        i32.eq
        tee_local $5
        if (result i32)
         get_local $2
         i32.const 1
         i32.add
         get_local $3
         i32.lt_u
        else         
         get_local $5
        end
        tee_local $5
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
         get_local $5
        end
        if
         get_local $1
         i32.const 4
         i32.add
         set_local $1
         get_local $2
         i32.const 2
         i32.add
         set_local $2
        else         
         get_local $1
         i32.const 3
         i32.add
         set_local $1
         get_local $2
         i32.const 1
         i32.add
         set_local $2
        end
       end
      end
     end
     br $continue|0
    end
   end
  end
  get_local $1
 )
 (func $~lib/string/String#toUTF8 (; 162 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  (local $7 i32)
  block $~lib/memory/memory.allocate|inlined.3 (result i32)
   get_local $0
   call $~lib/string/String#get:lengthUTF8
   set_local $1
   get_local $1
   call $~lib/allocator/arena/__memory_allocate
   br $~lib/memory/memory.allocate|inlined.3
  end
  set_local $2
  i32.const 0
  set_local $3
  get_local $0
  i32.load
  set_local $4
  i32.const 0
  set_local $5
  block $break|0
   loop $continue|0
    get_local $3
    get_local $4
    i32.lt_u
    if
     block
      get_local $0
      get_local $3
      i32.const 1
      i32.shl
      i32.add
      i32.load16_u offset=4
      set_local $1
      get_local $1
      i32.const 128
      i32.lt_u
      if
       get_local $2
       get_local $5
       i32.add
       get_local $1
       i32.store8
       get_local $5
       i32.const 1
       i32.add
       set_local $5
       get_local $3
       i32.const 1
       i32.add
       set_local $3
      else       
       get_local $1
       i32.const 2048
       i32.lt_u
       if
        get_local $2
        get_local $5
        i32.add
        set_local $6
        get_local $6
        get_local $1
        i32.const 6
        i32.shr_u
        i32.const 192
        i32.or
        i32.store8
        get_local $6
        get_local $1
        i32.const 63
        i32.and
        i32.const 128
        i32.or
        i32.store8 offset=1
        get_local $5
        i32.const 2
        i32.add
        set_local $5
        get_local $3
        i32.const 1
        i32.add
        set_local $3
       else        
        get_local $2
        get_local $5
        i32.add
        set_local $6
        get_local $1
        i32.const 64512
        i32.and
        i32.const 55296
        i32.eq
        tee_local $7
        if (result i32)
         get_local $3
         i32.const 1
         i32.add
         get_local $4
         i32.lt_u
        else         
         get_local $7
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
         set_local $7
         get_local $7
         i32.const 64512
         i32.and
         i32.const 56320
         i32.eq
         if
          i32.const 65536
          get_local $1
          i32.const 1023
          i32.and
          i32.const 10
          i32.shl
          i32.add
          get_local $7
          i32.const 1023
          i32.and
          i32.add
          set_local $1
          get_local $6
          get_local $1
          i32.const 18
          i32.shr_u
          i32.const 240
          i32.or
          i32.store8
          get_local $6
          get_local $1
          i32.const 12
          i32.shr_u
          i32.const 63
          i32.and
          i32.const 128
          i32.or
          i32.store8 offset=1
          get_local $6
          get_local $1
          i32.const 6
          i32.shr_u
          i32.const 63
          i32.and
          i32.const 128
          i32.or
          i32.store8 offset=2
          get_local $6
          get_local $1
          i32.const 63
          i32.and
          i32.const 128
          i32.or
          i32.store8 offset=3
          get_local $5
          i32.const 4
          i32.add
          set_local $5
          get_local $3
          i32.const 2
          i32.add
          set_local $3
          br $continue|0
         end
        end
        get_local $6
        get_local $1
        i32.const 12
        i32.shr_u
        i32.const 224
        i32.or
        i32.store8
        get_local $6
        get_local $1
        i32.const 6
        i32.shr_u
        i32.const 63
        i32.and
        i32.const 128
        i32.or
        i32.store8 offset=1
        get_local $6
        get_local $1
        i32.const 63
        i32.and
        i32.const 128
        i32.or
        i32.store8 offset=2
        get_local $5
        i32.const 3
        i32.add
        set_local $5
        get_local $3
        i32.const 1
        i32.add
        set_local $3
       end
      end
     end
     br $continue|0
    end
   end
  end
  get_local $2
  get_local $5
  i32.add
  i32.const 0
  i32.store8
  get_local $2
 )
 (func $~lib/datastream/DataStream#writeString (; 163 ;) (type $iiv) (param $0 i32) (param $1 i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  get_local $1
  call $~lib/string/String#get:lengthUTF8
  i32.const 1
  i32.sub
  set_local $2
  get_local $0
  get_local $2
  call $~lib/datastream/DataStream#writeVarint32
  get_local $2
  i32.const 0
  i32.eq
  if
   return
  end
  get_local $0
  call $~lib/datastream/DataStream#isMeasureMode
  i32.eqz
  if
   get_local $1
   call $~lib/string/String#toUTF8
   set_local $3
   get_local $0
   i32.load
   get_local $0
   i32.load offset=8
   i32.add
   set_local $4
   get_local $4
   get_local $3
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
 (func $~lib/ultrain-ts-lib/src/action/TransferParams#serialize (; 164 ;) (type $iiv) (param $0 i32) (param $1 i32)
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
  call $~lib/ultrain-ts-lib/src/asset/Asset#serialize
  get_local $1
  get_local $0
  i32.load offset=20
  call $~lib/datastream/DataStream#writeString
 )
 (func $~lib/datastream/DataStream.measure<TransferParams> (; 165 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  i32.const 0
  i32.const 0
  i32.const 0
  call $~lib/datastream/DataStream#constructor
  set_local $1
  get_local $0
  get_local $1
  call $~lib/ultrain-ts-lib/src/action/TransferParams#serialize
  get_local $1
  i32.load offset=8
 )
 (func $~lib/datastream/DataStream#toArray<u8> (; 166 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  get_local $0
  i32.load offset=4
  i32.const 0
  i32.eq
  if
   i32.const 0
   i32.const 0
   call $~lib/array/Array<u8>#constructor
   return
  end
  get_local $0
  i32.load offset=4
  i32.const 1
  i32.div_u
  set_local $1
  i32.const 0
  get_local $1
  call $~lib/array/Array<u8>#constructor
  set_local $2
  i32.const 0
  set_local $3
  block $break|0
   i32.const 0
   set_local $4
   loop $repeat|0
    get_local $4
    get_local $1
    i32.lt_u
    i32.eqz
    br_if $break|0
    block
     get_local $0
     i32.load
     get_local $3
     i32.add
     i32.load8_u
     set_local $5
     get_local $2
     get_local $4
     get_local $5
     call $~lib/array/Array<u8>#__set
     get_local $3
     i32.const 1
     i32.add
     set_local $3
    end
    get_local $4
    i32.const 1
    i32.add
    set_local $4
    br $repeat|0
    unreachable
   end
   unreachable
  end
  get_local $2
 )
 (func $~lib/ultrain-ts-lib/src/action/SerializableToArray<TransferParams> (; 167 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  (local $3 i32)
  get_local $0
  call $~lib/datastream/DataStream.measure<TransferParams>
  set_local $1
  i32.const 0
  get_local $1
  call $~lib/internal/typedarray/TypedArray<u8_u32>#constructor
  set_local $2
  i32.const 0
  get_local $2
  i32.load
  get_local $1
  call $~lib/datastream/DataStream#constructor
  set_local $3
  get_local $0
  get_local $3
  call $~lib/ultrain-ts-lib/src/action/TransferParams#serialize
  get_local $3
  call $~lib/datastream/DataStream#toArray<u8>
 )
 (func $~lib/ultrain-ts-lib/lib/name_ex/NameEx#serialize (; 168 ;) (type $iiv) (param $0 i32) (param $1 i32)
  get_local $1
  get_local $0
  i64.load
  call $~lib/datastream/DataStream#write<u64>
  get_local $1
  get_local $0
  i64.load offset=8
  call $~lib/datastream/DataStream#write<u64>
 )
 (func $~lib/array/Array<PermissionLevel>#__get (; 169 ;) (type $iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  get_local $0
  i32.load
  set_local $2
  get_local $1
  get_local $2
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
 (func $~lib/ultrain-ts-lib/src/permission-level/PermissionLevel#serialize (; 170 ;) (type $iiv) (param $0 i32) (param $1 i32)
  get_local $1
  get_local $0
  i64.load
  call $~lib/datastream/DataStream#write<u64>
  get_local $1
  get_local $0
  i64.load offset=8
  call $~lib/datastream/DataStream#write<u64>
 )
 (func $~lib/datastream/DataStream#writeComplexVector<PermissionLevel> (; 171 ;) (type $iiv) (param $0 i32) (param $1 i32)
  (local $2 i32)
  (local $3 i32)
  block $~lib/array/Array<PermissionLevel>#get:length|inlined.0 (result i32)
   get_local $1
   i32.load offset=4
  end
  set_local $2
  get_local $0
  get_local $2
  call $~lib/datastream/DataStream#writeVarint32
  block $break|0
   i32.const 0
   set_local $3
   loop $repeat|0
    get_local $3
    get_local $2
    i32.lt_u
    i32.eqz
    br_if $break|0
    get_local $1
    get_local $3
    call $~lib/array/Array<PermissionLevel>#__get
    get_local $0
    call $~lib/ultrain-ts-lib/src/permission-level/PermissionLevel#serialize
    get_local $3
    i32.const 1
    i32.add
    set_local $3
    br $repeat|0
    unreachable
   end
   unreachable
  end
 )
 (func $~lib/datastream/DataStream#writeVector<u8> (; 172 ;) (type $iiv) (param $0 i32) (param $1 i32)
  (local $2 i32)
  (local $3 i32)
  block $~lib/array/Array<u8>#get:length|inlined.0 (result i32)
   get_local $1
   i32.load offset=4
  end
  set_local $2
  get_local $0
  get_local $2
  call $~lib/datastream/DataStream#writeVarint32
  block $break|0
   i32.const 0
   set_local $3
   loop $repeat|0
    get_local $3
    get_local $2
    i32.lt_u
    i32.eqz
    br_if $break|0
    get_local $0
    get_local $1
    get_local $3
    call $~lib/array/Array<u8>#__get
    call $~lib/datastream/DataStream#write<u8>
    get_local $3
    i32.const 1
    i32.add
    set_local $3
    br $repeat|0
    unreachable
   end
   unreachable
  end
 )
 (func $~lib/ultrain-ts-lib/src/action/ActionImpl#serialize (; 173 ;) (type $iiv) (param $0 i32) (param $1 i32)
  get_local $1
  get_local $0
  i64.load
  call $~lib/datastream/DataStream#write<u64>
  get_local $0
  i32.load offset=8
  get_local $1
  call $~lib/ultrain-ts-lib/lib/name_ex/NameEx#serialize
  get_local $1
  get_local $0
  i32.load offset=12
  call $~lib/datastream/DataStream#writeComplexVector<PermissionLevel>
  get_local $1
  get_local $0
  i32.load offset=16
  call $~lib/datastream/DataStream#writeVector<u8>
 )
 (func $~lib/datastream/DataStream.measure<ActionImpl> (; 174 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  i32.const 0
  i32.const 0
  i32.const 0
  call $~lib/datastream/DataStream#constructor
  set_local $1
  get_local $0
  get_local $1
  call $~lib/ultrain-ts-lib/src/action/ActionImpl#serialize
  get_local $1
  i32.load offset=8
 )
 (func $~lib/ultrain-ts-lib/src/action/composeActionData<TransferParams> (; 175 ;) (type $iIiii) (param $0 i32) (param $1 i64) (param $2 i32) (param $3 i32) (result i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  (local $7 i32)
  i32.const 0
  call $~lib/ultrain-ts-lib/src/action/ActionImpl#constructor
  set_local $4
  get_local $4
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
  call $~lib/datastream/DataStream.measure<ActionImpl>
  set_local $5
  i32.const 0
  get_local $5
  call $~lib/internal/typedarray/TypedArray<u8_u32>#constructor
  set_local $6
  i32.const 0
  get_local $6
  i32.load
  get_local $5
  call $~lib/datastream/DataStream#constructor
  set_local $7
  get_local $4
  get_local $7
  call $~lib/ultrain-ts-lib/src/action/ActionImpl#serialize
  get_local $7
 )
 (func $~lib/ultrain-ts-lib/src/action/Action.sendInline<TransferParams> (; 176 ;) (type $iIiiv) (param $0 i32) (param $1 i64) (param $2 i32) (param $3 i32)
  (local $4 i32)
  get_local $0
  get_local $1
  get_local $2
  get_local $3
  call $~lib/ultrain-ts-lib/src/action/composeActionData<TransferParams>
  set_local $4
  get_local $4
  i32.load
  get_local $4
  i32.load offset=8
  call $~lib/ultrain-ts-lib/internal/action.d/env.send_inline
 )
 (func $~lib/ultrain-ts-lib/src/asset/Asset.transfer (; 177 ;) (type $IIiiv) (param $0 i64) (param $1 i64) (param $2 i32) (param $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  i32.const 0
  i64.const 0
  i64.const 0
  call $~lib/ultrain-ts-lib/src/permission-level/PermissionLevel#constructor
  set_local $4
  get_local $4
  get_local $0
  i64.store
  get_local $4
  i32.const 3120
  call $~lib/ultrain-ts-lib/src/account/NAME
  i64.store offset=8
  i32.const 0
  get_local $0
  get_local $1
  get_local $2
  get_local $3
  call $~lib/ultrain-ts-lib/src/action/TransferParams#constructor
  set_local $5
  block (result i32)
   i32.const 0
   i32.const 1
   call $~lib/array/Array<PermissionLevel>#constructor
   set_local $6
   get_local $6
   i32.const 0
   get_local $4
   call $~lib/array/Array<PermissionLevel>#__unchecked_set
   get_local $6
  end
  i32.const 2040
  call $~lib/ultrain-ts-lib/src/account/NAME
  i32.const 2016
  call $~lib/ultrain-ts-lib/src/action/ACTION
  call $~lib/ultrain-ts-lib/src/action/Action#get:code
  get_local $5
  call $~lib/ultrain-ts-lib/src/action/Action.sendInline<TransferParams>
 )
 (func $contract/ultrainio.rand/RandContract#unregister~anonymous|1 (; 178 ;) (type $iiii) (param $0 i32) (param $1 i32) (param $2 i32) (result i32)
  get_local $0
  i64.extend_s/i32
  call $~lib/ultrain-ts-lib/src/action/Action.get:sender
  i64.ne
 )
 (func $~lib/array/Array<u64>#filter (; 179 ;) (type $iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  (local $7 i64)
  get_local $0
  i32.load
  set_local $2
  get_local $0
  i32.load offset=4
  set_local $3
  i32.const 0
  i32.const 0
  call $~lib/array/Array<u64>#constructor
  set_local $4
  block $break|0
   i32.const 0
   set_local $5
   loop $repeat|0
    get_local $5
    get_local $3
    i32.lt_s
    tee_local $6
    if (result i32)
     get_local $5
     get_local $0
     i32.load offset=4
     i32.lt_s
    else     
     get_local $6
    end
    i32.eqz
    br_if $break|0
    block
     block $~lib/internal/arraybuffer/loadUnsafe<u64_u64>|inlined.1 (result i64)
      get_local $2
      get_local $5
      i32.const 3
      i32.shl
      i32.add
      i64.load offset=8
     end
     set_local $7
     block (result i32)
      i32.const 3
      set_global $~argc
      get_local $7
      get_local $5
      get_local $0
      get_local $1
      call_indirect (type $Iiii)
     end
     i32.const 0
     i32.ne
     if
      get_local $4
      get_local $7
      call $~lib/array/Array<u64>#push
      drop
     end
    end
    get_local $5
    i32.const 1
    i32.add
    set_local $5
    br $repeat|0
    unreachable
   end
   unreachable
  end
  get_local $4
 )
 (func $contract/ultrainio.rand/RandContract#unregister (; 180 ;) (type $iv) (param $0 i32)
  (local $1 i32)
  (local $2 i32)
  (local $3 i32)
  get_local $0
  i32.load offset=12
  call $~lib/ultrain-ts-lib/src/action/Action.get:sender
  call $~lib/dbmanager/DBManager<Voter>#exists
  i32.const 2992
  call $~lib/env/ultrain_assert
  i32.const 0
  call $contract/lib/random.lib/Voter#constructor
  set_local $1
  get_local $0
  i32.load offset=12
  call $~lib/ultrain-ts-lib/src/action/Action.get:sender
  get_local $1
  call $~lib/dbmanager/DBManager<Voter>#get
  drop
  get_local $1
  call $contract/lib/random.lib/Voter#isMainVoter
  if
   get_local $0
   call $~lib/ultrain-ts-lib/src/contract/Contract#get:receiver
   call $~lib/ultrain-ts-lib/src/action/Action.get:sender
   get_global $contract/ultrainio.rand/MAIN_DEPOSIT_AMOUNT
   i32.const 3072
   call $~lib/ultrain-ts-lib/src/asset/Asset.transfer
   get_local $0
   i64.const -1
   call $contract/ultrainio.rand/RandContract#updateCountMainVoter
  else   
   get_local $0
   call $~lib/ultrain-ts-lib/src/contract/Contract#get:receiver
   call $~lib/ultrain-ts-lib/src/action/Action.get:sender
   get_global $contract/ultrainio.rand/WAITER_DEPOSIT_AMOUNT
   i32.const 3072
   call $~lib/ultrain-ts-lib/src/asset/Asset.transfer
   block (result i32)
    i32.const 4
    call $~lib/memory/memory.allocate
    set_local $2
    get_local $2
    i32.const 0
    i32.const 0
    call $~lib/array/Array<u64>#constructor
    i32.store
    get_local $2
   end
   set_local $2
   get_local $0
   i32.load offset=16
   i64.const 0
   get_local $2
   call $~lib/dbmanager/DBManager<Waiter>#get
   drop
   get_local $2
   i32.load
   i32.const 1
   call $~lib/array/Array<u64>#filter
   set_local $3
   get_local $2
   get_local $3
   i32.store
   get_local $0
   i32.load offset=16
   call $~lib/ultrain-ts-lib/src/action/Action.get:sender
   get_local $2
   call $~lib/dbmanager/DBManager<Waiter>#modify
  end
  get_local $1
  i64.const 0
  i64.store offset=24
  get_local $1
  get_local $0
  call $contract/ultrainio.rand/RandContract#curtBckNum
  i64.store offset=32
  get_local $0
  i32.load offset=12
  call $~lib/ultrain-ts-lib/src/action/Action.get:sender
  get_local $1
  call $~lib/dbmanager/DBManager<Voter>#modify
 )
 (func $contract/lib/random.lib/Voter#redeemable (; 181 ;) (type $ii) (param $0 i32) (result i32)
  call $~lib/ultrain-ts-lib/src/block/Block.get:number
  i64.extend_u/i32
  get_local $0
  i64.load offset=32
  i64.const 10
  i64.add
  i64.gt_u
  if (result i32)
   i32.const 1
  else   
   i32.const 0
  end
 )
 (func $~lib/dbmanager/DBManager<Voter>#erase (; 182 ;) (type $iIv) (param $0 i32) (param $1 i64)
  (local $2 i32)
  get_local $0
  i64.load offset=8
  call $~lib/env/current_receiver
  i64.eq
  i32.const 3184
  call $~lib/env/ultrain_assert
  get_local $0
  get_local $1
  call $~lib/dbmanager/DBManager<Voter>#find
  set_local $2
  get_local $2
  i32.const 0
  i32.ge_s
  if
   get_local $2
   call $~lib/env/db_remove_i64
  else   
   nop
  end
 )
 (func $contract/ultrainio.rand/RandContract#redeem (; 183 ;) (type $iv) (param $0 i32)
  (local $1 i32)
  get_local $0
  i32.load offset=12
  call $~lib/ultrain-ts-lib/src/action/Action.get:sender
  call $~lib/dbmanager/DBManager<Voter>#exists
  i32.const 2992
  call $~lib/env/ultrain_assert
  i32.const 0
  call $contract/lib/random.lib/Voter#constructor
  set_local $1
  get_local $0
  i32.load offset=12
  call $~lib/ultrain-ts-lib/src/action/Action.get:sender
  get_local $1
  call $~lib/dbmanager/DBManager<Voter>#get
  drop
  get_local $1
  call $contract/lib/random.lib/Voter#redeemable
  if
   get_local $0
   call $~lib/ultrain-ts-lib/src/contract/Contract#get:receiver
   call $~lib/ultrain-ts-lib/src/action/Action.get:sender
   i32.const 0
   get_local $1
   i64.load offset=16
   i64.const 357577479428
   call $~lib/ultrain-ts-lib/src/asset/Asset#constructor
   i32.const 3072
   call $~lib/ultrain-ts-lib/src/asset/Asset.transfer
   get_local $0
   i32.load offset=12
   call $~lib/ultrain-ts-lib/src/action/Action.get:sender
   call $~lib/dbmanager/DBManager<Voter>#erase
  else   
   i32.const 0
   i32.const 3296
   call $~lib/env/ultrain_assert
  end
 )
 (func $contract/ultrainio.rand/RandContract#indexOf (; 184 ;) (type $iIi) (param $0 i32) (param $1 i64) (result i32)
  get_local $1
  get_global $contract/lib/random.lib/EPOCH
  i64.rem_u
  i32.wrap/i64
 )
 (func $contract/ultrainio.rand/RandContract#belongRandNum (; 185 ;) (type $iII) (param $0 i32) (param $1 i64) (result i64)
  get_local $1
  get_global $contract/lib/random.lib/EPOCH
  i64.add
 )
 (func $contract/lib/random.lib/RandRecord#deserialize (; 186 ;) (type $iiv) (param $0 i32) (param $1 i32)
  get_local $0
  get_local $1
  call $~lib/datastream/DataStream#read<u64>
  i64.store
  get_local $0
  get_local $1
  call $~lib/datastream/DataStream#read<u64>
  i64.store offset=8
  get_local $0
  get_local $1
  call $~lib/datastream/DataStream#read<i32>
  i32.store offset=16
 )
 (func $~lib/dbmanager/DBManager<RandRecord>#loadObjectByPrimaryIterator (; 187 ;) (type $iiiv) (param $0 i32) (param $1 i32) (param $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  get_local $1
  i32.const 0
  i32.const 0
  call $~lib/env/db_get_i64
  set_local $3
  i32.const 0
  get_local $3
  call $~lib/internal/typedarray/TypedArray<u8_u32>#constructor
  set_local $4
  i32.const 0
  get_local $4
  i32.load
  get_local $3
  call $~lib/datastream/DataStream#constructor
  set_local $5
  get_local $1
  get_local $4
  i32.load
  get_local $3
  call $~lib/env/db_get_i64
  drop
  get_local $2
  get_local $5
  call $contract/lib/random.lib/RandRecord#deserialize
 )
 (func $~lib/dbmanager/DBManager<RandRecord>#get (; 188 ;) (type $iIii) (param $0 i32) (param $1 i64) (param $2 i32) (result i32)
  (local $3 i32)
  get_local $0
  i64.load offset=8
  get_local $0
  i64.load offset=16
  get_local $0
  i64.load
  get_local $1
  call $~lib/env/db_find_i64
  set_local $3
  get_local $3
  i32.const 0
  i32.lt_s
  if
   i32.const 0
   return
  end
  get_local $0
  get_local $3
  get_local $2
  call $~lib/dbmanager/DBManager<RandRecord>#loadObjectByPrimaryIterator
  i32.const 1
 )
 (func $contract/lib/random.lib/Random#getRandlastRand (; 189 ;) (type $ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  (local $3 i32)
  block (result i32)
   i32.const 20
   call $~lib/memory/memory.allocate
   set_local $1
   get_local $1
   i64.const 0
   i64.store
   get_local $1
   i64.const 0
   i64.store offset=8
   get_local $1
   i32.const 0
   i32.store offset=16
   get_local $1
  end
  set_local $2
  get_local $0
  i32.load offset=8
  i64.const 1
  get_local $2
  call $~lib/dbmanager/DBManager<RandRecord>#get
  drop
  block (result i32)
   i32.const 20
   call $~lib/memory/memory.allocate
   set_local $1
   get_local $1
   i64.const 0
   i64.store
   get_local $1
   i64.const 0
   i64.store offset=8
   get_local $1
   i32.const 0
   i32.store offset=16
   get_local $1
  end
  set_local $3
  get_local $0
  i32.load offset=8
  get_local $2
  i64.load offset=8
  get_local $3
  call $~lib/dbmanager/DBManager<RandRecord>#get
  drop
  get_local $3
 )
 (func $contract/lib/random.lib/Random#hash (; 190 ;) (type $iII) (param $0 i32) (param $1 i64) (result i64)
  (local $2 i32)
  (local $3 i32)
  i32.const 0
  call $~lib/ultrain-ts-lib/src/crypto/SHA256#constructor
  set_local $2
  get_local $2
  get_local $1
  i32.const 0
  call $~lib/ultrain-ts-lib/src/utils/intToString
  call $~lib/ultrain-ts-lib/src/crypto/SHA256#hash
  set_local $3
  get_local $3
  i32.const 0
  i32.const 14
  call $~lib/string/String#substring
  i32.const 16
  call $~lib/string/parseInt
  i64.trunc_u/f64
 )
 (func $contract/lib/random.lib/Random#getMainVoteVal (; 191 ;) (type $iIIi) (param $0 i32) (param $1 i64) (param $2 i64) (result i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  i32.const 0
  call $contract/lib/random.lib/Voter#constructor
  set_local $3
  get_local $0
  i32.load
  get_global $contract/lib/random.lib/RAND_KEY
  get_local $3
  call $~lib/dbmanager/DBManager<Voter>#get
  drop
  block $contract/lib/random.lib/Random#indexOf|inlined.0 (result i32)
   get_local $1
   get_global $contract/lib/random.lib/EPOCH
   i64.rem_u
   i32.wrap/i64
  end
  set_local $4
  block (result i32)
   i32.const 20
   call $~lib/memory/memory.allocate
   set_local $5
   get_local $5
   i64.const 0
   i64.store
   get_local $5
   i64.const 0
   i64.store offset=8
   get_local $5
   i32.const 0
   i32.store offset=16
   get_local $5
  end
  set_local $6
  get_local $3
  i32.load offset=8
  get_local $4
  call $~lib/array/Array<u64>#__get
  get_local $1
  i64.eq
  if
   get_local $6
   get_local $3
   i32.load offset=12
   get_local $4
   call $~lib/array/Array<u64>#__get
   i64.store offset=8
   get_local $6
   i32.const 0
   i32.store offset=16
  else   
   get_local $6
   get_local $0
   get_local $2
   call $contract/lib/random.lib/Random#hash
   i64.store offset=8
   get_local $6
   i32.const 1
   i32.store offset=16
  end
  get_local $6
  get_local $1
  i64.store
  get_local $6
 )
 (func $contract/lib/random.lib/RandRecord#setFields (; 192 ;) (type $iIIiv) (param $0 i32) (param $1 i64) (param $2 i64) (param $3 i32)
  get_local $0
  get_local $1
  i64.store
  get_local $0
  get_local $2
  i64.store offset=8
  get_local $0
  get_local $3
  i32.store offset=16
 )
 (func $contract/lib/random.lib/Random#getWaiterVoteVal (; 193 ;) (type $iIIi) (param $0 i32) (param $1 i64) (param $2 i64) (result i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  (local $7 i32)
  (local $8 i32)
  (local $9 i32)
  (local $10 i32)
  (local $11 i32)
  (local $12 i64)
  (local $13 i64)
  (local $14 i64)
  block (result i32)
   i32.const 4
   call $~lib/memory/memory.allocate
   set_local $3
   get_local $3
   i32.const 0
   i32.const 0
   call $~lib/array/Array<u64>#constructor
   i32.store
   get_local $3
  end
  set_local $4
  get_local $0
  i32.load offset=4
  i64.const 0
  get_local $4
  call $~lib/dbmanager/DBManager<Waiter>#get
  drop
  get_local $4
  i32.load
  set_local $3
  block $~lib/array/Array<u64>#get:length|inlined.2 (result i32)
   get_local $3
   i32.load offset=4
  end
  set_local $5
  i32.const 0
  set_local $6
  i32.const 20
  set_local $7
  block (result i32)
   i32.const 20
   call $~lib/memory/memory.allocate
   set_local $8
   get_local $8
   i64.const 0
   i64.store
   get_local $8
   i64.const 0
   i64.store offset=8
   get_local $8
   i32.const 0
   i32.store offset=16
   get_local $8
  end
  set_local $9
  get_local $9
  get_local $1
  i64.store
  get_local $9
  get_local $0
  get_local $2
  call $contract/lib/random.lib/Random#hash
  i64.store offset=8
  get_local $9
  i32.const 1
  i32.store offset=16
  get_local $5
  i32.const 0
  i32.eq
  if
   get_local $9
   return
  end
  i32.const 0
  call $contract/lib/random.lib/Voter#constructor
  set_local $10
  block $contract/lib/random.lib/Random#indexOf|inlined.1 (result i32)
   get_local $1
   get_global $contract/lib/random.lib/EPOCH
   i64.rem_u
   i32.wrap/i64
  end
  set_local $11
  get_local $2
  set_local $12
  block $break|0
   loop $continue|0
    block
     get_local $12
     get_local $5
     i64.extend_s/i32
     i64.rem_u
     i32.wrap/i64
     set_local $8
     get_local $4
     i32.load
     get_local $8
     call $~lib/array/Array<u64>#__get
     set_local $13
     get_local $0
     i32.load
     get_local $13
     get_local $10
     call $~lib/dbmanager/DBManager<Voter>#get
     drop
     get_local $10
     i32.load offset=8
     get_local $11
     call $~lib/array/Array<u64>#__get
     set_local $14
     get_local $1
     get_local $14
     i64.ne
     if
      get_local $0
      get_local $12
      call $contract/lib/random.lib/Random#hash
      set_local $12
     else      
      get_local $9
      get_local $10
      i32.load offset=12
      get_local $11
      call $~lib/array/Array<u64>#__get
      i64.store offset=8
      get_local $9
      i32.const 0
      i32.store offset=16
      br $break|0
     end
     get_local $6
     i32.const 1
     i32.add
     set_local $6
    end
    get_local $6
    get_local $7
    i32.lt_s
    br_if $continue|0
   end
  end
  get_local $9
 )
 (func $contract/lib/random.lib/RandRecord.calcCode (; 194 ;) (type $iiii) (param $0 i32) (param $1 i32) (param $2 i32) (result i32)
  (local $3 i32)
  i32.const 0
  set_local $3
  get_local $0
  i32.const 7
  i32.eq
  if
   get_local $3
   i32.const 4
   i32.add
   set_local $3
  end
  get_local $1
  i32.const 0
  i32.gt_s
  if
   get_local $3
   i32.const 2
   i32.add
   set_local $3
  end
  get_local $2
  i32.const 0
  i32.gt_s
  if
   get_local $3
   i32.const 1
   i32.add
   set_local $3
  end
  get_local $3
 )
 (func $~lib/dbmanager/DBManager<RandRecord>#modify (; 195 ;) (type $iIiv) (param $0 i32) (param $1 i64) (param $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  get_local $0
  get_local $2
  call $contract/lib/random.lib/RandRecord#primaryKey
  call $~lib/dbmanager/DBManager<RandRecord>#find
  set_local $3
  get_local $3
  i32.const 0
  i32.ge_s
  i32.const 2424
  call $~lib/env/ultrain_assert
  get_local $0
  i64.load offset=8
  call $~lib/env/current_receiver
  i64.eq
  i32.const 2544
  call $~lib/env/ultrain_assert
  get_local $2
  call $~lib/datastream/DataStream.measure<RandRecord>
  set_local $4
  i32.const 0
  get_local $4
  call $~lib/internal/typedarray/TypedArray<u8_u32>#constructor
  set_local $5
  i32.const 0
  get_local $5
  i32.load
  get_local $4
  call $~lib/datastream/DataStream#constructor
  set_local $6
  get_local $2
  get_local $6
  call $contract/lib/random.lib/RandRecord#serialize
  get_local $3
  get_local $1
  get_local $6
  i32.load
  get_local $6
  i32.load offset=8
  call $~lib/env/db_update_i64
 )
 (func $contract/lib/random.lib/Random#saveRandMaxBckNum (; 196 ;) (type $iII) (param $0 i32) (param $1 i64) (result i64)
  (local $2 i32)
  (local $3 i32)
  block (result i32)
   i32.const 20
   call $~lib/memory/memory.allocate
   set_local $2
   get_local $2
   i64.const 0
   i64.store
   get_local $2
   i64.const 0
   i64.store offset=8
   get_local $2
   i32.const 0
   i32.store offset=16
   get_local $2
  end
  set_local $3
  get_local $0
  i32.load offset=8
  i64.const 1
  get_local $3
  call $~lib/dbmanager/DBManager<RandRecord>#get
  drop
  get_local $1
  get_local $3
  i64.load offset=8
  i64.gt_u
  if
   get_local $3
   get_local $1
   i64.store offset=8
   get_local $0
   i32.load offset=8
   call $~lib/ultrain-ts-lib/src/action/Action.get:sender
   get_local $3
   call $~lib/dbmanager/DBManager<RandRecord>#modify
  end
  get_local $3
  i64.load offset=8
 )
 (func $contract/lib/random.lib/Random#getRandMinBckNum (; 197 ;) (type $iI) (param $0 i32) (result i64)
  (local $1 i32)
  (local $2 i32)
  block (result i32)
   i32.const 20
   call $~lib/memory/memory.allocate
   set_local $1
   get_local $1
   i64.const 0
   i64.store
   get_local $1
   i64.const 0
   i64.store offset=8
   get_local $1
   i32.const 0
   i32.store offset=16
   get_local $1
  end
  set_local $2
  get_local $0
  i32.load offset=8
  i64.const 0
  get_local $2
  call $~lib/dbmanager/DBManager<RandRecord>#get
  drop
  get_local $2
  i64.load offset=8
 )
 (func $~lib/dbmanager/DBManager<RandRecord>#erase (; 198 ;) (type $iIv) (param $0 i32) (param $1 i64)
  (local $2 i32)
  get_local $0
  i64.load offset=8
  call $~lib/env/current_receiver
  i64.eq
  i32.const 3184
  call $~lib/env/ultrain_assert
  get_local $0
  get_local $1
  call $~lib/dbmanager/DBManager<RandRecord>#find
  set_local $2
  get_local $2
  i32.const 0
  i32.ge_s
  if
   get_local $2
   call $~lib/env/db_remove_i64
  else   
   nop
  end
 )
 (func $contract/lib/random.lib/Random#clearPartRands (; 199 ;) (type $iIv) (param $0 i32) (param $1 i64)
  (local $2 i64)
  (local $3 i32)
  (local $4 i32)
  get_local $0
  call $contract/lib/random.lib/Random#getRandMinBckNum
  set_local $2
  block (result i32)
   i32.const 20
   call $~lib/memory/memory.allocate
   set_local $3
   get_local $3
   i64.const 0
   i64.store
   get_local $3
   i64.const 0
   i64.store offset=8
   get_local $3
   i32.const 0
   i32.store offset=16
   get_local $3
  end
  set_local $4
  get_local $1
  get_global $contract/lib/random.lib/CACHED_RAND_COUNT
  get_local $2
  i64.add
  i64.ge_u
  if
   get_local $0
   i32.load offset=8
   get_local $2
   call $~lib/dbmanager/DBManager<RandRecord>#exists
   if
    get_local $0
    i32.load offset=8
    get_local $2
    call $~lib/dbmanager/DBManager<RandRecord>#erase
   end
   get_local $4
   get_local $2
   i64.const 1
   i64.add
   i64.store offset=8
   get_local $1
   get_global $contract/lib/random.lib/CACHED_RAND_COUNT
   get_local $2
   i64.add
   i64.gt_u
   if
    get_local $0
    i32.load offset=8
    get_local $2
    i64.const 1
    i64.add
    call $~lib/dbmanager/DBManager<RandRecord>#exists
    if
     get_local $0
     i32.load offset=8
     get_local $2
     i64.const 1
     i64.add
     call $~lib/dbmanager/DBManager<RandRecord>#erase
    end
    get_local $4
    get_local $2
    i64.const 2
    i64.add
    i64.store offset=8
   end
   get_local $0
   i32.load offset=8
   call $~lib/ultrain-ts-lib/src/action/Action.get:sender
   get_local $4
   call $~lib/dbmanager/DBManager<RandRecord>#modify
  end
 )
 (func $contract/lib/random.lib/Random#saveAndClearPartRands (; 200 ;) (type $iIIiv) (param $0 i32) (param $1 i64) (param $2 i64) (param $3 i32)
  (local $4 i32)
  (local $5 i32)
  block (result i32)
   i32.const 20
   call $~lib/memory/memory.allocate
   set_local $4
   get_local $4
   i64.const 0
   i64.store
   get_local $4
   i64.const 0
   i64.store offset=8
   get_local $4
   i32.const 0
   i32.store offset=16
   get_local $4
  end
  set_local $5
  get_local $5
  get_local $1
  get_local $2
  get_local $3
  call $contract/lib/random.lib/RandRecord#setFields
  get_local $0
  i32.load offset=8
  get_local $1
  call $~lib/dbmanager/DBManager<RandRecord>#exists
  i32.eqz
  if
   get_local $0
   i32.load offset=8
   call $~lib/ultrain-ts-lib/src/action/Action.get:sender
   get_local $5
   call $~lib/dbmanager/DBManager<RandRecord>#emplace
  end
  get_local $0
  get_local $1
  call $contract/lib/random.lib/Random#saveRandMaxBckNum
  drop
  get_local $0
  get_local $1
  call $contract/lib/random.lib/Random#clearPartRands
 )
 (func $contract/lib/random.lib/Random#generateRand (; 201 ;) (type $iIii) (param $0 i32) (param $1 i64) (param $2 i32) (result i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i64)
  (local $7 i32)
  (local $8 i64)
  (local $9 i32)
  (local $10 i64)
  block (result i32)
   i32.const 20
   call $~lib/memory/memory.allocate
   set_local $3
   get_local $3
   i64.const 0
   i64.store
   get_local $3
   i64.const 0
   i64.store offset=8
   get_local $3
   i32.const 0
   i32.store offset=16
   get_local $3
  end
  set_local $4
  get_local $0
  i32.load offset=8
  get_local $1
  call $~lib/dbmanager/DBManager<RandRecord>#exists
  if
   get_local $0
   i32.load offset=8
   get_local $1
   get_local $4
   call $~lib/dbmanager/DBManager<RandRecord>#get
   drop
   get_local $4
   return
  end
  get_local $0
  call $contract/lib/random.lib/Random#getRandlastRand
  set_local $5
  get_local $5
  i64.load
  set_local $6
  block (result i32)
   i32.const 20
   call $~lib/memory/memory.allocate
   set_local $3
   get_local $3
   i64.const 0
   i64.store
   get_local $3
   i64.const 0
   i64.store offset=8
   get_local $3
   i32.const 0
   i32.store offset=16
   get_local $3
  end
  set_local $7
  block $break|0
   get_local $6
   i64.const 1
   i64.add
   set_local $8
   loop $repeat|0
    get_local $8
    get_local $1
    i64.le_u
    i32.eqz
    br_if $break|0
    block
     get_local $0
     get_local $8
     get_local $5
     i64.load offset=8
     call $contract/lib/random.lib/Random#getMainVoteVal
     set_local $3
     get_local $8
     get_local $6
     get_global $contract/lib/random.lib/EPOCH
     i64.add
     i64.ge_u
     if
      get_local $7
      get_local $8
      get_local $0
      get_local $3
      i64.load offset=8
      call $contract/lib/random.lib/Random#hash
      i32.const 1
      call $contract/lib/random.lib/RandRecord#setFields
     else      
      get_local $0
      get_local $8
      get_local $3
      i64.load offset=8
      call $contract/lib/random.lib/Random#getWaiterVoteVal
      set_local $7
     end
     get_local $5
     i32.load offset=16
     get_local $3
     i32.load offset=16
     get_local $7
     i32.load offset=16
     call $contract/lib/random.lib/RandRecord.calcCode
     set_local $9
     get_local $5
     i64.load offset=8
     get_local $3
     i64.load offset=8
     i64.xor
     get_local $7
     i64.load offset=8
     i64.xor
     set_local $10
     get_local $4
     get_local $8
     get_local $10
     get_local $9
     call $contract/lib/random.lib/RandRecord#setFields
     get_local $2
     i32.const 0
     i32.ne
     if
      get_local $0
      get_local $8
      get_local $10
      get_local $9
      call $contract/lib/random.lib/Random#saveAndClearPartRands
     end
     get_local $4
     set_local $5
    end
    get_local $8
    i64.const 1
    i64.add
    set_local $8
    br $repeat|0
    unreachable
   end
   unreachable
  end
  get_local $5
 )
 (func $contract/ultrainio.rand/RandContract#triggerRandGenerate (; 202 ;) (type $iv) (param $0 i32)
  (local $1 i32)
  call $~lib/ultrain-ts-lib/src/block/Block.get:number
  set_local $1
  get_local $0
  i32.load offset=20
  get_local $1
  i64.extend_u/i32
  call $~lib/dbmanager/DBManager<RandRecord>#exists
  i32.eqz
  if
   get_local $0
   i32.load offset=24
   get_local $1
   i64.extend_u/i32
   i32.const 1
   call $contract/lib/random.lib/Random#generateRand
   drop
  end
 )
 (func $contract/ultrainio.rand/RandContract#getExistRand (; 203 ;) (type $iII) (param $0 i32) (param $1 i64) (result i64)
  (local $2 i32)
  (local $3 i32)
  block (result i32)
   i32.const 20
   call $~lib/memory/memory.allocate
   set_local $2
   get_local $2
   i64.const 0
   i64.store
   get_local $2
   i64.const 0
   i64.store offset=8
   get_local $2
   i32.const 0
   i32.store offset=16
   get_local $2
  end
  set_local $3
  get_local $0
  i32.load offset=20
  get_local $1
  call $~lib/dbmanager/DBManager<RandRecord>#exists
  i32.const 3672
  call $~lib/env/ultrain_assert
  get_local $0
  i32.load offset=20
  get_local $1
  get_local $3
  call $~lib/dbmanager/DBManager<RandRecord>#get
  drop
  get_local $3
  i64.load offset=8
 )
 (func $~lib/ultrain-ts-lib/src/account/Account.publicKeyOf (; 204 ;) (type $Iii) (param $0 i64) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  i32.const 0
  i32.const 128
  call $~lib/internal/typedarray/TypedArray<u8_u32>#constructor
  set_local $2
  get_local $0
  get_local $2
  i32.load
  block $~lib/internal/typedarray/TypedArray<u8_u32>#get:length|inlined.1 (result i32)
   get_local $2
   i32.load offset=8
   i32.const 0
   i32.shr_u
  end
  get_local $1
  call $~lib/ultrain-ts-lib/src/utils/string2cstr
  call $~lib/ultrain-ts-lib/internal/crypto.d/env.ts_public_key_of_account
  set_local $3
  get_local $3
  i32.const 0
  i32.gt_s
  if
   get_local $2
   i32.load
   get_local $3
   call $~lib/string/String.fromUTF8
   return
  else   
   i32.const 1952
   return
  end
  unreachable
  unreachable
 )
 (func $~lib/ultrain-ts-lib/src/crypto/verify_with_pk (; 205 ;) (type $iiii) (param $0 i32) (param $1 i32) (param $2 i32) (result i32)
  (local $3 i32)
  get_local $0
  call $~lib/ultrain-ts-lib/src/utils/string2cstr
  get_local $1
  call $~lib/ultrain-ts-lib/src/utils/string2cstr
  get_local $2
  call $~lib/ultrain-ts-lib/src/utils/string2cstr
  call $~lib/ultrain-ts-lib/internal/crypto.d/env.ts_verify_with_pk
  set_local $3
  get_local $3
  i32.const 1
  i32.eq
  if (result i32)
   i32.const 1
  else   
   i32.const 0
  end
 )
 (func $contract/ultrainio.rand/RandContract#getVrfVal (; 206 ;) (type $iiII) (param $0 i32) (param $1 i32) (param $2 i64) (result i64)
  (local $3 i64)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  (local $7 i32)
  (local $8 i64)
  get_local $0
  get_local $2
  call $contract/ultrainio.rand/RandContract#getExistRand
  set_local $3
  get_local $3
  i32.const 0
  call $~lib/ultrain-ts-lib/src/utils/intToString
  set_local $4
  get_local $4
  i32.const 136
  i32.const 64
  get_local $4
  i32.load
  i32.sub
  call $~lib/string/String#repeat
  call $~lib/string/String#concat
  set_local $4
  call $~lib/ultrain-ts-lib/src/action/Action.get:sender
  i32.const 3768
  call $~lib/ultrain-ts-lib/src/account/Account.publicKeyOf
  set_local $5
  get_local $5
  get_local $1
  get_local $4
  call $~lib/ultrain-ts-lib/src/crypto/verify_with_pk
  i32.const 3784
  get_local $5
  call $~lib/string/String.__concat
  i32.const 8
  call $~lib/string/String.__concat
  get_local $1
  call $~lib/string/String.__concat
  i32.const 8
  call $~lib/string/String.__concat
  get_local $4
  call $~lib/string/String.__concat
  call $~lib/env/ultrain_assert
  i32.const 0
  call $~lib/ultrain-ts-lib/src/crypto/SHA256#constructor
  set_local $6
  get_local $6
  get_local $1
  i32.const 0
  i32.const 66
  call $~lib/string/String#substring
  call $~lib/ultrain-ts-lib/src/crypto/SHA256#hash
  set_local $7
  get_local $7
  i32.const 0
  i32.const 14
  call $~lib/string/String#substring
  i32.const 16
  call $~lib/string/parseInt
  i64.trunc_u/f64
  set_local $8
  get_local $8
 )
 (func $contract/ultrainio.rand/RandContract#saveVoterVoteStatus (; 207 ;) (type $iIIIv) (param $0 i32) (param $1 i64) (param $2 i64) (param $3 i64)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  (local $7 i32)
  get_local $0
  get_local $2
  call $contract/ultrainio.rand/RandContract#indexOf
  set_local $4
  i32.const 0
  get_local $4
  i32.le_s
  tee_local $5
  if (result i32)
   get_local $4
   i32.const 2
   i32.le_s
  else   
   get_local $5
  end
  i32.const 3856
  call $~lib/env/ultrain_assert
  call $~lib/ultrain-ts-lib/src/block/Block.get:number
  i32.const 1
  i32.add
  set_local $6
  i32.const 0
  call $contract/lib/random.lib/Voter#constructor
  set_local $7
  get_local $7
  get_local $1
  i64.store
  get_local $0
  i32.load offset=12
  get_local $1
  call $~lib/dbmanager/DBManager<Voter>#exists
  if
   get_local $0
   i32.load offset=12
   get_local $1
   get_local $7
   call $~lib/dbmanager/DBManager<Voter>#get
   drop
   get_local $0
   get_local $2
   call $contract/ultrainio.rand/RandContract#belongRandNum
   get_local $7
   i32.load offset=8
   get_local $4
   call $~lib/array/Array<u64>#__get
   i64.eq
   if
    i32.const 0
    i32.const 3936
    call $~lib/env/ultrain_assert
   end
   get_local $0
   get_local $2
   call $contract/ultrainio.rand/RandContract#belongRandNum
   get_local $6
   i64.extend_u/i32
   i64.ge_u
   tee_local $5
   if (result i32)
    get_local $5
   else    
    get_local $7
    i32.load offset=8
    get_local $4
    call $~lib/array/Array<u64>#__get
    i64.const 0
    i64.eq
   end
   if
    get_local $7
    i32.load offset=8
    get_local $4
    get_local $0
    get_local $2
    call $contract/ultrainio.rand/RandContract#belongRandNum
    call $~lib/array/Array<u64>#__set
    get_local $7
    i32.load offset=12
    get_local $4
    get_local $3
    call $~lib/array/Array<u64>#__set
    get_local $0
    i32.load offset=12
    get_local $1
    get_local $7
    call $~lib/dbmanager/DBManager<Voter>#modify
   else    
    i32.const 0
    i32.const 4088
    call $~lib/env/ultrain_assert
   end
  end
 )
 (func $contract/ultrainio.rand/RandContract#mainVoterVote (; 208 ;) (type $iIiIv) (param $0 i32) (param $1 i64) (param $2 i32) (param $3 i64)
  (local $4 i64)
  (local $5 i32)
  (local $6 i32)
  (local $7 i64)
  get_local $0
  get_local $2
  get_local $3
  call $contract/ultrainio.rand/RandContract#getVrfVal
  set_local $4
  get_local $0
  get_local $1
  get_local $3
  get_local $4
  call $contract/ultrainio.rand/RandContract#saveVoterVoteStatus
  get_local $0
  get_local $3
  call $contract/ultrainio.rand/RandContract#indexOf
  set_local $5
  i32.const 0
  call $contract/lib/random.lib/Voter#constructor
  set_local $6
  get_local $0
  i32.load offset=12
  get_global $contract/lib/random.lib/RAND_KEY
  get_local $6
  call $~lib/dbmanager/DBManager<Voter>#get
  drop
  get_local $6
  i32.load offset=8
  get_local $5
  call $~lib/array/Array<u64>#__get
  set_local $7
  get_local $0
  get_local $3
  call $contract/ultrainio.rand/RandContract#belongRandNum
  get_local $7
  i64.ne
  if
   get_local $6
   i32.load offset=12
   get_local $5
   get_local $0
   get_local $3
   call $contract/ultrainio.rand/RandContract#getExistRand
   call $~lib/array/Array<u64>#__set
   get_local $6
   i32.load offset=8
   get_local $5
   get_local $0
   get_local $3
   call $contract/ultrainio.rand/RandContract#belongRandNum
   call $~lib/array/Array<u64>#__set
  end
  get_local $6
  i32.load offset=12
  get_local $5
  get_local $6
  i32.load offset=12
  get_local $5
  call $~lib/array/Array<u64>#__get
  get_local $4
  i64.xor
  call $~lib/array/Array<u64>#__set
  get_local $0
  i32.load offset=12
  get_local $1
  get_local $6
  call $~lib/dbmanager/DBManager<Voter>#modify
  get_local $0
  call $~lib/ultrain-ts-lib/src/contract/Contract#get:receiver
  call $~lib/ultrain-ts-lib/src/action/Action.get:sender
  i32.const 0
  get_global $contract/ultrainio.rand/MAIN_BONUS
  i64.const 357577479428
  call $~lib/ultrain-ts-lib/src/asset/Asset#constructor
  i32.const 4136
  call $~lib/ultrain-ts-lib/src/asset/Asset.transfer
 )
 (func $contract/lib/random.lib/Voter#isWaiterVoter (; 209 ;) (type $ii) (param $0 i32) (result i32)
  get_local $0
  i32.load offset=40
  i32.const 2
  i32.eq
 )
 (func $contract/ultrainio.rand/RandContract#waiterVoterVote (; 210 ;) (type $iIiIv) (param $0 i32) (param $1 i64) (param $2 i32) (param $3 i64)
  (local $4 i64)
  get_local $0
  call $contract/ultrainio.rand/RandContract#curtBckNum
  get_local $3
  i64.sub
  i64.const 2
  i64.gt_u
  if
   i32.const 0
   i32.const 4168
   call $~lib/env/ultrain_assert
  end
  get_local $0
  get_local $2
  get_local $3
  call $contract/ultrainio.rand/RandContract#getVrfVal
  set_local $4
  get_local $0
  get_local $1
  get_local $3
  get_local $4
  call $contract/ultrainio.rand/RandContract#saveVoterVoteStatus
  get_local $0
  call $~lib/ultrain-ts-lib/src/contract/Contract#get:receiver
  call $~lib/ultrain-ts-lib/src/action/Action.get:sender
  i32.const 0
  get_global $contract/ultrainio.rand/WAITER_BONUS
  i64.const 357577479428
  call $~lib/ultrain-ts-lib/src/asset/Asset#constructor
  i32.const 4136
  call $~lib/ultrain-ts-lib/src/asset/Asset.transfer
 )
 (func $contract/ultrainio.rand/RandContract#vote (; 211 ;) (type $iiIv) (param $0 i32) (param $1 i32) (param $2 i64)
  (local $3 i64)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  call $~lib/ultrain-ts-lib/src/action/Action.get:sender
  set_local $3
  get_local $0
  i32.load offset=12
  get_local $3
  call $~lib/dbmanager/DBManager<Voter>#exists
  i32.const 3440
  call $~lib/env/ultrain_assert
  get_local $0
  get_local $2
  call $contract/ultrainio.rand/RandContract#indexOf
  set_local $4
  call $~lib/ultrain-ts-lib/src/block/Block.get:number
  i32.const 1
  i32.add
  set_local $5
  get_local $0
  get_local $2
  call $contract/ultrainio.rand/RandContract#belongRandNum
  get_local $5
  i64.extend_u/i32
  i64.ge_u
  i32.const 3512
  get_local $5
  i64.extend_u/i32
  i32.const 0
  call $~lib/ultrain-ts-lib/src/utils/intToString
  call $~lib/string/String.__concat
  i32.const 3576
  call $~lib/string/String.__concat
  get_local $2
  i32.const 0
  call $~lib/ultrain-ts-lib/src/utils/intToString
  call $~lib/string/String.__concat
  i32.const 3640
  call $~lib/string/String.__concat
  call $~lib/env/ultrain_assert
  get_local $0
  call $contract/ultrainio.rand/RandContract#triggerRandGenerate
  i32.const 0
  call $contract/lib/random.lib/Voter#constructor
  set_local $6
  get_local $0
  i32.load offset=12
  get_local $3
  get_local $6
  call $~lib/dbmanager/DBManager<Voter>#get
  drop
  get_local $6
  call $contract/lib/random.lib/Voter#isMainVoter
  if
   get_local $0
   get_local $3
   get_local $1
   get_local $2
   call $contract/ultrainio.rand/RandContract#mainVoterVote
  else   
   get_local $6
   call $contract/lib/random.lib/Voter#isWaiterVoter
   if
    get_local $0
    get_local $3
    get_local $1
    get_local $2
    call $contract/ultrainio.rand/RandContract#waiterVoterVote
   end
  end
 )
 (func $contract/lib/random.lib/Random#queryLatest (; 212 ;) (type $ii) (param $0 i32) (result i32)
  get_local $0
  call $~lib/ultrain-ts-lib/src/block/Block.get:number
  i64.extend_u/i32
  i32.const 0
  call $contract/lib/random.lib/Random#generateRand
 )
 (func $contract/lib/random.lib/RandRecord#toString (; 213 ;) (type $ii) (param $0 i32) (result i32)
  get_local $0
  i64.load
  i32.const 0
  call $~lib/ultrain-ts-lib/src/utils/intToString
  i32.const 104
  call $~lib/string/String.__concat
  get_local $0
  i64.load offset=8
  i32.const 0
  call $~lib/ultrain-ts-lib/src/utils/intToString
  call $~lib/string/String.__concat
  i32.const 104
  call $~lib/string/String.__concat
  get_local $0
  i32.load offset=16
  i64.extend_s/i32
  i32.const 0
  call $~lib/ultrain-ts-lib/src/utils/intToString
  call $~lib/string/String.__concat
 )
 (func $~lib/ultrain-ts-lib/src/return/Return<String> (; 214 ;) (type $iv) (param $0 i32)
  get_local $0
  call $~lib/ultrain-ts-lib/src/utils/string2cstr
  call $~lib/ultrain-ts-lib/src/return/env.set_result_str
 )
 (func $contract/ultrainio.rand/RandContract#query (; 215 ;) (type $iv) (param $0 i32)
  (local $1 i32)
  get_local $0
  i32.load offset=24
  call $contract/lib/random.lib/Random#queryLatest
  set_local $1
  get_local $1
  call $contract/lib/random.lib/RandRecord#toString
  call $~lib/ultrain-ts-lib/src/return/Return<String>
 )
 (func $contract/lib/random.lib/Random#query (; 216 ;) (type $iIi) (param $0 i32) (param $1 i64) (result i32)
  (local $2 i64)
  (local $3 i32)
  (local $4 i32)
  get_local $0
  call $contract/lib/random.lib/Random#getRandMinBckNum
  set_local $2
  call $~lib/ultrain-ts-lib/src/block/Block.get:number
  set_local $3
  get_local $2
  get_local $1
  i64.le_u
  tee_local $4
  if (result i32)
   get_local $1
   get_local $3
   i64.extend_u/i32
   i64.le_u
  else   
   get_local $4
  end
  i32.const 4352
  get_local $2
  i32.const 0
  call $~lib/ultrain-ts-lib/src/utils/intToString
  call $~lib/string/String.__concat
  i32.const 4472
  call $~lib/string/String.__concat
  get_local $3
  i64.extend_u/i32
  i32.const 0
  call $~lib/ultrain-ts-lib/src/utils/intToString
  call $~lib/string/String.__concat
  call $~lib/env/ultrain_assert
  get_local $0
  get_local $1
  i32.const 0
  call $contract/lib/random.lib/Random#generateRand
 )
 (func $contract/ultrainio.rand/RandContract#queryBck (; 217 ;) (type $iIv) (param $0 i32) (param $1 i64)
  (local $2 i32)
  get_local $0
  i32.load offset=24
  get_local $1
  call $contract/lib/random.lib/Random#query
  set_local $2
  get_local $2
  call $contract/lib/random.lib/RandRecord#toString
  call $~lib/ultrain-ts-lib/src/return/Return<String>
 )
 (func $~lib/ultrain-ts-lib/src/contract/Contract#onError (; 218 ;) (type $iv) (param $0 i32)
  nop
 )
 (func $~lib/ultrain-ts-lib/src/contract/Contract#onStop (; 219 ;) (type $iv) (param $0 i32)
  nop
 )
 (func $contract/ultrainio.rand/apply (; 220 ;) (type $IIIIv) (param $0 i64) (param $1 i64) (param $2 i64) (param $3 i64)
  (local $4 i32)
  (local $5 i32)
  (local $6 i64)
  (local $7 i64)
  (local $8 i32)
  (local $9 i32)
  i32.const 0
  get_local $0
  call $contract/ultrainio.rand/RandContract#constructor
  set_local $4
  get_local $4
  get_local $2
  get_local $3
  call $~lib/ultrain-ts-lib/src/contract/Contract#setActionName
  get_local $4
  get_local $1
  call $contract/ultrainio.rand/RandContract#filterAction
  if
   get_local $4
   call $~lib/ultrain-ts-lib/src/contract/Contract#onInit
   get_local $4
   call $~lib/ultrain-ts-lib/src/contract/Contract#getDataStream
   set_local $5
   get_local $4
   i32.const 2016
   call $~lib/ultrain-ts-lib/src/contract/Contract#isAction
   if
    get_local $5
    call $~lib/datastream/DataStream#read<u64>
    set_local $6
    get_local $5
    call $~lib/datastream/DataStream#read<u64>
    set_local $7
    i32.const 0
    i64.const 0
    i64.const 357577479428
    call $~lib/ultrain-ts-lib/src/asset/Asset#constructor
    set_local $8
    get_local $8
    get_local $5
    call $~lib/ultrain-ts-lib/src/asset/Asset#deserialize
    get_local $5
    call $~lib/datastream/DataStream#readString
    set_local $9
    get_local $4
    get_local $6
    get_local $7
    get_local $8
    get_local $9
    call $contract/ultrainio.rand/RandContract#transfer
   end
   get_local $4
   i32.const 2968
   call $~lib/ultrain-ts-lib/src/contract/Contract#isAction
   if
    get_local $4
    call $contract/ultrainio.rand/RandContract#unregister
   end
   get_local $4
   i32.const 3168
   call $~lib/ultrain-ts-lib/src/contract/Contract#isAction
   if
    get_local $4
    call $contract/ultrainio.rand/RandContract#redeem
   end
   get_local $4
   i32.const 3424
   call $~lib/ultrain-ts-lib/src/contract/Contract#isAction
   if
    get_local $5
    call $~lib/datastream/DataStream#readString
    set_local $9
    get_local $5
    call $~lib/datastream/DataStream#read<u64>
    set_local $7
    get_local $4
    get_local $9
    get_local $7
    call $contract/ultrainio.rand/RandContract#vote
   end
   get_local $4
   i32.const 4312
   call $~lib/ultrain-ts-lib/src/contract/Contract#isAction
   if
    get_local $4
    call $contract/ultrainio.rand/RandContract#query
   end
   get_local $4
   i32.const 4328
   call $~lib/ultrain-ts-lib/src/contract/Contract#isAction
   if
    get_local $5
    call $~lib/datastream/DataStream#read<u64>
    set_local $7
    get_local $4
    get_local $7
    call $contract/ultrainio.rand/RandContract#queryBck
   end
   get_local $4
   i32.const 4488
   call $~lib/ultrain-ts-lib/src/contract/Contract#isAction
   if
    get_local $4
    call $~lib/ultrain-ts-lib/src/contract/Contract#onError
   end
   get_local $4
   call $~lib/ultrain-ts-lib/src/contract/Contract#onStop
  end
 )
 (func $start (; 221 ;) (type $v)
  (local $0 i32)
  get_global $HEAP_BASE
  get_global $~lib/internal/allocator/AL_MASK
  i32.add
  get_global $~lib/internal/allocator/AL_MASK
  i32.const -1
  i32.xor
  i32.and
  set_global $~lib/allocator/arena/startOffset
  get_global $~lib/allocator/arena/startOffset
  set_global $~lib/allocator/arena/offset
  nop
  block (result i32)
   i32.const 0
   call $~lib/memory/memory.allocate
   set_local $0
   get_local $0
  end
  set_global $~lib/ultrain-ts-lib/src/log/Log
  i32.const 4
  i32.const 1280
  call $~lib/ultrain-ts-lib/src/asset/StringToSymbol
  set_global $~lib/ultrain-ts-lib/src/asset/SYS
  get_global $~lib/ultrain-ts-lib/src/asset/SYS
  i64.const 8
  i64.shr_u
  set_global $~lib/ultrain-ts-lib/src/asset/SYS_NAME
  i32.const 1696
  call $~lib/ultrain-ts-lib/src/account/NAME
  set_global $contract/lib/random.lib/RAND_KEY
  i32.const 1736
  call $~lib/ultrain-ts-lib/src/account/NAME
  set_global $contract/ultrainio.rand/MAIN_COUNT_KEY
  i32.const 0
  i64.const 20000
  i64.const 357577479428
  call $~lib/ultrain-ts-lib/src/asset/Asset#constructor
  set_global $contract/ultrainio.rand/MAIN_DEPOSIT_AMOUNT
  i32.const 0
  i64.const 2000
  i64.const 357577479428
  call $~lib/ultrain-ts-lib/src/asset/Asset#constructor
  set_global $contract/ultrainio.rand/WAITER_DEPOSIT_AMOUNT
 )
 (func $null (; 222 ;) (type $v)
 )
)
