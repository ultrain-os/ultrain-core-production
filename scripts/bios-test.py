#!/usr/bin/env python3

import argparse
import json
import os
import random
import re
import subprocess
import sys
import time
import string
import requests
import json

args = None
logFile = None

unlockTimeout = 999999999
defaultclu = '/root/workspace%s/ultrain-core/build/programs/clultrain/clultrain --wallet-url http://127.0.0.1:6666 '
defaultkul = '/root/workspace%s/ultrain-core/build/programs/kultraind/kultraind'
defaultcontracts_dir = '/root/workspace%s/ultrain-core/build/contracts/'
systemAccounts = [
    'utrio.bpay',
    'utrio.msig',
    'utrio.names',
    'utrio.ram',
    'utrio.ramfee',
    'utrio.saving',
    'utrio.stake',
    'utrio.token',
    'utrio.vpay',
    'utrio.fee',
    'hello',
]

initialAccounts = [
    'root',
    'root1',
    'root2',
    'root3',
    'root4',
    'root5',
]

accountsToResign = [
    'utrio.bpay',
    'utrio.msig',
    'utrio.names',
    'utrio.ram',
    'utrio.ramfee',
    'utrio.saving',
    'utrio.stake',
    'utrio.token',
    'utrio.vpay',
    'ultrainio',
]

accounts = [
"genesis",
".111",
".112",
".113",
".114",
".115",
".121",
".122",
".123",
".124",
".125",
".131",
".132",
".133",
".134",
".135",
".141",
".142",
".143",
".144",
".145",
".151",
".152",
".153",
".154",
".155",
".211",
".212",
".213",
".214",
".215",
".221",
".222",
".223",
".224",
".225",
".231",
".232",
".233",
".234",
".235",
".241",
".242",
".243",
".244",
".245",
".251",
".252",
".253",
".254",
".255",
".311",
".312",
".313",
".314",
".315",
".321",
".322",
".323",
".324",
".325",
".331",
".332",
".333",
".334",
".335",
".341",
".342",
".343",
".344",
".345",
".351",
".352",
".353",
".354",
".355",
".411",
".412",
".413",
".414",
".415",
".421",
".422",
".423",
".424",
".425",
".431",
".432",
".433",
".434",
".435",
".441",
".442",
".443",
".444",
".445",
".451",
".452",
".453",
".454",
".455",
".511",
".512",
]

#pk_list = ["b3f88e7694995cf2d46fb9bbe172b1e9d2ae8ea372ec26c01a6603bd415dc64d",
#           "e2e7339522395916f941c49b3d58dfc4c0c61e0e3910fcf568b3c2ce2005e32b",
#           "92f7b32418e79b2a4ba716f6745c361381411f0537376e438b2399486ed0c8dc",
#           "4141e8c7a4780df3cf840ed556d52108b08a3bc2ead12bece6bc06b9d9487eb2",
#           "933c5ceddf3d27af114351112c131f1bb4001a6a6669449365b204441db181a3",
#           "6fadc36ba297d6db53ec0a094c27a32ee266ab17a63cfa149609edfe881c7118",
#           "4ae81777689da3f6c6972effa4857cd32ddd3466fef42cb281babc0198546faa"
#           ]

