(module
 (type $FUNCSIG$v (func))
 (type $FUNCSIG$ii (func (param i32) (result i32)))
 (type $FUNCSIG$jii (func (param i32 i32) (result i64)))
 (type $FUNCSIG$vii (func (param i32 i32)))
 (type $FUNCSIG$iii (func (param i32 i32) (result i32)))
 (type $FUNCSIG$viii (func (param i32 i32 i32)))
 (type $FUNCSIG$vi (func (param i32)))
 (type $FUNCSIG$iiji (func (param i32 i64 i32) (result i32)))
 (type $FUNCSIG$vji (func (param i64 i32)))
 (type $FUNCSIG$ji (func (param i32) (result i64)))
 (type $FUNCSIG$iijj (func (param i32 i64 i64) (result i32)))
 (type $FUNCSIG$vjjjj (func (param i64 i64 i64 i64)))
 (type $FUNCSIG$iij (func (param i32 i64) (result i32)))
 (type $FUNCSIG$ijjj (func (param i64 i64 i64) (result i32)))
 (type $FUNCSIG$j (func (result i64)))
 (type $FUNCSIG$ijjjj (func (param i64 i64 i64 i64) (result i32)))
 (type $FUNCSIG$iiii (func (param i32 i32 i32) (result i32)))
 (type $FUNCSIG$vij (func (param i32 i64)))
 (type $FUNCSIG$ijjjjii (func (param i64 i64 i64 i64 i32 i32) (result i32)))
 (type $FUNCSIG$i (func (result i32)))
 (type $FUNCSIG$jij (func (param i32 i64) (result i64)))
 (type $FUNCSIG$iji (func (param i64 i32) (result i32)))
 (type $FUNCSIG$viiiii (func (param i32 i32 i32 i32 i32)))
 (type $FUNCSIG$viiii (func (param i32 i32 i32 i32)))
 (type $FUNCSIG$dii (func (param i32 i32) (result f64)))
 (type $FUNCSIG$vijj (func (param i32 i64 i64)))
 (type $FUNCSIG$ijji (func (param i64 i64 i32) (result i32)))
 (type $FUNCSIG$vijjii (func (param i32 i64 i64 i32 i32)))
 (type $FUNCSIG$vijii (func (param i32 i64 i32 i32)))
 (type $FUNCSIG$iiiiii (func (param i32 i32 i32 i32 i32) (result i32)))
 (type $FUNCSIG$viij (func (param i32 i32 i64)))
 (type $FUNCSIG$ij (func (param i64) (result i32)))
 (type $FUNCSIG$vjjii (func (param i64 i64 i32 i32)))
 (type $FUNCSIG$iijjii (func (param i32 i64 i64 i32 i32) (result i32)))
 (type $FUNCSIG$iijii (func (param i32 i64 i32 i32) (result i32)))
 (type $FUNCSIG$ijii (func (param i64 i32 i32) (result i32)))
 (type $FUNCSIG$vijji (func (param i32 i64 i64 i32)))
 (type $FUNCSIG$vijij (func (param i32 i64 i32 i64)))
 (type $FUNCSIG$jiij (func (param i32 i32 i64) (result i64)))
 (type $FUNCSIG$ijiii (func (param i64 i32 i32 i32) (result i32)))
 (type $FUNCSIG$vijjj (func (param i32 i64 i64 i64)))
 (import "env" "abort" (func $~lib/env/abort))
 (import "env" "ultrainio_assert" (func $~lib/env/ultrainio_assert (param i32 i32)))
 (import "env" "ts_log_print_s" (func $~lib/ultrain-ts-lib/src/log/env.ts_log_print_s (param i32)))
 (import "env" "ts_log_print_i" (func $~lib/ultrain-ts-lib/src/log/env.ts_log_print_i (param i64 i32)))
 (import "env" "ts_log_done" (func $~lib/ultrain-ts-lib/src/log/env.ts_log_done))
 (import "env" "current_receiver" (func $~lib/env/current_receiver (result i64)))
 (import "env" "db_find_i64" (func $~lib/env/db_find_i64 (param i64 i64 i64 i64) (result i32)))
 (import "env" "db_store_i64" (func $~lib/env/db_store_i64 (param i64 i64 i64 i64 i32 i32) (result i32)))
 (import "env" "head_block_number" (func $~lib/ultrain-ts-lib/lib/headblock/env.head_block_number (result i32)))
 (import "env" "ts_sha256" (func $~lib/ultrain-ts-lib/internal/crypto.d/env.ts_sha256 (param i32 i32 i32 i32)))
 (import "env" "action_data_size" (func $~lib/ultrain-ts-lib/internal/action.d/env.action_data_size (result i32)))
 (import "env" "read_action_data" (func $~lib/ultrain-ts-lib/internal/action.d/env.read_action_data (param i32 i32) (result i32)))
 (import "env" "db_get_i64" (func $~lib/env/db_get_i64 (param i32 i32 i32) (result i32)))
 (import "env" "db_update_i64" (func $~lib/env/db_update_i64 (param i32 i64 i32 i32)))
 (import "env" "current_sender" (func $~lib/ultrain-ts-lib/internal/action.d/env.current_sender (result i64)))
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
 (data (i32.const 1392) "|\01\00\00\00\00\00\00\08\00\00\00\10\00\00\00\18\00\00\00 \00\00\00(\00\00\000\00\00\008\00\00\00@\00\00\00H\00\00\00P\00\00\00X\00\00\00`\00\00\00h\00\00\00p\00\00\00x\00\00\00\80\00\00\00\88\00\00\00\90\00\00\00\98\00\00\00\a0\00\00\00\a8\00\00\00\b0\00\00\00\b8\00\00\00\c0\00\00\00\c8\00\00\00\d0\00\00\00\d8\00\00\00\e0\00\00\00\e8\00\00\00\f0\00\00\00\f8\00\00\00\00\01\00\00\08\01\00\00\10\01\00\00\18\01\00\00 \01\00\00(\01\00\000\01\00\008\01\00\00@\01\00\00H\01\00\00P\01\00\00X\01\00\00`\01\00\00h\01\00\00p\01\00\00x\01\00\00\80\01\00\00\88\01\00\00\90\01\00\00\98\01\00\00\98\01\00\00\a0\01\00\00\a8\01\00\00\b0\01\00\00\b8\01\00\00\c0\01\00\00\c8\01\00\00\d0\01\00\00\d8\01\00\00\e0\01\00\00\e8\01\00\00\f0\01\00\00\f8\01\00\00\00\02\00\00\08\02\00\00\10\02\00\00\18\02\00\00 \02\00\00(\02\00\000\02\00\008\02\00\00@\02\00\00H\02\00\00P\02\00\00X\02\00\00`\02\00\00h\02\00\00p\02\00\00x\02\00\00\80\02\00\00\88\02\00\00\90\02\00\00\98\02\00\00\a0\02\00\00\a8\02\00\00\b0\02\00\00\b8\02\00\00\c0\02\00\00\c8\02\00\00\d0\02\00\00\d8\02\00\00\e0\02\00\00\e8\02\00\00\f0\02\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00")
 (data (i32.const 1904) "p\05\00\00_\00\00\00")
 (data (i32.const 1912) "\0d\00\00\00~\00l\00i\00b\00/\00a\00r\00r\00a\00y\00.\00t\00s\00")
 (data (i32.const 1944) "\1c\00\00\00~\00l\00i\00b\00/\00i\00n\00t\00e\00r\00n\00a\00l\00/\00a\00r\00r\00a\00y\00b\00u\00f\00f\00e\00r\00.\00t\00s\00")
 (data (i32.const 2008) "\0e\00\00\00~\00l\00i\00b\00/\00s\00t\00r\00i\00n\00g\00.\00t\00s\00")
 (data (i32.const 2040) "0\00\00\00s\00t\00r\00i\00n\00g\00_\00t\00o\00_\00_\00s\00y\00m\00b\00o\00l\00 \00f\00a\00i\00l\00e\00d\00 \00f\00o\00r\00 \00n\00o\00t\00 \00s\00u\00p\00o\00o\00r\00t\00 \00c\00o\00d\00e\00 \00:\00 \00")
 (data (i32.const 2144) "\10\00\00\000\001\002\003\004\005\006\007\008\009\00a\00b\00c\00d\00e\00f\00")
 (data (i32.const 2184) "\05\00\00\00v\00o\00t\00e\00r\00")
 (data (i32.const 2200) "\06\00\00\00w\00a\00i\00t\00e\00r\00")
 (data (i32.const 2216) "\04\00\00\00r\00a\00n\00d\00")
 (data (i32.const 2232) "\n\00\00\00u\00t\00r\00i\00o\00.\00r\00a\00n\00d\00")
 (data (i32.const 2256) "\07\00\00\00m\00a\00i\00n\00n\00u\00m\00")
 (data (i32.const 2280) "\08\00\00\00v\00o\00t\00e\00s\00n\00u\00m\00")
 (data (i32.const 2304) "\1b\00\00\00~\00l\00i\00b\00/\00i\00n\00t\00e\00r\00n\00a\00l\00/\00t\00y\00p\00e\00d\00a\00r\00r\00a\00y\00.\00t\00s\00")
 (data (i32.const 2368) "\04\00\00\00n\00u\00l\00l\00")
 (data (i32.const 2384) "\00\00\00\00")
 (data (i32.const 2392) "\17\00\00\00~\00l\00i\00b\00/\00i\00n\00t\00e\00r\00n\00a\00l\00/\00s\00t\00r\00i\00n\00g\00.\00t\00s\00")
 (data (i32.const 2448) "\08\00\00\00t\00r\00a\00n\00s\00f\00e\00r\00")
 (data (i32.const 2472) "\0b\00\00\00u\00t\00r\00i\00o\00.\00t\00o\00k\00e\00n\00")
 (data (i32.const 2504) "\0c\00\00\00a\00s\00 \00c\00a\00n\00d\00i\00d\00a\00t\00e\00")
 (data (i32.const 2536) "\1e\00\00\00c\00a\00n\00n\00o\00t\00 \00a\00d\00d\00 \00e\00x\00i\00s\00t\00i\00n\00g\00 \00c\00a\00n\00d\00i\00d\00a\00t\00e\00.\00")
 (data (i32.const 2600) "&\00\00\00T\00h\00e\00 \00t\00o\00t\00a\00l\00 \00o\00f\00 \00m\00a\00i\00n\00 \00s\00h\00o\00u\00l\00d\00 \00b\00e\00 \00l\00e\00s\00s\00 \00t\00h\00a\00n\00 \00")
 (data (i32.const 2680) "7\00\00\00o\00b\00j\00e\00c\00t\00 \00p\00a\00s\00s\00e\00d\00 \00t\00o\00 \00m\00o\00d\00i\00f\00y\00 \00i\00s\00 \00n\00o\00t\00 \00f\00o\00u\00n\00d\00 \00i\00n\00 \00t\00h\00i\00s\00 \00D\00B\00M\00a\00n\00a\00g\00e\00r\00.\00")
 (data (i32.const 2800) "4\00\00\00c\00a\00n\00 \00n\00o\00t\00 \00m\00o\00d\00i\00f\00y\00 \00o\00b\00j\00e\00c\00t\00s\00 \00i\00n\00 \00t\00a\00b\00l\00e\00 \00o\00f\00 \00a\00n\00o\00t\00h\00e\00r\00 \00c\00o\00n\00t\00r\00a\00c\00t\00.\00")
 (data (i32.const 2912) ")\00\00\00T\00h\00e\00 \00t\00o\00t\00a\00l\00 \00o\00f\00 \00w\00a\00i\00t\00e\00r\00s\00 \00s\00h\00o\00u\00l\00d\00 \00b\00e\00 \00l\00e\00s\00s\00 \00t\00h\00a\00n\00 \00")
 (data (i32.const 3000) "\06\00\00\00U\00s\00e\00r\00:\00 \00")
 (data (i32.const 3016) " \00\00\00.\001\002\003\004\005\00a\00b\00c\00d\00e\00f\00g\00h\00i\00j\00k\00l\00m\00n\00o\00p\00q\00r\00s\00t\00u\00v\00w\00x\00y\00z\00")
 (data (i32.const 3088) "\0d\00\00\00\00\00\00\00.............\00\00\00\00\00\00\00\00\00\00\00")
 (data (i32.const 3120) "\10\0c\00\00\0d\00\00\00")
 (data (i32.const 3128) "\1a\00\00\00 \00d\00e\00p\00o\00s\00i\00t\00e\00d\00 \00m\00o\00n\00e\00y\00 \00a\00c\00c\00u\00r\00a\00t\00e\00.\00")
 (data (i32.const 3184) "I\00\00\00d\00e\00p\00o\00s\00i\00t\00 \00m\00o\00n\00e\00y\00 \00i\00s\00 \00n\00o\00t\00 \00a\00c\00c\00u\00r\00a\00t\00e\00,\00 \00r\00e\00q\00u\00i\00r\00e\00 \00d\00e\00p\00o\00s\00i\00t\00 \002\00.\000\000\000\000\00 \00U\00G\00A\00S\00 \00o\00r\00 \000\00.\002\000\000\000\00 \00U\00G\00A\00S\00")
 (data (i32.const 3336) "\n\00\00\00u\00n\00r\00e\00g\00i\00s\00t\00e\00r\00")
 (data (i32.const 3360) "%\00\00\00c\00a\00n\00n\00o\00t\00 \00u\00n\00r\00e\00g\00i\00s\00t\00e\00r\00 \00n\00o\00n\00-\00e\00x\00i\00s\00t\00i\00n\00g\00 \00v\00o\00t\00e\00r\00.\00")
 (data (i32.const 3440) "!\00\00\00c\00a\00n\00n\00o\00t\00 \00r\00e\00p\00e\00a\00t\00 \00t\00o\00 \00u\00n\00r\00e\00g\00s\00i\00t\00e\00r\00 \00v\00o\00t\00e\00r\00")
 (data (i32.const 3512) "\16\00\00\00r\00e\00t\00u\00r\00n\00 \00d\00e\00p\00o\00s\00i\00t\00e\00d\00 \00m\00o\00n\00e\00y\00")
 (data (i32.const 3560) "\06\00\00\00a\00c\00t\00i\00v\00e\00")
 (data (i32.const 3576) "\00\00\00\00\00\00\00\00")
 (data (i32.const 3584) "\f8\0d\00\00\00\00\00\00")
 (data (i32.const 3592) "\00\00\00\00\00\00\00\00")
 (data (i32.const 3600) "\08\0e\00\00\00\00\00\00")
 (data (i32.const 3608) "\06\00\00\00r\00e\00d\00e\00e\00m\00")
 (data (i32.const 3624) "!\00\00\00c\00a\00n\00n\00o\00t\00 \00r\00e\00d\00e\00e\00m\00 \00n\00o\00n\00-\00e\00x\00i\00s\00t\00i\00n\00g\00 \00v\00o\00t\00e\00r\00.\00")
 (data (i32.const 3696) "3\00\00\00c\00a\00n\00 \00n\00o\00t\00 \00e\00r\00a\00s\00e\00 \00o\00b\00j\00e\00c\00t\00s\00 \00i\00n\00 \00t\00a\00b\00l\00e\00 \00o\00f\00 \00a\00n\00o\00t\00h\00e\00r\00 \00c\00o\00n\00t\00r\00a\00c\00t\00.\00")
 (data (i32.const 3808) "S\00\00\00c\00a\00n\00n\00o\00t\00 \00r\00e\00d\00e\00e\00m\00 \00d\00e\00p\00o\00s\00i\00t\00,\00 \00y\00o\00u\00 \00m\00u\00s\00t\00 \00w\00a\00i\00t\00 \00f\00o\00r\00 \00s\00o\00m\00e\00 \00b\00l\00o\00c\00k\00s\00 \00g\00e\00n\00e\00r\00a\00t\00e\00d\00 \00o\00r\00 \00u\00n\00r\00e\00g\00i\00s\00t\00e\00r\00 \00f\00i\00r\00s\00t\00.\00")
 (data (i32.const 3984) "\04\00\00\00v\00o\00t\00e\00")
 (data (i32.const 4000) "\04\00\00\00Y\00o\00u\00 \00")
 (data (i32.const 4016) "\1d\00\00\00 \00s\00h\00o\00u\00l\00d\00 \00b\00e\00 \00a\00 \00c\00a\00n\00d\00i\00d\00a\00t\00e\00 \00f\00i\00r\00s\00t\00.\00")
 (data (i32.const 4080) "\1c\00\00\00T\00h\00e\00 \00c\00u\00r\00r\00e\00n\00t\00 \00b\00l\00o\00c\00k\00 \00n\00u\00m\00b\00e\00r\00 \00i\00s\00 \00")
 (data (i32.const 4144) "\1e\00\00\00.\00 \00T\00h\00e\00 \00v\00o\00t\00e\00 \00s\00e\00e\00d\00 \00b\00l\00o\00c\00k\00 \00n\00u\00m\00b\00e\00r\00:\00 \00")
 (data (i32.const 4208) "\0d\00\00\00 \00h\00a\00s\00 \00e\00x\00p\00i\00r\00e\00d\00.\00")
 (data (i32.const 4240) "\n\00\00\00T\00h\00e\00 \00v\00o\00t\00e\00r\00 \00")
 (data (i32.const 4264) "\17\00\00\00 \00s\00t\00a\00t\00u\00s\00 \00i\00s\00 \00n\00o\00t\00 \00v\00o\00t\00a\00b\00l\00e\00.\00")
 (data (i32.const 4320) "-\00\00\00T\00h\00e\00 \00r\00a\00n\00d\00o\00m\00 \00o\00f\00 \00b\00l\00o\00c\00k\00 \00h\00e\00i\00g\00h\00t\00 \00h\00a\00s\00 \00n\00o\00t\00 \00g\00e\00n\00e\00r\00a\00t\00e\00d\00.\00")
 (data (i32.const 4416) "\03\00\00\00h\00e\00x\00")
 (data (i32.const 4432) "!\00\00\00p\00l\00e\00a\00s\00e\00 \00p\00r\00o\00v\00i\00d\00e\00 \00a\00 \00v\00a\00l\00i\00d\00 \00V\00R\00F\00 \00p\00r\00o\00o\00f\00.\00")
 (data (i32.const 4504) "#\00\00\00T\00h\00e\00 \00i\00n\00d\00e\00x\00 \00s\00h\00o\00u\00l\00d\00 \00b\00e\00 \00b\00e\00t\00w\00e\00e\00n\00 \000\00 \00a\00n\00d\00 \002\00")
 (data (i32.const 4584) "G\00\00\00Y\00o\00u\00 \00h\00a\00d\00 \00a\00l\00r\00e\00a\00d\00y\00 \00s\00u\00b\00m\00i\00t\00t\00e\00d\00 \00t\00h\00e\00 \00v\00o\00t\00e\00,\00 \00y\00o\00u\00 \00s\00h\00o\00u\00l\00d\00 \00w\00a\00i\00t\00 \00f\00o\00r\00 \00t\00h\00e\00 \00n\00e\00x\00t\00 \00r\00o\00u\00n\00d\00.\00")
 (data (i32.const 4736) "\16\00\00\00Y\00o\00u\00r\00 \00v\00o\00t\00e\00 \00w\00a\00s\00 \00e\00x\00p\00i\00r\00e\00d\00.\00")
 (data (i32.const 4784) "\0b\00\00\00b\00o\00n\00u\00s\00 \00m\00o\00n\00e\00y\00")
 (data (i32.const 4816) "C\00\00\00T\00h\00e\00 \00w\00a\00i\00t\00e\00r\00 \00s\00h\00o\00u\00l\00d\00 \00v\00o\00t\00e\00 \00t\00h\00e\00 \00n\00e\00x\00t\00 \00t\00w\00o\00 \00b\00l\00o\00c\00k\00 \00p\00e\00r\00i\00o\00d\00 \00a\00f\00t\00e\00r\00 \00t\00h\00e\00 \00b\00l\00o\00c\00k\00N\00u\00m\00")
 (data (i32.const 4960) "\05\00\00\00q\00u\00e\00r\00y\00")
 (data (i32.const 4976) "\08\00\00\00q\00u\00e\00r\00y\00B\00c\00k\00")
 (data (i32.const 5000) "9\00\00\00C\00u\00r\00r\00e\00n\00t\00l\00y\00 \00t\00h\00e\00 \00b\00l\00o\00c\00k\00 \00n\00u\00m\00b\00e\00r\00 \00o\00f\00 \00t\00h\00e\00 \00r\00a\00n\00d\00 \00s\00h\00o\00u\00l\00d\00 \00b\00e\00 \00b\00e\00t\00w\00e\00e\00n\00 \00")
 (data (i32.const 5120) "\05\00\00\00 \00a\00n\00d\00 \00")
 (data (i32.const 5136) "\07\00\00\00o\00n\00e\00r\00r\00o\00r\00")
 (table $0 2 funcref)
 (elem (i32.const 0) $null $contract/ultrainio.rand/RandContract#unregister~anonymous|0)
 (global $~lib/allocator/arena/startOffset (mut i32) (i32.const 0))
 (global $~lib/allocator/arena/offset (mut i32) (i32.const 0))
 (global $~lib/ultrain-ts-lib/src/utils/PrintableChar i32 (i32.const 1272))
 (global $~lib/ultrain-ts-lib/src/log/Log (mut i32) (i32.const 0))
 (global $~lib/ultrain-ts-lib/src/asset/CHAR_A i32 (i32.const 65))
 (global $~lib/ultrain-ts-lib/src/asset/CHAR_Z i32 (i32.const 90))
 (global $~lib/utf8util/ASCIICHAR i32 (i32.const 1904))
 (global $~lib/ultrain-ts-lib/src/asset/SYS (mut i64) (i64.const 0))
 (global $~lib/ultrain-ts-lib/src/asset/SYS_NAME (mut i64) (i64.const 0))
 (global $~lib/ultrain-ts-lib/src/asset/MAX_AMOUNT i64 (i64.const 4611686018427387903))
 (global $~lib/ultrain-ts-lib/src/crypto/HexDigital i32 (i32.const 2144))
 (global $~lib/ultrain-ts-lib/src/crypto/CHAR0 i32 (i32.const 48))
 (global $~lib/ultrain-ts-lib/src/crypto/CHAR9 i32 (i32.const 57))
 (global $~lib/ultrain-ts-lib/src/crypto/CHARa i32 (i32.const 97))
 (global $~lib/ultrain-ts-lib/src/crypto/CHARf i32 (i32.const 102))
 (global $~lib/ultrain-ts-lib/src/crypto/CHARA i32 (i32.const 65))
 (global $~lib/ultrain-ts-lib/src/crypto/CHARF i32 (i32.const 70))
 (global $contract/lib/random.lib/VOTER_TABLE i32 (i32.const 2184))
 (global $contract/lib/random.lib/WAITER_TABLE i32 (i32.const 2200))
 (global $contract/lib/random.lib/RAND_TABLE i32 (i32.const 2216))
 (global $contract/lib/random.lib/CONT_NAME i32 (i32.const 2232))
 (global $contract/lib/random.lib/EPOCH i64 (i64.const 3))
 (global $contract/lib/random.lib/RAND_KEY (mut i64) (i64.const 0))
 (global $contract/lib/random.lib/MAIN_COUNT_KEY (mut i64) (i64.const 0))
 (global $contract/lib/random.lib/MAIN_VOTES_NUM_KEY (mut i64) (i64.const 0))
 (global $contract/lib/random.lib/CACHED_RAND_COUNT i64 (i64.const 999))
 (global $contract/ultrainio.rand/MAIN_DEPOSIT_AMOUNT (mut i32) (i32.const 0))
 (global $contract/ultrainio.rand/WAITER_DEPOSIT_AMOUNT (mut i32) (i32.const 0))
 (global $contract/ultrainio.rand/MAIN_BONUS i64 (i64.const 100))
 (global $contract/ultrainio.rand/WAITER_BONUS i64 (i64.const 8))
 (global $contract/ultrainio.rand/WAITER_NUM i64 (i64.const 100))
 (global $contract/ultrainio.rand/MAIN_NUM i64 (i64.const 20))
 (global $~lib/datastream/HEADER_SIZE i32 (i32.const 4))
 (global $~lib/ASC_SHRINK_LEVEL i32 (i32.const 0))
 (global $~lib/argc (mut i32) (i32.const 0))
 (global $~lib/memory/HEAP_BASE i32 (i32.const 5156))
 (export "memory" (memory $0))
 (export "table" (table $0))
 (export "apply" (func $contract/ultrainio.rand/apply))
 (start $start)
 (func $start:~lib/allocator/arena (; 20 ;) (type $FUNCSIG$v)
  global.get $~lib/memory/HEAP_BASE
  i32.const 7
  i32.add
  i32.const 7
  i32.const -1
  i32.xor
  i32.and
  global.set $~lib/allocator/arena/startOffset
  global.get $~lib/allocator/arena/startOffset
  global.set $~lib/allocator/arena/offset
 )
 (func $start:~lib/ultrain-ts-lib/lib/name_ex (; 21 ;) (type $FUNCSIG$v)
  call $start:~lib/allocator/arena
 )
 (func $~lib/allocator/arena/__memory_allocate (; 22 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  local.get $0
  i32.const 1073741824
  i32.gt_u
  if
   unreachable
  end
  global.get $~lib/allocator/arena/offset
  local.set $1
  local.get $1
  local.get $0
  local.tee $2
  i32.const 1
  local.tee $3
  local.get $2
  local.get $3
  i32.gt_u
  select
  i32.add
  i32.const 7
  i32.add
  i32.const 7
  i32.const -1
  i32.xor
  i32.and
  local.set $4
  current_memory
  local.set $5
  local.get $4
  local.get $5
  i32.const 16
  i32.shl
  i32.gt_u
  if
   local.get $4
   local.get $1
   i32.sub
   i32.const 65535
   i32.add
   i32.const 65535
   i32.const -1
   i32.xor
   i32.and
   i32.const 16
   i32.shr_u
   local.set $2
   local.get $5
   local.tee $3
   local.get $2
   local.tee $6
   local.get $3
   local.get $6
   i32.gt_s
   select
   local.set $3
   local.get $3
   grow_memory
   i32.const 0
   i32.lt_s
   if
    local.get $2
    grow_memory
    i32.const 0
    i32.lt_s
    if
     unreachable
    end
   end
  end
  local.get $4
  global.set $~lib/allocator/arena/offset
  local.get $1
 )
 (func $~lib/memory/memory.allocate (; 23 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  local.get $0
  call $~lib/allocator/arena/__memory_allocate
  return
 )
 (func $~lib/ultrain-ts-lib/src/log/Logger#constructor (; 24 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  local.get $0
  i32.eqz
  if
   i32.const 0
   call $~lib/memory/memory.allocate
   local.set $0
  end
  local.get $0
 )
 (func $start:~lib/ultrain-ts-lib/src/log (; 25 ;) (type $FUNCSIG$v)
  i32.const 0
  call $~lib/ultrain-ts-lib/src/log/Logger#constructor
  global.set $~lib/ultrain-ts-lib/src/log/Log
 )
 (func $~lib/internal/arraybuffer/computeSize (; 26 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  i32.const 1
  i32.const 32
  local.get $0
  i32.const 8
  i32.add
  i32.const 1
  i32.sub
  i32.clz
  i32.sub
  i32.shl
 )
 (func $~lib/internal/arraybuffer/allocateUnsafe (; 27 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  local.get $0
  i32.const 1073741816
  i32.le_u
  i32.eqz
  if
   call $~lib/env/abort
   unreachable
  end
  block $~lib/memory/memory.allocate|inlined.0 (result i32)
   local.get $0
   call $~lib/internal/arraybuffer/computeSize
   local.set $2
   local.get $2
   call $~lib/allocator/arena/__memory_allocate
   br $~lib/memory/memory.allocate|inlined.0
  end
  local.set $1
  local.get $1
  local.get $0
  i32.store
  local.get $1
 )
 (func $~lib/internal/memory/memset (; 28 ;) (type $FUNCSIG$viii) (param $0 i32) (param $1 i32) (param $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i64)
  local.get $2
  i32.eqz
  if
   return
  end
  local.get $0
  local.get $1
  i32.store8
  local.get $0
  local.get $2
  i32.add
  i32.const 1
  i32.sub
  local.get $1
  i32.store8
  local.get $2
  i32.const 2
  i32.le_u
  if
   return
  end
  local.get $0
  i32.const 1
  i32.add
  local.get $1
  i32.store8
  local.get $0
  i32.const 2
  i32.add
  local.get $1
  i32.store8
  local.get $0
  local.get $2
  i32.add
  i32.const 2
  i32.sub
  local.get $1
  i32.store8
  local.get $0
  local.get $2
  i32.add
  i32.const 3
  i32.sub
  local.get $1
  i32.store8
  local.get $2
  i32.const 6
  i32.le_u
  if
   return
  end
  local.get $0
  i32.const 3
  i32.add
  local.get $1
  i32.store8
  local.get $0
  local.get $2
  i32.add
  i32.const 4
  i32.sub
  local.get $1
  i32.store8
  local.get $2
  i32.const 8
  i32.le_u
  if
   return
  end
  i32.const 0
  local.get $0
  i32.sub
  i32.const 3
  i32.and
  local.set $3
  local.get $0
  local.get $3
  i32.add
  local.set $0
  local.get $2
  local.get $3
  i32.sub
  local.set $2
  local.get $2
  i32.const -4
  i32.and
  local.set $2
  i32.const -1
  i32.const 255
  i32.div_u
  local.get $1
  i32.const 255
  i32.and
  i32.mul
  local.set $4
  local.get $0
  local.get $4
  i32.store
  local.get $0
  local.get $2
  i32.add
  i32.const 4
  i32.sub
  local.get $4
  i32.store
  local.get $2
  i32.const 8
  i32.le_u
  if
   return
  end
  local.get $0
  i32.const 4
  i32.add
  local.get $4
  i32.store
  local.get $0
  i32.const 8
  i32.add
  local.get $4
  i32.store
  local.get $0
  local.get $2
  i32.add
  i32.const 12
  i32.sub
  local.get $4
  i32.store
  local.get $0
  local.get $2
  i32.add
  i32.const 8
  i32.sub
  local.get $4
  i32.store
  local.get $2
  i32.const 24
  i32.le_u
  if
   return
  end
  local.get $0
  i32.const 12
  i32.add
  local.get $4
  i32.store
  local.get $0
  i32.const 16
  i32.add
  local.get $4
  i32.store
  local.get $0
  i32.const 20
  i32.add
  local.get $4
  i32.store
  local.get $0
  i32.const 24
  i32.add
  local.get $4
  i32.store
  local.get $0
  local.get $2
  i32.add
  i32.const 28
  i32.sub
  local.get $4
  i32.store
  local.get $0
  local.get $2
  i32.add
  i32.const 24
  i32.sub
  local.get $4
  i32.store
  local.get $0
  local.get $2
  i32.add
  i32.const 20
  i32.sub
  local.get $4
  i32.store
  local.get $0
  local.get $2
  i32.add
  i32.const 16
  i32.sub
  local.get $4
  i32.store
  i32.const 24
  local.get $0
  i32.const 4
  i32.and
  i32.add
  local.set $3
  local.get $0
  local.get $3
  i32.add
  local.set $0
  local.get $2
  local.get $3
  i32.sub
  local.set $2
  local.get $4
  i64.extend_i32_u
  local.get $4
  i64.extend_i32_u
  i64.const 32
  i64.shl
  i64.or
  local.set $5
  block $break|0
   loop $continue|0
    local.get $2
    i32.const 32
    i32.ge_u
    if
     block
      local.get $0
      local.get $5
      i64.store
      local.get $0
      i32.const 8
      i32.add
      local.get $5
      i64.store
      local.get $0
      i32.const 16
      i32.add
      local.get $5
      i64.store
      local.get $0
      i32.const 24
      i32.add
      local.get $5
      i64.store
      local.get $2
      i32.const 32
      i32.sub
      local.set $2
      local.get $0
      i32.const 32
      i32.add
      local.set $0
     end
     br $continue|0
    end
   end
  end
 )
 (func $~lib/array/Array<u8>#constructor (; 29 ;) (type $FUNCSIG$iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  local.get $1
  i32.const 1073741816
  i32.gt_u
  if
   call $~lib/env/abort
   unreachable
  end
  local.get $1
  i32.const 0
  i32.shl
  local.set $2
  local.get $2
  call $~lib/internal/arraybuffer/allocateUnsafe
  local.set $3
  block (result i32)
   local.get $0
   i32.eqz
   if
    i32.const 8
    call $~lib/memory/memory.allocate
    local.set $0
   end
   local.get $0
   i32.const 0
   i32.store
   local.get $0
   i32.const 0
   i32.store offset=4
   local.get $0
  end
  local.get $3
  i32.store
  local.get $0
  local.get $1
  i32.store offset=4
  block $~lib/memory/memory.fill|inlined.0
   local.get $3
   i32.const 8
   i32.add
   local.set $4
   i32.const 0
   local.set $5
   local.get $2
   local.set $6
   local.get $4
   local.get $5
   local.get $6
   call $~lib/internal/memory/memset
  end
  local.get $0
 )
 (func $~lib/string/String#charCodeAt (; 30 ;) (type $FUNCSIG$iii) (param $0 i32) (param $1 i32) (result i32)
  local.get $0
  i32.const 0
  i32.ne
  i32.eqz
  if
   call $~lib/env/abort
   unreachable
  end
  local.get $1
  local.get $0
  i32.load
  i32.ge_u
  if
   i32.const -1
   return
  end
  local.get $0
  local.get $1
  i32.const 1
  i32.shl
  i32.add
  i32.load16_u offset=4
 )
 (func $~lib/internal/memory/memcpy (; 31 ;) (type $FUNCSIG$viii) (param $0 i32) (param $1 i32) (param $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  block $break|0
   loop $continue|0
    local.get $2
    if (result i32)
     local.get $1
     i32.const 3
     i32.and
    else     
     local.get $2
    end
    if
     block
      block (result i32)
       local.get $0
       local.tee $5
       i32.const 1
       i32.add
       local.set $0
       local.get $5
      end
      block (result i32)
       local.get $1
       local.tee $5
       i32.const 1
       i32.add
       local.set $1
       local.get $5
      end
      i32.load8_u
      i32.store8
      local.get $2
      i32.const 1
      i32.sub
      local.set $2
     end
     br $continue|0
    end
   end
  end
  local.get $0
  i32.const 3
  i32.and
  i32.const 0
  i32.eq
  if
   block $break|1
    loop $continue|1
     local.get $2
     i32.const 16
     i32.ge_u
     if
      block
       local.get $0
       local.get $1
       i32.load
       i32.store
       local.get $0
       i32.const 4
       i32.add
       local.get $1
       i32.const 4
       i32.add
       i32.load
       i32.store
       local.get $0
       i32.const 8
       i32.add
       local.get $1
       i32.const 8
       i32.add
       i32.load
       i32.store
       local.get $0
       i32.const 12
       i32.add
       local.get $1
       i32.const 12
       i32.add
       i32.load
       i32.store
       local.get $1
       i32.const 16
       i32.add
       local.set $1
       local.get $0
       i32.const 16
       i32.add
       local.set $0
       local.get $2
       i32.const 16
       i32.sub
       local.set $2
      end
      br $continue|1
     end
    end
   end
   local.get $2
   i32.const 8
   i32.and
   if
    local.get $0
    local.get $1
    i32.load
    i32.store
    local.get $0
    i32.const 4
    i32.add
    local.get $1
    i32.const 4
    i32.add
    i32.load
    i32.store
    local.get $0
    i32.const 8
    i32.add
    local.set $0
    local.get $1
    i32.const 8
    i32.add
    local.set $1
   end
   local.get $2
   i32.const 4
   i32.and
   if
    local.get $0
    local.get $1
    i32.load
    i32.store
    local.get $0
    i32.const 4
    i32.add
    local.set $0
    local.get $1
    i32.const 4
    i32.add
    local.set $1
   end
   local.get $2
   i32.const 2
   i32.and
   if
    local.get $0
    local.get $1
    i32.load16_u
    i32.store16
    local.get $0
    i32.const 2
    i32.add
    local.set $0
    local.get $1
    i32.const 2
    i32.add
    local.set $1
   end
   local.get $2
   i32.const 1
   i32.and
   if
    block (result i32)
     local.get $0
     local.tee $5
     i32.const 1
     i32.add
     local.set $0
     local.get $5
    end
    block (result i32)
     local.get $1
     local.tee $5
     i32.const 1
     i32.add
     local.set $1
     local.get $5
    end
    i32.load8_u
    i32.store8
   end
   return
  end
  local.get $2
  i32.const 32
  i32.ge_u
  if
   block $break|2
    block $case2|2
     block $case1|2
      block $case0|2
       local.get $0
       i32.const 3
       i32.and
       local.set $5
       local.get $5
       i32.const 1
       i32.eq
       br_if $case0|2
       local.get $5
       i32.const 2
       i32.eq
       br_if $case1|2
       local.get $5
       i32.const 3
       i32.eq
       br_if $case2|2
       br $break|2
      end
      block
       local.get $1
       i32.load
       local.set $3
       block (result i32)
        local.get $0
        local.tee $5
        i32.const 1
        i32.add
        local.set $0
        local.get $5
       end
       block (result i32)
        local.get $1
        local.tee $5
        i32.const 1
        i32.add
        local.set $1
        local.get $5
       end
       i32.load8_u
       i32.store8
       block (result i32)
        local.get $0
        local.tee $5
        i32.const 1
        i32.add
        local.set $0
        local.get $5
       end
       block (result i32)
        local.get $1
        local.tee $5
        i32.const 1
        i32.add
        local.set $1
        local.get $5
       end
       i32.load8_u
       i32.store8
       block (result i32)
        local.get $0
        local.tee $5
        i32.const 1
        i32.add
        local.set $0
        local.get $5
       end
       block (result i32)
        local.get $1
        local.tee $5
        i32.const 1
        i32.add
        local.set $1
        local.get $5
       end
       i32.load8_u
       i32.store8
       local.get $2
       i32.const 3
       i32.sub
       local.set $2
       block $break|3
        loop $continue|3
         local.get $2
         i32.const 17
         i32.ge_u
         if
          block
           local.get $1
           i32.const 1
           i32.add
           i32.load
           local.set $4
           local.get $0
           local.get $3
           i32.const 24
           i32.shr_u
           local.get $4
           i32.const 8
           i32.shl
           i32.or
           i32.store
           local.get $1
           i32.const 5
           i32.add
           i32.load
           local.set $3
           local.get $0
           i32.const 4
           i32.add
           local.get $4
           i32.const 24
           i32.shr_u
           local.get $3
           i32.const 8
           i32.shl
           i32.or
           i32.store
           local.get $1
           i32.const 9
           i32.add
           i32.load
           local.set $4
           local.get $0
           i32.const 8
           i32.add
           local.get $3
           i32.const 24
           i32.shr_u
           local.get $4
           i32.const 8
           i32.shl
           i32.or
           i32.store
           local.get $1
           i32.const 13
           i32.add
           i32.load
           local.set $3
           local.get $0
           i32.const 12
           i32.add
           local.get $4
           i32.const 24
           i32.shr_u
           local.get $3
           i32.const 8
           i32.shl
           i32.or
           i32.store
           local.get $1
           i32.const 16
           i32.add
           local.set $1
           local.get $0
           i32.const 16
           i32.add
           local.set $0
           local.get $2
           i32.const 16
           i32.sub
           local.set $2
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
      local.get $1
      i32.load
      local.set $3
      block (result i32)
       local.get $0
       local.tee $5
       i32.const 1
       i32.add
       local.set $0
       local.get $5
      end
      block (result i32)
       local.get $1
       local.tee $5
       i32.const 1
       i32.add
       local.set $1
       local.get $5
      end
      i32.load8_u
      i32.store8
      block (result i32)
       local.get $0
       local.tee $5
       i32.const 1
       i32.add
       local.set $0
       local.get $5
      end
      block (result i32)
       local.get $1
       local.tee $5
       i32.const 1
       i32.add
       local.set $1
       local.get $5
      end
      i32.load8_u
      i32.store8
      local.get $2
      i32.const 2
      i32.sub
      local.set $2
      block $break|4
       loop $continue|4
        local.get $2
        i32.const 18
        i32.ge_u
        if
         block
          local.get $1
          i32.const 2
          i32.add
          i32.load
          local.set $4
          local.get $0
          local.get $3
          i32.const 16
          i32.shr_u
          local.get $4
          i32.const 16
          i32.shl
          i32.or
          i32.store
          local.get $1
          i32.const 6
          i32.add
          i32.load
          local.set $3
          local.get $0
          i32.const 4
          i32.add
          local.get $4
          i32.const 16
          i32.shr_u
          local.get $3
          i32.const 16
          i32.shl
          i32.or
          i32.store
          local.get $1
          i32.const 10
          i32.add
          i32.load
          local.set $4
          local.get $0
          i32.const 8
          i32.add
          local.get $3
          i32.const 16
          i32.shr_u
          local.get $4
          i32.const 16
          i32.shl
          i32.or
          i32.store
          local.get $1
          i32.const 14
          i32.add
          i32.load
          local.set $3
          local.get $0
          i32.const 12
          i32.add
          local.get $4
          i32.const 16
          i32.shr_u
          local.get $3
          i32.const 16
          i32.shl
          i32.or
          i32.store
          local.get $1
          i32.const 16
          i32.add
          local.set $1
          local.get $0
          i32.const 16
          i32.add
          local.set $0
          local.get $2
          i32.const 16
          i32.sub
          local.set $2
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
     local.get $1
     i32.load
     local.set $3
     block (result i32)
      local.get $0
      local.tee $5
      i32.const 1
      i32.add
      local.set $0
      local.get $5
     end
     block (result i32)
      local.get $1
      local.tee $5
      i32.const 1
      i32.add
      local.set $1
      local.get $5
     end
     i32.load8_u
     i32.store8
     local.get $2
     i32.const 1
     i32.sub
     local.set $2
     block $break|5
      loop $continue|5
       local.get $2
       i32.const 19
       i32.ge_u
       if
        block
         local.get $1
         i32.const 3
         i32.add
         i32.load
         local.set $4
         local.get $0
         local.get $3
         i32.const 8
         i32.shr_u
         local.get $4
         i32.const 24
         i32.shl
         i32.or
         i32.store
         local.get $1
         i32.const 7
         i32.add
         i32.load
         local.set $3
         local.get $0
         i32.const 4
         i32.add
         local.get $4
         i32.const 8
         i32.shr_u
         local.get $3
         i32.const 24
         i32.shl
         i32.or
         i32.store
         local.get $1
         i32.const 11
         i32.add
         i32.load
         local.set $4
         local.get $0
         i32.const 8
         i32.add
         local.get $3
         i32.const 8
         i32.shr_u
         local.get $4
         i32.const 24
         i32.shl
         i32.or
         i32.store
         local.get $1
         i32.const 15
         i32.add
         i32.load
         local.set $3
         local.get $0
         i32.const 12
         i32.add
         local.get $4
         i32.const 8
         i32.shr_u
         local.get $3
         i32.const 24
         i32.shl
         i32.or
         i32.store
         local.get $1
         i32.const 16
         i32.add
         local.set $1
         local.get $0
         i32.const 16
         i32.add
         local.set $0
         local.get $2
         i32.const 16
         i32.sub
         local.set $2
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
  local.get $2
  i32.const 16
  i32.and
  if
   block (result i32)
    local.get $0
    local.tee $5
    i32.const 1
    i32.add
    local.set $0
    local.get $5
   end
   block (result i32)
    local.get $1
    local.tee $5
    i32.const 1
    i32.add
    local.set $1
    local.get $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    local.get $0
    local.tee $5
    i32.const 1
    i32.add
    local.set $0
    local.get $5
   end
   block (result i32)
    local.get $1
    local.tee $5
    i32.const 1
    i32.add
    local.set $1
    local.get $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    local.get $0
    local.tee $5
    i32.const 1
    i32.add
    local.set $0
    local.get $5
   end
   block (result i32)
    local.get $1
    local.tee $5
    i32.const 1
    i32.add
    local.set $1
    local.get $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    local.get $0
    local.tee $5
    i32.const 1
    i32.add
    local.set $0
    local.get $5
   end
   block (result i32)
    local.get $1
    local.tee $5
    i32.const 1
    i32.add
    local.set $1
    local.get $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    local.get $0
    local.tee $5
    i32.const 1
    i32.add
    local.set $0
    local.get $5
   end
   block (result i32)
    local.get $1
    local.tee $5
    i32.const 1
    i32.add
    local.set $1
    local.get $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    local.get $0
    local.tee $5
    i32.const 1
    i32.add
    local.set $0
    local.get $5
   end
   block (result i32)
    local.get $1
    local.tee $5
    i32.const 1
    i32.add
    local.set $1
    local.get $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    local.get $0
    local.tee $5
    i32.const 1
    i32.add
    local.set $0
    local.get $5
   end
   block (result i32)
    local.get $1
    local.tee $5
    i32.const 1
    i32.add
    local.set $1
    local.get $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    local.get $0
    local.tee $5
    i32.const 1
    i32.add
    local.set $0
    local.get $5
   end
   block (result i32)
    local.get $1
    local.tee $5
    i32.const 1
    i32.add
    local.set $1
    local.get $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    local.get $0
    local.tee $5
    i32.const 1
    i32.add
    local.set $0
    local.get $5
   end
   block (result i32)
    local.get $1
    local.tee $5
    i32.const 1
    i32.add
    local.set $1
    local.get $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    local.get $0
    local.tee $5
    i32.const 1
    i32.add
    local.set $0
    local.get $5
   end
   block (result i32)
    local.get $1
    local.tee $5
    i32.const 1
    i32.add
    local.set $1
    local.get $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    local.get $0
    local.tee $5
    i32.const 1
    i32.add
    local.set $0
    local.get $5
   end
   block (result i32)
    local.get $1
    local.tee $5
    i32.const 1
    i32.add
    local.set $1
    local.get $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    local.get $0
    local.tee $5
    i32.const 1
    i32.add
    local.set $0
    local.get $5
   end
   block (result i32)
    local.get $1
    local.tee $5
    i32.const 1
    i32.add
    local.set $1
    local.get $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    local.get $0
    local.tee $5
    i32.const 1
    i32.add
    local.set $0
    local.get $5
   end
   block (result i32)
    local.get $1
    local.tee $5
    i32.const 1
    i32.add
    local.set $1
    local.get $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    local.get $0
    local.tee $5
    i32.const 1
    i32.add
    local.set $0
    local.get $5
   end
   block (result i32)
    local.get $1
    local.tee $5
    i32.const 1
    i32.add
    local.set $1
    local.get $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    local.get $0
    local.tee $5
    i32.const 1
    i32.add
    local.set $0
    local.get $5
   end
   block (result i32)
    local.get $1
    local.tee $5
    i32.const 1
    i32.add
    local.set $1
    local.get $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    local.get $0
    local.tee $5
    i32.const 1
    i32.add
    local.set $0
    local.get $5
   end
   block (result i32)
    local.get $1
    local.tee $5
    i32.const 1
    i32.add
    local.set $1
    local.get $5
   end
   i32.load8_u
   i32.store8
  end
  local.get $2
  i32.const 8
  i32.and
  if
   block (result i32)
    local.get $0
    local.tee $5
    i32.const 1
    i32.add
    local.set $0
    local.get $5
   end
   block (result i32)
    local.get $1
    local.tee $5
    i32.const 1
    i32.add
    local.set $1
    local.get $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    local.get $0
    local.tee $5
    i32.const 1
    i32.add
    local.set $0
    local.get $5
   end
   block (result i32)
    local.get $1
    local.tee $5
    i32.const 1
    i32.add
    local.set $1
    local.get $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    local.get $0
    local.tee $5
    i32.const 1
    i32.add
    local.set $0
    local.get $5
   end
   block (result i32)
    local.get $1
    local.tee $5
    i32.const 1
    i32.add
    local.set $1
    local.get $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    local.get $0
    local.tee $5
    i32.const 1
    i32.add
    local.set $0
    local.get $5
   end
   block (result i32)
    local.get $1
    local.tee $5
    i32.const 1
    i32.add
    local.set $1
    local.get $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    local.get $0
    local.tee $5
    i32.const 1
    i32.add
    local.set $0
    local.get $5
   end
   block (result i32)
    local.get $1
    local.tee $5
    i32.const 1
    i32.add
    local.set $1
    local.get $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    local.get $0
    local.tee $5
    i32.const 1
    i32.add
    local.set $0
    local.get $5
   end
   block (result i32)
    local.get $1
    local.tee $5
    i32.const 1
    i32.add
    local.set $1
    local.get $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    local.get $0
    local.tee $5
    i32.const 1
    i32.add
    local.set $0
    local.get $5
   end
   block (result i32)
    local.get $1
    local.tee $5
    i32.const 1
    i32.add
    local.set $1
    local.get $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    local.get $0
    local.tee $5
    i32.const 1
    i32.add
    local.set $0
    local.get $5
   end
   block (result i32)
    local.get $1
    local.tee $5
    i32.const 1
    i32.add
    local.set $1
    local.get $5
   end
   i32.load8_u
   i32.store8
  end
  local.get $2
  i32.const 4
  i32.and
  if
   block (result i32)
    local.get $0
    local.tee $5
    i32.const 1
    i32.add
    local.set $0
    local.get $5
   end
   block (result i32)
    local.get $1
    local.tee $5
    i32.const 1
    i32.add
    local.set $1
    local.get $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    local.get $0
    local.tee $5
    i32.const 1
    i32.add
    local.set $0
    local.get $5
   end
   block (result i32)
    local.get $1
    local.tee $5
    i32.const 1
    i32.add
    local.set $1
    local.get $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    local.get $0
    local.tee $5
    i32.const 1
    i32.add
    local.set $0
    local.get $5
   end
   block (result i32)
    local.get $1
    local.tee $5
    i32.const 1
    i32.add
    local.set $1
    local.get $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    local.get $0
    local.tee $5
    i32.const 1
    i32.add
    local.set $0
    local.get $5
   end
   block (result i32)
    local.get $1
    local.tee $5
    i32.const 1
    i32.add
    local.set $1
    local.get $5
   end
   i32.load8_u
   i32.store8
  end
  local.get $2
  i32.const 2
  i32.and
  if
   block (result i32)
    local.get $0
    local.tee $5
    i32.const 1
    i32.add
    local.set $0
    local.get $5
   end
   block (result i32)
    local.get $1
    local.tee $5
    i32.const 1
    i32.add
    local.set $1
    local.get $5
   end
   i32.load8_u
   i32.store8
   block (result i32)
    local.get $0
    local.tee $5
    i32.const 1
    i32.add
    local.set $0
    local.get $5
   end
   block (result i32)
    local.get $1
    local.tee $5
    i32.const 1
    i32.add
    local.set $1
    local.get $5
   end
   i32.load8_u
   i32.store8
  end
  local.get $2
  i32.const 1
  i32.and
  if
   block (result i32)
    local.get $0
    local.tee $5
    i32.const 1
    i32.add
    local.set $0
    local.get $5
   end
   block (result i32)
    local.get $1
    local.tee $5
    i32.const 1
    i32.add
    local.set $1
    local.get $5
   end
   i32.load8_u
   i32.store8
  end
 )
 (func $~lib/internal/memory/memmove (; 32 ;) (type $FUNCSIG$viii) (param $0 i32) (param $1 i32) (param $2 i32)
  (local $3 i32)
  local.get $0
  local.get $1
  i32.eq
  if
   return
  end
  local.get $1
  local.get $2
  i32.add
  local.get $0
  i32.le_u
  local.tee $3
  if (result i32)
   local.get $3
  else   
   local.get $0
   local.get $2
   i32.add
   local.get $1
   i32.le_u
  end
  if
   local.get $0
   local.get $1
   local.get $2
   call $~lib/internal/memory/memcpy
   return
  end
  local.get $0
  local.get $1
  i32.lt_u
  if
   local.get $1
   i32.const 7
   i32.and
   local.get $0
   i32.const 7
   i32.and
   i32.eq
   if
    block $break|0
     loop $continue|0
      local.get $0
      i32.const 7
      i32.and
      if
       block
        local.get $2
        i32.eqz
        if
         return
        end
        local.get $2
        i32.const 1
        i32.sub
        local.set $2
        block (result i32)
         local.get $0
         local.tee $3
         i32.const 1
         i32.add
         local.set $0
         local.get $3
        end
        block (result i32)
         local.get $1
         local.tee $3
         i32.const 1
         i32.add
         local.set $1
         local.get $3
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
      local.get $2
      i32.const 8
      i32.ge_u
      if
       block
        local.get $0
        local.get $1
        i64.load
        i64.store
        local.get $2
        i32.const 8
        i32.sub
        local.set $2
        local.get $0
        i32.const 8
        i32.add
        local.set $0
        local.get $1
        i32.const 8
        i32.add
        local.set $1
       end
       br $continue|1
      end
     end
    end
   end
   block $break|2
    loop $continue|2
     local.get $2
     if
      block
       block (result i32)
        local.get $0
        local.tee $3
        i32.const 1
        i32.add
        local.set $0
        local.get $3
       end
       block (result i32)
        local.get $1
        local.tee $3
        i32.const 1
        i32.add
        local.set $1
        local.get $3
       end
       i32.load8_u
       i32.store8
       local.get $2
       i32.const 1
       i32.sub
       local.set $2
      end
      br $continue|2
     end
    end
   end
  else   
   local.get $1
   i32.const 7
   i32.and
   local.get $0
   i32.const 7
   i32.and
   i32.eq
   if
    block $break|3
     loop $continue|3
      local.get $0
      local.get $2
      i32.add
      i32.const 7
      i32.and
      if
       block
        local.get $2
        i32.eqz
        if
         return
        end
        local.get $0
        local.get $2
        i32.const 1
        i32.sub
        local.tee $2
        i32.add
        local.get $1
        local.get $2
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
      local.get $2
      i32.const 8
      i32.ge_u
      if
       block
        local.get $2
        i32.const 8
        i32.sub
        local.set $2
        local.get $0
        local.get $2
        i32.add
        local.get $1
        local.get $2
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
     local.get $2
     if
      local.get $0
      local.get $2
      i32.const 1
      i32.sub
      local.tee $2
      i32.add
      local.get $1
      local.get $2
      i32.add
      i32.load8_u
      i32.store8
      br $continue|5
     end
    end
   end
  end
 )
 (func $~lib/allocator/arena/__memory_free (; 33 ;) (type $FUNCSIG$vi) (param $0 i32)
  nop
 )
 (func $~lib/internal/arraybuffer/reallocateUnsafe (; 34 ;) (type $FUNCSIG$iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  local.get $0
  i32.load
  local.set $2
  local.get $1
  local.get $2
  i32.gt_s
  if
   local.get $1
   i32.const 1073741816
   i32.le_s
   i32.eqz
   if
    call $~lib/env/abort
    unreachable
   end
   local.get $1
   local.get $2
   call $~lib/internal/arraybuffer/computeSize
   i32.const 8
   i32.sub
   i32.le_s
   if
    local.get $0
    local.get $1
    i32.store
   else    
    local.get $1
    call $~lib/internal/arraybuffer/allocateUnsafe
    local.set $3
    block $~lib/memory/memory.copy|inlined.0
     local.get $3
     i32.const 8
     i32.add
     local.set $4
     local.get $0
     i32.const 8
     i32.add
     local.set $5
     local.get $2
     local.set $6
     local.get $4
     local.get $5
     local.get $6
     call $~lib/internal/memory/memmove
    end
    block $~lib/memory/memory.free|inlined.0
     local.get $0
     local.set $6
     local.get $6
     call $~lib/allocator/arena/__memory_free
     br $~lib/memory/memory.free|inlined.0
    end
    local.get $3
    local.set $0
   end
   block $~lib/memory/memory.fill|inlined.1
    local.get $0
    i32.const 8
    i32.add
    local.get $2
    i32.add
    local.set $3
    i32.const 0
    local.set $6
    local.get $1
    local.get $2
    i32.sub
    local.set $5
    local.get $3
    local.get $6
    local.get $5
    call $~lib/internal/memory/memset
   end
  else   
   local.get $1
   local.get $2
   i32.lt_s
   if
    local.get $1
    i32.const 0
    i32.ge_s
    i32.eqz
    if
     call $~lib/env/abort
     unreachable
    end
    local.get $0
    local.get $1
    i32.store
   end
  end
  local.get $0
 )
 (func $~lib/array/Array<u8>#push (; 35 ;) (type $FUNCSIG$iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  (local $7 i32)
  (local $8 i32)
  (local $9 i32)
  local.get $0
  i32.load offset=4
  local.set $2
  local.get $0
  i32.load
  local.set $3
  local.get $3
  i32.load
  i32.const 0
  i32.shr_u
  local.set $4
  local.get $2
  i32.const 1
  i32.add
  local.set $5
  local.get $2
  local.get $4
  i32.ge_u
  if
   local.get $2
   i32.const 1073741816
   i32.ge_u
   if
    call $~lib/env/abort
    unreachable
   end
   local.get $3
   local.get $5
   i32.const 0
   i32.shl
   call $~lib/internal/arraybuffer/reallocateUnsafe
   local.set $3
   local.get $0
   local.get $3
   i32.store
  end
  local.get $0
  local.get $5
  i32.store offset=4
  block $~lib/internal/arraybuffer/STORE<u8,u8>|inlined.0
   local.get $3
   local.set $6
   local.get $2
   local.set $7
   local.get $1
   local.set $8
   i32.const 0
   local.set $9
   local.get $6
   local.get $7
   i32.const 0
   i32.shl
   i32.add
   local.get $9
   i32.add
   local.get $8
   i32.store8 offset=8
  end
  local.get $5
 )
 (func $~lib/utf8util/toUTF8Array (; 36 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  i32.const 0
  i32.const 0
  call $~lib/array/Array<u8>#constructor
  local.set $1
  block $break|0
   i32.const 0
   local.set $2
   loop $repeat|0
    local.get $2
    local.get $0
    i32.load
    i32.lt_s
    i32.eqz
    br_if $break|0
    block
     local.get $0
     local.get $2
     call $~lib/string/String#charCodeAt
     local.set $3
     local.get $3
     i32.const 128
     i32.lt_s
     if
      local.get $1
      local.get $3
      call $~lib/array/Array<u8>#push
      drop
     else      
      local.get $3
      i32.const 2048
      i32.lt_s
      if
       local.get $1
       i32.const 192
       local.get $3
       i32.const 6
       i32.shr_s
       i32.or
       call $~lib/array/Array<u8>#push
       drop
       local.get $1
       i32.const 128
       local.get $3
       i32.const 63
       i32.and
       i32.or
       call $~lib/array/Array<u8>#push
       drop
      else       
       local.get $3
       i32.const 55296
       i32.lt_s
       local.tee $4
       if (result i32)
        local.get $4
       else        
        local.get $3
        i32.const 57344
        i32.ge_s
       end
       if
        local.get $1
        i32.const 224
        local.get $3
        i32.const 12
        i32.shr_s
        i32.or
        call $~lib/array/Array<u8>#push
        drop
        local.get $1
        i32.const 128
        local.get $3
        i32.const 6
        i32.shr_s
        i32.const 63
        i32.and
        i32.or
        call $~lib/array/Array<u8>#push
        drop
        local.get $1
        i32.const 128
        local.get $3
        i32.const 63
        i32.and
        i32.or
        call $~lib/array/Array<u8>#push
        drop
       else        
        local.get $2
        i32.const 1
        i32.add
        local.set $2
        i32.const 65536
        local.get $3
        i32.const 1023
        i32.and
        i32.const 10
        i32.shl
        local.get $0
        local.get $2
        call $~lib/string/String#charCodeAt
        i32.const 1023
        i32.and
        i32.or
        i32.add
        local.set $3
        local.get $1
        i32.const 240
        local.get $3
        i32.const 18
        i32.shr_s
        i32.or
        call $~lib/array/Array<u8>#push
        drop
        local.get $1
        i32.const 128
        local.get $3
        i32.const 12
        i32.shr_s
        i32.const 63
        i32.and
        i32.or
        call $~lib/array/Array<u8>#push
        drop
        local.get $1
        i32.const 128
        local.get $3
        i32.const 6
        i32.shr_s
        i32.const 63
        i32.and
        i32.or
        call $~lib/array/Array<u8>#push
        drop
        local.get $1
        i32.const 128
        local.get $3
        i32.const 63
        i32.and
        i32.or
        call $~lib/array/Array<u8>#push
        drop
       end
      end
     end
    end
    local.get $2
    i32.const 1
    i32.add
    local.set $2
    br $repeat|0
    unreachable
   end
   unreachable
  end
  local.get $1
  i32.const 0
  call $~lib/array/Array<u8>#push
  drop
  local.get $1
 )
 (func $~lib/utf8util/string2cstr (; 37 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  local.get $0
  call $~lib/utf8util/toUTF8Array
  local.set $1
  local.get $1
  i32.load
  local.set $2
  local.get $2
  i32.const 8
  i32.add
 )
 (func $~lib/env/ultrain_assert (; 38 ;) (type $FUNCSIG$vii) (param $0 i32) (param $1 i32)
  local.get $0
  i32.const 0
  i32.ne
  i32.const 0
  i32.eq
  if
   i32.const 0
   local.get $1
   call $~lib/utf8util/string2cstr
   call $~lib/env/ultrainio_assert
  end
 )
 (func $~lib/ultrain-ts-lib/src/utils/toUTF8Array (; 39 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  i32.const 0
  i32.const 0
  call $~lib/array/Array<u8>#constructor
  local.set $1
  block $break|0
   i32.const 0
   local.set $2
   loop $repeat|0
    local.get $2
    local.get $0
    i32.load
    i32.lt_s
    i32.eqz
    br_if $break|0
    block
     local.get $0
     local.get $2
     call $~lib/string/String#charCodeAt
     local.set $3
     local.get $3
     i32.const 128
     i32.lt_s
     if
      local.get $1
      local.get $3
      call $~lib/array/Array<u8>#push
      drop
     else      
      local.get $3
      i32.const 2048
      i32.lt_s
      if
       local.get $1
       i32.const 192
       local.get $3
       i32.const 6
       i32.shr_s
       i32.or
       call $~lib/array/Array<u8>#push
       drop
       local.get $1
       i32.const 128
       local.get $3
       i32.const 63
       i32.and
       i32.or
       call $~lib/array/Array<u8>#push
       drop
      else       
       local.get $3
       i32.const 55296
       i32.lt_s
       local.tee $4
       if (result i32)
        local.get $4
       else        
        local.get $3
        i32.const 57344
        i32.ge_s
       end
       if
        local.get $1
        i32.const 224
        local.get $3
        i32.const 12
        i32.shr_s
        i32.or
        call $~lib/array/Array<u8>#push
        drop
        local.get $1
        i32.const 128
        local.get $3
        i32.const 6
        i32.shr_s
        i32.const 63
        i32.and
        i32.or
        call $~lib/array/Array<u8>#push
        drop
        local.get $1
        i32.const 128
        local.get $3
        i32.const 63
        i32.and
        i32.or
        call $~lib/array/Array<u8>#push
        drop
       else        
        local.get $2
        i32.const 1
        i32.add
        local.set $2
        i32.const 65536
        local.get $3
        i32.const 1023
        i32.and
        i32.const 10
        i32.shl
        local.get $0
        local.get $2
        call $~lib/string/String#charCodeAt
        i32.const 1023
        i32.and
        i32.or
        i32.add
        local.set $3
        local.get $1
        i32.const 240
        local.get $3
        i32.const 18
        i32.shr_s
        i32.or
        call $~lib/array/Array<u8>#push
        drop
        local.get $1
        i32.const 128
        local.get $3
        i32.const 12
        i32.shr_s
        i32.const 63
        i32.and
        i32.or
        call $~lib/array/Array<u8>#push
        drop
        local.get $1
        i32.const 128
        local.get $3
        i32.const 6
        i32.shr_s
        i32.const 63
        i32.and
        i32.or
        call $~lib/array/Array<u8>#push
        drop
        local.get $1
        i32.const 128
        local.get $3
        i32.const 63
        i32.and
        i32.or
        call $~lib/array/Array<u8>#push
        drop
       end
      end
     end
    end
    local.get $2
    i32.const 1
    i32.add
    local.set $2
    br $repeat|0
    unreachable
   end
   unreachable
  end
  local.get $1
  i32.const 0
  call $~lib/array/Array<u8>#push
  drop
  local.get $1
 )
 (func $~lib/ultrain-ts-lib/src/utils/string2cstr (; 40 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  local.get $0
  call $~lib/ultrain-ts-lib/src/utils/toUTF8Array
  local.set $1
  local.get $1
  i32.load
  local.set $2
  local.get $2
  i32.const 8
  i32.add
 )
 (func $~lib/ultrain-ts-lib/src/log/Logger#s (; 41 ;) (type $FUNCSIG$iii) (param $0 i32) (param $1 i32) (result i32)
  local.get $1
  call $~lib/ultrain-ts-lib/src/utils/string2cstr
  call $~lib/ultrain-ts-lib/src/log/env.ts_log_print_s
  local.get $0
 )
 (func $~lib/ultrain-ts-lib/src/log/Logger#i (; 42 ;) (type $FUNCSIG$iiji) (param $0 i32) (param $1 i64) (param $2 i32) (result i32)
  local.get $1
  local.get $2
  call $~lib/ultrain-ts-lib/src/log/env.ts_log_print_i
  local.get $0
 )
 (func $~lib/ultrain-ts-lib/src/log/Logger#flush (; 43 ;) (type $FUNCSIG$vi) (param $0 i32)
  call $~lib/ultrain-ts-lib/src/log/env.ts_log_done
 )
 (func $~lib/ultrain-ts-lib/src/asset/StringToSymbol (; 44 ;) (type $FUNCSIG$jii) (param $0 i32) (param $1 i32) (result i64)
  (local $2 i32)
  (local $3 i64)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  local.get $1
  i32.load
  local.set $2
  local.get $2
  i32.const 255
  i32.and
  i32.const 7
  i32.le_u
  i32.const 1296
  call $~lib/env/ultrain_assert
  i64.const 0
  local.set $3
  block $break|0
   i32.const 0
   local.set $4
   loop $repeat|0
    local.get $4
    local.get $2
    i32.const 255
    i32.and
    i32.lt_u
    i32.eqz
    br_if $break|0
    block
     local.get $1
     local.get $4
     i32.const 255
     i32.and
     call $~lib/string/String#charCodeAt
     i32.const 255
     i32.and
     local.set $5
     local.get $5
     global.get $~lib/ultrain-ts-lib/src/asset/CHAR_A
     i32.lt_u
     local.tee $6
     if (result i32)
      local.get $6
     else      
      local.get $5
      global.get $~lib/ultrain-ts-lib/src/asset/CHAR_Z
      i32.gt_u
     end
     if
      global.get $~lib/ultrain-ts-lib/src/log/Log
      i32.const 2040
      call $~lib/ultrain-ts-lib/src/log/Logger#s
      local.get $5
      i64.extend_i32_u
      i32.const 16
      call $~lib/ultrain-ts-lib/src/log/Logger#i
      call $~lib/ultrain-ts-lib/src/log/Logger#flush
     else      
      local.get $3
      local.get $5
      i64.extend_i32_u
      i64.const 8
      local.get $4
      i32.const 1
      i32.add
      i32.const 255
      i32.and
      i64.extend_i32_u
      i64.mul
      i64.shl
      i64.or
      local.set $3
     end
    end
    local.get $4
    i32.const 1
    i32.add
    local.set $4
    br $repeat|0
    unreachable
   end
   unreachable
  end
  local.get $3
  local.get $0
  i32.const 255
  i32.and
  i64.extend_i32_u
  i64.or
  local.set $3
  local.get $3
 )
 (func $start:~lib/ultrain-ts-lib/src/asset (; 45 ;) (type $FUNCSIG$v)
  call $start:~lib/ultrain-ts-lib/src/log
  i32.const 4
  i32.const 1280
  call $~lib/ultrain-ts-lib/src/asset/StringToSymbol
  global.set $~lib/ultrain-ts-lib/src/asset/SYS
  global.get $~lib/ultrain-ts-lib/src/asset/SYS
  i64.const 8
  i64.shr_u
  global.set $~lib/ultrain-ts-lib/src/asset/SYS_NAME
 )
 (func $start:~lib/ultrain-ts-lib/src/account (; 46 ;) (type $FUNCSIG$v)
  call $start:~lib/ultrain-ts-lib/src/asset
 )
 (func $start:~lib/ultrain-ts-lib/src/contract (; 47 ;) (type $FUNCSIG$v)
  call $start:~lib/ultrain-ts-lib/lib/name_ex
  call $start:~lib/ultrain-ts-lib/src/account
 )
 (func $~lib/ultrain-ts-lib/lib/name/char_to_symbol (; 48 ;) (type $FUNCSIG$ji) (param $0 i32) (result i64)
  (local $1 i32)
  local.get $0
  i32.const 255
  i32.and
  i32.const 97
  i32.ge_u
  local.tee $1
  if (result i32)
   local.get $0
   i32.const 255
   i32.and
   i32.const 122
   i32.le_u
  else   
   local.get $1
  end
  if
   local.get $0
   i32.const 97
   i32.sub
   i32.const 6
   i32.add
   i32.const 255
   i32.and
   i64.extend_i32_u
   return
  end
  local.get $0
  i32.const 255
  i32.and
  i32.const 49
  i32.ge_u
  local.tee $1
  if (result i32)
   local.get $0
   i32.const 255
   i32.and
   i32.const 53
   i32.le_u
  else   
   local.get $1
  end
  if
   local.get $0
   i32.const 49
   i32.sub
   i32.const 1
   i32.add
   i32.const 255
   i32.and
   i64.extend_i32_u
   return
  end
  i64.const 0
 )
 (func $~lib/ultrain-ts-lib/lib/name/N (; 49 ;) (type $FUNCSIG$ji) (param $0 i32) (result i64)
  (local $1 i32)
  (local $2 i64)
  (local $3 i32)
  (local $4 i64)
  (local $5 i32)
  local.get $0
  i32.load
  local.set $1
  i64.const 0
  local.set $2
  block $break|0
   i32.const 0
   local.set $3
   loop $repeat|0
    local.get $3
    i32.const 12
    i32.le_u
    i32.eqz
    br_if $break|0
    block
     i64.const 0
     local.set $4
     local.get $3
     local.get $1
     i32.lt_u
     local.tee $5
     if (result i32)
      local.get $3
      i32.const 12
      i32.le_u
     else      
      local.get $5
     end
     if
      local.get $0
      local.get $3
      call $~lib/string/String#charCodeAt
      i32.const 255
      i32.and
      call $~lib/ultrain-ts-lib/lib/name/char_to_symbol
      local.set $4
     end
     local.get $3
     i32.const 12
     i32.lt_u
     if
      local.get $4
      i64.const 31
      i64.and
      local.set $4
      local.get $4
      i64.const 64
      i64.const 5
      local.get $3
      i32.const 1
      i32.add
      i64.extend_i32_u
      i64.mul
      i64.sub
      i64.shl
      local.set $4
     else      
      local.get $4
      i64.const 15
      i64.and
      local.set $4
     end
     local.get $2
     local.get $4
     i64.or
     local.set $2
    end
    local.get $3
    i32.const 1
    i32.add
    local.set $3
    br $repeat|0
    unreachable
   end
   unreachable
  end
  local.get $2
 )
 (func $~lib/ultrain-ts-lib/src/account/NAME (; 50 ;) (type $FUNCSIG$ji) (param $0 i32) (result i64)
  local.get $0
  call $~lib/ultrain-ts-lib/lib/name/N
 )
 (func $start:contract/lib/random.lib (; 51 ;) (type $FUNCSIG$v)
  i32.const 2216
  call $~lib/ultrain-ts-lib/src/account/NAME
  global.set $contract/lib/random.lib/RAND_KEY
  i32.const 2256
  call $~lib/ultrain-ts-lib/src/account/NAME
  global.set $contract/lib/random.lib/MAIN_COUNT_KEY
  i32.const 2280
  call $~lib/ultrain-ts-lib/src/account/NAME
  global.set $contract/lib/random.lib/MAIN_VOTES_NUM_KEY
 )
 (func $~lib/ultrain-ts-lib/src/asset/Asset#constructor (; 52 ;) (type $FUNCSIG$iijj) (param $0 i32) (param $1 i64) (param $2 i64) (result i32)
  block (result i32)
   local.get $0
   i32.eqz
   if
    i32.const 16
    call $~lib/memory/memory.allocate
    local.set $0
   end
   local.get $0
   i64.const 0
   i64.store
   local.get $0
   i64.const 0
   i64.store offset=8
   local.get $0
  end
  local.get $1
  i64.store
  local.get $0
  local.get $2
  i64.store offset=8
  local.get $0
 )
 (func $start:contract/ultrainio.rand (; 53 ;) (type $FUNCSIG$v)
  call $start:~lib/ultrain-ts-lib/src/contract
  call $start:contract/lib/random.lib
  i32.const 0
  i64.const 20000
  i64.const 357577479428
  call $~lib/ultrain-ts-lib/src/asset/Asset#constructor
  global.set $contract/ultrainio.rand/MAIN_DEPOSIT_AMOUNT
  i32.const 0
  i64.const 2000
  i64.const 357577479428
  call $~lib/ultrain-ts-lib/src/asset/Asset#constructor
  global.set $contract/ultrainio.rand/WAITER_DEPOSIT_AMOUNT
 )
 (func $~lib/ultrain-ts-lib/src/contract/Contract#constructor (; 54 ;) (type $FUNCSIG$iij) (param $0 i32) (param $1 i64) (result i32)
  block (result i32)
   local.get $0
   i32.eqz
   if
    i32.const 12
    call $~lib/memory/memory.allocate
    local.set $0
   end
   local.get $0
   i64.const 0
   i64.store
   local.get $0
   i32.const 0
   i32.store offset=8
   local.get $0
  end
  local.get $1
  i64.store
  local.get $0
 )
 (func $~lib/dbmanager/DBManager<Voter>#constructor (; 55 ;) (type $FUNCSIG$iijj) (param $0 i32) (param $1 i64) (param $2 i64) (result i32)
  block (result i32)
   local.get $0
   i32.eqz
   if
    i32.const 24
    call $~lib/memory/memory.allocate
    local.set $0
   end
   local.get $0
   i64.const 0
   i64.store
   local.get $0
   i64.const 0
   i64.store offset=8
   local.get $0
   i64.const 0
   i64.store offset=16
   local.get $0
  end
  local.get $1
  i64.store
  local.get $0
  call $~lib/env/current_receiver
  i64.store offset=8
  local.get $0
  local.get $2
  i64.store offset=16
  local.get $0
 )
 (func $~lib/dbmanager/DBManager.newInstance<Voter> (; 56 ;) (type $FUNCSIG$ijjj) (param $0 i64) (param $1 i64) (param $2 i64) (result i32)
  (local $3 i32)
  i32.const 0
  local.get $0
  local.get $2
  call $~lib/dbmanager/DBManager<Voter>#constructor
  local.set $3
  local.get $3
  local.get $1
  i64.store offset=8
  local.get $3
 )
 (func $~lib/dbmanager/DBManager<Waiter>#constructor (; 57 ;) (type $FUNCSIG$iijj) (param $0 i32) (param $1 i64) (param $2 i64) (result i32)
  block (result i32)
   local.get $0
   i32.eqz
   if
    i32.const 24
    call $~lib/memory/memory.allocate
    local.set $0
   end
   local.get $0
   i64.const 0
   i64.store
   local.get $0
   i64.const 0
   i64.store offset=8
   local.get $0
   i64.const 0
   i64.store offset=16
   local.get $0
  end
  local.get $1
  i64.store
  local.get $0
  call $~lib/env/current_receiver
  i64.store offset=8
  local.get $0
  local.get $2
  i64.store offset=16
  local.get $0
 )
 (func $~lib/dbmanager/DBManager.newInstance<Waiter> (; 58 ;) (type $FUNCSIG$ijjj) (param $0 i64) (param $1 i64) (param $2 i64) (result i32)
  (local $3 i32)
  i32.const 0
  local.get $0
  local.get $2
  call $~lib/dbmanager/DBManager<Waiter>#constructor
  local.set $3
  local.get $3
  local.get $1
  i64.store offset=8
  local.get $3
 )
 (func $~lib/dbmanager/DBManager<RandRecord>#constructor (; 59 ;) (type $FUNCSIG$iijj) (param $0 i32) (param $1 i64) (param $2 i64) (result i32)
  block (result i32)
   local.get $0
   i32.eqz
   if
    i32.const 24
    call $~lib/memory/memory.allocate
    local.set $0
   end
   local.get $0
   i64.const 0
   i64.store
   local.get $0
   i64.const 0
   i64.store offset=8
   local.get $0
   i64.const 0
   i64.store offset=16
   local.get $0
  end
  local.get $1
  i64.store
  local.get $0
  call $~lib/env/current_receiver
  i64.store offset=8
  local.get $0
  local.get $2
  i64.store offset=16
  local.get $0
 )
 (func $~lib/dbmanager/DBManager.newInstance<RandRecord> (; 60 ;) (type $FUNCSIG$ijjj) (param $0 i64) (param $1 i64) (param $2 i64) (result i32)
  (local $3 i32)
  i32.const 0
  local.get $0
  local.get $2
  call $~lib/dbmanager/DBManager<RandRecord>#constructor
  local.set $3
  local.get $3
  local.get $1
  i64.store offset=8
  local.get $3
 )
 (func $contract/lib/random.lib/Random#constructor (; 61 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  block (result i32)
   local.get $0
   i32.eqz
   if
    i32.const 12
    call $~lib/memory/memory.allocate
    local.set $0
   end
   local.get $0
   i32.const 0
   i32.store
   local.get $0
   i32.const 0
   i32.store offset=4
   local.get $0
   i32.const 0
   i32.store offset=8
   local.get $0
  end
  global.get $contract/lib/random.lib/VOTER_TABLE
  call $~lib/ultrain-ts-lib/src/account/NAME
  global.get $contract/lib/random.lib/CONT_NAME
  call $~lib/ultrain-ts-lib/src/account/NAME
  global.get $contract/lib/random.lib/VOTER_TABLE
  call $~lib/ultrain-ts-lib/src/account/NAME
  call $~lib/dbmanager/DBManager.newInstance<Voter>
  i32.store
  local.get $0
  global.get $contract/lib/random.lib/WAITER_TABLE
  call $~lib/ultrain-ts-lib/src/account/NAME
  global.get $contract/lib/random.lib/CONT_NAME
  call $~lib/ultrain-ts-lib/src/account/NAME
  global.get $contract/lib/random.lib/WAITER_TABLE
  call $~lib/ultrain-ts-lib/src/account/NAME
  call $~lib/dbmanager/DBManager.newInstance<Waiter>
  i32.store offset=4
  local.get $0
  global.get $contract/lib/random.lib/RAND_TABLE
  call $~lib/ultrain-ts-lib/src/account/NAME
  global.get $contract/lib/random.lib/CONT_NAME
  call $~lib/ultrain-ts-lib/src/account/NAME
  global.get $contract/lib/random.lib/RAND_TABLE
  call $~lib/ultrain-ts-lib/src/account/NAME
  call $~lib/dbmanager/DBManager.newInstance<RandRecord>
  i32.store offset=8
  local.get $0
 )
 (func $~lib/dbmanager/DBManager<Voter>#find (; 62 ;) (type $FUNCSIG$iij) (param $0 i32) (param $1 i64) (result i32)
  (local $2 i32)
  local.get $0
  i64.load offset=8
  local.get $0
  i64.load offset=16
  local.get $0
  i64.load
  local.get $1
  call $~lib/env/db_find_i64
  local.set $2
  local.get $2
 )
 (func $~lib/dbmanager/DBManager<Voter>#exists (; 63 ;) (type $FUNCSIG$iij) (param $0 i32) (param $1 i64) (result i32)
  (local $2 i32)
  local.get $0
  local.get $1
  call $~lib/dbmanager/DBManager<Voter>#find
  local.set $2
  local.get $2
  i32.const 0
  i32.lt_s
  if (result i32)
   i32.const 0
  else   
   i32.const 1
  end
 )
 (func $~lib/array/Array<u64>#constructor (; 64 ;) (type $FUNCSIG$iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  local.get $1
  i32.const 134217727
  i32.gt_u
  if
   call $~lib/env/abort
   unreachable
  end
  local.get $1
  i32.const 3
  i32.shl
  local.set $2
  local.get $2
  call $~lib/internal/arraybuffer/allocateUnsafe
  local.set $3
  block (result i32)
   local.get $0
   i32.eqz
   if
    i32.const 8
    call $~lib/memory/memory.allocate
    local.set $0
   end
   local.get $0
   i32.const 0
   i32.store
   local.get $0
   i32.const 0
   i32.store offset=4
   local.get $0
  end
  local.get $3
  i32.store
  local.get $0
  local.get $1
  i32.store offset=4
  block $~lib/memory/memory.fill|inlined.2
   local.get $3
   i32.const 8
   i32.add
   local.set $4
   i32.const 0
   local.set $5
   local.get $2
   local.set $6
   local.get $4
   local.get $5
   local.get $6
   call $~lib/internal/memory/memset
  end
  local.get $0
 )
 (func $contract/lib/random.lib/Voter#constructor (; 65 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  block (result i32)
   local.get $0
   i32.eqz
   if
    i32.const 44
    call $~lib/memory/memory.allocate
    local.set $0
   end
   local.get $0
   i64.const 0
   i64.store
   local.get $0
   i32.const 0
   i32.store offset=8
   local.get $0
   i32.const 0
   i32.store offset=12
   local.get $0
   i64.const 0
   i64.store offset=16
   local.get $0
   i64.const 0
   i64.store offset=24
   local.get $0
   i64.const 0
   i64.store offset=32
   local.get $0
   i32.const 0
   i32.store offset=40
   local.get $0
  end
  i32.const 0
  global.get $contract/lib/random.lib/EPOCH
  i32.wrap_i64
  call $~lib/array/Array<u64>#constructor
  i32.store offset=8
  local.get $0
  i32.const 0
  global.get $contract/lib/random.lib/EPOCH
  i32.wrap_i64
  call $~lib/array/Array<u64>#constructor
  i32.store offset=12
  local.get $0
 )
 (func $~lib/datastream/DataStream#constructor (; 66 ;) (type $FUNCSIG$iiii) (param $0 i32) (param $1 i32) (param $2 i32) (result i32)
  block (result i32)
   local.get $0
   i32.eqz
   if
    i32.const 12
    call $~lib/memory/memory.allocate
    local.set $0
   end
   local.get $0
   i32.const 0
   i32.store
   local.get $0
   i32.const 0
   i32.store offset=4
   local.get $0
   i32.const 0
   i32.store offset=8
   local.get $0
  end
  local.get $1
  i32.store
  local.get $0
  local.get $2
  i32.store offset=4
  local.get $0
  i32.const 0
  i32.store offset=8
  local.get $0
 )
 (func $~lib/datastream/DataStream#isMeasureMode (; 67 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  local.get $0
  i32.load
  i32.const 0
  i32.eq
 )
 (func $~lib/datastream/DataStream#write<u64> (; 68 ;) (type $FUNCSIG$vij) (param $0 i32) (param $1 i64)
  local.get $0
  call $~lib/datastream/DataStream#isMeasureMode
  i32.eqz
  if
   local.get $0
   i32.load
   local.get $0
   i32.load offset=8
   i32.add
   local.get $1
   i64.store
  end
  local.get $0
  local.get $0
  i32.load offset=8
  i32.const 8
  i32.add
  i32.store offset=8
 )
 (func $~lib/datastream/DataStream#write<u8> (; 69 ;) (type $FUNCSIG$vii) (param $0 i32) (param $1 i32)
  local.get $0
  call $~lib/datastream/DataStream#isMeasureMode
  i32.eqz
  if
   local.get $0
   i32.load
   local.get $0
   i32.load offset=8
   i32.add
   local.get $1
   i32.store8
  end
  local.get $0
  local.get $0
  i32.load offset=8
  i32.const 1
  i32.add
  i32.store offset=8
 )
 (func $~lib/datastream/DataStream#writeVarint32 (; 70 ;) (type $FUNCSIG$vii) (param $0 i32) (param $1 i32)
  (local $2 i32)
  loop $continue|0
   block
    local.get $1
    i32.const 127
    i32.and
    local.set $2
    local.get $1
    i32.const 7
    i32.shr_u
    local.set $1
    local.get $2
    local.get $1
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
    local.set $2
    local.get $0
    local.get $2
    call $~lib/datastream/DataStream#write<u8>
   end
   local.get $1
   br_if $continue|0
  end
 )
 (func $~lib/array/Array<u64>#__get (; 71 ;) (type $FUNCSIG$jii) (param $0 i32) (param $1 i32) (result i64)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  local.get $0
  i32.load
  local.set $2
  local.get $1
  local.get $2
  i32.load
  i32.const 3
  i32.shr_u
  i32.lt_u
  if (result i64)
   local.get $2
   local.set $3
   local.get $1
   local.set $4
   i32.const 0
   local.set $5
   local.get $3
   local.get $4
   i32.const 3
   i32.shl
   i32.add
   local.get $5
   i32.add
   i64.load offset=8
  else   
   unreachable
  end
 )
 (func $~lib/datastream/DataStream#writeVector<u64> (; 72 ;) (type $FUNCSIG$vii) (param $0 i32) (param $1 i32)
  (local $2 i32)
  (local $3 i32)
  block $~lib/array/Array<u64>#get:length|inlined.0 (result i32)
   local.get $1
   local.set $2
   local.get $2
   i32.load offset=4
  end
  local.set $2
  local.get $0
  local.get $2
  call $~lib/datastream/DataStream#writeVarint32
  block $break|0
   i32.const 0
   local.set $3
   loop $repeat|0
    local.get $3
    local.get $2
    i32.lt_u
    i32.eqz
    br_if $break|0
    local.get $0
    local.get $1
    local.get $3
    call $~lib/array/Array<u64>#__get
    call $~lib/datastream/DataStream#write<u64>
    local.get $3
    i32.const 1
    i32.add
    local.set $3
    br $repeat|0
    unreachable
   end
   unreachable
  end
 )
 (func $~lib/datastream/DataStream#write<i32> (; 73 ;) (type $FUNCSIG$vii) (param $0 i32) (param $1 i32)
  local.get $0
  call $~lib/datastream/DataStream#isMeasureMode
  i32.eqz
  if
   local.get $0
   i32.load
   local.get $0
   i32.load offset=8
   i32.add
   local.get $1
   i32.store
  end
  local.get $0
  local.get $0
  i32.load offset=8
  i32.const 4
  i32.add
  i32.store offset=8
 )
 (func $contract/lib/random.lib/Voter#serialize (; 74 ;) (type $FUNCSIG$vii) (param $0 i32) (param $1 i32)
  local.get $1
  local.get $0
  i64.load
  call $~lib/datastream/DataStream#write<u64>
  local.get $1
  local.get $0
  i32.load offset=8
  call $~lib/datastream/DataStream#writeVector<u64>
  local.get $1
  local.get $0
  i32.load offset=12
  call $~lib/datastream/DataStream#writeVector<u64>
  local.get $1
  local.get $0
  i64.load offset=16
  call $~lib/datastream/DataStream#write<u64>
  local.get $1
  local.get $0
  i64.load offset=24
  call $~lib/datastream/DataStream#write<u64>
  local.get $1
  local.get $0
  i64.load offset=32
  call $~lib/datastream/DataStream#write<u64>
  local.get $1
  local.get $0
  i32.load offset=40
  call $~lib/datastream/DataStream#write<i32>
 )
 (func $~lib/datastream/DataStream.measure<Voter> (; 75 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  (local $1 i32)
  i32.const 0
  i32.const 0
  i32.const 0
  call $~lib/datastream/DataStream#constructor
  local.set $1
  local.get $0
  local.get $1
  call $contract/lib/random.lib/Voter#serialize
  local.get $1
  i32.load offset=8
 )
 (func $~lib/internal/typedarray/TypedArray<u8>#constructor (; 76 ;) (type $FUNCSIG$iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  local.get $1
  i32.const 1073741816
  i32.gt_u
  if
   call $~lib/env/abort
   unreachable
  end
  local.get $1
  i32.const 0
  i32.shl
  local.set $2
  local.get $2
  call $~lib/internal/arraybuffer/allocateUnsafe
  local.set $3
  block $~lib/memory/memory.fill|inlined.3
   local.get $3
   i32.const 8
   i32.add
   local.set $4
   i32.const 0
   local.set $5
   local.get $2
   local.set $6
   local.get $4
   local.get $5
   local.get $6
   call $~lib/internal/memory/memset
  end
  block (result i32)
   local.get $0
   i32.eqz
   if
    i32.const 12
    call $~lib/memory/memory.allocate
    local.set $0
   end
   local.get $0
   i32.const 0
   i32.store
   local.get $0
   i32.const 0
   i32.store offset=4
   local.get $0
   i32.const 0
   i32.store offset=8
   local.get $0
  end
  local.get $3
  i32.store
  local.get $0
  i32.const 0
  i32.store offset=4
  local.get $0
  local.get $2
  i32.store offset=8
  local.get $0
 )
 (func $~lib/typedarray/Uint8Array#constructor (; 77 ;) (type $FUNCSIG$iii) (param $0 i32) (param $1 i32) (result i32)
  local.get $0
  i32.eqz
  if
   i32.const 12
   call $~lib/memory/memory.allocate
   local.set $0
  end
  local.get $0
  local.get $1
  call $~lib/internal/typedarray/TypedArray<u8>#constructor
  local.set $0
  local.get $0
 )
 (func $contract/lib/random.lib/Voter#primaryKey (; 78 ;) (type $FUNCSIG$ji) (param $0 i32) (result i64)
  local.get $0
  i64.load
 )
 (func $~lib/dbmanager/DBManager<Voter>#emplace (; 79 ;) (type $FUNCSIG$vii) (param $0 i32) (param $1 i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i64)
  local.get $1
  call $~lib/datastream/DataStream.measure<Voter>
  local.set $2
  i32.const 0
  local.get $2
  call $~lib/typedarray/Uint8Array#constructor
  local.set $3
  i32.const 0
  local.get $3
  i32.load
  local.get $2
  call $~lib/datastream/DataStream#constructor
  local.set $4
  local.get $1
  local.get $4
  call $contract/lib/random.lib/Voter#serialize
  local.get $1
  call $contract/lib/random.lib/Voter#primaryKey
  local.set $5
  local.get $0
  i64.load offset=16
  local.get $0
  i64.load
  local.get $0
  i64.load offset=8
  local.get $5
  local.get $4
  i32.load
  local.get $4
  i32.load offset=8
  call $~lib/env/db_store_i64
  drop
 )
 (func $~lib/dbmanager/DBManager<Waiter>#find (; 80 ;) (type $FUNCSIG$iij) (param $0 i32) (param $1 i64) (result i32)
  (local $2 i32)
  local.get $0
  i64.load offset=8
  local.get $0
  i64.load offset=16
  local.get $0
  i64.load
  local.get $1
  call $~lib/env/db_find_i64
  local.set $2
  local.get $2
 )
 (func $~lib/dbmanager/DBManager<Waiter>#exists (; 81 ;) (type $FUNCSIG$iij) (param $0 i32) (param $1 i64) (result i32)
  (local $2 i32)
  local.get $0
  local.get $1
  call $~lib/dbmanager/DBManager<Waiter>#find
  local.set $2
  local.get $2
  i32.const 0
  i32.lt_s
  if (result i32)
   i32.const 0
  else   
   i32.const 1
  end
 )
 (func $contract/lib/random.lib/Waiter#constructor (; 82 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  local.get $0
  i32.eqz
  if
   i32.const 4
   call $~lib/memory/memory.allocate
   local.set $0
  end
  local.get $0
  i32.const 0
  i32.const 0
  call $~lib/array/Array<u64>#constructor
  i32.store
  local.get $0
 )
 (func $contract/lib/random.lib/Waiter#serialize (; 83 ;) (type $FUNCSIG$vii) (param $0 i32) (param $1 i32)
  local.get $1
  local.get $0
  i32.load
  call $~lib/datastream/DataStream#writeVector<u64>
 )
 (func $~lib/datastream/DataStream.measure<Waiter> (; 84 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  (local $1 i32)
  i32.const 0
  i32.const 0
  i32.const 0
  call $~lib/datastream/DataStream#constructor
  local.set $1
  local.get $0
  local.get $1
  call $contract/lib/random.lib/Waiter#serialize
  local.get $1
  i32.load offset=8
 )
 (func $contract/lib/random.lib/Waiter#primaryKey (; 85 ;) (type $FUNCSIG$ji) (param $0 i32) (result i64)
  i64.const 0
 )
 (func $~lib/dbmanager/DBManager<Waiter>#emplace (; 86 ;) (type $FUNCSIG$vii) (param $0 i32) (param $1 i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i64)
  local.get $1
  call $~lib/datastream/DataStream.measure<Waiter>
  local.set $2
  i32.const 0
  local.get $2
  call $~lib/typedarray/Uint8Array#constructor
  local.set $3
  i32.const 0
  local.get $3
  i32.load
  local.get $2
  call $~lib/datastream/DataStream#constructor
  local.set $4
  local.get $1
  local.get $4
  call $contract/lib/random.lib/Waiter#serialize
  local.get $1
  call $contract/lib/random.lib/Waiter#primaryKey
  local.set $5
  local.get $0
  i64.load offset=16
  local.get $0
  i64.load
  local.get $0
  i64.load offset=8
  local.get $5
  local.get $4
  i32.load
  local.get $4
  i32.load offset=8
  call $~lib/env/db_store_i64
  drop
 )
 (func $~lib/dbmanager/DBManager<RandRecord>#find (; 87 ;) (type $FUNCSIG$iij) (param $0 i32) (param $1 i64) (result i32)
  (local $2 i32)
  local.get $0
  i64.load offset=8
  local.get $0
  i64.load offset=16
  local.get $0
  i64.load
  local.get $1
  call $~lib/env/db_find_i64
  local.set $2
  local.get $2
 )
 (func $~lib/dbmanager/DBManager<RandRecord>#exists (; 88 ;) (type $FUNCSIG$iij) (param $0 i32) (param $1 i64) (result i32)
  (local $2 i32)
  local.get $0
  local.get $1
  call $~lib/dbmanager/DBManager<RandRecord>#find
  local.set $2
  local.get $2
  i32.const 0
  i32.lt_s
  if (result i32)
   i32.const 0
  else   
   i32.const 1
  end
 )
 (func $contract/lib/random.lib/RandRecord#constructor (; 89 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  local.get $0
  i32.eqz
  if
   i32.const 20
   call $~lib/memory/memory.allocate
   local.set $0
  end
  local.get $0
  i64.const 0
  i64.store
  local.get $0
  i64.const 0
  i64.store offset=8
  local.get $0
  i32.const 0
  i32.store offset=16
  local.get $0
 )
 (func $~lib/ultrain-ts-lib/lib/headblock/HeadBlock.number.get:number (; 90 ;) (type $FUNCSIG$i) (result i32)
  call $~lib/ultrain-ts-lib/lib/headblock/env.head_block_number
 )
 (func $~lib/ultrain-ts-lib/src/block/Block.number.get:number (; 91 ;) (type $FUNCSIG$i) (result i32)
  call $~lib/ultrain-ts-lib/lib/headblock/HeadBlock.number.get:number
 )
 (func $contract/lib/random.lib/RandRecord#serialize (; 92 ;) (type $FUNCSIG$vii) (param $0 i32) (param $1 i32)
  local.get $1
  local.get $0
  i64.load
  call $~lib/datastream/DataStream#write<u64>
  local.get $1
  local.get $0
  i64.load offset=8
  call $~lib/datastream/DataStream#write<u64>
  local.get $1
  local.get $0
  i32.load offset=16
  call $~lib/datastream/DataStream#write<i32>
 )
 (func $~lib/datastream/DataStream.measure<RandRecord> (; 93 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  (local $1 i32)
  i32.const 0
  i32.const 0
  i32.const 0
  call $~lib/datastream/DataStream#constructor
  local.set $1
  local.get $0
  local.get $1
  call $contract/lib/random.lib/RandRecord#serialize
  local.get $1
  i32.load offset=8
 )
 (func $contract/lib/random.lib/RandRecord#primaryKey (; 94 ;) (type $FUNCSIG$ji) (param $0 i32) (result i64)
  local.get $0
  i64.load
 )
 (func $~lib/dbmanager/DBManager<RandRecord>#emplace (; 95 ;) (type $FUNCSIG$vii) (param $0 i32) (param $1 i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i64)
  local.get $1
  call $~lib/datastream/DataStream.measure<RandRecord>
  local.set $2
  i32.const 0
  local.get $2
  call $~lib/typedarray/Uint8Array#constructor
  local.set $3
  i32.const 0
  local.get $3
  i32.load
  local.get $2
  call $~lib/datastream/DataStream#constructor
  local.set $4
  local.get $1
  local.get $4
  call $contract/lib/random.lib/RandRecord#serialize
  local.get $1
  call $contract/lib/random.lib/RandRecord#primaryKey
  local.set $5
  local.get $0
  i64.load offset=16
  local.get $0
  i64.load
  local.get $0
  i64.load offset=8
  local.get $5
  local.get $4
  i32.load
  local.get $4
  i32.load offset=8
  call $~lib/env/db_store_i64
  drop
 )
 (func $~lib/ultrain-ts-lib/src/crypto/Crypto#constructor (; 96 ;) (type $FUNCSIG$iii) (param $0 i32) (param $1 i32) (result i32)
  block (result i32)
   local.get $0
   i32.eqz
   if
    i32.const 4
    call $~lib/memory/memory.allocate
    local.set $0
   end
   local.get $0
   i32.const 0
   i32.store
   local.get $0
  end
  i32.const 0
  local.get $1
  call $~lib/typedarray/Uint8Array#constructor
  i32.store
  local.get $0
 )
 (func $~lib/ultrain-ts-lib/src/crypto/SHA256#constructor (; 97 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  local.get $0
  if (result i32)
   local.get $0
  else   
   i32.const 4
   call $~lib/memory/memory.allocate
  end
  i32.const 32
  call $~lib/ultrain-ts-lib/src/crypto/Crypto#constructor
  local.set $0
  local.get $0
 )
 (func $~lib/array/Array<String>#__get (; 98 ;) (type $FUNCSIG$iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  local.get $0
  i32.load
  local.set $2
  local.get $1
  local.get $2
  i32.load
  i32.const 2
  i32.shr_u
  i32.lt_u
  if (result i32)
   local.get $2
   local.set $3
   local.get $1
   local.set $4
   i32.const 0
   local.set $5
   local.get $3
   local.get $4
   i32.const 2
   i32.shl
   i32.add
   local.get $5
   i32.add
   i32.load offset=8
  else   
   unreachable
  end
 )
 (func $~lib/internal/string/allocateUnsafe (; 99 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  local.get $0
  i32.const 0
  i32.gt_s
  local.tee $1
  if (result i32)
   local.get $0
   i32.const 536870910
   i32.le_s
  else   
   local.get $1
  end
  i32.eqz
  if
   call $~lib/env/abort
   unreachable
  end
  block $~lib/memory/memory.allocate|inlined.1 (result i32)
   i32.const 4
   local.get $0
   i32.const 1
   i32.shl
   i32.add
   local.set $1
   local.get $1
   call $~lib/allocator/arena/__memory_allocate
   br $~lib/memory/memory.allocate|inlined.1
  end
  local.set $2
  local.get $2
  local.get $0
  i32.store
  local.get $2
 )
 (func $~lib/internal/string/copyUnsafe (; 100 ;) (type $FUNCSIG$viiiii) (param $0 i32) (param $1 i32) (param $2 i32) (param $3 i32) (param $4 i32)
  (local $5 i32)
  (local $6 i32)
  (local $7 i32)
  local.get $0
  local.get $1
  i32.const 1
  i32.shl
  i32.add
  i32.const 4
  i32.add
  local.set $5
  local.get $2
  local.get $3
  i32.const 1
  i32.shl
  i32.add
  i32.const 4
  i32.add
  local.set $6
  local.get $4
  i32.const 1
  i32.shl
  local.set $7
  local.get $5
  local.get $6
  local.get $7
  call $~lib/internal/memory/memmove
 )
 (func $~lib/string/String#concat (; 101 ;) (type $FUNCSIG$iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  local.get $0
  i32.const 0
  i32.ne
  i32.eqz
  if
   call $~lib/env/abort
   unreachable
  end
  local.get $1
  i32.const 0
  i32.eq
  if
   i32.const 2368
   local.set $1
  end
  local.get $0
  i32.load
  local.set $2
  local.get $1
  i32.load
  local.set $3
  local.get $2
  local.get $3
  i32.add
  local.set $4
  local.get $4
  i32.const 0
  i32.eq
  if
   i32.const 2384
   return
  end
  local.get $4
  call $~lib/internal/string/allocateUnsafe
  local.set $5
  local.get $5
  i32.const 0
  local.get $0
  i32.const 0
  local.get $2
  call $~lib/internal/string/copyUnsafe
  local.get $5
  local.get $2
  local.get $1
  i32.const 0
  local.get $3
  call $~lib/internal/string/copyUnsafe
  local.get $5
 )
 (func $~lib/string/String.__concat (; 102 ;) (type $FUNCSIG$iii) (param $0 i32) (param $1 i32) (result i32)
  local.get $0
  i32.eqz
  if
   i32.const 2368
   local.set $0
  end
  local.get $0
  local.get $1
  call $~lib/string/String#concat
 )
 (func $~lib/internal/string/repeatUnsafe (; 103 ;) (type $FUNCSIG$viiii) (param $0 i32) (param $1 i32) (param $2 i32) (param $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  (local $7 i32)
  (local $8 i32)
  (local $9 i64)
  (local $10 i32)
  (local $11 i32)
  (local $12 i32)
  (local $13 i32)
  local.get $2
  i32.load
  local.set $4
  block $break|0
   block $case5|0
    block $case4|0
     block $case3|0
      block $case2|0
       block $case1|0
        block $case0|0
         local.get $4
         local.set $5
         local.get $5
         i32.const 0
         i32.eq
         br_if $case0|0
         local.get $5
         i32.const 1
         i32.eq
         br_if $case1|0
         local.get $5
         i32.const 2
         i32.eq
         br_if $case2|0
         local.get $5
         i32.const 3
         i32.eq
         br_if $case3|0
         local.get $5
         i32.const 4
         i32.eq
         br_if $case4|0
         br $case5|0
        end
        br $break|0
       end
       block
        local.get $2
        i32.load16_u offset=4
        local.set $5
        local.get $0
        local.get $1
        i32.const 1
        i32.shl
        i32.add
        local.set $6
        block $break|1
         i32.const 0
         local.set $7
         loop $repeat|1
          local.get $7
          local.get $3
          i32.lt_s
          i32.eqz
          br_if $break|1
          local.get $6
          local.get $7
          i32.const 1
          i32.shl
          i32.add
          local.get $5
          i32.store16 offset=4
          local.get $7
          i32.const 1
          i32.add
          local.set $7
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
       local.get $2
       i32.load offset=4
       local.set $6
       local.get $0
       local.get $1
       i32.const 1
       i32.shl
       i32.add
       local.set $5
       block $break|2
        i32.const 0
        local.set $7
        loop $repeat|2
         local.get $7
         local.get $3
         i32.lt_s
         i32.eqz
         br_if $break|2
         local.get $5
         local.get $7
         i32.const 2
         i32.shl
         i32.add
         local.get $6
         i32.store offset=4
         local.get $7
         i32.const 1
         i32.add
         local.set $7
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
      local.get $2
      i32.load offset=4
      local.set $5
      local.get $2
      i32.load16_u offset=8
      local.set $6
      local.get $0
      local.get $1
      i32.const 1
      i32.shl
      i32.add
      local.set $7
      block $break|3
       i32.const 0
       local.set $8
       loop $repeat|3
        local.get $8
        local.get $3
        i32.lt_s
        i32.eqz
        br_if $break|3
        block
         local.get $7
         local.get $8
         i32.const 2
         i32.shl
         i32.add
         local.get $5
         i32.store offset=4
         local.get $7
         local.get $8
         i32.const 1
         i32.shl
         i32.add
         local.get $6
         i32.store16 offset=8
        end
        local.get $8
        i32.const 1
        i32.add
        local.set $8
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
     local.get $2
     i64.load offset=4
     local.set $9
     local.get $0
     local.get $1
     i32.const 1
     i32.shl
     i32.add
     local.set $7
     block $break|4
      i32.const 0
      local.set $6
      loop $repeat|4
       local.get $6
       local.get $3
       i32.lt_s
       i32.eqz
       br_if $break|4
       local.get $7
       local.get $6
       i32.const 3
       i32.shl
       i32.add
       local.get $9
       i64.store offset=4
       local.get $6
       i32.const 1
       i32.add
       local.set $6
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
    local.get $4
    i32.const 1
    i32.shl
    local.set $7
    local.get $0
    i32.const 4
    i32.add
    local.get $1
    i32.const 1
    i32.shl
    i32.add
    local.set $6
    local.get $2
    i32.const 4
    i32.add
    local.set $5
    block $break|5
     block
      i32.const 0
      local.set $8
      local.get $7
      local.get $3
      i32.mul
      local.set $10
     end
     loop $repeat|5
      local.get $8
      local.get $10
      i32.lt_s
      i32.eqz
      br_if $break|5
      block $~lib/memory/memory.copy|inlined.2
       local.get $6
       local.get $8
       i32.add
       local.set $11
       local.get $5
       local.set $12
       local.get $7
       local.set $13
       local.get $11
       local.get $12
       local.get $13
       call $~lib/internal/memory/memmove
      end
      local.get $8
      local.get $7
      i32.add
      local.set $8
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
 (func $~lib/string/String#repeat (; 104 ;) (type $FUNCSIG$iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  local.get $0
  i32.const 0
  i32.ne
  i32.eqz
  if
   call $~lib/env/abort
   unreachable
  end
  local.get $0
  i32.load
  local.set $2
  local.get $1
  i32.const 0
  i32.lt_s
  local.tee $3
  if (result i32)
   local.get $3
  else   
   local.get $2
   local.get $1
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
  local.get $1
  i32.const 0
  i32.eq
  local.tee $3
  if (result i32)
   local.get $3
  else   
   local.get $2
   i32.eqz
  end
  if
   i32.const 2384
   return
  end
  local.get $1
  i32.const 1
  i32.eq
  if
   local.get $0
   return
  end
  local.get $2
  local.get $1
  i32.mul
  call $~lib/internal/string/allocateUnsafe
  local.set $4
  local.get $4
  i32.const 0
  local.get $0
  local.get $1
  call $~lib/internal/string/repeatUnsafe
  local.get $4
 )
 (func $~lib/ultrain-ts-lib/src/utils/intToString (; 105 ;) (type $FUNCSIG$iji) (param $0 i64) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i64)
  (local $4 i32)
  (local $5 i32)
  local.get $0
  i64.const 10
  i64.rem_u
  i32.wrap_i64
  local.set $2
  local.get $0
  i64.const 10
  i64.div_u
  local.set $3
  global.get $~lib/ultrain-ts-lib/src/utils/PrintableChar
  i32.const 16
  local.get $2
  i32.add
  call $~lib/array/Array<String>#__get
  local.set $4
  block $break|0
   loop $continue|0
    local.get $3
    i64.const 0
    i64.ne
    if
     block
      local.get $3
      i64.const 10
      i64.rem_u
      i32.wrap_i64
      local.set $2
      local.get $3
      i64.const 10
      i64.div_u
      local.set $3
      global.get $~lib/ultrain-ts-lib/src/utils/PrintableChar
      i32.const 16
      local.get $2
      i32.add
      call $~lib/array/Array<String>#__get
      local.get $4
      call $~lib/string/String.__concat
      local.set $4
     end
     br $continue|0
    end
   end
  end
  local.get $1
  i32.const 0
  i32.ne
  local.tee $5
  if (result i32)
   local.get $4
   i32.load
   local.get $1
   i32.lt_s
  else   
   local.get $5
  end
  if
   i32.const 136
   local.set $5
   local.get $5
   local.get $1
   local.get $4
   i32.load
   i32.sub
   call $~lib/string/String#repeat
   drop
   local.get $5
   local.get $4
   call $~lib/string/String.__concat
   local.set $4
  end
  local.get $4
 )
 (func $~lib/ultrain-ts-lib/src/crypto/Crypto#get:buffer (; 106 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  local.get $0
  i32.load
  i32.load
 )
 (func $~lib/ultrain-ts-lib/src/crypto/Crypto#get:bufferSize (; 107 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  (local $1 i32)
  local.get $0
  i32.load
  local.set $1
  local.get $1
  i32.load offset=8
  i32.const 0
  i32.shr_u
 )
 (func $~lib/string/String#charAt (; 108 ;) (type $FUNCSIG$iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  local.get $0
  i32.const 0
  i32.ne
  i32.eqz
  if
   call $~lib/env/abort
   unreachable
  end
  local.get $1
  local.get $0
  i32.load
  i32.ge_u
  if
   i32.const 2384
   return
  end
  i32.const 1
  call $~lib/internal/string/allocateUnsafe
  local.set $2
  local.get $2
  local.get $0
  local.get $1
  i32.const 1
  i32.shl
  i32.add
  i32.load16_u offset=4
  i32.store16 offset=4
  local.get $2
 )
 (func $~lib/ultrain-ts-lib/src/crypto/to_hex (; 109 ;) (type $FUNCSIG$iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  i32.const 2384
  local.set $2
  block $break|0
   i32.const 0
   local.set $3
   loop $repeat|0
    local.get $3
    local.get $1
    i32.lt_u
    i32.eqz
    br_if $break|0
    block
     local.get $0
     local.get $3
     i32.add
     i32.load8_u
     local.set $4
     local.get $4
     i32.const 255
     i32.and
     local.set $5
     local.get $2
     global.get $~lib/ultrain-ts-lib/src/crypto/HexDigital
     local.get $5
     i32.const 4
     i32.shr_u
     call $~lib/string/String#charAt
     call $~lib/string/String.__concat
     global.get $~lib/ultrain-ts-lib/src/crypto/HexDigital
     local.get $5
     i32.const 15
     i32.and
     call $~lib/string/String#charAt
     call $~lib/string/String.__concat
     local.set $2
    end
    local.get $3
    i32.const 1
    i32.add
    local.set $3
    br $repeat|0
    unreachable
   end
   unreachable
  end
  local.get $2
 )
 (func $~lib/ultrain-ts-lib/src/crypto/Crypto#toString (; 110 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  local.get $0
  call $~lib/ultrain-ts-lib/src/crypto/Crypto#get:buffer
  local.get $0
  call $~lib/ultrain-ts-lib/src/crypto/Crypto#get:bufferSize
  call $~lib/ultrain-ts-lib/src/crypto/to_hex
 )
 (func $~lib/ultrain-ts-lib/src/crypto/SHA256#hash (; 111 ;) (type $FUNCSIG$iii) (param $0 i32) (param $1 i32) (result i32)
  local.get $1
  call $~lib/ultrain-ts-lib/src/utils/string2cstr
  local.get $1
  i32.load
  local.get $0
  call $~lib/ultrain-ts-lib/src/crypto/Crypto#get:buffer
  local.get $0
  call $~lib/ultrain-ts-lib/src/crypto/Crypto#get:bufferSize
  call $~lib/ultrain-ts-lib/internal/crypto.d/env.ts_sha256
  local.get $0
  call $~lib/ultrain-ts-lib/src/crypto/Crypto#toString
 )
 (func $~lib/string/String#substring (; 112 ;) (type $FUNCSIG$iiii) (param $0 i32) (param $1 i32) (param $2 i32) (result i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  (local $7 i32)
  (local $8 i32)
  (local $9 i32)
  (local $10 i32)
  local.get $0
  i32.const 0
  i32.ne
  i32.eqz
  if
   call $~lib/env/abort
   unreachable
  end
  local.get $0
  i32.load
  local.set $3
  local.get $1
  local.tee $4
  i32.const 0
  local.tee $5
  local.get $4
  local.get $5
  i32.gt_s
  select
  local.tee $4
  local.get $3
  local.tee $5
  local.get $4
  local.get $5
  i32.lt_s
  select
  local.set $6
  local.get $2
  local.tee $4
  i32.const 0
  local.tee $5
  local.get $4
  local.get $5
  i32.gt_s
  select
  local.tee $4
  local.get $3
  local.tee $5
  local.get $4
  local.get $5
  i32.lt_s
  select
  local.set $7
  local.get $6
  local.tee $4
  local.get $7
  local.tee $5
  local.get $4
  local.get $5
  i32.lt_s
  select
  local.set $8
  local.get $6
  local.tee $4
  local.get $7
  local.tee $5
  local.get $4
  local.get $5
  i32.gt_s
  select
  local.set $9
  local.get $9
  local.get $8
  i32.sub
  local.set $3
  local.get $3
  i32.eqz
  if
   i32.const 2384
   return
  end
  local.get $8
  i32.eqz
  local.tee $4
  if (result i32)
   local.get $9
   local.get $0
   i32.load
   i32.eq
  else   
   local.get $4
  end
  if
   local.get $0
   return
  end
  local.get $3
  call $~lib/internal/string/allocateUnsafe
  local.set $10
  local.get $10
  i32.const 0
  local.get $0
  local.get $8
  local.get $3
  call $~lib/internal/string/copyUnsafe
  local.get $10
 )
 (func $~lib/internal/string/parse<f64> (; 113 ;) (type $FUNCSIG$dii) (param $0 i32) (param $1 i32) (result f64)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 f64)
  (local $6 i32)
  (local $7 f64)
  local.get $0
  i32.load
  local.set $2
  local.get $2
  i32.eqz
  if
   f64.const nan:0x8000000000000
   return
  end
  local.get $0
  local.set $3
  local.get $3
  i32.load16_u offset=4
  local.set $4
  local.get $4
  i32.const 45
  i32.eq
  if
   local.get $2
   i32.const 1
   i32.sub
   local.tee $2
   i32.eqz
   if
    f64.const nan:0x8000000000000
    return
   end
   local.get $3
   i32.const 2
   i32.add
   local.tee $3
   i32.load16_u offset=4
   local.set $4
   f64.const -1
   local.set $5
  else   
   local.get $4
   i32.const 43
   i32.eq
   if
    local.get $2
    i32.const 1
    i32.sub
    local.tee $2
    i32.eqz
    if
     f64.const nan:0x8000000000000
     return
    end
    local.get $3
    i32.const 2
    i32.add
    local.tee $3
    i32.load16_u offset=4
    local.set $4
    f64.const 1
    local.set $5
   else    
    f64.const 1
    local.set $5
   end
  end
  local.get $1
  i32.eqz
  if
   local.get $4
   i32.const 48
   i32.eq
   local.tee $6
   if (result i32)
    local.get $2
    i32.const 2
    i32.gt_s
   else    
    local.get $6
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
            local.get $3
            i32.const 2
            i32.add
            i32.load16_u offset=4
            local.set $6
            local.get $6
            i32.const 66
            i32.eq
            br_if $case0|0
            local.get $6
            i32.const 98
            i32.eq
            br_if $case1|0
            local.get $6
            i32.const 79
            i32.eq
            br_if $case2|0
            local.get $6
            i32.const 111
            i32.eq
            br_if $case3|0
            local.get $6
            i32.const 88
            i32.eq
            br_if $case4|0
            local.get $6
            i32.const 120
            i32.eq
            br_if $case5|0
            br $case6|0
           end
          end
          block
           local.get $3
           i32.const 4
           i32.add
           local.set $3
           local.get $2
           i32.const 2
           i32.sub
           local.set $2
           i32.const 2
           local.set $1
           br $break|0
           unreachable
          end
          unreachable
         end
        end
        block
         local.get $3
         i32.const 4
         i32.add
         local.set $3
         local.get $2
         i32.const 2
         i32.sub
         local.set $2
         i32.const 8
         local.set $1
         br $break|0
         unreachable
        end
        unreachable
       end
      end
      block
       local.get $3
       i32.const 4
       i32.add
       local.set $3
       local.get $2
       i32.const 2
       i32.sub
       local.set $2
       i32.const 16
       local.set $1
       br $break|0
       unreachable
      end
      unreachable
     end
     i32.const 10
     local.set $1
    end
   else    
    i32.const 10
    local.set $1
   end
  else   
   local.get $1
   i32.const 2
   i32.lt_s
   local.tee $6
   if (result i32)
    local.get $6
   else    
    local.get $1
    i32.const 36
    i32.gt_s
   end
   if
    f64.const nan:0x8000000000000
    return
   end
  end
  f64.const 0
  local.set $7
  block $break|1
   loop $continue|1
    block (result i32)
     local.get $2
     local.tee $6
     i32.const 1
     i32.sub
     local.set $2
     local.get $6
    end
    if
     block
      local.get $3
      i32.load16_u offset=4
      local.set $4
      local.get $4
      i32.const 48
      i32.ge_s
      local.tee $6
      if (result i32)
       local.get $4
       i32.const 57
       i32.le_s
      else       
       local.get $6
      end
      if
       local.get $4
       i32.const 48
       i32.sub
       local.set $4
      else       
       local.get $4
       i32.const 65
       i32.ge_s
       local.tee $6
       if (result i32)
        local.get $4
        i32.const 90
        i32.le_s
       else        
        local.get $6
       end
       if
        local.get $4
        i32.const 65
        i32.const 10
        i32.sub
        i32.sub
        local.set $4
       else        
        local.get $4
        i32.const 97
        i32.ge_s
        local.tee $6
        if (result i32)
         local.get $4
         i32.const 122
         i32.le_s
        else         
         local.get $6
        end
        if
         local.get $4
         i32.const 97
         i32.const 10
         i32.sub
         i32.sub
         local.set $4
        else         
         br $break|1
        end
       end
      end
      local.get $4
      local.get $1
      i32.ge_s
      if
       br $break|1
      end
      local.get $7
      local.get $1
      f64.convert_i32_s
      f64.mul
      local.get $4
      f64.convert_i32_s
      f64.add
      local.set $7
      local.get $3
      i32.const 2
      i32.add
      local.set $3
     end
     br $continue|1
    end
   end
  end
  local.get $5
  local.get $7
  f64.mul
 )
 (func $~lib/string/parseInt (; 114 ;) (type $FUNCSIG$dii) (param $0 i32) (param $1 i32) (result f64)
  local.get $0
  local.get $1
  call $~lib/internal/string/parse<f64>
 )
 (func $contract/ultrainio.rand/RandContract#hash (; 115 ;) (type $FUNCSIG$jij) (param $0 i32) (param $1 i64) (result i64)
  (local $2 i32)
  (local $3 i32)
  i32.const 0
  call $~lib/ultrain-ts-lib/src/crypto/SHA256#constructor
  local.set $2
  local.get $2
  local.get $1
  i32.const 0
  call $~lib/ultrain-ts-lib/src/utils/intToString
  call $~lib/ultrain-ts-lib/src/crypto/SHA256#hash
  local.set $3
  local.get $3
  i32.const 0
  i32.const 14
  call $~lib/string/String#substring
  i32.const 16
  call $~lib/string/parseInt
  i64.trunc_f64_u
 )
 (func $contract/ultrainio.rand/RandContract#constructor (; 116 ;) (type $FUNCSIG$iij) (param $0 i32) (param $1 i64) (result i32)
  (local $2 i32)
  (local $3 i32)
  local.get $0
  if (result i32)
   local.get $0
  else   
   i32.const 28
   call $~lib/memory/memory.allocate
  end
  local.get $1
  call $~lib/ultrain-ts-lib/src/contract/Contract#constructor
  local.set $0
  local.get $0
  i32.const 0
  i32.store offset=12
  local.get $0
  i32.const 0
  i32.store offset=16
  local.get $0
  i32.const 0
  i32.store offset=20
  local.get $0
  i32.const 0
  i32.store offset=24
  local.get $0
  i32.const 0
  call $contract/lib/random.lib/Random#constructor
  i32.store offset=24
  local.get $0
  local.get $0
  i32.load offset=24
  i32.load
  i32.store offset=12
  local.get $0
  local.get $0
  i32.load offset=24
  i32.load offset=4
  i32.store offset=16
  local.get $0
  local.get $0
  i32.load offset=24
  i32.load offset=8
  i32.store offset=20
  local.get $0
  i32.load offset=12
  global.get $contract/lib/random.lib/RAND_KEY
  call $~lib/dbmanager/DBManager<Voter>#exists
  i32.eqz
  if
   i32.const 0
   call $contract/lib/random.lib/Voter#constructor
   local.set $2
   local.get $2
   global.get $contract/lib/random.lib/RAND_KEY
   i64.store
   local.get $0
   i32.load offset=12
   local.get $2
   call $~lib/dbmanager/DBManager<Voter>#emplace
  end
  local.get $0
  i32.load offset=12
  global.get $contract/lib/random.lib/MAIN_COUNT_KEY
  call $~lib/dbmanager/DBManager<Voter>#exists
  i32.eqz
  if
   i32.const 0
   call $contract/lib/random.lib/Voter#constructor
   local.set $2
   local.get $2
   global.get $contract/lib/random.lib/MAIN_COUNT_KEY
   i64.store
   local.get $0
   i32.load offset=12
   local.get $2
   call $~lib/dbmanager/DBManager<Voter>#emplace
  end
  local.get $0
  i32.load offset=12
  global.get $contract/lib/random.lib/MAIN_VOTES_NUM_KEY
  call $~lib/dbmanager/DBManager<Voter>#exists
  i32.eqz
  if
   i32.const 0
   call $contract/lib/random.lib/Voter#constructor
   local.set $2
   local.get $2
   global.get $contract/lib/random.lib/MAIN_VOTES_NUM_KEY
   i64.store
   local.get $0
   i32.load offset=12
   local.get $2
   call $~lib/dbmanager/DBManager<Voter>#emplace
  end
  local.get $0
  i32.load offset=16
  i64.const 0
  call $~lib/dbmanager/DBManager<Waiter>#exists
  i32.eqz
  if
   i32.const 0
   call $contract/lib/random.lib/Waiter#constructor
   local.set $2
   local.get $0
   i32.load offset=16
   local.get $2
   call $~lib/dbmanager/DBManager<Waiter>#emplace
  end
  local.get $0
  i32.load offset=20
  i64.const 0
  call $~lib/dbmanager/DBManager<RandRecord>#exists
  i32.eqz
  if
   i32.const 0
   call $contract/lib/random.lib/RandRecord#constructor
   local.set $2
   call $~lib/ultrain-ts-lib/src/block/Block.number.get:number
   i32.const 2
   i32.lt_u
   if (result i32)
    i32.const 2
   else    
    call $~lib/ultrain-ts-lib/src/block/Block.number.get:number
   end
   local.set $3
   local.get $2
   i64.const 0
   i64.store
   local.get $2
   local.get $3
   i64.extend_i32_s
   i64.store offset=8
   local.get $0
   i32.load offset=20
   local.get $2
   call $~lib/dbmanager/DBManager<RandRecord>#emplace
   local.get $2
   i64.const 1
   i64.store
   local.get $2
   local.get $3
   i64.extend_i32_s
   i64.store offset=8
   local.get $0
   i32.load offset=20
   local.get $2
   call $~lib/dbmanager/DBManager<RandRecord>#emplace
   local.get $2
   local.get $3
   i64.extend_i32_s
   i64.store
   local.get $2
   local.get $0
   local.get $3
   i64.extend_i32_s
   call $contract/ultrainio.rand/RandContract#hash
   i64.store offset=8
   local.get $2
   i32.const 7
   i32.store offset=16
   local.get $0
   i32.load offset=20
   local.get $2
   call $~lib/dbmanager/DBManager<RandRecord>#emplace
  end
  local.get $0
 )
 (func $~lib/ultrain-ts-lib/lib/name_ex/NameEx#constructor (; 117 ;) (type $FUNCSIG$iijj) (param $0 i32) (param $1 i64) (param $2 i64) (result i32)
  block (result i32)
   local.get $0
   i32.eqz
   if
    i32.const 16
    call $~lib/memory/memory.allocate
    local.set $0
   end
   local.get $0
   i64.const 0
   i64.store
   local.get $0
   i64.const 0
   i64.store offset=8
   local.get $0
  end
  local.get $1
  i64.store
  local.get $0
  local.get $2
  i64.store offset=8
  local.get $0
 )
 (func $~lib/ultrain-ts-lib/src/contract/Contract#setActionName (; 118 ;) (type $FUNCSIG$vijj) (param $0 i32) (param $1 i64) (param $2 i64)
  local.get $0
  i32.const 0
  local.get $1
  local.get $2
  call $~lib/ultrain-ts-lib/lib/name_ex/NameEx#constructor
  i32.store offset=8
 )
 (func $~lib/ultrain-ts-lib/src/contract/Contract#get:receiver (; 119 ;) (type $FUNCSIG$ji) (param $0 i32) (result i64)
  local.get $0
  i64.load
 )
 (func $~lib/ultrain-ts-lib/src/contract/Contract#get:action (; 120 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  local.get $0
  i32.load offset=8
 )
 (func $~lib/ultrain-ts-lib/lib/name_ex/char_to_symbol_ex (; 121 ;) (type $FUNCSIG$ji) (param $0 i32) (result i64)
  (local $1 i32)
  local.get $0
  i32.const 255
  i32.and
  i32.const 46
  i32.eq
  if
   i64.const 0
   return
  end
  local.get $0
  i32.const 255
  i32.and
  i32.const 95
  i32.eq
  if
   i64.const 1
   return
  end
  local.get $0
  i32.const 255
  i32.and
  i32.const 48
  i32.ge_u
  local.tee $1
  if (result i32)
   local.get $0
   i32.const 255
   i32.and
   i32.const 57
   i32.le_u
  else   
   local.get $1
  end
  if
   local.get $0
   i32.const 48
   i32.sub
   i32.const 2
   i32.add
   i32.const 255
   i32.and
   i64.extend_i32_u
   return
  end
  local.get $0
  i32.const 255
  i32.and
  i32.const 97
  i32.ge_u
  local.tee $1
  if (result i32)
   local.get $0
   i32.const 255
   i32.and
   i32.const 122
   i32.le_u
  else   
   local.get $1
  end
  if
   local.get $0
   i32.const 97
   i32.sub
   i32.const 12
   i32.add
   i32.const 255
   i32.and
   i64.extend_i32_u
   return
  end
  local.get $0
  i32.const 255
  i32.and
  i32.const 65
  i32.ge_u
  local.tee $1
  if (result i32)
   local.get $0
   i32.const 255
   i32.and
   i32.const 90
   i32.le_u
  else   
   local.get $1
  end
  if
   local.get $0
   i32.const 65
   i32.sub
   i32.const 38
   i32.add
   i32.const 255
   i32.and
   i64.extend_i32_u
   return
  end
  i64.const 255
 )
 (func $~lib/ultrain-ts-lib/lib/name_ex/NEX (; 122 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
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
  local.set $1
  i64.const 0
  local.set $2
  local.get $0
  i32.load
  local.set $3
  block $break|0
   i32.const 0
   local.set $4
   loop $repeat|0
    local.get $4
    local.get $3
    i32.lt_s
    i32.eqz
    br_if $break|0
    block
     local.get $0
     local.get $4
     call $~lib/string/String#charCodeAt
     call $~lib/ultrain-ts-lib/lib/name_ex/char_to_symbol_ex
     local.set $5
     local.get $4
     i32.const 9
     i32.le_s
     if
      local.get $2
      local.get $5
      i64.const 6
      local.get $4
      i64.extend_i32_s
      i64.mul
      i64.shl
      i64.or
      local.set $2
     else      
      local.get $4
      i32.const 10
      i32.eq
      if
       local.get $5
       i64.const 15
       i64.and
       local.set $6
       local.get $2
       local.get $6
       i64.const 6
       local.get $4
       i64.extend_i32_s
       i64.mul
       i64.shl
       i64.or
       local.set $2
       local.get $1
       local.get $2
       i64.store offset=8
       local.get $5
       i64.const 48
       i64.and
       i64.const 4
       i64.shr_u
       local.set $7
       local.get $7
       local.set $2
      else       
       local.get $2
       local.get $5
       i64.const 6
       local.get $4
       i32.const 11
       i32.sub
       i64.extend_i32_s
       i64.mul
       i64.const 2
       i64.add
       i64.shl
       i64.or
       local.set $2
      end
     end
     local.get $3
     i32.const 10
     i32.le_s
     if
      local.get $1
      local.get $2
      i64.store offset=8
     else      
      local.get $1
      local.get $2
      i64.store
     end
    end
    local.get $4
    i32.const 1
    i32.add
    local.set $4
    br $repeat|0
    unreachable
   end
   unreachable
  end
  local.get $1
 )
 (func $~lib/ultrain-ts-lib/lib/name_ex/NameEx._neq (; 123 ;) (type $FUNCSIG$iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  local.get $0
  i64.load
  local.get $1
  i64.load
  i64.ne
  local.tee $2
  if (result i32)
   local.get $2
  else   
   local.get $0
   i64.load offset=8
   local.get $1
   i64.load offset=8
   i64.ne
  end
 )
 (func $~lib/ultrain-ts-lib/lib/name_ex/NameEx._eq (; 124 ;) (type $FUNCSIG$iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  local.get $0
  i64.load
  local.get $1
  i64.load
  i64.eq
  local.tee $2
  if (result i32)
   local.get $0
   i64.load offset=8
   local.get $1
   i64.load offset=8
   i64.eq
  else   
   local.get $2
  end
 )
 (func $~lib/ultrain-ts-lib/src/contract/Contract.filterAcceptTransferTokenAction (; 125 ;) (type $FUNCSIG$ijji) (param $0 i64) (param $1 i64) (param $2 i32) (result i32)
  (local $3 i32)
  local.get $1
  local.get $0
  i64.eq
  local.tee $3
  if (result i32)
   local.get $2
   i32.const 2448
   call $~lib/ultrain-ts-lib/lib/name_ex/NEX
   call $~lib/ultrain-ts-lib/lib/name_ex/NameEx._neq
  else   
   local.get $3
  end
  local.tee $3
  if (result i32)
   local.get $3
  else   
   local.get $1
   i32.const 2472
   call $~lib/ultrain-ts-lib/src/account/NAME
   i64.eq
   local.tee $3
   if (result i32)
    local.get $2
    i32.const 2448
    call $~lib/ultrain-ts-lib/lib/name_ex/NEX
    call $~lib/ultrain-ts-lib/lib/name_ex/NameEx._eq
   else    
    local.get $3
   end
  end
 )
 (func $contract/ultrainio.rand/RandContract#filterAction (; 126 ;) (type $FUNCSIG$iij) (param $0 i32) (param $1 i64) (result i32)
  local.get $0
  call $~lib/ultrain-ts-lib/src/contract/Contract#get:receiver
  local.get $1
  local.get $0
  call $~lib/ultrain-ts-lib/src/contract/Contract#get:action
  call $~lib/ultrain-ts-lib/src/contract/Contract.filterAcceptTransferTokenAction
 )
 (func $~lib/ultrain-ts-lib/src/contract/Contract#onInit (; 127 ;) (type $FUNCSIG$vi) (param $0 i32)
  nop
 )
 (func $~lib/ultrain-ts-lib/src/contract/DataStreamFromCurrentAction (; 128 ;) (type $FUNCSIG$i) (result i32)
  (local $0 i32)
  (local $1 i32)
  (local $2 i32)
  call $~lib/ultrain-ts-lib/internal/action.d/env.action_data_size
  local.set $0
  i32.const 0
  local.get $0
  call $~lib/typedarray/Uint8Array#constructor
  local.set $1
  local.get $1
  i32.load
  local.get $0
  call $~lib/ultrain-ts-lib/internal/action.d/env.read_action_data
  drop
  i32.const 0
  local.get $1
  i32.load
  local.get $0
  call $~lib/datastream/DataStream#constructor
  local.set $2
  local.get $2
 )
 (func $~lib/ultrain-ts-lib/src/contract/Contract#getDataStream (; 129 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  call $~lib/ultrain-ts-lib/src/contract/DataStreamFromCurrentAction
 )
 (func $~lib/ultrain-ts-lib/src/contract/Contract#isAction (; 130 ;) (type $FUNCSIG$iii) (param $0 i32) (param $1 i32) (result i32)
  local.get $0
  i32.load offset=8
  local.get $1
  call $~lib/ultrain-ts-lib/lib/name_ex/NEX
  call $~lib/ultrain-ts-lib/lib/name_ex/NameEx._eq
 )
 (func $~lib/datastream/DataStream#read<u64> (; 131 ;) (type $FUNCSIG$ji) (param $0 i32) (result i64)
  (local $1 i64)
  local.get $0
  i32.load
  local.get $0
  i32.load offset=8
  i32.add
  i64.load
  local.set $1
  local.get $0
  local.get $0
  i32.load offset=8
  i32.const 8
  i32.add
  i32.store offset=8
  local.get $1
 )
 (func $~lib/ultrain-ts-lib/src/asset/Asset#deserialize (; 132 ;) (type $FUNCSIG$vii) (param $0 i32) (param $1 i32)
  local.get $0
  local.get $1
  call $~lib/datastream/DataStream#read<u64>
  i64.store
  local.get $0
  local.get $1
  call $~lib/datastream/DataStream#read<u64>
  i64.store offset=8
 )
 (func $~lib/datastream/DataStream#read<u8> (; 133 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  (local $1 i32)
  local.get $0
  i32.load
  local.get $0
  i32.load offset=8
  i32.add
  i32.load8_u
  local.set $1
  local.get $0
  local.get $0
  i32.load offset=8
  i32.const 1
  i32.add
  i32.store offset=8
  local.get $1
 )
 (func $~lib/datastream/DataStream#readVarint32 (; 134 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  i32.const 0
  local.set $1
  i32.const 0
  local.set $2
  block $break|0
   loop $continue|0
    block
     local.get $0
     call $~lib/datastream/DataStream#read<u8>
     local.set $3
     local.get $1
     local.get $3
     i32.const 127
     i32.and
     i32.const 7
     block (result i32)
      local.get $2
      local.tee $4
      i32.const 1
      i32.add
      local.set $2
      local.get $4
     end
     i32.mul
     i32.shl
     i32.or
     local.set $1
    end
    local.get $3
    i32.const 128
    i32.and
    br_if $continue|0
   end
  end
  local.get $1
 )
 (func $~lib/string/String.fromUTF8 (; 135 ;) (type $FUNCSIG$iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  (local $7 i32)
  (local $8 i32)
  local.get $1
  i32.const 1
  i32.lt_u
  if
   i32.const 2384
   return
  end
  i32.const 0
  local.set $2
  block $~lib/memory/memory.allocate|inlined.2 (result i32)
   local.get $1
   i32.const 1
   i32.shl
   local.set $3
   local.get $3
   call $~lib/allocator/arena/__memory_allocate
   br $~lib/memory/memory.allocate|inlined.2
  end
  local.set $4
  i32.const 0
  local.set $5
  block $break|0
   loop $continue|0
    local.get $2
    local.get $1
    i32.lt_u
    if
     block
      local.get $0
      block (result i32)
       local.get $2
       local.tee $3
       i32.const 1
       i32.add
       local.set $2
       local.get $3
      end
      i32.add
      i32.load8_u
      local.set $3
      local.get $3
      i32.const 128
      i32.lt_u
      if
       local.get $4
       local.get $5
       i32.add
       local.get $3
       i32.store16
       local.get $5
       i32.const 2
       i32.add
       local.set $5
      else       
       local.get $3
       i32.const 191
       i32.gt_u
       local.tee $6
       if (result i32)
        local.get $3
        i32.const 224
        i32.lt_u
       else        
        local.get $6
       end
       if
        local.get $2
        i32.const 1
        i32.add
        local.get $1
        i32.le_u
        i32.eqz
        if
         call $~lib/env/abort
         unreachable
        end
        local.get $4
        local.get $5
        i32.add
        local.get $3
        i32.const 31
        i32.and
        i32.const 6
        i32.shl
        local.get $0
        block (result i32)
         local.get $2
         local.tee $6
         i32.const 1
         i32.add
         local.set $2
         local.get $6
        end
        i32.add
        i32.load8_u
        i32.const 63
        i32.and
        i32.or
        i32.store16
        local.get $5
        i32.const 2
        i32.add
        local.set $5
       else        
        local.get $3
        i32.const 239
        i32.gt_u
        local.tee $6
        if (result i32)
         local.get $3
         i32.const 365
         i32.lt_u
        else         
         local.get $6
        end
        if
         local.get $2
         i32.const 3
         i32.add
         local.get $1
         i32.le_u
         i32.eqz
         if
          call $~lib/env/abort
          unreachable
         end
         local.get $3
         i32.const 7
         i32.and
         i32.const 18
         i32.shl
         local.get $0
         block (result i32)
          local.get $2
          local.tee $6
          i32.const 1
          i32.add
          local.set $2
          local.get $6
         end
         i32.add
         i32.load8_u
         i32.const 63
         i32.and
         i32.const 12
         i32.shl
         i32.or
         local.get $0
         block (result i32)
          local.get $2
          local.tee $6
          i32.const 1
          i32.add
          local.set $2
          local.get $6
         end
         i32.add
         i32.load8_u
         i32.const 63
         i32.and
         i32.const 6
         i32.shl
         i32.or
         local.get $0
         block (result i32)
          local.get $2
          local.tee $6
          i32.const 1
          i32.add
          local.set $2
          local.get $6
         end
         i32.add
         i32.load8_u
         i32.const 63
         i32.and
         i32.or
         i32.const 65536
         i32.sub
         local.set $3
         local.get $4
         local.get $5
         i32.add
         i32.const 55296
         local.get $3
         i32.const 10
         i32.shr_u
         i32.add
         i32.store16
         local.get $5
         i32.const 2
         i32.add
         local.set $5
         local.get $4
         local.get $5
         i32.add
         i32.const 56320
         local.get $3
         i32.const 1023
         i32.and
         i32.add
         i32.store16
         local.get $5
         i32.const 2
         i32.add
         local.set $5
        else         
         local.get $2
         i32.const 2
         i32.add
         local.get $1
         i32.le_u
         i32.eqz
         if
          call $~lib/env/abort
          unreachable
         end
         local.get $4
         local.get $5
         i32.add
         local.get $3
         i32.const 15
         i32.and
         i32.const 12
         i32.shl
         local.get $0
         block (result i32)
          local.get $2
          local.tee $6
          i32.const 1
          i32.add
          local.set $2
          local.get $6
         end
         i32.add
         i32.load8_u
         i32.const 63
         i32.and
         i32.const 6
         i32.shl
         i32.or
         local.get $0
         block (result i32)
          local.get $2
          local.tee $6
          i32.const 1
          i32.add
          local.set $2
          local.get $6
         end
         i32.add
         i32.load8_u
         i32.const 63
         i32.and
         i32.or
         i32.store16
         local.get $5
         i32.const 2
         i32.add
         local.set $5
        end
       end
      end
     end
     br $continue|0
    end
   end
  end
  local.get $2
  local.get $1
  i32.eq
  i32.eqz
  if
   call $~lib/env/abort
   unreachable
  end
  local.get $5
  i32.const 1
  i32.shr_u
  call $~lib/internal/string/allocateUnsafe
  local.set $7
  block $~lib/memory/memory.copy|inlined.4
   local.get $7
   i32.const 4
   i32.add
   local.set $3
   local.get $4
   local.set $6
   local.get $5
   local.set $8
   local.get $3
   local.get $6
   local.get $8
   call $~lib/internal/memory/memmove
  end
  block $~lib/memory/memory.free|inlined.1
   local.get $4
   local.set $8
   local.get $8
   call $~lib/allocator/arena/__memory_free
   br $~lib/memory/memory.free|inlined.1
  end
  local.get $7
 )
 (func $~lib/datastream/DataStream#readString (; 136 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  local.get $0
  call $~lib/datastream/DataStream#readVarint32
  local.set $1
  local.get $1
  i32.const 0
  i32.eq
  if
   i32.const 2384
   return
  end
  i32.const 0
  local.get $1
  call $~lib/typedarray/Uint8Array#constructor
  local.set $2
  block $~lib/memory/memory.copy|inlined.3
   local.get $2
   i32.load
   local.set $3
   local.get $0
   i32.load
   local.get $0
   i32.load offset=8
   i32.add
   local.set $4
   local.get $1
   local.set $5
   local.get $3
   local.get $4
   local.get $5
   call $~lib/internal/memory/memmove
  end
  local.get $0
  local.get $0
  i32.load offset=8
  local.get $1
  i32.add
  i32.store offset=8
  local.get $2
  i32.load
  local.get $1
  call $~lib/string/String.fromUTF8
 )
 (func $~lib/internal/string/compareUnsafe (; 137 ;) (type $FUNCSIG$iiiiii) (param $0 i32) (param $1 i32) (param $2 i32) (param $3 i32) (param $4 i32) (result i32)
  (local $5 i32)
  (local $6 i32)
  (local $7 i32)
  i32.const 0
  local.set $5
  local.get $0
  local.get $1
  i32.const 1
  i32.shl
  i32.add
  local.set $6
  local.get $2
  local.get $3
  i32.const 1
  i32.shl
  i32.add
  local.set $7
  block $break|0
   loop $continue|0
    local.get $4
    if (result i32)
     local.get $6
     i32.load16_u offset=4
     local.get $7
     i32.load16_u offset=4
     i32.sub
     local.tee $5
     i32.eqz
    else     
     local.get $4
    end
    if
     block
      local.get $4
      i32.const 1
      i32.sub
      local.set $4
      local.get $6
      i32.const 2
      i32.add
      local.set $6
      local.get $7
      i32.const 2
      i32.add
      local.set $7
     end
     br $continue|0
    end
   end
  end
  local.get $5
 )
 (func $~lib/string/String.__eq (; 138 ;) (type $FUNCSIG$iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  local.get $0
  local.get $1
  i32.eq
  if
   i32.const 1
   return
  end
  local.get $0
  i32.const 0
  i32.eq
  local.tee $2
  if (result i32)
   local.get $2
  else   
   local.get $1
   i32.const 0
   i32.eq
  end
  if
   i32.const 0
   return
  end
  local.get $0
  i32.load
  local.set $3
  local.get $3
  local.get $1
  i32.load
  i32.ne
  if
   i32.const 0
   return
  end
  local.get $0
  i32.const 0
  local.get $1
  i32.const 0
  local.get $3
  call $~lib/internal/string/compareUnsafe
  i32.eqz
 )
 (func $~lib/string/String.__ne (; 139 ;) (type $FUNCSIG$iii) (param $0 i32) (param $1 i32) (result i32)
  local.get $0
  local.get $1
  call $~lib/string/String.__eq
  i32.eqz
 )
 (func $~lib/ultrain-ts-lib/src/asset/Asset#eq (; 140 ;) (type $FUNCSIG$iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  local.get $0
  i64.load offset=8
  local.get $1
  i64.load offset=8
  i64.eq
  local.tee $2
  if (result i32)
   local.get $0
   i64.load
   local.get $1
   i64.load
   i64.eq
  else   
   local.get $2
  end
 )
 (func $contract/lib/random.lib/Voter#setMainVoter (; 141 ;) (type $FUNCSIG$vi) (param $0 i32)
  local.get $0
  i32.const 1
  i32.store offset=40
 )
 (func $~lib/array/Array<u64>#__set (; 142 ;) (type $FUNCSIG$viij) (param $0 i32) (param $1 i32) (param $2 i64)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  (local $7 i64)
  (local $8 i32)
  local.get $0
  i32.load
  local.set $3
  local.get $3
  i32.load
  i32.const 3
  i32.shr_u
  local.set $4
  local.get $1
  local.get $4
  i32.ge_u
  if
   local.get $1
   i32.const 134217727
   i32.ge_u
   if
    call $~lib/env/abort
    unreachable
   end
   local.get $3
   local.get $1
   i32.const 1
   i32.add
   i32.const 3
   i32.shl
   call $~lib/internal/arraybuffer/reallocateUnsafe
   local.set $3
   local.get $0
   local.get $3
   i32.store
   local.get $0
   local.get $1
   i32.const 1
   i32.add
   i32.store offset=4
  end
  block $~lib/internal/arraybuffer/STORE<u64,u64>|inlined.0
   local.get $3
   local.set $5
   local.get $1
   local.set $6
   local.get $2
   local.set $7
   i32.const 0
   local.set $8
   local.get $5
   local.get $6
   i32.const 3
   i32.shl
   i32.add
   local.get $8
   i32.add
   local.get $7
   i64.store offset=8
  end
 )
 (func $~lib/datastream/DataStream#readVector<u64> (; 143 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  (local $3 i32)
  local.get $0
  call $~lib/datastream/DataStream#readVarint32
  local.set $1
  local.get $1
  i32.const 0
  i32.eq
  if
   i32.const 0
   i32.const 0
   call $~lib/array/Array<u64>#constructor
   return
  end
  i32.const 0
  local.get $1
  call $~lib/array/Array<u64>#constructor
  local.set $2
  block $break|0
   i32.const 0
   local.set $3
   loop $repeat|0
    local.get $3
    local.get $1
    i32.lt_u
    i32.eqz
    br_if $break|0
    local.get $2
    local.get $3
    local.get $0
    call $~lib/datastream/DataStream#read<u64>
    call $~lib/array/Array<u64>#__set
    local.get $3
    i32.const 1
    i32.add
    local.set $3
    br $repeat|0
    unreachable
   end
   unreachable
  end
  local.get $2
 )
 (func $~lib/datastream/DataStream#read<i32> (; 144 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  (local $1 i32)
  local.get $0
  i32.load
  local.get $0
  i32.load offset=8
  i32.add
  i32.load
  local.set $1
  local.get $0
  local.get $0
  i32.load offset=8
  i32.const 4
  i32.add
  i32.store offset=8
  local.get $1
 )
 (func $contract/lib/random.lib/Voter#deserialize (; 145 ;) (type $FUNCSIG$vii) (param $0 i32) (param $1 i32)
  local.get $0
  local.get $1
  call $~lib/datastream/DataStream#read<u64>
  i64.store
  local.get $0
  local.get $1
  call $~lib/datastream/DataStream#readVector<u64>
  i32.store offset=8
  local.get $0
  local.get $1
  call $~lib/datastream/DataStream#readVector<u64>
  i32.store offset=12
  local.get $0
  local.get $1
  call $~lib/datastream/DataStream#read<u64>
  i64.store offset=16
  local.get $0
  local.get $1
  call $~lib/datastream/DataStream#read<u64>
  i64.store offset=24
  local.get $0
  local.get $1
  call $~lib/datastream/DataStream#read<u64>
  i64.store offset=32
  local.get $0
  local.get $1
  call $~lib/datastream/DataStream#read<i32>
  i32.store offset=40
 )
 (func $~lib/dbmanager/DBManager<Voter>#loadObjectByPrimaryIterator (; 146 ;) (type $FUNCSIG$viii) (param $0 i32) (param $1 i32) (param $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  local.get $1
  i32.const 0
  i32.const 0
  call $~lib/env/db_get_i64
  local.set $3
  i32.const 0
  local.get $3
  call $~lib/typedarray/Uint8Array#constructor
  local.set $4
  i32.const 0
  local.get $4
  i32.load
  local.get $3
  call $~lib/datastream/DataStream#constructor
  local.set $5
  local.get $1
  local.get $4
  i32.load
  local.get $3
  call $~lib/env/db_get_i64
  drop
  local.get $2
  local.get $5
  call $contract/lib/random.lib/Voter#deserialize
 )
 (func $~lib/dbmanager/DBManager<Voter>#get (; 147 ;) (type $FUNCSIG$iiji) (param $0 i32) (param $1 i64) (param $2 i32) (result i32)
  (local $3 i32)
  local.get $0
  i64.load offset=8
  local.get $0
  i64.load offset=16
  local.get $0
  i64.load
  local.get $1
  call $~lib/env/db_find_i64
  local.set $3
  local.get $3
  i32.const 0
  i32.lt_s
  if
   i32.const 0
   return
  end
  local.get $0
  local.get $3
  local.get $2
  call $~lib/dbmanager/DBManager<Voter>#loadObjectByPrimaryIterator
  i32.const 1
 )
 (func $contract/lib/random.lib/Random#countMainVoter (; 148 ;) (type $FUNCSIG$ji) (param $0 i32) (result i64)
  (local $1 i32)
  i32.const 0
  call $contract/lib/random.lib/Voter#constructor
  local.set $1
  local.get $0
  i32.load
  global.get $contract/lib/random.lib/MAIN_COUNT_KEY
  local.get $1
  call $~lib/dbmanager/DBManager<Voter>#get
  drop
  local.get $1
  i32.load offset=12
  i32.const 0
  call $~lib/array/Array<u64>#__get
 )
 (func $~lib/dbmanager/DBManager<Voter>#modify (; 149 ;) (type $FUNCSIG$vii) (param $0 i32) (param $1 i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  local.get $0
  local.get $1
  call $contract/lib/random.lib/Voter#primaryKey
  call $~lib/dbmanager/DBManager<Voter>#find
  local.set $2
  local.get $2
  i32.const 0
  i32.ge_s
  i32.const 2680
  call $~lib/env/ultrain_assert
  local.get $0
  i64.load offset=8
  call $~lib/env/current_receiver
  i64.eq
  i32.const 2800
  call $~lib/env/ultrain_assert
  local.get $1
  call $~lib/datastream/DataStream.measure<Voter>
  local.set $3
  i32.const 0
  local.get $3
  call $~lib/typedarray/Uint8Array#constructor
  local.set $4
  i32.const 0
  local.get $4
  i32.load
  local.get $3
  call $~lib/datastream/DataStream#constructor
  local.set $5
  local.get $1
  local.get $5
  call $contract/lib/random.lib/Voter#serialize
  local.get $2
  local.get $0
  i64.load offset=8
  local.get $5
  i32.load
  local.get $5
  i32.load offset=8
  call $~lib/env/db_update_i64
 )
 (func $contract/lib/random.lib/Random#updateCountMainVoter (; 150 ;) (type $FUNCSIG$vij) (param $0 i32) (param $1 i64)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  i32.const 0
  call $contract/lib/random.lib/Voter#constructor
  local.set $2
  local.get $0
  i32.load
  global.get $contract/lib/random.lib/MAIN_COUNT_KEY
  local.get $2
  call $~lib/dbmanager/DBManager<Voter>#get
  drop
  local.get $2
  i32.load offset=12
  i32.const 0
  block (result i64)
   local.get $2
   i32.load offset=12
   local.tee $3
   i32.const 0
   local.tee $4
   local.get $1
   call $~lib/array/Array<u64>#__set
   local.get $3
   local.get $4
   call $~lib/array/Array<u64>#__get
  end
  call $~lib/array/Array<u64>#__set
  local.get $0
  i32.load
  local.get $2
  call $~lib/dbmanager/DBManager<Voter>#modify
 )
 (func $contract/lib/random.lib/Voter#setWaiterVoter (; 151 ;) (type $FUNCSIG$vi) (param $0 i32)
  local.get $0
  i32.const 2
  i32.store offset=40
 )
 (func $contract/lib/random.lib/Waiter#deserialize (; 152 ;) (type $FUNCSIG$vii) (param $0 i32) (param $1 i32)
  local.get $0
  local.get $1
  call $~lib/datastream/DataStream#readVector<u64>
  i32.store
 )
 (func $~lib/dbmanager/DBManager<Waiter>#loadObjectByPrimaryIterator (; 153 ;) (type $FUNCSIG$viii) (param $0 i32) (param $1 i32) (param $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  local.get $1
  i32.const 0
  i32.const 0
  call $~lib/env/db_get_i64
  local.set $3
  i32.const 0
  local.get $3
  call $~lib/typedarray/Uint8Array#constructor
  local.set $4
  i32.const 0
  local.get $4
  i32.load
  local.get $3
  call $~lib/datastream/DataStream#constructor
  local.set $5
  local.get $1
  local.get $4
  i32.load
  local.get $3
  call $~lib/env/db_get_i64
  drop
  local.get $2
  local.get $5
  call $contract/lib/random.lib/Waiter#deserialize
 )
 (func $~lib/dbmanager/DBManager<Waiter>#get (; 154 ;) (type $FUNCSIG$iiji) (param $0 i32) (param $1 i64) (param $2 i32) (result i32)
  (local $3 i32)
  local.get $0
  i64.load offset=8
  local.get $0
  i64.load offset=16
  local.get $0
  i64.load
  local.get $1
  call $~lib/env/db_find_i64
  local.set $3
  local.get $3
  i32.const 0
  i32.lt_s
  if
   i32.const 0
   return
  end
  local.get $0
  local.get $3
  local.get $2
  call $~lib/dbmanager/DBManager<Waiter>#loadObjectByPrimaryIterator
  i32.const 1
 )
 (func $~lib/array/Array<u64>#push (; 155 ;) (type $FUNCSIG$iij) (param $0 i32) (param $1 i64) (result i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  (local $7 i32)
  (local $8 i64)
  (local $9 i32)
  local.get $0
  i32.load offset=4
  local.set $2
  local.get $0
  i32.load
  local.set $3
  local.get $3
  i32.load
  i32.const 3
  i32.shr_u
  local.set $4
  local.get $2
  i32.const 1
  i32.add
  local.set $5
  local.get $2
  local.get $4
  i32.ge_u
  if
   local.get $2
   i32.const 134217727
   i32.ge_u
   if
    call $~lib/env/abort
    unreachable
   end
   local.get $3
   local.get $5
   i32.const 3
   i32.shl
   call $~lib/internal/arraybuffer/reallocateUnsafe
   local.set $3
   local.get $0
   local.get $3
   i32.store
  end
  local.get $0
  local.get $5
  i32.store offset=4
  block $~lib/internal/arraybuffer/STORE<u64,u64>|inlined.1
   local.get $3
   local.set $6
   local.get $2
   local.set $7
   local.get $1
   local.set $8
   i32.const 0
   local.set $9
   local.get $6
   local.get $7
   i32.const 3
   i32.shl
   i32.add
   local.get $9
   i32.add
   local.get $8
   i64.store offset=8
  end
  local.get $5
 )
 (func $~lib/dbmanager/DBManager<Waiter>#modify (; 156 ;) (type $FUNCSIG$vii) (param $0 i32) (param $1 i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  local.get $0
  local.get $1
  call $contract/lib/random.lib/Waiter#primaryKey
  call $~lib/dbmanager/DBManager<Waiter>#find
  local.set $2
  local.get $2
  i32.const 0
  i32.ge_s
  i32.const 2680
  call $~lib/env/ultrain_assert
  local.get $0
  i64.load offset=8
  call $~lib/env/current_receiver
  i64.eq
  i32.const 2800
  call $~lib/env/ultrain_assert
  local.get $1
  call $~lib/datastream/DataStream.measure<Waiter>
  local.set $3
  i32.const 0
  local.get $3
  call $~lib/typedarray/Uint8Array#constructor
  local.set $4
  i32.const 0
  local.get $4
  i32.load
  local.get $3
  call $~lib/datastream/DataStream#constructor
  local.set $5
  local.get $1
  local.get $5
  call $contract/lib/random.lib/Waiter#serialize
  local.get $2
  local.get $0
  i64.load offset=8
  local.get $5
  i32.load
  local.get $5
  i32.load offset=8
  call $~lib/env/db_update_i64
 )
 (func $~lib/array/Array<u8>#__set (; 157 ;) (type $FUNCSIG$viii) (param $0 i32) (param $1 i32) (param $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  (local $7 i32)
  (local $8 i32)
  local.get $0
  i32.load
  local.set $3
  local.get $3
  i32.load
  i32.const 0
  i32.shr_u
  local.set $4
  local.get $1
  local.get $4
  i32.ge_u
  if
   local.get $1
   i32.const 1073741816
   i32.ge_u
   if
    call $~lib/env/abort
    unreachable
   end
   local.get $3
   local.get $1
   i32.const 1
   i32.add
   i32.const 0
   i32.shl
   call $~lib/internal/arraybuffer/reallocateUnsafe
   local.set $3
   local.get $0
   local.get $3
   i32.store
   local.get $0
   local.get $1
   i32.const 1
   i32.add
   i32.store offset=4
  end
  block $~lib/internal/arraybuffer/STORE<u8,u8>|inlined.1
   local.get $3
   local.set $5
   local.get $1
   local.set $6
   local.get $2
   local.set $7
   i32.const 0
   local.set $8
   local.get $5
   local.get $6
   i32.const 0
   i32.shl
   i32.add
   local.get $8
   i32.add
   local.get $7
   i32.store8 offset=8
  end
 )
 (func $~lib/array/Array<u8>#__get (; 158 ;) (type $FUNCSIG$iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  local.get $0
  i32.load
  local.set $2
  local.get $1
  local.get $2
  i32.load
  i32.const 0
  i32.shr_u
  i32.lt_u
  if (result i32)
   local.get $2
   local.set $3
   local.get $1
   local.set $4
   i32.const 0
   local.set $5
   local.get $3
   local.get $4
   i32.const 0
   i32.shl
   i32.add
   local.get $5
   i32.add
   i32.load8_u offset=8
  else   
   unreachable
  end
 )
 (func $~lib/ultrain-ts-lib/lib/name/RN (; 159 ;) (type $FUNCSIG$ij) (param $0 i64) (result i32)
  (local $1 i32)
  (local $2 i64)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  local.get $0
  i64.const 0
  i64.eq
  if
   i32.const 136
   return
  end
  i32.const 3120
  local.set $1
  local.get $0
  local.set $2
  block $break|0
   i32.const 0
   local.set $3
   loop $repeat|0
    local.get $3
    i32.const 12
    i32.le_u
    i32.eqz
    br_if $break|0
    block
     local.get $2
     local.get $3
     i32.const 0
     i32.eq
     if (result i64)
      i64.const 15
     else      
      i64.const 31
     end
     i64.and
     i32.wrap_i64
     local.set $4
     i32.const 3016
     local.get $4
     call $~lib/string/String#charCodeAt
     i32.const 255
     i32.and
     local.set $5
     local.get $1
     i32.const 12
     local.get $3
     i32.sub
     local.get $5
     call $~lib/array/Array<u8>#__set
     local.get $2
     local.get $3
     i32.const 0
     i32.eq
     if (result i64)
      i64.const 4
     else      
      i64.const 5
     end
     i64.shr_u
     local.set $2
    end
    local.get $3
    i32.const 1
    i32.add
    local.set $3
    br $repeat|0
    unreachable
   end
   unreachable
  end
  i32.const 2384
  local.set $3
  i32.const 1
  local.set $5
  block $break|1
   i32.const 12
   local.set $4
   loop $repeat|1
    local.get $4
    i32.const 0
    i32.ge_s
    i32.eqz
    br_if $break|1
    local.get $1
    local.get $4
    call $~lib/array/Array<u8>#__get
    i32.const 255
    i32.and
    i32.const 46
    i32.eq
    local.tee $6
    if (result i32)
     local.get $5
    else     
     local.get $6
    end
    if
     nop
    else     
     i32.const 0
     local.set $5
     local.get $1
     local.get $4
     call $~lib/array/Array<u8>#__get
     i32.const 32
     i32.sub
     local.set $6
     global.get $~lib/ultrain-ts-lib/src/utils/PrintableChar
     local.get $6
     i32.const 255
     i32.and
     call $~lib/array/Array<String>#__get
     local.get $3
     call $~lib/string/String.__concat
     local.set $3
    end
    local.get $4
    i32.const 1
    i32.sub
    local.set $4
    br $repeat|1
    unreachable
   end
   unreachable
  end
  local.get $3
 )
 (func $~lib/ultrain-ts-lib/src/account/RNAME (; 160 ;) (type $FUNCSIG$ij) (param $0 i64) (result i32)
  local.get $0
  call $~lib/ultrain-ts-lib/lib/name/RN
 )
 (func $~lib/ultrain-ts-lib/src/asset/Asset#getAmount (; 161 ;) (type $FUNCSIG$ji) (param $0 i32) (result i64)
  local.get $0
  i64.load
 )
 (func $contract/ultrainio.rand/RandContract#curtBckNum (; 162 ;) (type $FUNCSIG$ji) (param $0 i32) (result i64)
  call $~lib/ultrain-ts-lib/src/block/Block.number.get:number
  i32.const 1
  i32.add
  i64.extend_i32_u
 )
 (func $contract/ultrainio.rand/RandContract#register (; 163 ;) (type $FUNCSIG$vijii) (param $0 i32) (param $1 i64) (param $2 i32) (param $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  local.get $1
  local.get $0
  call $~lib/ultrain-ts-lib/src/contract/Contract#get:receiver
  i64.eq
  if
   return
  end
  local.get $3
  i32.const 2504
  call $~lib/string/String.__ne
  if
   return
  end
  local.get $0
  i32.load offset=12
  local.get $1
  call $~lib/dbmanager/DBManager<Voter>#exists
  i32.eqz
  i32.const 2536
  call $~lib/env/ultrain_assert
  i32.const 0
  call $contract/lib/random.lib/Voter#constructor
  local.set $4
  local.get $4
  local.get $1
  i64.store
  local.get $2
  global.get $contract/ultrainio.rand/MAIN_DEPOSIT_AMOUNT
  call $~lib/ultrain-ts-lib/src/asset/Asset#eq
  if
   local.get $4
   call $contract/lib/random.lib/Voter#setMainVoter
   local.get $0
   i32.load offset=24
   call $contract/lib/random.lib/Random#countMainVoter
   global.get $contract/ultrainio.rand/MAIN_NUM
   i64.le_u
   i32.const 2600
   global.get $contract/ultrainio.rand/MAIN_NUM
   i32.const 0
   call $~lib/ultrain-ts-lib/src/utils/intToString
   call $~lib/string/String.__concat
   call $~lib/env/ultrain_assert
   local.get $0
   i32.load offset=24
   i64.const 1
   call $contract/lib/random.lib/Random#updateCountMainVoter
  else   
   local.get $2
   global.get $contract/ultrainio.rand/WAITER_DEPOSIT_AMOUNT
   call $~lib/ultrain-ts-lib/src/asset/Asset#eq
   if
    local.get $4
    call $contract/lib/random.lib/Voter#setWaiterVoter
    i32.const 0
    call $contract/lib/random.lib/Waiter#constructor
    local.set $5
    local.get $0
    i32.load offset=16
    i64.const 0
    local.get $5
    call $~lib/dbmanager/DBManager<Waiter>#get
    drop
    block $~lib/array/Array<u64>#get:length|inlined.1 (result i32)
     local.get $5
     i32.load
     local.set $6
     local.get $6
     i32.load offset=4
    end
    global.get $contract/ultrainio.rand/WAITER_NUM
    i32.wrap_i64
    i32.le_s
    i32.const 2912
    global.get $contract/ultrainio.rand/WAITER_NUM
    i32.const 0
    call $~lib/ultrain-ts-lib/src/utils/intToString
    call $~lib/string/String.__concat
    call $~lib/env/ultrain_assert
    local.get $5
    i32.load
    local.get $1
    call $~lib/array/Array<u64>#push
    drop
    local.get $0
    i32.load offset=16
    local.get $5
    call $~lib/dbmanager/DBManager<Waiter>#modify
   else    
    global.get $~lib/ultrain-ts-lib/src/log/Log
    i32.const 3000
    call $~lib/ultrain-ts-lib/src/log/Logger#s
    local.get $1
    call $~lib/ultrain-ts-lib/src/account/RNAME
    call $~lib/ultrain-ts-lib/src/log/Logger#s
    i32.const 3128
    call $~lib/ultrain-ts-lib/src/log/Logger#s
    call $~lib/ultrain-ts-lib/src/log/Logger#flush
    i32.const 0
    i32.const 3184
    call $~lib/env/ultrain_assert
   end
  end
  local.get $4
  local.get $2
  call $~lib/ultrain-ts-lib/src/asset/Asset#getAmount
  i64.store offset=16
  local.get $4
  local.get $0
  call $contract/ultrainio.rand/RandContract#curtBckNum
  i64.store offset=24
  local.get $4
  i64.const 0
  i64.store offset=32
  local.get $0
  i32.load offset=12
  local.get $4
  call $~lib/dbmanager/DBManager<Voter>#emplace
 )
 (func $contract/ultrainio.rand/RandContract#transfer (; 164 ;) (type $FUNCSIG$vijjii) (param $0 i32) (param $1 i64) (param $2 i64) (param $3 i32) (param $4 i32)
  local.get $0
  local.get $1
  local.get $3
  local.get $4
  call $contract/ultrainio.rand/RandContract#register
 )
 (func $~lib/ultrain-ts-lib/src/action/Action.sender.get:sender (; 165 ;) (type $FUNCSIG$j) (result i64)
  call $~lib/ultrain-ts-lib/internal/action.d/env.current_sender
 )
 (func $contract/lib/random.lib/Voter#isUnregister (; 166 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  local.get $0
  i64.load offset=32
  i64.const 0
  i64.ne
 )
 (func $contract/lib/random.lib/Voter#isMainVoter (; 167 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  local.get $0
  i32.load offset=40
  i32.const 1
  i32.eq
 )
 (func $~lib/ultrain-ts-lib/src/permission-level/PermissionLevel#constructor (; 168 ;) (type $FUNCSIG$iijj) (param $0 i32) (param $1 i64) (param $2 i64) (result i32)
  block (result i32)
   local.get $0
   i32.eqz
   if
    i32.const 16
    call $~lib/memory/memory.allocate
    local.set $0
   end
   local.get $0
   i64.const 0
   i64.store
   local.get $0
   i64.const 0
   i64.store offset=8
   local.get $0
  end
  local.get $1
  i64.store
  local.get $0
  local.get $2
  i64.store offset=8
  local.get $0
 )
 (func $~lib/ultrain-ts-lib/src/action/TransferParams#constructor (; 169 ;) (type $FUNCSIG$iijjii) (param $0 i32) (param $1 i64) (param $2 i64) (param $3 i32) (param $4 i32) (result i32)
  block (result i32)
   local.get $0
   i32.eqz
   if
    i32.const 24
    call $~lib/memory/memory.allocate
    local.set $0
   end
   local.get $0
   i64.const 0
   i64.store
   local.get $0
   i64.const 0
   i64.store offset=8
   local.get $0
   i32.const 0
   i32.store offset=16
   local.get $0
   i32.const 0
   i32.store offset=20
   local.get $0
  end
  local.get $1
  i64.store
  local.get $0
  local.get $2
  i64.store offset=8
  local.get $0
  local.get $3
  i32.store offset=16
  local.get $0
  local.get $4
  i32.store offset=20
  local.get $0
 )
 (func $~lib/ultrain-ts-lib/src/action/Action#constructor (; 170 ;) (type $FUNCSIG$iijj) (param $0 i32) (param $1 i64) (param $2 i64) (result i32)
  block (result i32)
   local.get $0
   i32.eqz
   if
    i32.const 4
    call $~lib/memory/memory.allocate
    local.set $0
   end
   local.get $0
   i32.const 0
   i32.store
   local.get $0
  end
  i32.const 0
  local.get $1
  local.get $2
  call $~lib/ultrain-ts-lib/lib/name_ex/NameEx#constructor
  i32.store
  local.get $0
 )
 (func $~lib/ultrain-ts-lib/src/action/ACTION (; 171 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  (local $1 i32)
  local.get $0
  call $~lib/ultrain-ts-lib/lib/name_ex/NEX
  local.set $1
  i32.const 0
  local.get $1
  i64.load
  local.get $1
  i64.load offset=8
  call $~lib/ultrain-ts-lib/src/action/Action#constructor
 )
 (func $~lib/ultrain-ts-lib/src/action/Action#get:code (; 172 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  local.get $0
  i32.load
 )
 (func $~lib/array/Array<PermissionLevel>#constructor (; 173 ;) (type $FUNCSIG$iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  local.get $1
  i32.const 268435454
  i32.gt_u
  if
   call $~lib/env/abort
   unreachable
  end
  local.get $1
  i32.const 2
  i32.shl
  local.set $2
  local.get $2
  call $~lib/internal/arraybuffer/allocateUnsafe
  local.set $3
  block (result i32)
   local.get $0
   i32.eqz
   if
    i32.const 8
    call $~lib/memory/memory.allocate
    local.set $0
   end
   local.get $0
   i32.const 0
   i32.store
   local.get $0
   i32.const 0
   i32.store offset=4
   local.get $0
  end
  local.get $3
  i32.store
  local.get $0
  local.get $1
  i32.store offset=4
  block $~lib/memory/memory.fill|inlined.4
   local.get $3
   i32.const 8
   i32.add
   local.set $4
   i32.const 0
   local.set $5
   local.get $2
   local.set $6
   local.get $4
   local.get $5
   local.get $6
   call $~lib/internal/memory/memset
  end
  local.get $0
 )
 (func $~lib/array/Array<PermissionLevel>#__unchecked_set (; 174 ;) (type $FUNCSIG$viii) (param $0 i32) (param $1 i32) (param $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  local.get $0
  i32.load
  local.set $3
  local.get $1
  local.set $4
  local.get $2
  local.set $5
  i32.const 0
  local.set $6
  local.get $3
  local.get $4
  i32.const 2
  i32.shl
  i32.add
  local.get $6
  i32.add
  local.get $5
  i32.store offset=8
 )
 (func $~lib/ultrain-ts-lib/src/action/ActionImpl#constructor (; 175 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  block (result i32)
   local.get $0
   i32.eqz
   if
    i32.const 20
    call $~lib/memory/memory.allocate
    local.set $0
   end
   local.get $0
   i64.const 0
   i64.store
   local.get $0
   i32.const 0
   i32.store offset=8
   local.get $0
   i32.const 0
   i32.store offset=12
   local.get $0
   i32.const 0
   i32.store offset=16
   local.get $0
  end
  i64.const 0
  i64.store
  local.get $0
  i32.const 0
  i64.const 0
  i64.const 0
  call $~lib/ultrain-ts-lib/lib/name_ex/NameEx#constructor
  i32.store offset=8
  local.get $0
  i32.const 3584
  i32.store offset=12
  local.get $0
  i32.const 3600
  i32.store offset=16
  local.get $0
 )
 (func $~lib/ultrain-ts-lib/src/asset/Asset#serialize (; 176 ;) (type $FUNCSIG$vii) (param $0 i32) (param $1 i32)
  local.get $1
  local.get $0
  i64.load
  call $~lib/datastream/DataStream#write<u64>
  local.get $1
  local.get $0
  i64.load offset=8
  call $~lib/datastream/DataStream#write<u64>
 )
 (func $~lib/string/String#get:lengthUTF8 (; 177 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  i32.const 1
  local.set $1
  i32.const 0
  local.set $2
  local.get $0
  i32.load
  local.set $3
  block $break|0
   loop $continue|0
    local.get $2
    local.get $3
    i32.lt_u
    if
     block
      local.get $0
      local.get $2
      i32.const 1
      i32.shl
      i32.add
      i32.load16_u offset=4
      local.set $4
      local.get $4
      i32.const 128
      i32.lt_u
      if
       local.get $1
       i32.const 1
       i32.add
       local.set $1
       local.get $2
       i32.const 1
       i32.add
       local.set $2
      else       
       local.get $4
       i32.const 2048
       i32.lt_u
       if
        local.get $1
        i32.const 2
        i32.add
        local.set $1
        local.get $2
        i32.const 1
        i32.add
        local.set $2
       else        
        local.get $4
        i32.const 64512
        i32.and
        i32.const 55296
        i32.eq
        local.tee $5
        if (result i32)
         local.get $2
         i32.const 1
         i32.add
         local.get $3
         i32.lt_u
        else         
         local.get $5
        end
        local.tee $5
        if (result i32)
         local.get $0
         local.get $2
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
         local.get $5
        end
        if
         local.get $1
         i32.const 4
         i32.add
         local.set $1
         local.get $2
         i32.const 2
         i32.add
         local.set $2
        else         
         local.get $1
         i32.const 3
         i32.add
         local.set $1
         local.get $2
         i32.const 1
         i32.add
         local.set $2
        end
       end
      end
     end
     br $continue|0
    end
   end
  end
  local.get $1
 )
 (func $~lib/string/String#toUTF8 (; 178 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  (local $7 i32)
  block $~lib/memory/memory.allocate|inlined.3 (result i32)
   local.get $0
   call $~lib/string/String#get:lengthUTF8
   local.set $1
   local.get $1
   call $~lib/allocator/arena/__memory_allocate
   br $~lib/memory/memory.allocate|inlined.3
  end
  local.set $2
  i32.const 0
  local.set $3
  local.get $0
  i32.load
  local.set $4
  i32.const 0
  local.set $5
  block $break|0
   loop $continue|0
    local.get $3
    local.get $4
    i32.lt_u
    if
     block
      local.get $0
      local.get $3
      i32.const 1
      i32.shl
      i32.add
      i32.load16_u offset=4
      local.set $1
      local.get $1
      i32.const 128
      i32.lt_u
      if
       local.get $2
       local.get $5
       i32.add
       local.get $1
       i32.store8
       local.get $5
       i32.const 1
       i32.add
       local.set $5
       local.get $3
       i32.const 1
       i32.add
       local.set $3
      else       
       local.get $1
       i32.const 2048
       i32.lt_u
       if
        local.get $2
        local.get $5
        i32.add
        local.set $6
        local.get $6
        local.get $1
        i32.const 6
        i32.shr_u
        i32.const 192
        i32.or
        i32.store8
        local.get $6
        local.get $1
        i32.const 63
        i32.and
        i32.const 128
        i32.or
        i32.store8 offset=1
        local.get $5
        i32.const 2
        i32.add
        local.set $5
        local.get $3
        i32.const 1
        i32.add
        local.set $3
       else        
        local.get $2
        local.get $5
        i32.add
        local.set $6
        local.get $1
        i32.const 64512
        i32.and
        i32.const 55296
        i32.eq
        local.tee $7
        if (result i32)
         local.get $3
         i32.const 1
         i32.add
         local.get $4
         i32.lt_u
        else         
         local.get $7
        end
        if
         local.get $0
         local.get $3
         i32.const 1
         i32.add
         i32.const 1
         i32.shl
         i32.add
         i32.load16_u offset=4
         local.set $7
         local.get $7
         i32.const 64512
         i32.and
         i32.const 56320
         i32.eq
         if
          i32.const 65536
          local.get $1
          i32.const 1023
          i32.and
          i32.const 10
          i32.shl
          i32.add
          local.get $7
          i32.const 1023
          i32.and
          i32.add
          local.set $1
          local.get $6
          local.get $1
          i32.const 18
          i32.shr_u
          i32.const 240
          i32.or
          i32.store8
          local.get $6
          local.get $1
          i32.const 12
          i32.shr_u
          i32.const 63
          i32.and
          i32.const 128
          i32.or
          i32.store8 offset=1
          local.get $6
          local.get $1
          i32.const 6
          i32.shr_u
          i32.const 63
          i32.and
          i32.const 128
          i32.or
          i32.store8 offset=2
          local.get $6
          local.get $1
          i32.const 63
          i32.and
          i32.const 128
          i32.or
          i32.store8 offset=3
          local.get $5
          i32.const 4
          i32.add
          local.set $5
          local.get $3
          i32.const 2
          i32.add
          local.set $3
          br $continue|0
         end
        end
        local.get $6
        local.get $1
        i32.const 12
        i32.shr_u
        i32.const 224
        i32.or
        i32.store8
        local.get $6
        local.get $1
        i32.const 6
        i32.shr_u
        i32.const 63
        i32.and
        i32.const 128
        i32.or
        i32.store8 offset=1
        local.get $6
        local.get $1
        i32.const 63
        i32.and
        i32.const 128
        i32.or
        i32.store8 offset=2
        local.get $5
        i32.const 3
        i32.add
        local.set $5
        local.get $3
        i32.const 1
        i32.add
        local.set $3
       end
      end
     end
     br $continue|0
    end
   end
  end
  local.get $2
  local.get $5
  i32.add
  i32.const 0
  i32.store8
  local.get $2
 )
 (func $~lib/datastream/DataStream#writeString (; 179 ;) (type $FUNCSIG$vii) (param $0 i32) (param $1 i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  local.get $1
  call $~lib/string/String#get:lengthUTF8
  i32.const 1
  i32.sub
  local.set $2
  local.get $0
  local.get $2
  call $~lib/datastream/DataStream#writeVarint32
  local.get $2
  i32.const 0
  i32.eq
  if
   return
  end
  local.get $0
  call $~lib/datastream/DataStream#isMeasureMode
  i32.eqz
  if
   local.get $1
   call $~lib/string/String#toUTF8
   local.set $3
   block $~lib/memory/memory.copy|inlined.5
    local.get $0
    i32.load
    local.get $0
    i32.load offset=8
    i32.add
    local.set $4
    local.get $3
    local.set $5
    local.get $2
    local.set $6
    local.get $4
    local.get $5
    local.get $6
    call $~lib/internal/memory/memmove
   end
  end
  local.get $0
  local.get $0
  i32.load offset=8
  local.get $2
  i32.add
  i32.store offset=8
 )
 (func $~lib/ultrain-ts-lib/src/action/TransferParams#serialize (; 180 ;) (type $FUNCSIG$vii) (param $0 i32) (param $1 i32)
  local.get $1
  local.get $0
  i64.load
  call $~lib/datastream/DataStream#write<u64>
  local.get $1
  local.get $0
  i64.load offset=8
  call $~lib/datastream/DataStream#write<u64>
  local.get $0
  i32.load offset=16
  local.get $1
  call $~lib/ultrain-ts-lib/src/asset/Asset#serialize
  local.get $1
  local.get $0
  i32.load offset=20
  call $~lib/datastream/DataStream#writeString
 )
 (func $~lib/datastream/DataStream.measure<TransferParams> (; 181 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  (local $1 i32)
  i32.const 0
  i32.const 0
  i32.const 0
  call $~lib/datastream/DataStream#constructor
  local.set $1
  local.get $0
  local.get $1
  call $~lib/ultrain-ts-lib/src/action/TransferParams#serialize
  local.get $1
  i32.load offset=8
 )
 (func $~lib/datastream/DataStream#toArray<u8> (; 182 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  local.get $0
  i32.load offset=4
  i32.const 0
  i32.eq
  if
   i32.const 0
   i32.const 0
   call $~lib/array/Array<u8>#constructor
   return
  end
  local.get $0
  i32.load offset=4
  i32.const 1
  i32.div_u
  local.set $1
  i32.const 0
  local.get $1
  call $~lib/array/Array<u8>#constructor
  local.set $2
  i32.const 0
  local.set $3
  block $break|0
   i32.const 0
   local.set $4
   loop $repeat|0
    local.get $4
    local.get $1
    i32.lt_u
    i32.eqz
    br_if $break|0
    block
     local.get $0
     i32.load
     local.get $3
     i32.add
     i32.load8_u
     local.set $5
     local.get $2
     local.get $4
     local.get $5
     call $~lib/array/Array<u8>#__set
     local.get $3
     i32.const 1
     i32.add
     local.set $3
    end
    local.get $4
    i32.const 1
    i32.add
    local.set $4
    br $repeat|0
    unreachable
   end
   unreachable
  end
  local.get $2
 )
 (func $~lib/ultrain-ts-lib/src/action/SerializableToArray<TransferParams> (; 183 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  (local $3 i32)
  local.get $0
  call $~lib/datastream/DataStream.measure<TransferParams>
  local.set $1
  i32.const 0
  local.get $1
  call $~lib/typedarray/Uint8Array#constructor
  local.set $2
  i32.const 0
  local.get $2
  i32.load
  local.get $1
  call $~lib/datastream/DataStream#constructor
  local.set $3
  local.get $0
  local.get $3
  call $~lib/ultrain-ts-lib/src/action/TransferParams#serialize
  local.get $3
  call $~lib/datastream/DataStream#toArray<u8>
 )
 (func $~lib/ultrain-ts-lib/lib/name_ex/NameEx#serialize (; 184 ;) (type $FUNCSIG$vii) (param $0 i32) (param $1 i32)
  local.get $1
  local.get $0
  i64.load
  call $~lib/datastream/DataStream#write<u64>
  local.get $1
  local.get $0
  i64.load offset=8
  call $~lib/datastream/DataStream#write<u64>
 )
 (func $~lib/array/Array<PermissionLevel>#__get (; 185 ;) (type $FUNCSIG$iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  local.get $0
  i32.load
  local.set $2
  local.get $1
  local.get $2
  i32.load
  i32.const 2
  i32.shr_u
  i32.lt_u
  if (result i32)
   local.get $2
   local.set $3
   local.get $1
   local.set $4
   i32.const 0
   local.set $5
   local.get $3
   local.get $4
   i32.const 2
   i32.shl
   i32.add
   local.get $5
   i32.add
   i32.load offset=8
  else   
   unreachable
  end
 )
 (func $~lib/ultrain-ts-lib/src/permission-level/PermissionLevel#serialize (; 186 ;) (type $FUNCSIG$vii) (param $0 i32) (param $1 i32)
  local.get $1
  local.get $0
  i64.load
  call $~lib/datastream/DataStream#write<u64>
  local.get $1
  local.get $0
  i64.load offset=8
  call $~lib/datastream/DataStream#write<u64>
 )
 (func $~lib/datastream/DataStream#writeComplexVector<PermissionLevel> (; 187 ;) (type $FUNCSIG$vii) (param $0 i32) (param $1 i32)
  (local $2 i32)
  (local $3 i32)
  block $~lib/array/Array<PermissionLevel>#get:length|inlined.0 (result i32)
   local.get $1
   local.set $2
   local.get $2
   i32.load offset=4
  end
  local.set $2
  local.get $0
  local.get $2
  call $~lib/datastream/DataStream#writeVarint32
  block $break|0
   i32.const 0
   local.set $3
   loop $repeat|0
    local.get $3
    local.get $2
    i32.lt_u
    i32.eqz
    br_if $break|0
    local.get $1
    local.get $3
    call $~lib/array/Array<PermissionLevel>#__get
    local.get $0
    call $~lib/ultrain-ts-lib/src/permission-level/PermissionLevel#serialize
    local.get $3
    i32.const 1
    i32.add
    local.set $3
    br $repeat|0
    unreachable
   end
   unreachable
  end
 )
 (func $~lib/datastream/DataStream#writeVector<u8> (; 188 ;) (type $FUNCSIG$vii) (param $0 i32) (param $1 i32)
  (local $2 i32)
  (local $3 i32)
  block $~lib/array/Array<u8>#get:length|inlined.0 (result i32)
   local.get $1
   local.set $2
   local.get $2
   i32.load offset=4
  end
  local.set $2
  local.get $0
  local.get $2
  call $~lib/datastream/DataStream#writeVarint32
  block $break|0
   i32.const 0
   local.set $3
   loop $repeat|0
    local.get $3
    local.get $2
    i32.lt_u
    i32.eqz
    br_if $break|0
    local.get $0
    local.get $1
    local.get $3
    call $~lib/array/Array<u8>#__get
    call $~lib/datastream/DataStream#write<u8>
    local.get $3
    i32.const 1
    i32.add
    local.set $3
    br $repeat|0
    unreachable
   end
   unreachable
  end
 )
 (func $~lib/ultrain-ts-lib/src/action/ActionImpl#serialize (; 189 ;) (type $FUNCSIG$vii) (param $0 i32) (param $1 i32)
  local.get $1
  local.get $0
  i64.load
  call $~lib/datastream/DataStream#write<u64>
  local.get $0
  i32.load offset=8
  local.get $1
  call $~lib/ultrain-ts-lib/lib/name_ex/NameEx#serialize
  local.get $1
  local.get $0
  i32.load offset=12
  call $~lib/datastream/DataStream#writeComplexVector<PermissionLevel>
  local.get $1
  local.get $0
  i32.load offset=16
  call $~lib/datastream/DataStream#writeVector<u8>
 )
 (func $~lib/datastream/DataStream.measure<ActionImpl> (; 190 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  (local $1 i32)
  i32.const 0
  i32.const 0
  i32.const 0
  call $~lib/datastream/DataStream#constructor
  local.set $1
  local.get $0
  local.get $1
  call $~lib/ultrain-ts-lib/src/action/ActionImpl#serialize
  local.get $1
  i32.load offset=8
 )
 (func $~lib/ultrain-ts-lib/src/action/composeActionData<TransferParams> (; 191 ;) (type $FUNCSIG$iijii) (param $0 i32) (param $1 i64) (param $2 i32) (param $3 i32) (result i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  (local $7 i32)
  i32.const 0
  call $~lib/ultrain-ts-lib/src/action/ActionImpl#constructor
  local.set $4
  local.get $4
  local.get $0
  i32.store offset=12
  local.get $4
  local.get $1
  i64.store
  local.get $4
  local.get $2
  i32.store offset=8
  local.get $4
  local.get $3
  call $~lib/ultrain-ts-lib/src/action/SerializableToArray<TransferParams>
  i32.store offset=16
  local.get $4
  call $~lib/datastream/DataStream.measure<ActionImpl>
  local.set $5
  i32.const 0
  local.get $5
  call $~lib/typedarray/Uint8Array#constructor
  local.set $6
  i32.const 0
  local.get $6
  i32.load
  local.get $5
  call $~lib/datastream/DataStream#constructor
  local.set $7
  local.get $4
  local.get $7
  call $~lib/ultrain-ts-lib/src/action/ActionImpl#serialize
  local.get $7
 )
 (func $~lib/ultrain-ts-lib/src/action/Action.sendInline<TransferParams> (; 192 ;) (type $FUNCSIG$vijii) (param $0 i32) (param $1 i64) (param $2 i32) (param $3 i32)
  (local $4 i32)
  local.get $0
  local.get $1
  local.get $2
  local.get $3
  call $~lib/ultrain-ts-lib/src/action/composeActionData<TransferParams>
  local.set $4
  local.get $4
  i32.load
  local.get $4
  i32.load offset=8
  call $~lib/ultrain-ts-lib/internal/action.d/env.send_inline
 )
 (func $~lib/ultrain-ts-lib/src/asset/Asset.transfer (; 193 ;) (type $FUNCSIG$vjjii) (param $0 i64) (param $1 i64) (param $2 i32) (param $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  (local $7 i32)
  i32.const 0
  i64.const 0
  i64.const 0
  call $~lib/ultrain-ts-lib/src/permission-level/PermissionLevel#constructor
  local.set $4
  local.get $4
  local.get $0
  i64.store
  local.get $4
  i32.const 3560
  call $~lib/ultrain-ts-lib/src/account/NAME
  i64.store offset=8
  i32.const 0
  local.get $0
  local.get $1
  local.get $2
  local.get $3
  call $~lib/ultrain-ts-lib/src/action/TransferParams#constructor
  local.set $5
  i32.const 2448
  call $~lib/ultrain-ts-lib/src/action/ACTION
  call $~lib/ultrain-ts-lib/src/action/Action#get:code
  local.set $6
  block (result i32)
   i32.const 0
   i32.const 1
   call $~lib/array/Array<PermissionLevel>#constructor
   local.set $7
   local.get $7
   i32.const 0
   local.get $4
   call $~lib/array/Array<PermissionLevel>#__unchecked_set
   local.get $7
  end
  i32.const 2472
  call $~lib/ultrain-ts-lib/src/account/NAME
  local.get $6
  local.get $5
  call $~lib/ultrain-ts-lib/src/action/Action.sendInline<TransferParams>
 )
 (func $contract/ultrainio.rand/RandContract#unregister~anonymous|0 (; 194 ;) (type $FUNCSIG$ijii) (param $0 i64) (param $1 i32) (param $2 i32) (result i32)
  local.get $0
  call $~lib/ultrain-ts-lib/src/action/Action.sender.get:sender
  i64.ne
 )
 (func $~lib/array/Array<u64>#filter (; 195 ;) (type $FUNCSIG$iii) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  (local $7 i32)
  (local $8 i64)
  i32.const 0
  i32.const 0
  call $~lib/array/Array<u64>#constructor
  local.set $2
  block $break|0
   block
    i32.const 0
    local.set $3
    local.get $0
    i32.load offset=4
    local.set $4
   end
   loop $repeat|0
    local.get $3
    local.get $4
    local.tee $5
    local.get $0
    i32.load offset=4
    local.tee $6
    local.get $5
    local.get $6
    i32.lt_s
    select
    i32.lt_s
    i32.eqz
    br_if $break|0
    block
     block $~lib/internal/arraybuffer/LOAD<u64,u64>|inlined.1 (result i64)
      local.get $0
      i32.load
      local.set $5
      local.get $3
      local.set $6
      i32.const 0
      local.set $7
      local.get $5
      local.get $6
      i32.const 3
      i32.shl
      i32.add
      local.get $7
      i32.add
      i64.load offset=8
     end
     local.set $8
     block (result i32)
      i32.const 3
      global.set $~lib/argc
      local.get $8
      local.get $3
      local.get $0
      local.get $1
      call_indirect (type $FUNCSIG$ijii)
     end
     i32.const 0
     i32.ne
     if
      local.get $2
      local.get $8
      call $~lib/array/Array<u64>#push
      drop
     end
    end
    local.get $3
    i32.const 1
    i32.add
    local.set $3
    br $repeat|0
    unreachable
   end
   unreachable
  end
  local.get $2
 )
 (func $contract/ultrainio.rand/RandContract#unregister (; 196 ;) (type $FUNCSIG$vi) (param $0 i32)
  (local $1 i32)
  (local $2 i32)
  (local $3 i32)
  local.get $0
  i32.load offset=12
  call $~lib/ultrain-ts-lib/src/action/Action.sender.get:sender
  call $~lib/dbmanager/DBManager<Voter>#exists
  i32.const 3360
  call $~lib/env/ultrain_assert
  i32.const 0
  call $contract/lib/random.lib/Voter#constructor
  local.set $1
  local.get $0
  i32.load offset=12
  call $~lib/ultrain-ts-lib/src/action/Action.sender.get:sender
  local.get $1
  call $~lib/dbmanager/DBManager<Voter>#get
  drop
  local.get $1
  call $contract/lib/random.lib/Voter#isUnregister
  i32.eqz
  i32.const 3440
  call $~lib/env/ultrain_assert
  local.get $1
  call $contract/lib/random.lib/Voter#isMainVoter
  if
   local.get $0
   call $~lib/ultrain-ts-lib/src/contract/Contract#get:receiver
   call $~lib/ultrain-ts-lib/src/action/Action.sender.get:sender
   global.get $contract/ultrainio.rand/MAIN_DEPOSIT_AMOUNT
   i32.const 3512
   call $~lib/ultrain-ts-lib/src/asset/Asset.transfer
   local.get $0
   i32.load offset=24
   i64.const -1
   call $contract/lib/random.lib/Random#updateCountMainVoter
  else   
   local.get $0
   call $~lib/ultrain-ts-lib/src/contract/Contract#get:receiver
   call $~lib/ultrain-ts-lib/src/action/Action.sender.get:sender
   global.get $contract/ultrainio.rand/WAITER_DEPOSIT_AMOUNT
   i32.const 3512
   call $~lib/ultrain-ts-lib/src/asset/Asset.transfer
   i32.const 0
   call $contract/lib/random.lib/Waiter#constructor
   local.set $2
   local.get $0
   i32.load offset=16
   i64.const 0
   local.get $2
   call $~lib/dbmanager/DBManager<Waiter>#get
   drop
   local.get $2
   i32.load
   i32.const 1
   call $~lib/array/Array<u64>#filter
   local.set $3
   local.get $2
   local.get $3
   i32.store
   local.get $0
   i32.load offset=16
   local.get $2
   call $~lib/dbmanager/DBManager<Waiter>#modify
  end
  local.get $1
  i64.const 0
  i64.store offset=24
  local.get $1
  local.get $0
  call $contract/ultrainio.rand/RandContract#curtBckNum
  i64.store offset=32
  local.get $0
  i32.load offset=12
  local.get $1
  call $~lib/dbmanager/DBManager<Voter>#modify
 )
 (func $contract/lib/random.lib/Voter#redeemable (; 197 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  (local $1 i32)
  call $~lib/ultrain-ts-lib/src/block/Block.number.get:number
  i64.extend_i32_u
  local.get $0
  i64.load offset=32
  i64.const 10
  i64.add
  i64.gt_u
  local.tee $1
  if (result i32)
   local.get $0
   i64.load offset=32
   i64.const 0
   i64.ne
  else   
   local.get $1
  end
 )
 (func $~lib/dbmanager/DBManager<Voter>#erase (; 198 ;) (type $FUNCSIG$vij) (param $0 i32) (param $1 i64)
  (local $2 i32)
  local.get $0
  i64.load offset=8
  call $~lib/env/current_receiver
  i64.eq
  i32.const 3696
  call $~lib/env/ultrain_assert
  local.get $0
  local.get $1
  call $~lib/dbmanager/DBManager<Voter>#find
  local.set $2
  local.get $2
  i32.const 0
  i32.ge_s
  if
   local.get $2
   call $~lib/env/db_remove_i64
  else   
   nop
  end
 )
 (func $contract/ultrainio.rand/RandContract#redeem (; 199 ;) (type $FUNCSIG$vi) (param $0 i32)
  (local $1 i32)
  local.get $0
  i32.load offset=12
  call $~lib/ultrain-ts-lib/src/action/Action.sender.get:sender
  call $~lib/dbmanager/DBManager<Voter>#exists
  i32.const 3624
  call $~lib/env/ultrain_assert
  i32.const 0
  call $contract/lib/random.lib/Voter#constructor
  local.set $1
  local.get $0
  i32.load offset=12
  call $~lib/ultrain-ts-lib/src/action/Action.sender.get:sender
  local.get $1
  call $~lib/dbmanager/DBManager<Voter>#get
  drop
  local.get $1
  call $contract/lib/random.lib/Voter#redeemable
  if
   local.get $0
   call $~lib/ultrain-ts-lib/src/contract/Contract#get:receiver
   call $~lib/ultrain-ts-lib/src/action/Action.sender.get:sender
   i32.const 0
   local.get $1
   i64.load offset=16
   i64.const 357577479428
   call $~lib/ultrain-ts-lib/src/asset/Asset#constructor
   i32.const 3512
   call $~lib/ultrain-ts-lib/src/asset/Asset.transfer
   local.get $0
   i32.load offset=12
   call $~lib/ultrain-ts-lib/src/action/Action.sender.get:sender
   call $~lib/dbmanager/DBManager<Voter>#erase
  else   
   i32.const 0
   i32.const 3808
   call $~lib/env/ultrain_assert
  end
 )
 (func $contract/ultrainio.rand/RandContract#belongRandNum (; 200 ;) (type $FUNCSIG$jij) (param $0 i32) (param $1 i64) (result i64)
  local.get $1
  global.get $contract/lib/random.lib/EPOCH
  i64.add
 )
 (func $contract/lib/random.lib/Voter#votable (; 201 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  (local $1 i32)
  call $~lib/ultrain-ts-lib/src/block/Block.number.get:number
  i64.extend_i32_u
  local.get $0
  i64.load offset=24
  i64.const 3
  i64.add
  i64.gt_u
  local.tee $1
  if (result i32)
   local.get $0
   i64.load offset=24
   i64.const 0
   i64.ne
  else   
   local.get $1
  end
 )
 (func $contract/lib/random.lib/RandRecord#deserialize (; 202 ;) (type $FUNCSIG$vii) (param $0 i32) (param $1 i32)
  local.get $0
  local.get $1
  call $~lib/datastream/DataStream#read<u64>
  i64.store
  local.get $0
  local.get $1
  call $~lib/datastream/DataStream#read<u64>
  i64.store offset=8
  local.get $0
  local.get $1
  call $~lib/datastream/DataStream#read<i32>
  i32.store offset=16
 )
 (func $~lib/dbmanager/DBManager<RandRecord>#loadObjectByPrimaryIterator (; 203 ;) (type $FUNCSIG$viii) (param $0 i32) (param $1 i32) (param $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  local.get $1
  i32.const 0
  i32.const 0
  call $~lib/env/db_get_i64
  local.set $3
  i32.const 0
  local.get $3
  call $~lib/typedarray/Uint8Array#constructor
  local.set $4
  i32.const 0
  local.get $4
  i32.load
  local.get $3
  call $~lib/datastream/DataStream#constructor
  local.set $5
  local.get $1
  local.get $4
  i32.load
  local.get $3
  call $~lib/env/db_get_i64
  drop
  local.get $2
  local.get $5
  call $contract/lib/random.lib/RandRecord#deserialize
 )
 (func $~lib/dbmanager/DBManager<RandRecord>#get (; 204 ;) (type $FUNCSIG$iiji) (param $0 i32) (param $1 i64) (param $2 i32) (result i32)
  (local $3 i32)
  local.get $0
  i64.load offset=8
  local.get $0
  i64.load offset=16
  local.get $0
  i64.load
  local.get $1
  call $~lib/env/db_find_i64
  local.set $3
  local.get $3
  i32.const 0
  i32.lt_s
  if
   i32.const 0
   return
  end
  local.get $0
  local.get $3
  local.get $2
  call $~lib/dbmanager/DBManager<RandRecord>#loadObjectByPrimaryIterator
  i32.const 1
 )
 (func $contract/lib/random.lib/Random#getLastRand (; 205 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  i32.const 0
  call $contract/lib/random.lib/RandRecord#constructor
  local.set $1
  local.get $0
  i32.load offset=8
  i64.const 1
  local.get $1
  call $~lib/dbmanager/DBManager<RandRecord>#get
  drop
  i32.const 0
  call $contract/lib/random.lib/RandRecord#constructor
  local.set $2
  local.get $0
  i32.load offset=8
  local.get $1
  i64.load offset=8
  local.get $2
  call $~lib/dbmanager/DBManager<RandRecord>#get
  drop
  local.get $2
 )
 (func $contract/lib/random.lib/Random#hash (; 206 ;) (type $FUNCSIG$jij) (param $0 i32) (param $1 i64) (result i64)
  (local $2 i32)
  (local $3 i32)
  i32.const 0
  call $~lib/ultrain-ts-lib/src/crypto/SHA256#constructor
  local.set $2
  local.get $2
  local.get $1
  i32.const 0
  call $~lib/ultrain-ts-lib/src/utils/intToString
  call $~lib/ultrain-ts-lib/src/crypto/SHA256#hash
  local.set $3
  local.get $3
  i32.const 0
  i32.const 14
  call $~lib/string/String#substring
  i32.const 16
  call $~lib/string/parseInt
  i64.trunc_f64_u
 )
 (func $contract/lib/random.lib/RandRecord#setFields (; 207 ;) (type $FUNCSIG$vijji) (param $0 i32) (param $1 i64) (param $2 i64) (param $3 i32)
  local.get $0
  local.get $1
  i64.store
  local.get $0
  local.get $2
  i64.store offset=8
  local.get $0
  local.get $3
  i32.store offset=16
 )
 (func $~lib/dbmanager/DBManager<RandRecord>#modify (; 208 ;) (type $FUNCSIG$vii) (param $0 i32) (param $1 i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  local.get $0
  local.get $1
  call $contract/lib/random.lib/RandRecord#primaryKey
  call $~lib/dbmanager/DBManager<RandRecord>#find
  local.set $2
  local.get $2
  i32.const 0
  i32.ge_s
  i32.const 2680
  call $~lib/env/ultrain_assert
  local.get $0
  i64.load offset=8
  call $~lib/env/current_receiver
  i64.eq
  i32.const 2800
  call $~lib/env/ultrain_assert
  local.get $1
  call $~lib/datastream/DataStream.measure<RandRecord>
  local.set $3
  i32.const 0
  local.get $3
  call $~lib/typedarray/Uint8Array#constructor
  local.set $4
  i32.const 0
  local.get $4
  i32.load
  local.get $3
  call $~lib/datastream/DataStream#constructor
  local.set $5
  local.get $1
  local.get $5
  call $contract/lib/random.lib/RandRecord#serialize
  local.get $2
  local.get $0
  i64.load offset=8
  local.get $5
  i32.load
  local.get $5
  i32.load offset=8
  call $~lib/env/db_update_i64
 )
 (func $contract/lib/random.lib/Random#saveRandMaxBckNum (; 209 ;) (type $FUNCSIG$jij) (param $0 i32) (param $1 i64) (result i64)
  (local $2 i32)
  i32.const 0
  call $contract/lib/random.lib/RandRecord#constructor
  local.set $2
  local.get $0
  i32.load offset=8
  i64.const 1
  local.get $2
  call $~lib/dbmanager/DBManager<RandRecord>#get
  drop
  local.get $1
  local.get $2
  i64.load offset=8
  i64.gt_u
  if
   local.get $2
   local.get $1
   i64.store offset=8
   local.get $0
   i32.load offset=8
   local.get $2
   call $~lib/dbmanager/DBManager<RandRecord>#modify
  end
  local.get $2
  i64.load offset=8
 )
 (func $contract/lib/random.lib/Random#getRandMinBckNum (; 210 ;) (type $FUNCSIG$ji) (param $0 i32) (result i64)
  (local $1 i32)
  i32.const 0
  call $contract/lib/random.lib/RandRecord#constructor
  local.set $1
  local.get $0
  i32.load offset=8
  i64.const 0
  local.get $1
  call $~lib/dbmanager/DBManager<RandRecord>#get
  drop
  local.get $1
  i64.load offset=8
 )
 (func $~lib/dbmanager/DBManager<RandRecord>#erase (; 211 ;) (type $FUNCSIG$vij) (param $0 i32) (param $1 i64)
  (local $2 i32)
  local.get $0
  i64.load offset=8
  call $~lib/env/current_receiver
  i64.eq
  i32.const 3696
  call $~lib/env/ultrain_assert
  local.get $0
  local.get $1
  call $~lib/dbmanager/DBManager<RandRecord>#find
  local.set $2
  local.get $2
  i32.const 0
  i32.ge_s
  if
   local.get $2
   call $~lib/env/db_remove_i64
  else   
   nop
  end
 )
 (func $contract/lib/random.lib/Random#clearPartRands (; 212 ;) (type $FUNCSIG$vij) (param $0 i32) (param $1 i64)
  (local $2 i64)
  (local $3 i32)
  local.get $0
  call $contract/lib/random.lib/Random#getRandMinBckNum
  local.set $2
  i32.const 0
  call $contract/lib/random.lib/RandRecord#constructor
  local.set $3
  local.get $1
  global.get $contract/lib/random.lib/CACHED_RAND_COUNT
  local.get $2
  i64.add
  i64.ge_u
  if
   local.get $0
   i32.load offset=8
   local.get $2
   call $~lib/dbmanager/DBManager<RandRecord>#exists
   if
    local.get $0
    i32.load offset=8
    local.get $2
    call $~lib/dbmanager/DBManager<RandRecord>#erase
   end
   local.get $3
   local.get $2
   i64.const 1
   i64.add
   i64.store offset=8
   local.get $1
   global.get $contract/lib/random.lib/CACHED_RAND_COUNT
   local.get $2
   i64.add
   i64.gt_u
   if
    local.get $0
    i32.load offset=8
    local.get $2
    i64.const 1
    i64.add
    call $~lib/dbmanager/DBManager<RandRecord>#exists
    if
     local.get $0
     i32.load offset=8
     local.get $2
     i64.const 1
     i64.add
     call $~lib/dbmanager/DBManager<RandRecord>#erase
    end
    local.get $3
    local.get $2
    i64.const 2
    i64.add
    i64.store offset=8
   end
   local.get $0
   i32.load offset=8
   local.get $3
   call $~lib/dbmanager/DBManager<RandRecord>#modify
  end
 )
 (func $contract/lib/random.lib/Random#saveAndClearPartRands (; 213 ;) (type $FUNCSIG$vijji) (param $0 i32) (param $1 i64) (param $2 i64) (param $3 i32)
  (local $4 i32)
  i32.const 0
  call $contract/lib/random.lib/RandRecord#constructor
  local.set $4
  local.get $4
  local.get $1
  local.get $2
  local.get $3
  call $contract/lib/random.lib/RandRecord#setFields
  local.get $0
  i32.load offset=8
  local.get $1
  call $~lib/dbmanager/DBManager<RandRecord>#exists
  i32.eqz
  if
   local.get $0
   i32.load offset=8
   local.get $4
   call $~lib/dbmanager/DBManager<RandRecord>#emplace
  end
  local.get $0
  local.get $1
  call $contract/lib/random.lib/Random#saveRandMaxBckNum
  drop
  local.get $0
  local.get $1
  call $contract/lib/random.lib/Random#clearPartRands
 )
 (func $contract/lib/random.lib/Random#voteMainVoteNum (; 214 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  (local $1 i32)
  i32.const 0
  call $contract/lib/random.lib/Voter#constructor
  local.set $1
  local.get $0
  i32.load
  global.get $contract/lib/random.lib/MAIN_VOTES_NUM_KEY
  call $~lib/dbmanager/DBManager<Voter>#exists
  if
   local.get $0
   i32.load
   global.get $contract/lib/random.lib/MAIN_VOTES_NUM_KEY
   local.get $1
   call $~lib/dbmanager/DBManager<Voter>#get
   drop
  end
  local.get $1
 )
 (func $contract/lib/random.lib/Random#getMainVoteVal (; 215 ;) (type $FUNCSIG$iijj) (param $0 i32) (param $1 i64) (param $2 i64) (result i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i64)
  (local $6 i32)
  (local $7 i32)
  (local $8 i64)
  (local $9 i32)
  i32.const 0
  call $contract/lib/random.lib/Voter#constructor
  local.set $3
  local.get $0
  i32.load
  global.get $contract/lib/random.lib/RAND_KEY
  local.get $3
  call $~lib/dbmanager/DBManager<Voter>#get
  drop
  block $contract/lib/random.lib/Random#indexOf|inlined.0 (result i32)
   local.get $0
   local.set $4
   local.get $1
   local.set $5
   local.get $5
   global.get $contract/lib/random.lib/EPOCH
   i64.rem_u
   i32.wrap_i64
  end
  local.set $6
  i32.const 0
  call $contract/lib/random.lib/RandRecord#constructor
  local.set $7
  local.get $3
  i32.load offset=8
  local.get $6
  call $~lib/array/Array<u64>#__get
  local.get $1
  i64.eq
  if
   local.get $7
   local.get $3
   i32.load offset=12
   local.get $6
   call $~lib/array/Array<u64>#__get
   i64.store offset=8
   local.get $0
   call $contract/lib/random.lib/Random#countMainVoter
   local.set $8
   local.get $0
   call $contract/lib/random.lib/Random#voteMainVoteNum
   local.set $9
   local.get $7
   local.get $8
   local.get $9
   i32.load offset=12
   local.get $6
   call $~lib/array/Array<u64>#__get
   i64.eq
   if (result i32)
    i32.const 0
   else    
    i32.const 1
   end
   i32.store offset=16
  else   
   local.get $7
   local.get $0
   local.get $2
   call $contract/lib/random.lib/Random#hash
   i64.store offset=8
   local.get $7
   i32.const 1
   i32.store offset=16
  end
  local.get $7
  local.get $1
  i64.store
  local.get $7
 )
 (func $contract/lib/random.lib/Random#getWaiterVoteVal (; 216 ;) (type $FUNCSIG$iijj) (param $0 i32) (param $1 i64) (param $2 i64) (result i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  (local $7 i32)
  (local $8 i32)
  (local $9 i32)
  (local $10 i32)
  (local $11 i64)
  (local $12 i32)
  (local $13 i64)
  (local $14 i64)
  i32.const 0
  call $contract/lib/random.lib/Waiter#constructor
  local.set $3
  local.get $0
  i32.load offset=4
  i64.const 0
  local.get $3
  call $~lib/dbmanager/DBManager<Waiter>#get
  drop
  local.get $3
  i32.load
  local.set $4
  block $~lib/array/Array<u64>#get:length|inlined.2 (result i32)
   local.get $4
   local.set $5
   local.get $5
   i32.load offset=4
  end
  local.set $6
  i32.const 0
  local.set $7
  i32.const 10
  local.set $8
  i32.const 0
  call $contract/lib/random.lib/RandRecord#constructor
  local.set $9
  local.get $9
  local.get $1
  i64.store
  local.get $9
  local.get $0
  local.get $2
  call $contract/lib/random.lib/Random#hash
  i64.store offset=8
  local.get $9
  i32.const 1
  i32.store offset=16
  local.get $6
  i32.const 0
  i32.eq
  if
   local.get $9
   return
  end
  i32.const 0
  call $contract/lib/random.lib/Voter#constructor
  local.set $10
  block $contract/lib/random.lib/Random#indexOf|inlined.1 (result i32)
   local.get $0
   local.set $5
   local.get $1
   local.set $11
   local.get $11
   global.get $contract/lib/random.lib/EPOCH
   i64.rem_u
   i32.wrap_i64
  end
  local.set $12
  local.get $2
  local.set $13
  block $break|0
   loop $continue|0
    block
     local.get $13
     local.get $6
     i64.extend_i32_s
     i64.rem_u
     i32.wrap_i64
     local.set $5
     local.get $3
     i32.load
     local.get $5
     call $~lib/array/Array<u64>#__get
     local.set $11
     local.get $0
     i32.load
     local.get $11
     local.get $10
     call $~lib/dbmanager/DBManager<Voter>#get
     drop
     local.get $10
     i32.load offset=8
     local.get $12
     call $~lib/array/Array<u64>#__get
     local.set $14
     local.get $1
     local.get $14
     i64.ne
     if
      local.get $0
      local.get $13
      call $contract/lib/random.lib/Random#hash
      local.set $13
     else      
      local.get $9
      local.get $10
      i32.load offset=12
      local.get $12
      call $~lib/array/Array<u64>#__get
      i64.store offset=8
      local.get $9
      i32.const 0
      i32.store offset=16
      br $break|0
     end
     local.get $7
     i32.const 1
     i32.add
     local.set $7
    end
    local.get $7
    local.get $8
    i32.lt_s
    br_if $continue|0
   end
  end
  local.get $9
 )
 (func $contract/lib/random.lib/RandRecord.calcCode (; 217 ;) (type $FUNCSIG$iiii) (param $0 i32) (param $1 i32) (param $2 i32) (result i32)
  (local $3 i32)
  i32.const 0
  local.set $3
  local.get $0
  i32.const 7
  i32.eq
  if
   local.get $3
   i32.const 4
   i32.add
   local.set $3
  end
  local.get $1
  i32.const 0
  i32.gt_s
  if
   local.get $3
   i32.const 2
   i32.add
   local.set $3
  end
  local.get $2
  i32.const 0
  i32.gt_s
  if
   local.get $3
   i32.const 1
   i32.add
   local.set $3
  end
  local.get $3
 )
 (func $contract/lib/random.lib/Random#generateRand (; 218 ;) (type $FUNCSIG$iiji) (param $0 i32) (param $1 i64) (param $2 i32) (result i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i64)
  (local $6 i64)
  (local $7 i64)
  (local $8 i64)
  (local $9 i64)
  (local $10 i32)
  (local $11 i32)
  (local $12 i32)
  i32.const 0
  call $contract/lib/random.lib/RandRecord#constructor
  local.set $3
  local.get $0
  i32.load offset=8
  local.get $1
  call $~lib/dbmanager/DBManager<RandRecord>#exists
  if
   local.get $0
   i32.load offset=8
   local.get $1
   local.get $3
   call $~lib/dbmanager/DBManager<RandRecord>#get
   drop
   local.get $3
   return
  end
  local.get $0
  call $contract/lib/random.lib/Random#getLastRand
  local.set $4
  local.get $4
  i64.load
  local.set $5
  local.get $1
  local.get $5
  i64.const 13
  i64.add
  i64.gt_u
  if
   local.get $4
   i64.load offset=8
   local.set $6
   local.get $0
   local.get $1
   call $contract/lib/random.lib/Random#hash
   local.set $7
   local.get $0
   local.get $7
   call $contract/lib/random.lib/Random#hash
   local.set $8
   local.get $6
   local.get $7
   i64.xor
   local.get $8
   i64.xor
   local.set $9
   local.get $4
   local.get $1
   local.get $9
   i32.const 7
   call $contract/lib/random.lib/RandRecord#setFields
   local.get $2
   i32.const 0
   i32.ne
   if
    local.get $0
    local.get $1
    local.get $9
    i32.const 7
    call $contract/lib/random.lib/Random#saveAndClearPartRands
   end
   local.get $4
   return
  end
  i32.const 0
  call $contract/lib/random.lib/RandRecord#constructor
  local.set $10
  block $break|0
   local.get $5
   i64.const 1
   i64.add
   local.set $9
   loop $repeat|0
    local.get $9
    local.get $1
    i64.le_u
    i32.eqz
    br_if $break|0
    block
     local.get $0
     local.get $9
     local.get $4
     i64.load offset=8
     call $contract/lib/random.lib/Random#getMainVoteVal
     local.set $11
     local.get $9
     local.get $5
     global.get $contract/lib/random.lib/EPOCH
     i64.add
     i64.ge_u
     if
      local.get $10
      local.get $9
      local.get $0
      local.get $11
      i64.load offset=8
      call $contract/lib/random.lib/Random#hash
      i32.const 1
      call $contract/lib/random.lib/RandRecord#setFields
     else      
      local.get $0
      local.get $9
      local.get $11
      i64.load offset=8
      call $contract/lib/random.lib/Random#getWaiterVoteVal
      local.set $10
     end
     local.get $4
     i32.load offset=16
     local.get $11
     i32.load offset=16
     local.get $10
     i32.load offset=16
     call $contract/lib/random.lib/RandRecord.calcCode
     local.set $12
     local.get $4
     i64.load offset=8
     local.get $11
     i64.load offset=8
     i64.xor
     local.get $10
     i64.load offset=8
     i64.xor
     local.set $8
     local.get $3
     local.get $9
     local.get $8
     local.get $12
     call $contract/lib/random.lib/RandRecord#setFields
     local.get $2
     i32.const 0
     i32.ne
     if
      local.get $0
      local.get $9
      local.get $8
      local.get $12
      call $contract/lib/random.lib/Random#saveAndClearPartRands
     end
     local.get $3
     local.set $4
    end
    local.get $9
    i64.const 1
    i64.add
    local.set $9
    br $repeat|0
    unreachable
   end
   unreachable
  end
  local.get $4
 )
 (func $contract/ultrainio.rand/RandContract#triggerRandGenerate (; 219 ;) (type $FUNCSIG$vi) (param $0 i32)
  (local $1 i32)
  call $~lib/ultrain-ts-lib/src/block/Block.number.get:number
  local.set $1
  local.get $0
  i32.load offset=20
  local.get $1
  i64.extend_i32_u
  call $~lib/dbmanager/DBManager<RandRecord>#exists
  i32.eqz
  if
   local.get $0
   i32.load offset=24
   local.get $1
   i64.extend_i32_u
   i32.const 1
   call $contract/lib/random.lib/Random#generateRand
   drop
  end
 )
 (func $contract/ultrainio.rand/RandContract#getExistRand (; 220 ;) (type $FUNCSIG$jij) (param $0 i32) (param $1 i64) (result i64)
  (local $2 i32)
  i32.const 0
  call $contract/lib/random.lib/RandRecord#constructor
  local.set $2
  local.get $0
  i32.load offset=20
  local.get $1
  call $~lib/dbmanager/DBManager<RandRecord>#exists
  i32.const 4320
  call $~lib/env/ultrain_assert
  local.get $0
  i32.load offset=20
  local.get $1
  local.get $2
  call $~lib/dbmanager/DBManager<RandRecord>#get
  drop
  local.get $2
  i64.load offset=8
 )
 (func $~lib/ultrain-ts-lib/src/account/Account.publicKeyOf (; 221 ;) (type $FUNCSIG$iji) (param $0 i64) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  i32.const 0
  i32.const 128
  call $~lib/typedarray/Uint8Array#constructor
  local.set $2
  local.get $0
  local.get $2
  i32.load
  block $~lib/internal/typedarray/TypedArray<u8>#get:length|inlined.1 (result i32)
   local.get $2
   local.set $3
   local.get $3
   i32.load offset=8
   i32.const 0
   i32.shr_u
  end
  local.get $1
  call $~lib/ultrain-ts-lib/src/utils/string2cstr
  call $~lib/ultrain-ts-lib/internal/crypto.d/env.ts_public_key_of_account
  local.set $3
  local.get $3
  i32.const 0
  i32.gt_s
  if
   local.get $2
   i32.load
   local.get $3
   call $~lib/string/String.fromUTF8
   return
  else   
   i32.const 2384
   return
  end
  unreachable
  unreachable
 )
 (func $~lib/ultrain-ts-lib/src/crypto/verify_with_pk (; 222 ;) (type $FUNCSIG$iiii) (param $0 i32) (param $1 i32) (param $2 i32) (result i32)
  (local $3 i32)
  local.get $0
  call $~lib/ultrain-ts-lib/src/utils/string2cstr
  local.get $1
  call $~lib/ultrain-ts-lib/src/utils/string2cstr
  local.get $2
  call $~lib/ultrain-ts-lib/src/utils/string2cstr
  call $~lib/ultrain-ts-lib/internal/crypto.d/env.ts_verify_with_pk
  local.set $3
  local.get $3
  i32.const 1
  i32.eq
  if (result i32)
   i32.const 1
  else   
   i32.const 0
  end
 )
 (func $contract/ultrainio.rand/RandContract#getVrfVal (; 223 ;) (type $FUNCSIG$jiij) (param $0 i32) (param $1 i32) (param $2 i64) (result i64)
  (local $3 i64)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  (local $7 i32)
  (local $8 i64)
  local.get $0
  local.get $2
  call $contract/ultrainio.rand/RandContract#getExistRand
  local.set $3
  local.get $3
  i32.const 0
  call $~lib/ultrain-ts-lib/src/utils/intToString
  local.set $4
  local.get $4
  i32.const 136
  i32.const 64
  local.get $4
  i32.load
  i32.sub
  call $~lib/string/String#repeat
  call $~lib/string/String#concat
  local.set $4
  call $~lib/ultrain-ts-lib/src/action/Action.sender.get:sender
  i32.const 4416
  call $~lib/ultrain-ts-lib/src/account/Account.publicKeyOf
  local.set $5
  local.get $5
  local.get $1
  local.get $4
  call $~lib/ultrain-ts-lib/src/crypto/verify_with_pk
  i32.const 4432
  local.get $5
  call $~lib/string/String.__concat
  i32.const 8
  call $~lib/string/String.__concat
  local.get $1
  call $~lib/string/String.__concat
  i32.const 8
  call $~lib/string/String.__concat
  local.get $4
  call $~lib/string/String.__concat
  call $~lib/env/ultrain_assert
  i32.const 0
  call $~lib/ultrain-ts-lib/src/crypto/SHA256#constructor
  local.set $6
  local.get $6
  local.get $1
  i32.const 0
  i32.const 66
  call $~lib/string/String#substring
  call $~lib/ultrain-ts-lib/src/crypto/SHA256#hash
  local.set $7
  local.get $7
  i32.const 0
  i32.const 14
  call $~lib/string/String#substring
  i32.const 16
  call $~lib/string/parseInt
  i64.trunc_f64_u
  local.set $8
  local.get $8
 )
 (func $contract/ultrainio.rand/RandContract#indexOf (; 224 ;) (type $FUNCSIG$iij) (param $0 i32) (param $1 i64) (result i32)
  local.get $1
  global.get $contract/lib/random.lib/EPOCH
  i64.rem_u
  i32.wrap_i64
 )
 (func $contract/ultrainio.rand/RandContract#saveVoterVoteStatus (; 225 ;) (type $FUNCSIG$vijjj) (param $0 i32) (param $1 i64) (param $2 i64) (param $3 i64)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  (local $7 i32)
  local.get $0
  local.get $2
  call $contract/ultrainio.rand/RandContract#indexOf
  local.set $4
  i32.const 0
  local.get $4
  i32.le_s
  local.tee $5
  if (result i32)
   local.get $4
   i32.const 2
   i32.le_s
  else   
   local.get $5
  end
  i32.const 4504
  call $~lib/env/ultrain_assert
  call $~lib/ultrain-ts-lib/src/block/Block.number.get:number
  i32.const 1
  i32.add
  local.set $6
  i32.const 0
  call $contract/lib/random.lib/Voter#constructor
  local.set $7
  local.get $7
  local.get $1
  i64.store
  local.get $0
  i32.load offset=12
  local.get $1
  call $~lib/dbmanager/DBManager<Voter>#exists
  if
   local.get $0
   i32.load offset=12
   local.get $1
   local.get $7
   call $~lib/dbmanager/DBManager<Voter>#get
   drop
   local.get $0
   local.get $2
   call $contract/ultrainio.rand/RandContract#belongRandNum
   local.get $7
   i32.load offset=8
   local.get $4
   call $~lib/array/Array<u64>#__get
   i64.eq
   if
    i32.const 0
    i32.const 4584
    call $~lib/env/ultrain_assert
   end
   local.get $0
   local.get $2
   call $contract/ultrainio.rand/RandContract#belongRandNum
   local.get $6
   i64.extend_i32_u
   i64.ge_u
   local.tee $5
   if (result i32)
    local.get $5
   else    
    local.get $7
    i32.load offset=8
    local.get $4
    call $~lib/array/Array<u64>#__get
    i64.const 0
    i64.eq
   end
   if
    local.get $7
    i32.load offset=8
    local.get $4
    local.get $0
    local.get $2
    call $contract/ultrainio.rand/RandContract#belongRandNum
    call $~lib/array/Array<u64>#__set
    local.get $7
    i32.load offset=12
    local.get $4
    local.get $3
    call $~lib/array/Array<u64>#__set
    local.get $0
    i32.load offset=12
    local.get $7
    call $~lib/dbmanager/DBManager<Voter>#modify
   else    
    i32.const 0
    i32.const 4736
    call $~lib/env/ultrain_assert
   end
  end
 )
 (func $contract/lib/random.lib/Random#belongRandNum (; 226 ;) (type $FUNCSIG$jij) (param $0 i32) (param $1 i64) (result i64)
  local.get $1
  global.get $contract/lib/random.lib/EPOCH
  i64.add
 )
 (func $contract/lib/random.lib/Random#updateMainVoteCount (; 227 ;) (type $FUNCSIG$vij) (param $0 i32) (param $1 i64)
  (local $2 i32)
  (local $3 i32)
  (local $4 i64)
  (local $5 i32)
  (local $6 i64)
  i32.const 0
  call $contract/lib/random.lib/Voter#constructor
  local.set $2
  local.get $0
  i32.load
  global.get $contract/lib/random.lib/MAIN_VOTES_NUM_KEY
  local.get $2
  call $~lib/dbmanager/DBManager<Voter>#get
  drop
  block $contract/lib/random.lib/Random#indexOf|inlined.2 (result i32)
   local.get $0
   local.set $3
   local.get $1
   local.set $4
   local.get $4
   global.get $contract/lib/random.lib/EPOCH
   i64.rem_u
   i32.wrap_i64
  end
  local.set $5
  local.get $2
  i32.load offset=8
  local.get $5
  call $~lib/array/Array<u64>#__get
  local.set $6
  local.get $0
  local.get $1
  call $contract/lib/random.lib/Random#belongRandNum
  local.get $6
  i64.ne
  if
   local.get $2
   i32.load offset=12
   local.get $5
   i64.const 1
   call $~lib/array/Array<u64>#__set
  else   
   local.get $2
   i32.load offset=12
   local.get $5
   local.get $2
   i32.load offset=12
   local.get $5
   call $~lib/array/Array<u64>#__get
   i64.const 1
   i64.add
   call $~lib/array/Array<u64>#__set
  end
  local.get $2
  i32.load offset=8
  local.get $5
  local.get $0
  local.get $1
  call $contract/lib/random.lib/Random#belongRandNum
  call $~lib/array/Array<u64>#__set
  local.get $0
  i32.load
  local.get $2
  call $~lib/dbmanager/DBManager<Voter>#modify
 )
 (func $contract/ultrainio.rand/RandContract#mainVoterVote (; 228 ;) (type $FUNCSIG$vijij) (param $0 i32) (param $1 i64) (param $2 i32) (param $3 i64)
  (local $4 i64)
  (local $5 i32)
  (local $6 i32)
  (local $7 i64)
  local.get $0
  local.get $2
  local.get $3
  call $contract/ultrainio.rand/RandContract#getVrfVal
  local.set $4
  local.get $0
  local.get $1
  local.get $3
  local.get $4
  call $contract/ultrainio.rand/RandContract#saveVoterVoteStatus
  local.get $0
  local.get $3
  call $contract/ultrainio.rand/RandContract#indexOf
  local.set $5
  i32.const 0
  call $contract/lib/random.lib/Voter#constructor
  local.set $6
  local.get $0
  i32.load offset=12
  global.get $contract/lib/random.lib/RAND_KEY
  local.get $6
  call $~lib/dbmanager/DBManager<Voter>#get
  drop
  local.get $6
  i32.load offset=8
  local.get $5
  call $~lib/array/Array<u64>#__get
  local.set $7
  local.get $0
  local.get $3
  call $contract/ultrainio.rand/RandContract#belongRandNum
  local.get $7
  i64.ne
  if
   local.get $6
   i32.load offset=12
   local.get $5
   local.get $0
   local.get $3
   call $contract/ultrainio.rand/RandContract#getExistRand
   call $~lib/array/Array<u64>#__set
   local.get $6
   i32.load offset=8
   local.get $5
   local.get $0
   local.get $3
   call $contract/ultrainio.rand/RandContract#belongRandNum
   call $~lib/array/Array<u64>#__set
  end
  local.get $6
  i32.load offset=12
  local.get $5
  local.get $6
  i32.load offset=12
  local.get $5
  call $~lib/array/Array<u64>#__get
  local.get $4
  i64.xor
  call $~lib/array/Array<u64>#__set
  local.get $0
  i32.load offset=12
  local.get $6
  call $~lib/dbmanager/DBManager<Voter>#modify
  local.get $0
  i32.load offset=24
  local.get $3
  call $contract/lib/random.lib/Random#updateMainVoteCount
  local.get $0
  call $~lib/ultrain-ts-lib/src/contract/Contract#get:receiver
  call $~lib/ultrain-ts-lib/src/action/Action.sender.get:sender
  i32.const 0
  global.get $contract/ultrainio.rand/MAIN_BONUS
  i64.const 357577479428
  call $~lib/ultrain-ts-lib/src/asset/Asset#constructor
  i32.const 4784
  call $~lib/ultrain-ts-lib/src/asset/Asset.transfer
 )
 (func $contract/lib/random.lib/Voter#isWaiterVoter (; 229 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  local.get $0
  i32.load offset=40
  i32.const 2
  i32.eq
 )
 (func $contract/ultrainio.rand/RandContract#waiterVoterVote (; 230 ;) (type $FUNCSIG$vijij) (param $0 i32) (param $1 i64) (param $2 i32) (param $3 i64)
  (local $4 i64)
  local.get $0
  call $contract/ultrainio.rand/RandContract#curtBckNum
  local.get $3
  i64.sub
  i64.const 2
  i64.gt_u
  if
   i32.const 0
   i32.const 4816
   call $~lib/env/ultrain_assert
  end
  local.get $0
  local.get $2
  local.get $3
  call $contract/ultrainio.rand/RandContract#getVrfVal
  local.set $4
  local.get $0
  local.get $1
  local.get $3
  local.get $4
  call $contract/ultrainio.rand/RandContract#saveVoterVoteStatus
  local.get $0
  call $~lib/ultrain-ts-lib/src/contract/Contract#get:receiver
  call $~lib/ultrain-ts-lib/src/action/Action.sender.get:sender
  i32.const 0
  global.get $contract/ultrainio.rand/WAITER_BONUS
  i64.const 357577479428
  call $~lib/ultrain-ts-lib/src/asset/Asset#constructor
  i32.const 4784
  call $~lib/ultrain-ts-lib/src/asset/Asset.transfer
 )
 (func $contract/ultrainio.rand/RandContract#vote (; 231 ;) (type $FUNCSIG$viij) (param $0 i32) (param $1 i32) (param $2 i64)
  (local $3 i64)
  (local $4 i32)
  (local $5 i32)
  call $~lib/ultrain-ts-lib/src/action/Action.sender.get:sender
  local.set $3
  local.get $0
  i32.load offset=12
  local.get $3
  call $~lib/dbmanager/DBManager<Voter>#exists
  i32.const 4000
  local.get $3
  call $~lib/ultrain-ts-lib/src/account/RNAME
  call $~lib/string/String.__concat
  i32.const 4016
  call $~lib/string/String.__concat
  call $~lib/env/ultrain_assert
  call $~lib/ultrain-ts-lib/src/block/Block.number.get:number
  i32.const 1
  i32.add
  local.set $4
  local.get $0
  local.get $2
  call $contract/ultrainio.rand/RandContract#belongRandNum
  local.get $4
  i64.extend_i32_u
  i64.ge_u
  i32.const 4080
  local.get $4
  i64.extend_i32_u
  i32.const 0
  call $~lib/ultrain-ts-lib/src/utils/intToString
  call $~lib/string/String.__concat
  i32.const 4144
  call $~lib/string/String.__concat
  local.get $2
  i32.const 0
  call $~lib/ultrain-ts-lib/src/utils/intToString
  call $~lib/string/String.__concat
  i32.const 4208
  call $~lib/string/String.__concat
  call $~lib/env/ultrain_assert
  i32.const 0
  call $contract/lib/random.lib/Voter#constructor
  local.set $5
  local.get $0
  i32.load offset=12
  local.get $3
  local.get $5
  call $~lib/dbmanager/DBManager<Voter>#get
  drop
  local.get $5
  call $contract/lib/random.lib/Voter#votable
  i32.const 4240
  local.get $3
  call $~lib/ultrain-ts-lib/src/account/RNAME
  call $~lib/string/String.__concat
  i32.const 4264
  call $~lib/string/String.__concat
  call $~lib/env/ultrain_assert
  local.get $0
  call $contract/ultrainio.rand/RandContract#triggerRandGenerate
  local.get $5
  call $contract/lib/random.lib/Voter#isMainVoter
  if
   local.get $0
   local.get $3
   local.get $1
   local.get $2
   call $contract/ultrainio.rand/RandContract#mainVoterVote
  else   
   local.get $5
   call $contract/lib/random.lib/Voter#isWaiterVoter
   if
    local.get $0
    local.get $3
    local.get $1
    local.get $2
    call $contract/ultrainio.rand/RandContract#waiterVoterVote
   end
  end
 )
 (func $contract/lib/random.lib/Random#queryLatest (; 232 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  local.get $0
  call $~lib/ultrain-ts-lib/src/block/Block.number.get:number
  i64.extend_i32_u
  i32.const 0
  call $contract/lib/random.lib/Random#generateRand
 )
 (func $contract/lib/random.lib/RandRecord#toString (; 233 ;) (type $FUNCSIG$ii) (param $0 i32) (result i32)
  local.get $0
  i64.load
  i32.const 0
  call $~lib/ultrain-ts-lib/src/utils/intToString
  i32.const 104
  call $~lib/string/String.__concat
  local.get $0
  i64.load offset=8
  i32.const 0
  call $~lib/ultrain-ts-lib/src/utils/intToString
  call $~lib/string/String.__concat
  i32.const 104
  call $~lib/string/String.__concat
  local.get $0
  i32.load offset=16
  i64.extend_i32_s
  i32.const 0
  call $~lib/ultrain-ts-lib/src/utils/intToString
  call $~lib/string/String.__concat
 )
 (func $~lib/ultrain-ts-lib/src/return/Return<String> (; 234 ;) (type $FUNCSIG$vi) (param $0 i32)
  local.get $0
  call $~lib/ultrain-ts-lib/src/utils/string2cstr
  call $~lib/ultrain-ts-lib/src/return/env.set_result_str
 )
 (func $contract/ultrainio.rand/RandContract#query (; 235 ;) (type $FUNCSIG$vi) (param $0 i32)
  (local $1 i32)
  local.get $0
  i32.load offset=24
  call $contract/lib/random.lib/Random#queryLatest
  local.set $1
  local.get $1
  call $contract/lib/random.lib/RandRecord#toString
  call $~lib/ultrain-ts-lib/src/return/Return<String>
 )
 (func $contract/lib/random.lib/Random#query (; 236 ;) (type $FUNCSIG$iij) (param $0 i32) (param $1 i64) (result i32)
  (local $2 i64)
  (local $3 i32)
  (local $4 i32)
  local.get $0
  call $contract/lib/random.lib/Random#getRandMinBckNum
  local.set $2
  call $~lib/ultrain-ts-lib/src/block/Block.number.get:number
  local.set $3
  local.get $2
  local.get $1
  i64.le_u
  local.tee $4
  if (result i32)
   local.get $1
   local.get $3
   i64.extend_i32_u
   i64.le_u
  else   
   local.get $4
  end
  i32.const 5000
  local.get $2
  i32.const 0
  call $~lib/ultrain-ts-lib/src/utils/intToString
  call $~lib/string/String.__concat
  i32.const 5120
  call $~lib/string/String.__concat
  local.get $3
  i64.extend_i32_u
  i32.const 0
  call $~lib/ultrain-ts-lib/src/utils/intToString
  call $~lib/string/String.__concat
  call $~lib/env/ultrain_assert
  local.get $0
  local.get $1
  i32.const 0
  call $contract/lib/random.lib/Random#generateRand
 )
 (func $contract/ultrainio.rand/RandContract#queryBck (; 237 ;) (type $FUNCSIG$vij) (param $0 i32) (param $1 i64)
  (local $2 i32)
  local.get $0
  i32.load offset=24
  local.get $1
  call $contract/lib/random.lib/Random#query
  local.set $2
  local.get $2
  call $contract/lib/random.lib/RandRecord#toString
  call $~lib/ultrain-ts-lib/src/return/Return<String>
 )
 (func $~lib/ultrain-ts-lib/src/contract/Contract#onError (; 238 ;) (type $FUNCSIG$vi) (param $0 i32)
  nop
 )
 (func $~lib/ultrain-ts-lib/src/contract/Contract#onStop (; 239 ;) (type $FUNCSIG$vi) (param $0 i32)
  nop
 )
 (func $contract/ultrainio.rand/apply (; 240 ;) (type $FUNCSIG$vjjjj) (param $0 i64) (param $1 i64) (param $2 i64) (param $3 i64)
  (local $4 i32)
  (local $5 i32)
  (local $6 i64)
  (local $7 i64)
  (local $8 i32)
  (local $9 i32)
  i32.const 0
  local.get $0
  call $contract/ultrainio.rand/RandContract#constructor
  local.set $4
  local.get $4
  local.get $2
  local.get $3
  call $~lib/ultrain-ts-lib/src/contract/Contract#setActionName
  local.get $4
  local.get $1
  call $contract/ultrainio.rand/RandContract#filterAction
  if
   local.get $4
   call $~lib/ultrain-ts-lib/src/contract/Contract#onInit
   local.get $4
   call $~lib/ultrain-ts-lib/src/contract/Contract#getDataStream
   local.set $5
   local.get $4
   i32.const 2448
   call $~lib/ultrain-ts-lib/src/contract/Contract#isAction
   if
    local.get $5
    call $~lib/datastream/DataStream#read<u64>
    local.set $6
    local.get $5
    call $~lib/datastream/DataStream#read<u64>
    local.set $7
    i32.const 0
    i64.const 0
    i64.const 357577479428
    call $~lib/ultrain-ts-lib/src/asset/Asset#constructor
    local.set $8
    local.get $8
    local.get $5
    call $~lib/ultrain-ts-lib/src/asset/Asset#deserialize
    local.get $5
    call $~lib/datastream/DataStream#readString
    local.set $9
    local.get $4
    local.get $6
    local.get $7
    local.get $8
    local.get $9
    call $contract/ultrainio.rand/RandContract#transfer
   end
   local.get $4
   i32.const 3336
   call $~lib/ultrain-ts-lib/src/contract/Contract#isAction
   if
    local.get $4
    call $contract/ultrainio.rand/RandContract#unregister
   end
   local.get $4
   i32.const 3608
   call $~lib/ultrain-ts-lib/src/contract/Contract#isAction
   if
    local.get $4
    call $contract/ultrainio.rand/RandContract#redeem
   end
   local.get $4
   i32.const 3984
   call $~lib/ultrain-ts-lib/src/contract/Contract#isAction
   if
    local.get $5
    call $~lib/datastream/DataStream#readString
    local.set $9
    local.get $5
    call $~lib/datastream/DataStream#read<u64>
    local.set $7
    local.get $4
    local.get $9
    local.get $7
    call $contract/ultrainio.rand/RandContract#vote
   end
   local.get $4
   i32.const 4960
   call $~lib/ultrain-ts-lib/src/contract/Contract#isAction
   if
    local.get $4
    call $contract/ultrainio.rand/RandContract#query
   end
   local.get $4
   i32.const 4976
   call $~lib/ultrain-ts-lib/src/contract/Contract#isAction
   if
    local.get $5
    call $~lib/datastream/DataStream#read<u64>
    local.set $7
    local.get $4
    local.get $7
    call $contract/ultrainio.rand/RandContract#queryBck
   end
   local.get $4
   i32.const 5136
   call $~lib/ultrain-ts-lib/src/contract/Contract#isAction
   if
    local.get $4
    call $~lib/ultrain-ts-lib/src/contract/Contract#onError
   end
   local.get $4
   call $~lib/ultrain-ts-lib/src/contract/Contract#onStop
  end
 )
 (func $start (; 241 ;) (type $FUNCSIG$v)
  call $start:contract/ultrainio.rand
 )
 (func $null (; 242 ;) (type $FUNCSIG$v)
 )
)