pk_list = ["369c31f242bfc5093815511e4a4eda297f4b8772a7ff98f7806ce7a80ffffb35",
"b7e0a16fdca44d4ece1b14d8e7e6207402a6447115ca7d2d7edb08958e6d8ed5",
"4031a95071a092eca8646d3999c438bdfde368d4837770755af65a11b4520a48",
"2b43a3d8e0523a85141bbfca41006cf9abc47587d0ff3c46d257551ec05f1677",
"b9a55c3c661abd8c539b7a7c05c8176036f87aeeb6b117a138327cfdb374cc23",
"8f6e3b3336276138023617f0ae0e6fd0c37d27aa9995f9803fe78df4941dd3ec",
"3696fdfb2a9716ae4b1bca0d5b0a861a1e5d64562994aeb515eed49290c9f1c2",
"578451935c370d9c7fbcdd77e35a40e49bf0a5311e065035778ac27e6263b10d",
"9ea2644ec77f3ecde8a47db1c197e63088c91af3edf186c54038f90650056c3d",
"eb185504c078a9c87f6042746d03566183b0217cb4a12ae202c235b175955441",
"a380f12af673da43e1e8f836f4a2621200ff737342892d80c3a8ef597e949413",
"1aef98b82a525ae61220b7e34270eb7362cb2537805694e6179afb73ac638313",
"cecbfd84857edc1a5fa2e945bec767dbfc816f5dc3b9a2d98920d6826f839cfc",
"c1052bdd25d892f446268deb6c36b1c94252145067fb1990d5afe8150d789fd8",
"c7e4c4fd86b08bd15939bcb82a3f2bc7621bfb606797f483ecd9afd8c61dd94a",
"c42fbeec466677bb873abb62ab8363942f60dd30cdfb4345f7b658c1d375ee3d",
"b5b96cd09fb3bf098c9a8b6c8453e641618ab7477bdcc02695d5c64351b4474e",
"7038618da8a7bcc68797ea5e529d48749501b48d8d50b636ca7c7177c232c7cd",
"9a5621d38478261d77919ce89c64e1bcfd892a54f9aab3f55c861120a632b293",
"2a8aad345502e8351294886151306dfade499a8891df991e2da449c40481479f",
"3491bda3185f56632c25da89ccc7c61a7140546c8e9014021d845dc399265f3c",
"aaacec343513dda6ac912e23a16e7c4a97855cedb732599c99d22b0ad9124e9b",
"1263638be8d736c9cc313c2bbcbda1b95c0677187c286f2f7db15ab3bbf6f08f",
"9fb6e571e31e3c053cc40eb5ac71fc9a1e062c41e219b902dcfc7fb5e05aac13",
"f17273e68bc07668464dd5e8a8d96dc2a9d479b5948627d04ec03e721c172028",
"77b038699c3f4bc665b71187e64678c39fd6e83230aaeae9ba1c720f22959c94",
"febb254bb517abe8302bc702ffba746b788ca8135d31008e0c2ba91263346430",
"bef327a19755173824c85e299f1ecb0d79f7d655a06a362236e5033efda5d3e7",
"e229c2d7a28a0c606d514d0bc94cd1b7219da8cd49bc80dedef24d49028aadec",
"e14a187953ed64a29fccc5c9f5f402b7d327672bf08589cc45060c3bc268114f",
"127d47deed92b0d06f7abfc6a74ba7bee94908cf47c23399ca06d772c6e9e8a3",
"90d5f71464791141f580b30e36e575b2cf51741a3b4a811a416d319f9af15834",
"1148e1559ef544f396e94843ac85291e6997c9b019fca62a0eb959ff892f6398",
"08ffd968a7d5b18d95fe8341192766f9f7b1d698234e7000832b73b6986b149d",
"24a8dc0cb0fbeacaf3c2fce88c1b7b04c80a41a14c03f64a71ceade9db51f712",
"0626c726da6efb60434db31bb1dbf6e60d2e7754b8e53bd9bcd4f9085eef2231",
"2201163f42e5ae2cbaa6ca4b5935930d555dbc6a9552ad439a69f828cd533f68",
"b46a99f26f6d4290d02f6eeec52a7477072717543435cd571fdf11179d5f1b93",
"256bbd582d2d8c2d7c16564492cb9471276b57795abb58d674e7cd703f29f2f4",
"f4fa66cbbc970780ecf80a48b4eaa1afd80800f7ef95cf83956acacb5d717a03",
"dd3ed6bfc08020f54d0d569ce9e9c246a02e26343e81a6ea3bd3415d2863fd6d",
"21dd0d2f9ee890469f9bc91af57690edb0a5694f580c0ccbb2e6e57a64f93abe",
"6f199296848c0d0dbae6ff9802fda7c3e5da666aadf52452d97d08a923cd3ef2",
"9ae46fd2ae6688dd1da7b97e4e821affa0febc04f4d23f922134210998e0571e",
"27b2fe7b394fc16bc00d6644306a46cb0536eafb459f1db86845b7b00638df0e",
"8828ecc32c987f223b8a63e3e3bbebf9ef3394511b4fd3d0ccfa61befcc1b343",
"b10795055c234da092b1660ff2d0e6ddcf44e0d9be33272728bf1e0c5c24ad1c",
"40da9622e65568ebd55046c8fe0b1759e888a349ffbe79875c7cd218006e1084",
"e9f369ef15ca48d6623ea8bd0a4cd673b1f6a599edf24bbb8ad08ee129b55dea",
"17ba9478a057c634d2f8ca5bbc7d2dfa651b9308f128723d2a632a39bcd4b900",
"ae82fa352eca43bb1f8cff098014e0e1ba9f54bed88d5befe92e0913242f3cf4",
"18d0bd1664c0843be054473ab7a9895986fee484dcaadf452a4bc8b28fc19425",
"85f59e966f6df5f9a916909936d11f4787c5ece7df32f206094831071567da96",
"3b2a69e0416f9cf3be51703dcf83e01a5b749672e102819cadbb9460e48cf9da",
"ea0935edafc112bec56e5e92d7dc6115af1e9c2e309c3dc81a677cc8cbaa93e4",
"540f9eb5a30f27b0972d9a75a483e0d95a358e35d9a29b8298db9677fd8a26cb",
"48c1565930dee7bc90315000204e9fdbb4a454e63b97b4a1d640a371698109a9",
"09d9dd2b34b6d459d99fc1831a65f54bb05bfdb7079cc3209fb81ac311f201f6",
"e1472ffb38a9b895c3d845119026e0a4f899f9139bf1522e9019563dddf66650",
"6ffa14f75f04e1a0d533be500a12e63296b12b3d642cdf898aa3f46d3d472684",
"100ca825d8031f3d965b4cc9187f81a32403972e6f41727c7a4923d5859350fb",
"237a95da1df88825a5184e5ef17478846b67e6f698dfc347ea116db71a13cc83",
"6c43c41865abdb8be448edf3c57494f6b35b3c8266819d1d3a523be0718fab5a",
"ae37c040f2630bde1ece12d22a03e705bb44b173dd7225f6da01fb1803bb6463",
"2c939dacd500c4bfbadc5c3eaa4f885d90539c04c3904908d6caba5a46b92cad",
"5068fdd1aa86d8de1a2c3a067273ad3b59149a4c71486edf064cfa73fb772a08",
"bfb401fca7fee9b0d22d32bc0f1e23311bf1aa4c169a89111fc0b1dfb1d19429",
"8f346758ee41c40b3cb2529f4f42c00612ebe1349aa657763f24dec9ee2a2c2e",
"3da6b8f635db2c3081c18ed465eb3f96efe98c964675422d455581f723bd81f4",
"e4d77ba5edf199414c4f93bbb50416f98118db7254e0c902da35c78779effb56",
"debc916a912481cb7945c3b10b8d9e39f1e51e67d901970e26a923a9fdad5d37",
"f0da13b6d9317e2f0e2426cd7938de8f0a9d27a76f1caeb5ea202ee8150ee231",
"1dd1e67c67cd8c201d3fdf02cf1cd94b86f7c3158d5095e4c82e6c65b40ed6e5",
"1b21dab935b232c6bd9ab7c777b039f0cf2d02173e1c59c8028bc7e4bde6f4d5",
"5c91c32161c7695885bdc67d052094540733ac3ea74142f685b7e787f65a3674",
"89c2c878bf7a9710b615a5e0d805c7d40422971bb94c9b7a4722011d2bc64b42",
"4d91179fb6b4191cfef9880b5d8b4adf46febb426c0f9012067a9e7494c9f2cb",
"f35cdfe0e09f1ebe81f578f90e884929813892b1f2ff9edb9a793df63320275e",
"0cb818dc5f8e3503f384d750bac810a15115d041e178e9443673efddb4d33530",
"b03f409f791e2287bcabbc0991748776ebdb014fc47f2aef3d3c886e2996b96c",
"1ef8de12f718ed524850df9fde5bb566c86f71feb0ee861706fb1f80f001989f",
"40d8f4e39a9e57330986299f99aa14f382a01fe1c03da133ade0fcadff787288",
"54d2bc5dcdb86cdf62ea744484e47b574594171b13440427bea559e51d1f6cc9",
"bd6beaa83315ee770fed18529fabfd1c068b364c95a0d3b0e8e1581d19054780",
"d1cd477aa4f0596526aa56cff1a5febf34ed1c6284132b2730a44c94c89b4cda",
"05edd860aa5b7a2ac369657ffa01736254ac9f1dbe1bb8ea3c8c297393cbc855",
"1b1e277d2c8cb606d4124d3d65b2c592a15bf348a29c4e9a0baffa1eaeb2bf6f",
"f6d66980288d49f526c2f45fdfa85dfb4a5ee92a01c8b1ce11279c9a2a2bafcd",
"f4a7bbea49d9c6f5860e3efe35033c014419e5817aace3333faf529928b7a2ac",
"ee358bb76744e614bb6de369b9841284acf5071b043a13ea2ac7ffd359db4dd9",
"53ea0eddb860435954e96d537a1ab1a3f0101e986a0ae5f208be637c57951eb0",
"7b442f420448be491a4c4f10a6787aeda1854fcb376ba83f92d5b00b61cc70ca",
"101fdd01c352ddae5f01f2185cf37b5253a8126607f5013ef1af7942004788c3",
"204f70919460de9677d8c4e262d165390dd84ee61bc4a64c1724e36381aa10ee",
"040cf0855e2963e2edcb44055d2b80dcd9455354cca8a159f5729bba9541e668",
"24ba064c7ce3e01b1f4bfebba84385f5b55f337fb64ea7b59768312cf4104064",
"5acc38319c05b172034984e92a5e86693fac5c3197ae7b13830bdd9d181a4d32",
"f7e248533ec9d8b7436eab8eaac68fb30330a3fed82682d1ad8efd5674eb4155",
"7f8ea2d823570652b276cc5a195a1582b59dabf810390b980db5e23ca10fbdd8",
"4a611c59cda52c545b6cb3b84028ecee7326d188f55794a84423eb02d7289521",
"35a6e036a7bb35d360b1a9356b8f24b62cb31ffbba952444ce7f7a91a8ed8143",
"e39bdb5078641724c4251a95b8a77968e3f9b08449702257e95969790e252c0c",
"c72165307915143d4243584d9e858cd203c1490d60f27d40f639cecd7147e64d",
]

account_pk_list = [
   "UTR7Z3j9M7dcXfrBGcBbENQphuG8hAEYyPueT7cc8XzKgzymppJ9Y",
   "UTR6r3BNrssrD9F5jJo17aszYN3S7mrYcCYqXGDgaEB1WPgF9LVfe",
   "UTR6BThE2vci6Jqq5YyfYCy8mHMdk14Dfx9n5RWfkL8jcMDW7PJ5b",
   "UTR5kNr7rCgkkkyqXXZh51ZAHm8QbvhhX1KHH6wT4zzVMi7w8miZz",
   "UTR7bggm3iyTDmZMHfsnZ96N8wQnGmam6x97uuTYtxgSLWR3zyCix",
   "UTR8YCCmCEAM4GGqhSwQTB4SwP3J4sJTs721cAjZ6AXHr55qbig29",
   "UTR6DQVtj2frjePAFBiLnT288KfG3HAiMHCmbK9YzcHEaJQEmvyeU",
   "UTR6k8qHBrc1CYxNUdbHSThtWid5E4xwNLF9kxk8bXuHV1q3BaQ39",
   "UTR77phXVHqBtKm1b7NZbyhHt839oRB67Mr2A8pRmavNkTFcPq6yy",
   "UTR5dCt3266NBtNzXaMfBoa5GAjGv4o8K7g1bUqNWKuEAimWCmUhK",
   "UTR66C35vMUv9Zn4WhtHCyrVfzPR3cqmszUvLCnxjrnhC6scTsSSJ",
   "UTR5HV2evAxaty7eonw6gzqa6L4ipfTEqnbL426rXhuYtxqrKjN6H",
   "UTR6ruimZSqg9uWsAXo6JiP6dtjCeNSYH5158VvVTxUpcuyC1FADM",
   "UTR5xrJz8r5WDBErKkT8qjsUYLTAGMM5FWcMiE3Hzk3jKvd14NhSK",
   "UTR5UbEK64MDX74QT5EEPSWAxQocgJZu4eraYQP2vFdngCkA47e6r",
   "UTR5517AaVg7qgz7VdDCurDrXwfdVQ2TSj1ToCWxCnvfxXhyZkpYa",
   "UTR8PD6cNkD8f8VqbAUHdmWvqDhXK6f6jsAQXEfgCZkZpSEEhnDzz",
   "UTR6nQQp1iwom8crq2NN9pf67pWtXvGpURXC9E7gFqGYeoCETo3nW",
   "UTR63tRkbAg8KjqYBbiVnH8U5NMnCi4RFX3my3JMdqvxQv9qQZdv7",
   "UTR6XP4oPPvzpPvApKBKzvFJthY5tdGrBK5hfBbnCPs5qPuvXk87z",
   "UTR5rX3vVgt9pzu1HrVK7NCqaPP3QRQYD9yFndfAFfiQkZ4mtHFeD",
   "UTR55ZvvtpCB9JqED7KCt3MCMGrg5rYgqtpTsBBiSnGsZmCMtMrn1",
   "UTR6NMww1sj1tjqVARP6UPP4ZmJkeUfV34y6pDTdaCvQxSJAqLk7a",
   "UTR5YdXdKVKZ72U3hqrr42r8neZAXWakMUetMis7ez1xURfwqhJnH",
   "UTR7BQawZaKAt84h3RYPBSFLZhZzutNgzdbbWdwPwpbajVtmLy4pT",
   "UTR7CdHfryy81LYBzqjjW9ifXJa8t2H9xxNgvRQCSgNaB791iHcYn",
   "UTR5rcjZHTadwhdJvWT9DxXKJXQxYAoFRjKu84JrD5DFJeBQffRh5",
   "UTR7piAr5kRGU1fLvD4knecVcc95LjVepDAs8GLZToVfZ4tGxPtZ4",
   "UTR5Qu9wnwtNhGmXBWqhxdYNTZaXL64GpGcZNq5ibxiMntwKQxKKy",
   "UTR7VzpTEZ7pXd3Kj1TURXd5zcg1MNXCKoqRQuHEbPTT3jK4SaL6X",
   "UTR6u7toUokBbgS2wSNQajWdm6mgsLBTWs2tPa2k7wesZDnGH5oub",
   "UTR6bQbxhdWGr9fUq2E9WM11CWPWiMz8kUr4w9LDFL5edJG6PTw3n",
   "UTR6ocXAPW9ATn2828DwTjtGq41ZjGxuAUAqPNmgeGRpod4wLF8Bf",
   "UTR7gF2ffa2juoVqpGw1NaoKZuiTJEEjfh3cLwKPZcG3UWWrn2vsH",
   "UTR7bdEQUF4dKC63PgmDuxphS1Rj1o3kkCtVveACwhYzAmAAW2WVC",
   "UTR5yMKZxfgCvZ7sDLknPP1DnTbx2yazGzy7XsZVAskC6SKKPrCmF",
   "UTR8mfeZtVFGWfVbQCR2QYwfFC73JQeLAzofZ5tNLrqcXxL2wWRWR",
   "UTR7pGFnWZaASuBc55oUUdhqhtTzBVFJMav3o3Lu7EybjYoAzijUg",
   "UTR6ZZ2zjyygDDe7SEwzVhBZ65AXLemauZU4AyhpD79zqvppTSCzv",
   "UTR6LYiacD5oC1y1xBdFFTcPA1BaonrpjhBvhxrQ5SzuK8vwMvstf",
   "UTR7psF6WjpthhDAprTKgT9rYQEsLG61h9HVZMUg2oB9Pet3MDSK9",
   "UTR7EDPukB7JFLLbQqLNJZnNVRvTRVHkvvbKSN5RfJq2qk6DkdEmS",
   "UTR72XeDfs1KM7ReVPgoVvjScPXtQMe8BrxARuq6Q7Dw2gMEUMmRD",
   "UTR6xhWozukSvBAPPdco9bvnQVg5nsSZb4149dXqP1YzQq8rcttwN",
   "UTR8JX4vd7aiPjABAEL26mf1YxrRqu8f5Ajgp4tb2Xbtmq7Fy7wpB",
   "UTR63Ty68dxMjcYa6AMp3QFPVHWL1U1ot8GuzUCZdWN11hoNt6q9P",
   "UTR7TDbLPMfxeu3KWtrdPkzpcv6s1vEkmAuVUBwkfe9kTmkGdjdVm",
   "UTR8DbcJQs7cLs1abhEkGpk7W5JPDC6Xnb8TuSXt3Ea47LfhQZ5A2",
   "UTR5sNRnBK3YSEugCADukfFrQ37CnLTYHUf3eNs3TT1GzwaifY2Ky",
   "UTR7Zp7YCgVhqm595LynXfxrovEa6YSHp1p4asawznXxUbmfNm2nU",
   "UTR7zGp2XU6AhT6nDBoWBK4RX8WZy5cE27c6CR24pZwUQ9rrHViLq",
   "UTR6hWmULuSWJV7pSChV1mMmqX9uR9usdaEgXhtgNWwHzAxALKj55",
   "UTR75ZnbyFrcGmCBSC1k2natjnErHxRtEB8PchsTBPjgZtnakH4EW",
   "UTR73oXxXCKNm3WGGQ7gcyXtNKVt8YGVHpHEpMbfNbBxF9wKx667t",
   "UTR5tpRXfxRUbXsqdwpHa1hiGGetidJLVmbAQuzPDEVtMhNwvknGP",
   "UTR8jSJ3Xjkd1PxszKXpcWf6Af9NDuriJP2LMRgeche7nShNH9Uwc",
   "UTR722rWhYky7UevBrRdGgvMgeVGHJXGAGnqdtWEFGp5n2iqFtp8m",
   "UTR74DCevod8QmMykEaCv4vXd61jYZ15oh2oJKoeq56xzAKYNThMV",
   "UTR5MSsXAC4bgkz2EEZh6FKn3cxVo7kTpTmdCUhir6wdnLuexF6Yx",
   "UTR5RxjqAmWaHX61grk6A9EphCN9ndXSu4xRo4UfGPZgLyQaDY3fk",
   "UTR6XdQgZRJrjNofgKfXnHmqbqUjhBMzRXKMwtpwLHTvSfCvyFTxd",
   "UTR6oqPYRBLzf7KqmZNbomjKsPumHuBpKvCmeCF6uqzAGqoVhmLwC",
   "UTR7Enx7MsTZ4K4NjnvzvgUS15jEQPedhZqspQ3gXtzt97sVz4j4b",
   "UTR8EBUZUy4s4fNP3TnJrxajrHmY1PbDt1ZPm8yhhgN38D4vXqkNh",
   "UTR8JNWqMi1BLvHTRDVUHWqeTxM1LF9J8bVAphSeKWHLQbxqmMjFR",
   "UTR7B2f9rDtHXmM8Qg9EJarfCCXWGocgRQndW7VP9xjfyrhrf7zmi",
   "UTR7EprPZfbUTFCHeUMv9FAfGzw9EpSyQNdzkdBEfpppcML4HXtJ8",
   "UTR8KVmzj38vC1dEvES3NsWpM9QMSGnNF8KtYEUy1vyin14KoXjpq",
   "UTR7DJ922MKyUKrN8gYZhomT2anwX49ixhiAjWUULshwp1KALSete",
   "UTR5Wjju8nrAzg7ZE2xoYjniCTqpVi9JrwasZ2DyYThkGTNZiZoqk",
   "UTR68HLiC517XdryP2khL4AZfmhKnpLoGnfXPRmSzfZXyCgGpot8k",
   "UTR6PTKvZXdT4dxobxmEps85sdHBHX2Ambd8DPDduSoSqM9XUE1CC",
   "UTR79xv5QHyj6N3E3LcxNuBr1RSVWMZkDdH5ZJUqFy78T7ocwf9ee",
   "UTR8hZoopY3pyyWuQJpH4g11txmPNDmfP7btGLqvcAhbFZC7a2o7e",
   "UTR547en7mJrbfQGC2JQErWvRJJ24r1DaK1vkzd4LSmFdHDYedzEt",
   "UTR74CA2QHZEjWQNi6a1aoxVN2gJsPCxB74yBvcWRezkpjKkFrXAw",
   "UTR6Kqq7dmAberaE11Vpbfg5voDnb8awyVra3xnKmqZRUCw4ENoFi",
   "UTR81pbSg5gN3pjvDrGh3nqcKcZke2EJvk9JgnSGfidFf6U3fb4qm",
   "UTR5aVdQWRb1wcQxNzXB6hEftp4goAYPGiyhHauRTp5iLaKaPML3E",
   "UTR5h7Vg3rA4NxfLkEL5zogyW4GmXNcuNjfMK1cTtvXUjeZdg2En2",
   "UTR67RkkkF7j5k5muuxVFJNkAHZtRa6pMjkbLHkjrvy1VvM85NzAD",
   "UTR5qRjU9Cr9dZPpQBhJazfzyJ8q3QBFsSj1Yj7UwU46C9nM8zEJ7",
   "UTR75QeFueqQkGgPtyUhPfXuQwqn45fb5D1Sd6FQmEtGEqkYNKZyv",
   "UTR5tooFvaSKM2PcnVbGYeFLqcz6RpQqkmRNXghDn622cFVsCY8TF",
   "UTR5DJMVHyVkWatLy6sfH5u2sSY1tWgKxcrUCUXRx3rYvzT14dw89",
   "UTR5VP2rivnujUyiWYjaXKkWY1twCzrnki2X3vaFsPBSxTuHkL3hc",
   "UTR6Qgyuuabd5iL2MR2iQf1tXDLaNLbt34HF11RKpy6wwnhcbsCPt",
   "UTR5TinUf3xpS9UJAcqViASTdmMPgJmrqk27DdZDFmhtcGcPKpFA2",
   "UTR6y6NwgbWc4xqAFVtN2awgKqAPihkPY2NDobKSCfNVP2rdCMmLT",
   "UTR6764BdQ96PV6iaHytCA3awWkhu9Us45vhQioC6sScMJCJ1cJhs",
   "UTR6or2VW3fasCaSLaivhaMkYQgCQREHXcW9khgvbAr6xukVVwmH9",
   "UTR6owcuFs7oEsfNxdm7LBTn2zUkXgqxErLc1G3LdqNFacA5skm3s",
   "UTR8k4haciMEaziYKAu4NDmThDYg5TZ1agTv1iPfSGoauXcSzByDr",
   "UTR7htTprksKjXWyqS8LmfgN6FhrT3WCXeCZRxrnDKTmfEr72pQCh",
   "UTR8TthCZ91JegsFxzafrMUCdWaMBu35pufXvPcet7YwZm8sq3k4p",
   "UTR5WZyZbRE8UD5c3pEdyGirdG4Dc9BrdUyWExcTMns45j3Nptrt6",
   "UTR7FUdsaXPY7BGV3vXSyWM66F3EiXSguaSVgbQLsXFAUNSfQRd5N",
   "UTR5vdcHjiHR2rsMJSrMwDnudYGz8XE48cBNrrsHVPCpLdHXopBZZ",
   "UTR6CBq2WonMeq2HHzz61wL9UqewFLWW8sUb6iGhDkL6Jg5VFu5fw",
   "UTR6B12TynJzAMWDmGAzM4esFoU1jfmStMx8pCjyTujXiEULkmKDs",
   "UTR6L4fjVTE3R6u3ao9iqQTEPJYfKG3kD4uWDuUReimcGUkXE6AnP",
   "UTR714ijSQdnrZVE9gxH78EAFKGqEAfJUVeqkeroBXceQYiapeiJC",
   "UTR6pNkUjTvZsNXp97vy3HSLn92vAASDRxGknZCaJiYzk8uaJdcAP",
   ]
account_sk_list = [
   "5KkYKJbWHZ9zneyQMLPUZVFEeszCZmRCXNf5CbQQdPFL9FfsuED",
   "5JfSeQ3Jr8RyL7HThWsgctLmovDHGhBCnm7e6dQo3rV3TQ73oHB",
   "5KXoGjfRKb6qiJUR29jDPrt9DpUXPTMS1gpJpoSH8iN6axak89g",
   "5KSfov4mshaB2pqd74jT7FZFSRitpFnuoqw1jpy2WSB42BpyzD9",
   "5JYFkZtDzXM3xvvh8DaT6oYNx4KSpRNh16cb9ZGFin9KuUgaoW7",
   "5HryvWgTq235oBMcEpWZdbJKhzZvfz2s7iWWKcWdj1eV7trbe2e",
   "5KWwKASXddxnNupNpDpwh1Nh7ywfvdfEj7dRQah9AGP4FyNTeHp",
   "5JoRArdqnBE7jjebxadPmMrpcpdCpC1sNRGxqLFWUGwcWumCzB3",
   "5JUVwaEUbPTETDRDhTeh8pgWCwATZu5xoeqc6mrTBQykMH4kACL",
   "5J6SbAreMuLZbB1EBFUy3Pvi3xyYvRjr6Z5savF36AsefiLn6R9",
   "5J9aHWJ3EyUkfU4XseTwcAieojdc2s1oxZ8Lf3nP1xFVkMHPpbV",
   "5Jk1Hj5FmfQPTMxUcFJFBzKYx7fpSedmVyqGQ2Ei3EcRDJBCiHr",
   "5K8JR2wvpjPLb23WZpWWNyecT9F2a9pfDkRDKTaAzuikzyX6DXw",
   "5Kg6tZmENhe8t82Fz11DGGQXCDKgDuvRdAHnngYZqPWTvLYT8GJ",
   "5J2EX8ZtCtnz2XAZWuUasiH9dADdFKWUa1B7QaFk3vAXyNnefAE",
   "5JyGxXc2bigiqSTMZivQdfzyLe3MjdLpS1wL8hgsexcbq3KTfdB",
   "5KbZpdgHAAebHsRj7h4apzTvzoPg7Xj31VojXaWYGe4MNmVXAd9",
   "5Jr8qjNjNZcuZow1mQ47cPAvAuW1MB82AEYV6D1Z8UDMRa2W8u3",
   "5KFcvuPBvvbzrJUFouYx55HERh8pmgfXZLS5UinuNxuiFWyU4iP",
   "5JsBerwNiDpHXaG5hGYoPAgZHDFadWdFnYy43qZpTYfMp6RiajB",
   "5JkG7Krtm7VTUfTrBPKwAYzoMRouUB2tmYjHFrC2bbYTeTpGZhE",
   "5HsTpSmWRTzqeThfwpJ2JGAF5r11Hnc46UrgXYqHBFAJzbcbPFG",
   "5KTXaj3ZgXXGA1dfD24MoZp7CP77wqiRDUfbktSfhozV3BDsShG",
   "5KUb9kTZWLve8LSQk1N37P71Ms3yvfLm1snuhkKsyNEgHXNAUa2",
   "5KHiLCEXNePQhMLc48U66Z35oH1X35FeJBsFdCq3NDg9gQowQo4",
   "5JPHZELru6x798RCWt3R9gpWGFKRSiXQNQ6yXeqWEXPYJSccphm",
   "5Jfpeh1Sy3baWkvpX3qFCdjEwNXgXBnEtqUy2mf1G2ZRsD98txU",
   "5KkphB3QYYptJKsNJ1nKQ32nE7yKGFhpjEx4L2vN6EGLsJsbqMa",
   "5KMrxwZtPadKa7ZEXfTVdageh78weW5cFeihsJLGg3voBAXnar4",
   "5KdTNnPZ1hkfrKhyeZx8g88TyjSV7SFpCNoYeJBauFVq2VyknR6",
   "5JpeePh1w89LZXBpVkf6Y2Qjqud9J7t3gQ7s1NBKYk4ac8rFDHD",
   "5KZ6usQZWMWP2z5xs8Kc3nnmzEWYwt2wydAFvgsSqCuVfr8v6Ea",
   "5KfRqzHs1gQuN8VWKG2CyVrhEHLdvjQkd1PXcNwwTjdLKvUiU2Z",
   "5Khr4bb3HRmp8sRNRo2ujS5RrRiQ4Qmmq1iygf9Rtwp8kjmGz59",
   "5KL5FdLZmhVjBYXPxB53LjqTHmb4KFx8ZHTjhj2t3JNCU1CS7fX",
   "5J3oFNVCietaXa7DHyLWaX3JnVqHrMRUdrN246m7wNv4yt4avmX",
   "5JHqAFehkygfQGsGH8NXoXdpPiko5DyAkpPQwXdeiG5VZmKjbJW",
   "5JgXucPoJ1JUmx2u6CrcuV3erEjJ7dn8mDiAPkZcFwMQV7czHY3",
   "5HvaJrgnBRL6qCvRDTdsfa3ktrfVK6J5qKPoNiXMdkfoL5SeBLR",
   "5JGvuJPt2aRZEENwLpbqGeBxkC9HUoa1XR73U9Q5QCNZLmmdyrE",
   "5J8c2pMUp7gBArXDmsgUbw1aXYF9aVTm9wYaZTzDJBjqwXp7nS4",
   "5KJ99L1R4jdvXFLWMVW98DiWnBgnzckdjPcSyepMKBiW52aBnDV",
   "5Jmu4NGbVThJw8ouzwrjmtf9Zvw68jBnNLTTiDdNNcF8yTap2qe",
   "5KMFWctcKWdgiiDkeJRXKywY1V1mWicrphKuaP8AmdkammKm9Nb",
   "5HyNoJFk9HjcHN21zPJATpw7GsNQXTLY2pxRnLV6mvJkQpDDTTM",
   "5JRai9hVyGpmCEjfRhwAqRkyK6GTqpqrUdcWw2ciEPqeRQbvhK6",
   "5JkkVkp5FoqpuKjBAicshEEKvjfbKjQUDJkXfh4Sqzs1Ubgj61f",
   "5KhKeML8jzQTiukQF1vLD7STXyRgu6thZYE8nJfUTj917dy8u9c",
   "5JA8gNtMiqxmj9E7NHyVxMcMHhibydozaQcB5ZjSWu2xrog21uY",
   "5JxrRjcTzV5BA8e57FcL4pN99cbG5WadpmVWYqVrq3wj2ihrZQa",
   "5KRsvZd7quF8J6bNu2mW6hur2JMEjpVSw1acEWvEVQ79NKFVSwh",
   "5K4PWYX6Egt42RXv71WToW5vsLap1DZc8ugT5ji2Wvj6wX5is8Z",
   "5HqYDyQT4wG7C1mxQBgTrYxQHXr6hbzdSGWHUYyf4p7FFLgsWDC",
   "5JSAThNEManzymEucyrrTzQHW2jMsD9z4Di8mJYY6ubrmWn4pp1",
   "5KC3CigFvcxEbpeH2ibTe7aNytBnrbb7vd5qixG5qUR9LPSAYH8",
   "5JYBZ1hNvn7DVHo8uYJbBAof18bzaSwU53UVeXsfRbc9xFcR6mE",
   "5J4Qyjrm3uhDCUpbnzLvciM2MzE86Frf1emQ2Qk1rYprt2xBXEE",
   "5JgK99dG8peAobSPsjgGPzE7thwb67QeMUwZkQcvwYbhcwAzxW2",
   "5KHh5x6ew7o2r3Ye3AqRC72UYNvjAs5V1NthybHqiEEqVjYHVN6",
   "5KYuDXFkUJsD8nMyXWG98zkRE9F2GowCLShUHaCMwcwQdEfB315",
   "5KEj6hYiJUavqUdUu6azYnxThkwJYW6ntX7tD2d86e6vLMdJobX",
   "5JNNUAqnnuGAkpKXowGnV58qz2kAPXZtsTBp7esYc9DguiLXjfe",
   "5JvwAmujmuxDJ7CDzts6YXoc4owd5XXPpCBeffNFsNViiNZBJxg",
   "5KEwzSU23quossSDFpZbigQDGimxzn5efcoQLS3Yr5cFJoTvWFC",
   "5J6q3a3bP7JWseAAgfGTbvFMz4F4DUbX19r9G4t3paqkhdNsCBs",
   "5JsWQG8WjdsdJLfrG1D6a4iuomm9Yi2mc1i8UpccVeogbNzD3yD",
   "5KY3MfNj6zvmHbiqkxnAeLZ3EnFvnoxfgFaHks4wNdhk1JmzTSi",
   "5JKLDFzPRcSBKgbWzazBikcRd3iUzXoYDJVdctrAGtaJYfEc3Tf",
   "5JtV8QS9CQLzj4VYfR9epdDGtuJAV8Vjmh2LZmmpNwmEc2WiVeA",
   "5JeC9k35BgvjvGHxc4mny4xBWp5vsgXGpDMAPP5iG9SnCDe9uDr",
   "5JyLpB1k53WTJuDtRg2FJ5a1mBEhLx3TsbeY8hWgxvoaQUgSdBh",
   "5JPo6MLh46nERYXSCFcpKLGb1TpmHRAMRJaihnNiwZ2i1QPUEPP",
   "5KL231WnAmKcPwehGyTTZYi3xtWTLAaDKMmWF6UJfQ9Wa9LHMBR",
   "5JmV5dYAkZXhpqMn6XLTJMxMDrpb5pVKaApCQj2mhZL8iCisLtg",
   "5JbJtL498jSKi4Y7nJJNDGm7Z55pg94MHopDRxkQTCPTHvnPDw3",
   "5JnGWhpgGNvjCZfpxNtzB1mQrnovXB1xiBAWs2VtLmXaxc1kE6P",
   "5K53RocBWMZ1ehMZy1o1qgY7VroQQ4Fz9o39tWAedSgmN5heEWq",
   "5Jgx88YGQcV9fhYbSU8eePrDgW8FgKzTwTzgUGLkfDGmaKfKo3E",
   "5Ju95dNjSf72DiMY3nEEipypsuV9pcXbnLBSkbZYL2Qy1pFqGUP",
   "5Jubd9b1uUZXDV2wKXZxe4UQahbnXzFSphpMG2xUW6RSsVKH7FU",
   "5KCVjocALeXNcVwPZ4de8ZD84vdXwddjhxoFYLLh71n81qJBqAy",
   "5JSB1FrJGx77q7DDXWBmWYvGG8GsNsaHLwtctthDzPbWK6gfoar",
   "5JtMwHpKHDdL4yLH9b2DfwSSQYtmt3Ne8WgG4zxZyUmvbT7NTac",
   "5KM5df5g3oyrxW33MeF5rjiLTmsipNbrkxkYbELwfL52hYu5yMH",
   "5Jzj35G4Ned6iLBMyQfeAWzT9NbHQfHebKZ3ia1cbcB61ZGJe8e",
   "5HrKumZ6BqJPiGK83b2egTbvYrXuiGWstub1twf1kHmV9MNDmMJ",
   "5Jg5tZvqmeVq5zR7Vm5kzDnSo1bguubKRqcMaWkpYNEnMyMb1hr",
   "5K3gWPQEhyzsrw5r8Q9FPWvhsJyARpgvhTvWv5AAPLY3Ti8a8Hv",
   "5JydQ6TdTX6X9ni7smUAgNdHGUCAi3aaiNbdmvKW5oJFXtR3q9j",
   "5J1XBa58Bwyxp3sw4jt2XU6Jz7VFoyALXXBwrywYHyW76kRVeSy",
   "5K3pua514Q2mhiBevLcwbXukJyEG6QAwL4NPAycyxRirhEV4xLa",
   "5Jh8VLwqpZgjZBBbewU4PiqsT29mHrZJ8dK6TCVQgSBoP4twjpJ",
   "5JjkHdxAojf77F6UicxZEkFyr9JywBoVTy7tPsNsQDUxt5TkPKG",
   "5J2HsSQ2WNBRNDmEJquj8R34nc3yCMJXUjBxnvpGMjKhX9jar4U",
   "5K6zzF2eU2arab3ygYHaJMwvU31mXjHiShemV2smLig52pUFxxL",
   "5KMmkkg7pym19fuVEvWVegzqDxxWZcpnL7gx5PRnrQRSJuoyYNG",
   "5JsyQxyNsywSuxanctc8egmLTsB8uq8U34DTBHXrhMe9qQzQ3xT",
   "5JSqoetw6WJm9bTiqZaxMZTcQJ6HDC7n1C1wqjUgQu2svoeSoJd",
   "5KdW7DvTPu8BYbdjdvSVmXfQ1r67mGqVvKNVYbYEW6u69tEnkqT",
   "5JHMBid3AN1XFyTuBeHfDSgMKfAVRWeD64sxJx5GKFJQh1V2B5J",
   "5KNn5zXY8E7wBaqF5wHcmCHikm2n6zbVix3wrxC2AnSHgeqUPNz",
   "5K8ro796QpfszJWjGGkMrR3qxCxUi6GWm2Uy6fAJ1Nw6WtseTJP",
   "5KLUYwDavAhg8gyjQQqgDzSkbdhYV8xxa9nT4SGHiuCCKJWJQvw",
   ]

def jsonArg(a):
    return " '" + json.dumps(a) + "' "

def run(args):
    print('bios-boot-tutorial.py:', args)
    logFile.write(args + '\n')
    if subprocess.call(args, shell=True):
        print('bios-boot-tutorial.py: exiting because of error')
        sys.exit(1)

def simple_run(args):
    print('bios-boot-tutorial.py:', args)
    logFile.write(args + '\n')
    if subprocess.call(args, shell=True):
        print('bios-boot-tutorial.py: error')

def retry(args):
    while True:
        print('bios-boot-tutorial.py:', args)
        logFile.write(args + '\n')
        if subprocess.call(args, shell=True):
            print('*** Retry')
        else:
            break

def background(args):
    print('bios-boot-tutorial.py:', args)
    logFile.write(args + '\n')
    return subprocess.Popen(args, shell=True)

def sleep(t):
    print('sleep', t, '...')
    time.sleep(t)
    print('resume')

def importKeys():
    run(args.clultrain + 'wallet import --private-key ' + args.private_key)
    run(args.clultrain + 'wallet import --private-key ' + args.initacc_sk)
    for i in range(0, args.num_producers):
       run(args.clultrain + 'wallet import --private-key ' + account_sk_list[i])

def updateAuth(account, permission, parent, controller):
    run(args.clultrain + 'push action ultrainio updateauth' + jsonArg({
        'account': account,
        'permission': permission,
        'parent': parent,
        'auth': {
            'threshold': 1, 'keys': [], 'waits': [],
            'accounts': [{
                'weight': 1,
                'permission': {'actor': controller, 'permission': 'active'}
            }]
        }
    }) + '-p ' + account + '@' + permission)

def resign(account, controller):
    updateAuth(account, 'owner', '', controller)
    updateAuth(account, 'active', 'owner', controller)
    sleep(1)
    run(args.clultrain + 'get account ' + account)

def randomTransfer():
    subaccounts = accounts[1:3] #args.num_producers
    for i in subaccounts:
        for j in subaccounts:
            if i != j:
                simple_run(args.clultrain + 'transfer -f %s %s "0.%s UGAS" ' %(i, j, random.randint(1, 999)))
#    sleep(2)

def startWallet():
    run('rm -rf ' + os.path.abspath(args.wallet_dir))
    run('mkdir -p ' + os.path.abspath(args.wallet_dir))
    background(args.kultraind + ' --unlock-timeout %d --http-server-address 127.0.0.1:6666 --wallet-dir %s' % (unlockTimeout, os.path.abspath(args.wallet_dir)))
    sleep(1)
    run(args.clultrain + 'wallet create')

def stepKillAll():
    run('killall kultraind || true')
    sleep(1.5)

def stepStartWallet():
    startWallet()
    importKeys()

def createSystemAccounts():
    for a in systemAccounts:
        run(args.clultrain + 'create account -u ultrainio ' + a + ' ' + args.public_key)

def stepInstallSystemContracts():
    retry(args.clultrain + 'set contract utrio.token ' + args.contracts_dir + 'ultrainio.token/')
    retry(args.clultrain + 'set contract utrio.msig ' + args.contracts_dir + 'ultrainio.msig/')
    sleep(20)

def stepCreateTokens():
    retry(args.clultrain + 'push action utrio.token create \'["ultrainio", "1000000000.0000 UGAS"]\' -p utrio.token')
    retry(args.clultrain + 'push action utrio.token issue \'["ultrainio", "900000000.0000 UGAS", "memo"]\' -p ultrainio')
    sleep(15)

def stepSetSystemContract():
    retry(args.clultrain + 'set contract hello  ' + args.contracts_dir + 'hello/')
    retry(args.clultrain + 'set contract ultrainio ' + args.contracts_dir + 'ultrainio.system/')
    retry(args.clultrain + 'push action ultrainio setpriv' + jsonArg(['utrio.msig', 1]) + '-p ultrainio@active')
    sleep(15)

def stepCreateStakedAccounts():
    for i in range(0, args.num_producers):
        retry(args.clultrain + 'create account ultrainio %s %s ' % (accounts[i], account_pk_list[i]))
    sleep(10)
   #  for i in range(0, args.num_producers):
   #      retry(args.clultrain + 'system resourcelease ultrainio %s  1 100' % (accounts[i]))
   #      #retry(args.clultrain + 'system newaccount --transfer ultrainio %s %s --stake-net "1.0000 UGAS" --stake-cpu "1.0000 UGAS" --buy-ram "1000.000 UGAS" ' % (accounts[i], args.public_key))
   #  sleep(5)


def stepRegProducers():
    for i in range(1, args.num_producers):
        retry(args.clultrain + 'system regproducer %s %s https://%s.com 0 ' % (accounts[i], pk_list[i], accounts[i]))
    sleep(15)
    funds = 500000000 / args.num_producers / 2
    for i in range(1, args.num_producers):
        retry(args.clultrain + 'transfer ultrainio %s "%.4f UGAS"' % (accounts[i], 5000))
    sleep(20)
    for i in range(1, args.num_producers):
        retry(args.clultrain + 'system delegatecons utrio.stake %s  "%.4f UGAS" ' % (accounts[i], (funds*2)))
    sleep(20)
    run(args.clultrain + 'system listproducers')

def stpDelegateTestAcc():
    subaccounts = accounts[1:3]
    for testacc in subaccounts:
        retry(args.clultrain + 'system resourcelease ultrainio %s  4000 0' % testacc)


def stepCreateinitAccounts():
    for a in initialAccounts:
        retry(args.clultrain + ' create account ultrainio %s %s ' % (a, args.initacc_pk))
    sleep(10)
    for a in initialAccounts:
        retry(args.clultrain + 'transfer  ultrainio  %s  "%s UGAS" '  % (a,"100000000.0000"))

def stepResign():
    resign('ultrainio', 'utrio.null')
#    for a in accountsToResign:
#        resign(a, 'utrio.null')

def resourceTransaction(fromacc,recacc,value):
    retry(args.clultrain + 'system delegatebw  %s %s "%s UGAS"  "%s UGAS"'  % (fromacc,recacc,5000/value,5000/value))
    sleep(20)
    j = json.loads(requests.get("http://127.0.0.1:8888/v1/chain/get_account_info",data = json.dumps({"account_name":recacc})).text)
    assert j["cpu_weight"] ==5000/value*10000,'account:'+recacc+' cpu_weight:'+str(j["cpu_weight"])+'!='+str(5000/value*10000)
    assert j["net_weight"] ==5000/value*10000,'account:'+recacc+' net_weight:'+str(j["net_weight"])+'!='+str(5000/value*10000)
    retry(args.clultrain + 'system undelegatebw  %s  %s  "%s UGAS"  "%s UGAS" '  % (fromacc,recacc,50/value,60/value))
    sleep(20)
    j = json.loads(requests.get("http://127.0.0.1:8888/v1/chain/get_account_info",data = json.dumps({"account_name":recacc})).text)
    assert j["net_weight"] == 4950/value*10000,'undelegate account:'+recacc+' net_weight:'+str(j["net_weight"])+'!='+str(4950/value*10000)
    assert j["cpu_weight"] == 4940/value*10000,'undelegate account:'+recacc+' cpu_weight:'+str(j["cpu_weight"])+'!='+str(4940/value*10000)
    retry(args.clultrain + 'system buyram  %s  %s  "%s UGAS"  '  % (fromacc,recacc,50000/value))
    sleep(2)
    retry(args.clultrain + 'transfer  %s  %s  "%s UGAS" '  % (fromacc,recacc,20000/value))
    sleep(25)
    j = json.loads(requests.get("http://127.0.0.1:8888/v1/chain/get_account_info",data = json.dumps({"account_name":recacc})).text)
    ramjson = json.loads(requests.get("http://127.0.0.1:8888/v1/chain/get_table_records",data = json.dumps({"code":"ultrainio","scope":"ultrainio","table":"rammarket","json":"true","table_key":"","lower_bound":"","upper_bound":"","limit":10,"key_type":"","index_position":""})).text)   #Calculating ram ratio
    ramvalue = ramjson["rows"][0]["base"]["balance"].replace(" RAM","")
    sysvalue = ramjson["rows"][0]["quote"]["balance"].replace(" UGAS","")
    shouldbuyram = float(ramvalue)*50000/value/float(sysvalue)
    sellram_before = j["ram_quota"]
    assert j["ram_quota"] >= shouldbuyram,'buyram account:'+recacc+' RAM:'+str(j["ram_quota"])+'<'+str(shouldbuyram)
    core_liquid_balance = float(j["core_liquid_balance"].replace(" UGAS",""))
    assert core_liquid_balance == 20000/value,'transfer account:'+recacc+' balance:'+str(core_liquid_balance)+'!='+str(20000/value)
    #print(requests.get("http://127.0.0.1:8888/v1/chain/get_account_info",data = json.dumps({"account_name":recacc})).text)
    #retry(args.clultrain + 'get account  %s ' % (recacc))
    retry(args.clultrain + 'system sellram  %s  "1024 bytes" '  % (recacc))
    sleep(20)
    j = json.loads(requests.get("http://127.0.0.1:8888/v1/chain/get_account_info",data = json.dumps({"account_name":recacc})).text)
    assert (sellram_before-j["ram_quota"]) == 1024,'sellram account:'+recacc+' sell_ram:'+str(sellram_before-j["ram_quota"])+'!='+str(1024)
    retry(args.clultrain + 'set contract %s  %sultrainio.msig/' % (recacc,args.contracts_dir))
    sleep(20)
    j = json.loads(requests.get("http://127.0.0.1:8888/v1/chain/get_raw_code_and_abi",data = json.dumps({"account_name":recacc})).text)
    assert j["wasm"] != "",'set contract account:'+recacc+' failed ,wasm is null'

def stepResourceTransaction():
    resourceAccount = [
        "resacc11",
        "resacc22",
        "resacc33aaaa",
        "resacc44aaaa"
    ]
    retry(args.clultrain + 'system newaccount --transfer ultrainio %s %s --stake-net "0 UGAS" --stake-cpu "0 UGAS" --buy-ram "1.000 UGAS" ' % (resourceAccount[0], args.public_key))
    retry(args.clultrain + 'create account ultrainio %s %s ' % (resourceAccount[1], args.public_key))
    sleep(20)
    j = json.loads(requests.get("http://127.0.0.1:8888/v1/chain/get_account_info",data = json.dumps({"account_name":resourceAccount[0]})).text)
    assert j["ram_usage"] ==272,'system newaccount:'+resourceAccount[2]+' ramusage:'+str(j["ram_usage"])+'!=272'

    r = requests.get("http://127.0.0.1:8888/v1/chain/get_account_info",data = json.dumps({"account_name":resourceAccount[1]}))
    j = json.loads(r.text)
    assert j["ram_usage"] ==0,'create account:'+resourceAccount[2]+' ramusage:'+str(j["ram_usage"])+'!=0'
    resourceTransaction("ultrainio",resourceAccount[0],1)
    resourceTransaction("ultrainio",resourceAccount[1],1)
    retry(args.clultrain + 'system newaccount --transfer %s %s %s --stake-net "0 UGAS" --stake-cpu "0 UGAS" --buy-ram "1.000 UGAS" ' % (resourceAccount[0],resourceAccount[2], args.public_key))
    retry(args.clultrain + 'create account %s %s %s ' % (resourceAccount[1], resourceAccount[3], args.public_key))
    sleep(20)
    j = json.loads(requests.get("http://127.0.0.1:8888/v1/chain/get_account_info",data = json.dumps({"account_name":resourceAccount[2]})).text)
    assert j["ram_usage"] ==272,'system newaccount:'+resourceAccount[2]+' ramusage:'+str(j["ram_usage"])+'!=272'
    j = json.loads(requests.get("http://127.0.0.1:8888/v1/chain/get_account_info",data = json.dumps({"account_name":resourceAccount[3]})).text)
    assert j["ram_usage"] ==0,'create account:'+resourceAccount[3]+' ramusage:'+str(j["ram_usage"])+'!=0'
    resourceTransaction(resourceAccount[0],resourceAccount[2],5)
    resourceTransaction(resourceAccount[1], resourceAccount[3],5)

def stepTransfer():
    while True:
        randomTransfer()

# Command Line Arguments

parser = argparse.ArgumentParser()

commands = [
    ('k', 'kill',           stepKillAll,                True,    "Kill all nodultrain and kultraind processes"),
    ('w', 'wallet',         stepStartWallet,            True,    "Start kultraind, create wallet, fill with keys"),
#    ('b', 'boot',           stepStartBoot,              True,    "Start boot node"),
    ('s', 'sys',            createSystemAccounts,       True,    "Create system accounts (utrio.*)"),
    ('c', 'contracts',      stepInstallSystemContracts, True,    "Install system contracts (token, msig)"),
    ('t', 'tokens',         stepCreateTokens,           True,    "Create tokens"),
    ('S', 'sys-contract',   stepSetSystemContract,      True,    "Set system contract"),
    ('i', 'create-initacc', stepCreateinitAccounts,     True,    "create initial accounts"),
    ('T', 'stake',          stepCreateStakedAccounts,   True,    "Create staked accounts"),
    ('d', 'deletatetest',   stpDelegateTestAcc,         False,    "Conduct transfer test for *.111 and *.112 multi-agent resources"),
    ('P', 'reg-prod',       stepRegProducers,           True,    "Register producers"),
#    ('R', 'claim',          claimRewards,               True,    "Claim rewards"),
     ('q', 'resign',         stepResign,                 False,    "Resign utrio"),
#    ('m', 'msg-replace',    msigReplaceSystem,          False,   "Replace system contract using msig"),
    ('X', 'xfer',           stepTransfer,               False,   "Random transfer tokens (infinite loop)"),
#    ('l', 'log',            stepLog,                    True,    "Show tail of node's log"),
    ('R', 'resourcetrans',  stepResourceTransaction,    False,    "resource transaction")
]

parser.add_argument('--public-key', metavar='', help="ULTRAIN Public Key", default='UTR5t23dcRcnpXTTT7xFgbBkrJoEHvKuxz8FEjzbZrhkpkj2vmh8M', dest="public_key")
parser.add_argument('--private-Key', metavar='', help="ULTRAIN Private Key", default='5HvhChtH919sEgh5YjspCa1wgE7dKP61f7wVmTPsedw6enz6g7H', dest="private_key")
parser.add_argument('--initacc-pk', metavar='', help="ULTRAIN Public Key", default='UTR6XRzZpgATJaTtyeSKqGhZ6rH9yYn69f5fkLpjVx6y2mEv5iQTn', dest="initacc_pk")
parser.add_argument('--initacc-sk', metavar='', help="ULTRAIN Private Key", default='5KZ7mnSHiKN8VaJF7aYf3ymCRKyfr4NiTiqKC5KLxkyM56KdQEP', dest="initacc_sk")
parser.add_argument('--clultrain', metavar='', help="Clultrain command", default=defaultclu % '')
parser.add_argument('--kultraind', metavar='', help="Path to kultraind binary", default=defaultkul % '')
parser.add_argument('--contracts-dir', metavar='', help="Path to contracts directory", default=defaultcontracts_dir % '')
parser.add_argument('--genesis', metavar='', help="Path to genesis.json", default="./genesis.json")
parser.add_argument('--wallet-dir', metavar='', help="Path to wallet directory", default='./wallet/')
parser.add_argument('--log-path', metavar='', help="Path to log file", default='./output.log')
parser.add_argument('--symbol', metavar='', help="The utrio.system symbol", default='UGAS')
parser.add_argument('--num-producers', metavar='', help="Number of producers to register", type=int, default=6, dest="num_producers")
parser.add_argument('-a', '--all', action='store_true', help="Do everything marked with (*)")
parser.add_argument('-H', '--http-port', type=int, default=8000, metavar='', help='HTTP port for clultrain')
parser.add_argument('-p','--programpath', metavar='', help="set programpath params")
parser.add_argument('-m', '--masterchain', action='store_true', help="set current master chain")
for (flag, command, function, inAll, help) in commands:
    prefix = ''
    if inAll: prefix += '*'
    if prefix: help = '(' + prefix + ') ' + help
    if flag:
        parser.add_argument('-' + flag, '--' + command, action='store_true', help=help, dest=command)
    else:
        parser.add_argument('--' + command, action='store_true', help=help, dest=command)

args = parser.parse_args()
if args.programpath:
    args.clultrain = defaultclu % ('/'+args.programpath)
    args.kultraind = defaultkul % ('/'+args.programpath)
    args.contracts_dir = defaultcontracts_dir % ('/'+args.programpath)

print(args.clultrain)
print(args.kultraind)
print(args.contracts_dir)

adjustaccounts = ["genesis",]
if args.masterchain:
    for a in accounts[1:]:
        adjustaccounts.append("master"+a)
else:
    for a in accounts[1:]:
        adjustaccounts.append("user"+a)
accounts = adjustaccounts

#args.clultrain += '--url http://localhost:%d ' % args.http_port

logFile = open(args.log_path, 'a')

logFile.write('\n\n' + '*' * 80 + '\n\n\n')

haveCommand = False
for (flag, command, function, inAll, help) in commands:
    if getattr(args, command) or inAll and args.all:
        if function:
            haveCommand = True
            function()
if not haveCommand:
    print('bios-boot-tutorial.py: Tell me what to do. -a does almost everything. -h shows options.')
