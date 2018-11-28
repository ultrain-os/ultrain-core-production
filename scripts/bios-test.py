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
"user.111",
"user.112",
"user.113",
"user.114",
"user.115",
"user.121",
"user.122",
"user.123",
"user.124",
"user.125",
"user.131",
"user.132",
"user.133",
"user.134",
"user.135",
"user.141",
"user.142",
"user.143",
"user.144",
"user.145",
"user.151",
"user.152",
"user.153",
"user.154",
"user.155",
"user.211",
"user.212",
"user.213",
"user.214",
"user.215",
"user.221",
"user.222",
"user.223",
"user.224",
"user.225",
"user.231",
"user.232",
"user.233",
"user.234",
"user.235",
"user.241",
"user.242",
"user.243",
"user.244",
"user.245",
"user.251",
"user.252",
"user.253",
"user.254",
"user.255",
"user.311",
"user.312",
"user.313",
"user.314",
"user.315",
"user.321",
"user.322",
"user.323",
"user.324",
"user.325",
"user.331",
"user.332",
"user.333",
"user.334",
"user.335",
"user.341",
"user.342",
"user.343",
"user.344",
"user.345",
"user.351",
"user.352",
"user.353",
"user.354",
"user.355",
"user.411",
"user.412",
"user.413",
"user.414",
"user.415",
"user.421",
"user.422",
"user.423",
"user.424",
"user.425",
"user.431",
"user.432",
"user.433",
"user.434",
"user.435",
"user.441",
"user.442",
"user.443",
"user.444",
"user.445",
"user.451",
"user.452",
"user.453",
"user.454",
]

pk_list = [
    "9bb728c7e29e2f7a0c34c954382ea3ad8e4a93bf3455f9438afa086b3d67304b3b2a7dd684f0cda7d71ca9116128b1b7e49f31b2b10c0174b21b98c82161294da6d88bdbc92a897006e547c5ee5a3277f74fb4f9c0b9d8339fc3de8e6f467aac558859cfd0b9d3eea828e29b33b4f819f14ffbbcc7bea2ac585228fa8f12c42f",
    "149474beeef356cf558385aa40cc495312d40d821a1828a4bef44b542a91c60e8ad7bd88fe6293192c0f354b074d63413d95f157844e060df3b14224bedff2f2a2d901b96101b1840ecff5f4c63a67cc2acbae6943b395b0229d13fe1ffbb219c435e1fc978143fb03a5b46fa3c4c5f67b9fd9089b06b4c25cf15a699c2a1f7c",
    "8c1901520bee618f50db3c2a0ce0645e470b1551ee4a3beabd9ead29e301312915849b579c6edf3fc06eb309ab61e3a9257ec4161b181deacb019217ffb2972da72d967e0873f67f1c4434e5b11a3f96676b89f128b098f74b028226b49ba8c7dcce635ccaeb723b7e0d5e187024341f6227e6291570274b71982a9caab22f2c",
    "397b3b7c0e40f78df06deb35a5bfd9a8b39cd0e0c0c75a2dc4775c935b0dbbdb92590bd92965e309c013e642e13ca8619fc3a435d7f7322bcd8a6eb63793e4162d961b100681e45ac30767f76f50926331886d8a29ae7c0bcdbd1d6834c12783856c0b2a915536fb3d33213070869d2bff00a810a69f680e3c6071a4d0b9744d",
    "011fcc9c00fea448b506a2a770fe5ec4b3be78967bf05cc78cb94fdcf929862563ee0b01cd66d131f6d3a5d509b06fb95975e49f0539b401d8dff4a9876660ee22f387e02342c404112768e9fed8042591471baf4d657fc938df61b28711b7205af8ac9d44a9397306c724417d9c71956308140f3aea76a865048fa6e86b79b7",
    "2e9444984ea33e3ffd8fddc5b9f1bf512b18faf6b9a2f232eb33b36709baf5e92bb38c723ca333da2c571bdcbf81dffbdd07117dee4ebfda5251101c3fb1f559492576980c374546dd2776d1506466463f40338426a421872324b69ecd1cd849610607f76fc624840e805db991cda853047063733a181ddf07cb407f71525888",
    "4875ba243fcb67515719e1b6ee31655bed7319216c20f9b99a5111a4c8b03bf662c5040cc02fd9bd50836cec3331bdb7c1009aa14ac765b8d3111ebe3583674031bb2ea591059d36c1603a2fb204d494d871d9b972e5a1b4e8d2c31ee2db01a0dbf1932ccdd568fb752f1b0b9ad859f77f374f834246dc4e8755315b3d32b941",
    "878f302b3489b142a15e690c4f5d90093aa07e833836ee813209451ffa901a2efe3e527fe0bd62b37cc6583595c6813ea528ad0bc724cc8e92779e9674534623a696f849e01b93d5d1b0e10e3f223bc47dddaedd71cbaf7671cf7cedc1f9d923cec0eeb5bbb9b9d487ee0aa28c4ae2f1750aab4d48c5f6a8394df18a0c285129",
    "863f1289ad3161f6c153c0147431f5ea297d0eb202457d9167291e24f604ccda0d2331eb9926281eeb2b83db864b77ae2a34efe9b9c06656e63826fc0ed2b3aa109592f04856c3397649782556478bc31b1a3967454a2a6d4d136b9aaf4a1d5a32cc367e533ca212ac4797cb55c1747717105fdf5ac1bbc699a916109935a6b6",
    "4ed45f43df83717973a5523cc6036a8faf85b75504786e09beae6c4fbad000010093120def32d357f9ad6b5b7d1f4965529311c212a46180ec72b19578ef45b233aaa5a460db6ef9f0db352bcac20ac4dd18d4e28e58c4c11340af577071907b061435879fb51f8f65bf58530d847f2cb995fcf874384abecb75d5f8a88d90aa",
    "78f543ec2776cf95b7d3557d5bc2c56c74ddf7269184ddfa12ec810e3aedc180c5a7e6efd99cb99c0124498c8b41c2af7c94e67856935f4a8bc8e5f02b42b3993d54b1e44f261599e048780fc0c829dc30ddc9fcc3eed56ac5433d32e640575760da09dfd057983c12e4f4a59445185017206fa5303e88c4250fa7a76abb417f",
    "2eea4202c9f3d728c6e5369124fa3761b556699ed83bae8349e4df4fe1fb7f64d243c2c3e834ddbe9168b165ab3a498afc0f81da3f027ac5ff400f13b341879fa741c0b764240e738d5fb71e0e329b645a84e0e289e271d933e93b9677ec47b6cd957c6e9eef3620565bfaf484fd9b1276a68cbb26112723da7809a95e1f922d",
    "8d11750f9ca9b5f6d4832216147959657ab80534b8c1857f5b1a80c0d660e9759e26ae3ae1f6d21e0623c75886f634148f2e7cd77fe457adc24d67a837490f499cf34ccbb6a845e09558f4803b6c66c786aa4cc9dfe88291377d3bead9ede55a7498f42ad68b233beab26b9a0ce7785b4dbbb9dfd74dcb159a729dd43e6296d3",
    "3a86a149698516ce7426c676df16732d1e1e2a5dbe243066c41e831b2df64c6feeb6c3c1428378e1ce80a6bf51d2f794676769d26ab9a9a1adee95e74c727f378557951a2bc67914ea10034daf0a2d227e30c9f95efcbec0eb0ee3e591b5790bd7101aff6fa94e08b97d334cb4357795d3d1412f10138c1978305b067809d281",
    "a4ce5f5ca518f4f7139493b2b3767cf43e1205e60e4ceffdd2d90d34769c39beea4e3288dcd62219498cd724276624f481763949c244614c0abf8a24f29834c1a2b90e5b462cee675b0adc918f4e2de2d6f430acefcf4fd3d5c52edd2a260c17e59a6c149b7ded311cdfa766f48aa1754cc3bca83f86a9b75653f26664d9c505",
    "03c54da445e16d110e7b7566dc6112daf0e819fa083c372e95c23c59c7326295d58a29bce3d0d65a1f5f6ce18f5bdcd881cd299ed29b13cc49bf176e80cf2d50400c2a3c1ea9ac75934d323345defbfe3eca4c61ede641160b833c33f72b321fc3abba3d1b1c4a084092d97011fe69098690f2e65211dba554fd9e513cb19f71",
    "869e4585613adc0c623d82d932a9191717f485a6a1f3fa22c24bdf65bca65e5681d811295193b442bfe311bbd247cb5657154a75d459a9fb32822a4eeb5b92f94d68f9e169050b7554cf6dbc4e397a7bf3bcdc8fac70179f9884563c0b32619f65e71c1c9c3bfda48392f6a8097beb5b29bf025e6d589678434612b30e86353d",
    "83c5052168e2f3153592d6d93a479f5eb928c9edd40b04aba03d54beb3479bceaabb14923f82c730ee265b2dd05d41de018482282b52a0bc4401487c0c2976de289ad45b0905360960868b40547d2d2dc3dee9da1b0a157eb29834b6a5fb658f97d4294a74d7a6f0694a686429f0a6eaa648f6f691867da01a0ab33bed46fdec",
    "1aefb75a3c29b4c962c6c0eb16d326b7c3dde8e5e926f1d49d8c188520609a60749cb091278d0da172af52dfb2126f9aea98ec964dee184849f6e6e1a8faf9d10cc114f15df160a820c79f3e6983b4ebb02cc2e49f69e968aab10e7a44519fc94f2b814562d1a53453828ddc52fdb0c42481cc628eeee5b574e5afe045f32331",
    "5a199c914810669f885b7e8860159a9e42e1e14f3c1a8b076f772519052d1afa46b3e06b8036aac126d910a33fbaf6b274775c04ae4a167c4532a8c82bb1843e73122b46e3367f3ffb83d4f749fb462c7bb5ddb42b584d1577ba4f6f933214c154361f882f96b03b7d2678b6c74435d81c030c023f7f52d49fcf103cd6950cf7",
    "6dd91712795579de8cd869ecb80ad83c3aa3b4ffec164903ed5504a8245b0552cae45e3dc16380b9fbb3a45392b40be7f1d193cf6da1c4939480dde734be04123932975a1c1327c986f3ec1636e7b379ec1636916cc34167f03e35c3b757e7e8b7e0afc3c2e4763196175c79da1859e83f30e856e9a135f351e56ed5d289f7a8",
    "01d1e3feda403ab35bddf6c1383a57e5a3977de15e40a3174a0da6a98bde3caeb4de2df2f4bb27447e51ace7a001ffa85b272fc287a2c50e0f3ecd3377cb4f71a55ee76dfcbc7e54bf3f7137ffcfba11dd5d55d63fbac16c94c917a31914af40d7e8280ce359cb3b0adf400d28174fd071a816f3837f4cb4a35e37eac53a6520",
    "8f4e8e5e174a7aff8645b8e46a2781fc8fcf91dd0977bc345dd9e375c6afd53d88f97e0f723b0c13f51b9916a905ba96ab01693b7f4e469abbff09b49e1f38450b15c30174469593565b242856e92fb8b5df79ab91b2fbca623b96b6fa954f8742ed7c64dea421dcc310a4e7ade77f0c8fbf64a888d45956fc601bebaf39eac6",
    "23d78ab74a69b2751a9bd998f44565cbcb1b480670cb5fc7185f7950efc263e51e3244a9c22086c01a26d2313e557fd79b3f664494bceaaf7f084fcbdeeb99aa52709973dbbaf60411f454b3a8fbafcd3078e2b5396dd70e4eb9637d61f3fba2e386e0dfd2e177186b8c5c3f4470eda592a30e1a22cf97bf06c4ea5bdeaf0503",
    "400ca3ffba725df793343b115615909f16d9539f7b200ed64f3617f3137c113f753b3b2b9c1fe525ee91e4718fea8f6bdea0fafae026ac04a52cb0bd6de39bfe2fd8a2583e48acc84d55878427ee861f5eea34b294b653de87d11ba6e7cb9f1880115b8ec8126ef97b31da8f24cc85b25f7b8ac9b7f4008b1217a719b183f68a",
    "468f1daadf925e3137aced5af6924b896e50818e6b14261e77e672f231067217b04a06d5e5f2c7d3289c8ffe7a54226d75a74db9cdd5cc9f713210f9bd5fb7ae1d67caf8f9c1cb4edcea7ba6025afa17d25b34afb2555fd3b7732af95ac5a2b40493117682cc806c151c8d815f441e5d2979715d675b2f7953468394b706bf15",
    "5bf40969c546e6cfdc425871206da2a8dbd578d7445f56f697947c1f9a9b27320af0e2d9d92edfa54d1b66c9731c9d06f480ea59409a6b34e97d688bba9e98809f907e7bbc3c7d32447fcfebaa2873ea435dc4b508d78f73d6ac77d18f905ecb4132c3c72fb64e8e80333d292fc9da6bd12b20ad5e11a504fa624be6a2673278",
    "2259e121445f9e3caed65169a8719edec2a9acbce5ecc2263bbb5ecc99c52dc81f9759e48e97f9b29cb6722d301ce149689b7b3a26ea63683d3484d86346249f1c22438c9e43d1a969151b5f3e8f2f44f901f619292a1de48a39397c727788308b6fae10bc696832a5c2d1a1751b270fa49963273fa12b039d7f1013a86dbb60",
    "2384314bd9300e1afc5ef45a39cdebda069141668d603af34db95539014c446b0376890ef86640fcf1ac78a3d1922707f90522c51b61633dca8673f2c309559d391d8f4d0bdceb06d81d85a40b9bbb975dae1b3804c2b9101ba172cbc2f9da4a8975574b9e0f49c993feac091ad6f85e0e22505d57120abe57a52b5fea829837",
    "2e616685ab48d43bc10660318ef2db3ba9028df2930cda70e8c1449a6ac6b41acd0c2012cc550f792b0715b68319bef3b563808ebfdedc8a045f479a3a90b543a025ba1f0952d057fb24968ac6125f6d29cb2bc31eca149d285025969d2e6733655fa2c8557654ed24328777b747f9a38e7933c731f4198727d6ae0788767890",
    "88c95109f15991f39ea44d5a2d973b1aa978de5939b29c63d027f394f4c9bb5ba00a38f52897892e618c94cd9a01c749aa91bde7f79067c9e4fd42d4e788304d6d45ea12f1942731ae174825a0ebd15216ca77ce9c705ce88ff5c47205d5a023045608e9e4533f1edad2082b9f33c1d7becabe6af630406d0c153a49abaf8364",
    "6adceebbdfa85e59c494663b4f6b8dd04f9e48a452c17fa56d72b2cb1c5563191cb3df06e87ae6cecb61654a6c64bf6fa31e7d58a55fe2515317023584ded17787fc14fa3f57f9a28cce464803cb92ae6ff54c4b7df0cd78ea445fe057fbdfea2eac5099a66072794926a8e1e0ac471fd190c01876aba470e15f8c3ada9461cd",
    "733231275ed65392106b27cbb22fe7b4c65f51afca2f03408657beb2377501ad4c6d266a74eaa103557aa0a1f9aaa0ca3ba295f9bd7fd1806ed71db80d920db473baea67ff218d2bbb88095d0f346234b1cbf70a0d97bc7620196c525d7fa28c79aa3c8b095a32ceed910a745578f5c6c57ff6639329dca46a3ead35264a1d1f",
    "22e217f4a57feb850f9ca53b227cd52e31202528d10569a1d86bf76e2bf6125ca7d95b9ca8145926342c59010cefa352686687a7fe6dc5031b0c405a206932fa255a452534c47e5d7505e839b3c63d3f4eee7408270b202a9320af2f4446faf56fa035a391c7225fd7b5dc7f1094f43af42459f848be118b336bba39eb9323af",
    "600257463c653937286fe14ec6e8f96ac9fe121806328a6e323bd45413b25310e1263617edfe1e5c9809833ef31035340b700a4021640b72e687f3f5f99a9e0c82f12ea658e78016a56eff14763e3733dc9167fe85bf027ec32d455180bc5205ae36ca2d865def506f1c7533bf6ab7b37783d54b05ab7d537251b76e69607b53",
    "1cde7e7fbf0f4478155257d22b811a1336bf839931cbd840183344c8f17ad454a0bbf082aa1cf5848cfe64717e2eeecb707cfa05e4d72b67933dc9f43a37d3e294974af57c5aaf125b647f862a1e6612e6f2d7f57796b9ee526b12cce1710ead6a3e415af997e2fa8621205dea3b1cde31ba2645f8e699dd6aeb096b663e7fbf",
    "8c4e5318c70ebed58df61784572da994d981cce591c3048da3e91b554a67fea89d97b4dda6e944e5402efec92963a750a5e09fc65a413134e8b517cf40a879cb140e694fe94800f83726d12b61a4ad5f35ca539553866ac48d983d56e04c00888f5849cd1abe1fd4556b2936a6d2769d52a2f8a62b9efc04544367a8bb55a923",
    "46a5cb1428d3015e2f9d8fe6033aaba4a9813a94f1eec5f071ed3a9ec18b3aa7435756e8e0be64ad83bff36bc5103552ac1d0e4d994d06490acea11bab61cd621d10e95ba7cb23def66753b332984bd06a76c898a55e7664775fc26c0ab9834de14adc86c0061a4d157f544a0ecd59cd0db1199be6377300023b0652169769fb",
    "3e2210d3d44fc5e8ed18f3874faa56df6143bb9f89152602a7c08b2309cd8732d0aec7c64a8f5a7199ac5917a1f3afdd0cbe0b7ea9a24ceb2597b60f945188b371a0aeb5f48275c888aaea4ffeb8251e3d748aacba49445132599f86d552512942fddc4f7addeb5395d54c7558d1072905b0879d8abe1253c0fb4d1c63dabc56",
    "56c9140f21b6e8b0867ca161de3e93f7846db4dfc0f3f2a797fde18a678e9b1739d2026c8894ec157f798f22d275590ffbbe44a4399010ff8458ee85373724d56562f44e9da20c4cae0dd6f558daddf0c57dfa182cd45bbef76d347a7f14c74e98c13529ca0f82c95d64bfeeb42e0b1311239c95dc8094591603ae497ea5e531",
    "6096ac87a069ac4367b47e435c46b4dc55817cf82bb48a3d8c451ce0cfcc09a4d505d9136e2a0b8ea7839ed4dbfdefd913b8b450d0f8d146a7646d6af5dea7c196ded4f0167bf6ad2d2682512042e57d8a6ee1d61d6d65cce66e010529312feeee2cc5d2d297950dbd5a18a4c700971b372baea3c453aab80f5c294cd1f03876",
    "30c3b78205018e0a8014c330b887f408666e5c1a71d5b67566dc0fa8420785cdaa53a9b4a122bf463362e1297ea3a956b119ceefaa94f50531ab337372fbd4d025104eec0bf20c51542a58e43f75a9253b69cf19c60cd82afd601f64e56190990fc6b29ae9d91009c13b6b3b4b30ecc8498b87e1a2e437907b3b858c2bd7dd9d",
    "012b8e5b9ba80c2bfdd491537525e0ccf1b7efd0f9b3aff985368760e166245e0b287424b9c735e4eb50faa631f480b737cf66055f1ab678b7a814f0a1846eb62164f9803fb8394e8cdad9355e8e166c220f6c95443d83154f00075be818ad8634ff2ec79cab5be183bc3d8c271ed541ab7687322cb0e8e4e131675790dd5854",
    "288533547c670d16a60c4d04b7b35fcd3a5739e0d14743726ac997bf807956df58fda45f92bef52bb3b45bc078a7d0bec67bb7c5e448d05854193519314880be2d2b6572836d6afe2d1cf8e8f5382dcc1b48ce742c49369f583132e476354bd5b357abd82267ba45114540aad4874d62a21c865b7d7f07b9e751b86445d1de48",
    "a41500906f132ca6fab07ee0a5716014bfdd869823b6ae8cb829469cfb0d7f7e090809a862a72cc0694430865a55ade503c3be0e1081889e49630b056ea707125c03703ba3a9ae2c54ab57ae06995abc5c2ee5d54395f17ec6b30b81fa0e2ca87b575a4e1f4cf9d290a8d4a1b5cf988f15497f69fb814384b1a3cd83883be154",
    "75c5547c8f407dbca589f7f925cec946a8c28c03ed0f52a829c82c1f0b24fd923d73cbee20e8d1873adef36156af1f5203b00c04113a854f789f64a975d4003e22a91ef42ab0f5ac3923894e34c0fbf9cffbc282922a7aefb4b854072ecb0145cf11337bbf7f93cb9806364c5f6c57000cfe7f6afbdf70d1ed0aeeee69157c9c",
    "18dc4039ac8bcedb8e0e2f477f2eb65ffa195cc02560c0fe7be1b698631ec75c73dda44174c5272fe8ed22a80e7afa8b758be1c7a65325bca6e2aa9f514d69463da55eedd08b9b673d3fefa7df22c86b5343e863afca0c6638bff733e1c801259b87f2e5476eaa7bf26fc2a4d2eaa4cd2a00c0f8c19b351312f196e92564a246",
    "1b9445298eb9c0b396eea5a7cfc5a8666945394cb08ad454f68ec8079c9b8c11ea5c8b0ba9a5063de8eb4bfcd22101abcb547e058ecc23817f119ff0b9ace73313f19be13a17d8f0ee7720dae9354ee44929272fcb0c90b7283b60692ab0fb7bab03c78bd77cb8dda45b469bb698eb36431d81fc63dbfcaaf705343ef86280b6",
    "5602c6bcfbe67e3008f9d5afaa49e591edf9d756cb80d5120e7a9070bbeb341c9b77d5500eee8eb71e5a449a7c2143a93a4f8c35024810293985f8b247b9c0e1899b04b4635e2d13144b1d1d717535152e2298f71463642ca9ab14fe2bf90d362db62c9c6dc8f5edcb52572a1972ed57d7c64415dcad3f5163c851e1b47d2a67",
    "4b5604d6d3c4a50673aac268857ec32662852910143f5acc696e7caef73d6f6447f0d00a0f8b0f2f3d70d94a30d5d94032e48c65c98b1c139cc8f274188a940626cbea55d3033f791244880ce7c34fbabe451b238d88205bb94a3afe85687b587148169cfd18637a08bc71e0227995a2a6ba7b89db32f27a85e73c3731d8ea07",
    "64d81246988ddff17ae319dca0b7f4f89736b436dd3d77dbfb6f3b16b8f0c6cedae443238e189d765a7697a3cd98014415c0083730d8a83480e119c819d41d12477da036613dffaf1f175ea00d7afc78087fdd8dd4b1c371d071e5c8f37127381bc82295a74be04b5406e87b85001c48b1cf95e26996d40733da01677fe566c4",
    "163bfa40aa83e7b277c45fb07ffc3409438675326d582adbd9a903f89e8c4d1570b2dd1e4212ab35e7dc9716569725b73d7a6fc240b1d0a03fac24fea72e6e741a5629a3601734d197b52999b23d37740d8e2e77ac266510719893c2417cef9868d09550d77ac8cd7a2927067fd70c0618265c911706b8ff3be54713b0be9005",
    "2c8bc2c468eba05f824cf3a32de1b9ce0049799fe580d6ddfec41a530ed8fd1446916062b321479be3fd46f8acdd3744c7eb36ed29b706782ba0c40234f6c8cb6e7a342710789c56d0f62af3bc9818443c4615b3c3e36708a68eda37f4cf5f672b14a30fee8d242f29a0d779b60416677e0de5c61dbfd8ace2f26d2845f19a09",
    "6cce2daa5d177deaf952265265029e69d22037fd13c2bfa194e6d0a0ed5864441e924b69db201f1a75e83bbf72476e27ef65fa82255a03522c6e9951481d349325361a0ebe3e0c866742fd133074e1f1a87394f1a155e0a94d364024a63f840de809956aa943f0cb9fd12ef316c04e8ae73b4b02425721d303bc978efeaebd5c",
    "9c75511970fe635e1e578c6fda0b4eb0943d5bf4d931ca6884d15819029a85fb3890324453cdc3513d67f53ed60016cbe7d9ef383896f08846d265d6605ad2f7054fd99fb8ce5ef7c83ca6fa4dfa5f81c8150adbfed6781e2b5f1f653b83bbf388f834af3da42134ce38c31bcf383aaeafdd3dd661736401db92e5527acded49",
    "97e009dfc26ad0d0ce63f0d607b4b14afb3e2794f12a6042246eb8d9294a7d33b02d0071a79e521ea7539e5f05f6e2b7471b3be8be8047f6ba29b17e1e9dbb082cd08345b3123722452e039a66ba87c2c9f8a80e386e90c42d0346672a8aa5f4383799b9e06f8d492c514e2e867676d4e3f6329f9ab23fa93da56fb5ca19c74b",
    "138e09d390fa55a84af27eaf5c2667dab82d43c218754350369f427a77ec024c3171983dc19bb71b70dee90444f50bbf606560668ac97098cd478621d7bb2e814120f4c7d7f71d915785f15d09df7716c13340d0e493d45a603c67904f2dff8ae10e4a892e8ebf14b5a55b4f919063ea01bdf2485f2d2b9dbb75c5174de62b89",
    "a5de9c27531ac8da5452ed2f3ab781ada04ceb36fcb07fd289acf62c65ad017db607881a582503c7daf7ff5b62600594d86a718cae3685b698c6ae8adc909aa24eff392c2b29ab12d5b712c216aae45b7648235f7246becaf2f63867a05fda03be75c99f1eb091032292e33fd1c3607c42693ba5d8ee7aa798f7497522e0b1af",
    "42188322fbeaf4aca864362bb28ec4279aa3ea5a89a7ee352c6ec2a26b153b5d9a925a84bf6c859409e7acd2a2928da63d8f01790a96ae28546f4885821bfda65c281d46ea70df370a20961de1378b36849890188a3d7657b7fdcfb441c5996cdd7e3ca3536ce88132b0c75b7ef23cb0156c982846b7b2dcceaee7a0e88e8e2a",
    "108463b5e4b95b79687a42a81afc833024f341c03b872934fcf8c94559343ae076da013af15d417b0a0de52f2259a3887049d464c852d3b6130dce7414cc80d9187f7e6fedb3ecf7fecc60a6e3c11a78c62cffac05960217eb6512aeeec812f621e726cfdef0be4fb410c1cfbcac1d26b3d2c80324c7fb7eaead74e045148b6a",
    "1d48b15a675b75f7f68f156f3f4c4046baa1de838b3e6346c56117eabed645a7634b651e89d2105932e565f41140918be2cd2304e9ac9ab45d22854a9771193c575d784356fb9564f7594f734d92f214ce193d5b06f882185c6edadd020ebd1ee4c354fb4463d12d34989b19565c01ba46a8a50694cbe59a251acbb31d6e048f",
    "41fdc4692dbe4000e10fb298179501c9c797d9751e9d50ffc2f6b73e9c85e127d77d7ac06db634e7a3fc609077c0ac1b4ae21608dfbe5461003cdf6982fb0907654f5c054f00a2a4b1917ce41a5fa7098c57eaa6bed1984b23e7d1d87adbe8cfcdf04c27e20ad54b286690c3ee949d09b6a9112f1e37a8b83434f95243eefdfd",
    "8f1c2610e0543db87af750c521b53af64439b7d8c40ceafdbdb4da394d37ffeee4f9622466d77e05bf99066b155358cd86f2e6a7c99191f9343188d67b619aca908622c937db4b2847a6ae27d86864f42c00c15dd2b393f6bb6ec2398ec7889333162b73ce9c5c04cd502f80fdd64f45be0d9cf7cf14a5bf4299c669047661ff",
    "7e67ddfc975ffaaab8885532b33080ecf5330485ddeb484f70beee90d5a2d04619b1107b07f9068276ceb20f76fe9bab82fc4432267aaddee04bdd9fa7f1e6a2a1fb3c20d5ce23fb297aaa1134f1434515db4d4e50b0d98ac01f0b1fd23ced2f2780115f96b10d043eca372e61ce74a1199c069f8d84af2dec09328a9ada14f1",
    "33e8861deeff87cf5a349b5fc01f307d57eef9c9c433b021dd8ecfbfe7253897e389c10c603e987bc4d6d6cbe82d19ab43f249bc71902417f1ead2052596b143397c62e9e5e51acc8cd776db55b2faa6d73d4ee7cf9bb7008b115f427f427a1f5824d883dd895ab03d154be58a9c6a008ff85c0c9448b7445d7ca1feaa643aa7",
    "6ba5e256ec2e34acfea2cdd227eb948a60ff3bf124007191f9e2ef7459b8968e049306e577edb6d7f3b5c405194db24464607900e540768e2ce1f9ea863415783c44352177efd9e386a6e3c332b8708cbe6a83ebb32fdad6d89881b0f3c4bcb80ccef56e2f43caba2a965b51dfbd2242ab7dd2d6610452659da07880cc12995d",
    "4a808f4364e5bf55ec79cafa7a84820a4534d032f8c7e988568fd136c3ef761a6ae8f614defd4aa09c625dff207aad4860fad9d40e40c1ebfbd980de208bf81829fab9c768f7399a36bacf3be29a33893f38442cf2b0da0c98db23d1eb96e610277eddf5cc7560930ce0cc336ac232cbdea988ceb8912036fd849fd86ffb46b0",
    "06f6cc0248b6daeb86b55b4a69ac49b222c3d902b811b0751cd76b80b6f8ce865da19c4d85a694f2d245c77a9b98c24331e445c595f1020827095986ed8fe46789c438c8d5715ecd288d08c65c7c6914459c75895ac8cf8624c58d2f006d9bcaa2d32a3341441dc1402f7bbb1b6f19d38c4974039fc4af54ee30453b30f4a2c4",
    "62aa83cfe5704cee16a0e79439ebe14ad174fab53f84bd77ae03fe2c8fea4d14de6463d3ca7f747561a411bec7eda08261ab12b4be062a0c14564e8d7da6aa959da94fe8f19e21c45e2e177b5b088d3a4822c32148f918c1280b4a730772a7511609429aaa733569fcbabe752ae8fc4a5a588e0b84182802eeea39911732af4f",
    "82ebdf019299c20ef61df4f4d46c968ccefb376aa78af4ab512029fb418020530ff3150b21df1ab966b4129b92febff84ea9ab9105b74fe28790c8e7f4ff2ac705caba64c029b4f240be4118e8b9dd3121aa4416f06930981751c15ab177f2dc5cbe07e1814e309df85ec7ee1a0c40697488eedc0c851fbd7d04989e7a42f0aa",
    "4c1fb89f3726bcce38003e63bf663987cd3e7739482618b31490d2ac18cce96e8a983ffec4a55852a6573fd9a84ee9dc31a9bdc9f85e0b57bb8308452657086f3f4c86654394763e793e4e03f12a17c3a6ec9a596fdbb349d75781316c6d260b89e9f99ddef1f688103f0adda0aa0e74870b571c43f6e9145afd65e4cf1f04e3",
    "a106838c78f9682889cf7076b9d4800b22818fee28fd0b17734043b9f46db76d182b4a1252f1e5992d1bef1beb7df6f20f527dff602343306cd91c03692aea903d5b62a4a075dc61eb0fe877948932f914c3af30b248e84d0a2c148fd1539ef8bb5c61fac65798bcae1142a22ad0fd9555bf9f426cb86415e4c41ebbed892f5e",
    "7c315d9dac29a6bc63b36ef5db9e0eeed98b1ebe3669d49df19d5115bfdbd0ebb48e82f6ec0c7816b1d8817c0b158e5220a0beaedcc6ba2c9a13e8c52785a25f834256b91489ea23fad9ef6420e8241b96a299d0517b146d1e908eff57726b256d2d93e6c10fc28bf34ef015d58a15cb4a8860e0f1ec106fc1852c56d8e28eba",
    "6d3b678e5f8f5e9946305fc36cc2acaa71fd8438a74ac7a3a39333c18fbc3a358b080c52b2b82ea5259f3efd2fe5e192babfad1a778b231baf58e4aebd5eb6d0674cf28734bf1c6c17894d4827711b0ccaa20412c1a39b429c7bd7e9048c3bc7c25aab51ec33571d21e2df48f063916f60ccb7d99c9e470f7c83798f1a14868c",
    "83499e9d039c3c4e84edd55d42c7b7c5ff95b07bad6f100d248e147c552654fdc11a18fcb8bc0e50aadd57c2f6bcbaaeae4638cf285f8d8a73097407cfd462d319d1cf517cf86045914f029092aefc92c27b053305d3fca21046e4be2e8801368fff72daaa204eaf0a21075586aa08909df31edd4e1a4b4355473bc155d1a24a",
    "a35aa93969ad08fecef5a0c9749260d65a3e56f86d4421001b4dc90a9b2ddaa4dbddcf30ba7a83e5e88350675ff590dc8927f7f594d191fc7bd541409e4ea2cc7f7465eac3fef9e70313c269a8fcdb2a2318653dbfd08fbbbc63f2cfa7100e1d491d91be5088dad810f170ef19c15d353c1ae92d94554b842723880b5f886f4a",
    "461ebe7cff61ca2650038469202e249eaaae224e988ab623c8045d66e338ec8de502a2b97ff83997c63333345849dc2dd750054a9248d56936b1ff7af947b8ae1f1510d2f7bd2f9234cad1c524477bdae655f4ae8149df3e5f76a159b0cc4e2b6db947491ec64652803a8751628414de16abad0030a9c4c4506fd8f09ba327ff",
    "182c1f5a1c952164d22d1a4f71656ab7db06b735382cf8a61c4b5836accd8156fc675cf6a36c55e7ddfa0daebc9a495f0614e0660be92c85197d2cf6f302ca12311dbfcf4a1dd46632dd4d04991ece9c6c3be32e87cc3d5b8166a19a36c6ed2143fbb94096bf36c10564305bc25fb21beed8c30835a611695c6e7bf5b4cad93b",
    "2dfaec650e44e86cf1901eccdd28211b730cb88a69cb81c422636c0ab3ed80377c972dba2b9ae915d3b290ca2842fb91af76d0ea79b2cd1849e24b022fed7b0571b4231cef379e05ef2985b716700f8bc78881452a8f265eb210fabea2a39dd865781dc1fbfe03d879dcd40167922f438ca2c7584a146e35bd4c9556b0bf99c4",
    "398581e53663c02591dd6cfb5ea63cd13392814519d8cf63bad2123a1148597462ae6b9649a284ba67c33f1555b0dbe56acfa2f1ee85afa38e7536aa70f582a85d2e3eeee07616e23685aaa71d5d73fe1b5215e6b10342badc3ca8f39440d8c2fe96e280c553d08852b64efeea4ce0fdf4769e80ad2134877947fcd71536484b",
    "01ecac12389fc9532e971640fec163309612558234a007828c824a190cb06f8b62248cc9b5d2e514bb9cc6afb6105abd508a9c66b249a434e6af576c085eec2c7ee159cc7cdad299180ce4b0ed2c43725b2c387979cad30dc8e635bf8066ff26cfb7944006a684f798b2ddf86220b931ecf84b4472af3ccd4367dcdce7878392",
    "0b75a4b6728ba770f0f291a843dd0da27a2f0444e37c122df6eb15271b8efe86aefd186e2afe120e4344097152db8eb91325e857f14e30ddd0a9dead91a254189d13b81aa29fadcf4163a0bc43d929b57ecad05243951c12542ccc6500a87d59d33b29c4af8a88ecb91fb0f4e61a64755aeefc5bcacfccbf0b385bdee120c37f",
    "6e2131590e1d2cf32b0610e80fe100ac7f7728bf723a9936e9dbcc73c40f68ee98c058e61a50edbc72539accbc51c7e4ac3bdc1c1285b03e8ce73c55a488353629703b8e53f4494d453b71ddd1c82290122ccce9780421165362187ab52092489c86c309cfb57a70ae7e59626c3f53052f43770bf36aac4b5747927831ca77e0",
    "8b0e4feb178de92ad610f81387a585183af0cc7ef19b2e8cec1c155c5fc545e31b7a131a0516bafd84563d34f491ed3107085a81937b045dd02d7ac329456d3e6e24f3d2759d9f3af971490be3a20c2491e09a95a6dfd826d9029ff371ced165000943d2f8e5da1173e5d109f5240a5e6943a95cdfbff3f9e07492fdc4453483",
    "0e987215617b7f500bb1c300452ab7c9f28d04982d490094c56b4a95dadd27d26cea801d98995b060ecc6d779f61404547f1d98a6437c64011215c29b9bb55ea93688671f299302f03bdcf09e85c7c4c8ac184a3652f4cc617130bd8d7d10dd97d9a5d47e01d23f5dfb25d1b481dc43a3881cb618d42ddd212da915fadda7fa5",
    "1793a79ddfe142063928625e206b613752ddfd95036f133a1e86d74456d60f06291d19934dae1e23ae503e76b8f8a09b094d2f61d2b4b9a282cbf89ba85b5b025088d092ddbce79b37111b5141b52bc08c6252d49054bc701fc38ce589de3fabb3d49a435ef08ca424e357f8ed8241bdea1f844ac581a07f0c6b0ce336e31612",
    "a799b160beecc0b47f943705924d919f910a6d8b297b4bc9f7f6e9341cfa6ec88af1754d0c0bd4f5f37c90c17159523fb71ea9c44d1c64f94427466bc56d8abc378c838986b307f2c739875259db05dcd3c78119cc023c18faae0c1728ad4963458666798b644b0e8b24896ee50cea0b719ef9080fb132f73cee4068302390a5",
    "76ae8e89fe1e259e1d6dbfeed5c8d7f73fd1053273539845a9276fe58400c3a4dc1578e057de40350bca2998ddcaec30bfb9f3b31939a5df452d45a1a0b57d476d0556812f9a1cdeb475028485f3cb70a04bac5824763f7afcf490c9e25c244883a8a3e30e4ba25730fd80c5a186463f58048631b59b7de3ffccc3d207bcc43a",
    "206cb987ee816ecf7bcc1dabdc0b9cf0698c91891e973a9afe741e141317b2a39b8b868795bdd2702d18ef85980204957950a52a0f4053754878eb020174e75e4556b23d18bfdcdbbcbb26f910882a290bfcac31e8f198ae33892d34af0b0da2939ec0ed4ef7807791095ce22c46099f55d105fd8739d1ae6ccb78e6d9ad7f7b",
    "64bc0c3ba608622d0b9e5313c3972c7b5c009bbb61cef00fd4e6e2d1d083239d1cf51f56ae8716c7b3c9dd59a23a94371aa50aaf0065a99c116d03f4144306509240ccc920742ec2f89fea1e8c0cb00d65db4a5f00768eb58c63e73562c31c6c4523be745e6da46a4f5003f5b5113456ede8ddfb5488fdc92bf582718c97c66c",
    "4d8a9a334414f0edccf091a5c4a409795ee01dfbf4536afd5239b39f4ce30409347c3a3ac4a092e4940dbf242db57d64092c9f2ebeffba55ef37a3050c3f18754e1b1d25066346273ca615f3dc24b412c6afad0d7b1836850d9a6a4febba989cc5baaf865572d167b29fbd8649c3654af72b3e8a5193894ef604cac6aa672ce6",
    "67d3aba15b0ca0f0d8e2f6d824a1e0aa7691886cb2881626c607e2d834d171caef1950a4ed78747085d89db43132204837db11e41f536bcfd2ba115070be78a9267b01794c7b416a6b3a4a0071f1a048243b20f0fef92eb3ce35939eeca2e126b7f80f44db7bc56c0460f06a3ed35b8578d2e7a16c2a0c07b5748b779ba41f13",
    "96c18f78978a62e93dbf5287276c7e6cea880ef637873ba96ae023928ce7fb7f30e13ad85e7c925603f5ba5252c10e2fef78030c0d2f6f2e9b2125fffd2b523c52cc3fe3815799187e8124be45cfc5a32ef644813fe0c6b89dc41e0bf396657f58ce710da4de0814d890181b71a115599630386a1d050e88fb619e630b7ea0d8",
    "0efb81ab24847df55f42dc0900a007ff6c8f5306b9c6cf8b6b291ee23a40e31c3e6dd9098ccef1b41a3194180534628231e56114526b3c68a2a979db68286f9807a0d0f3dfaaeb913ce7bcb714f3f88d4b873b7a7bab4e9a96622affc75ffcc0567c461da89ab7e999a038521c8be0e9bdfbefcdb926aa850aee7d57da278397",
    "378dd16eedb03377633c1b2db1b746bbb59b0a2ce0bb27d3887f3ba57f8fafb70e9410649530527d271b0a28e14872b290dd6d33fe1475050e4e4b3f56b2b5280a600c515ac2164927effa1d8a59f9c711cc2b9d91e2faec2fc164018e13b28231af13511d6e8d9a6897724d99a0e9913c21793ce547b9c1d2b1f7150d454373",
    "92de235fd23aa265610400d92c652e4a0bbb6df755b071ef0cf195f72343c76b19e8511f4b317fce17a5606da6e44746873b75fc977bf268ad2fe34611d5498d198a1301345b3757a21fb90d003bf5be6d35d63a982ecc0509625ace4d10ad0df00a351692f567dbace3761c7029f5f9885514ce8edee53ee19b7aebf1a1495b",
    "6e3590e98b253e27c437ba27aae9c861a5b09c8224fdc74736055ddb7230974b2033f2e259c0a48af12076dbd0ef9e1a10630cf5df196c782935e9a4261ad90d883f22d05fdee395027b9b03e1d17905efe5255cd24e8005c6dd04b59463d5b6ab8eea2ad5dea27d2ce9a4a8b61ee56dffe6374c92b5d72ff8c28f63aa5c67fe",
    "6f6e422e6127a176727464c1ebb88f042e786e4751cec15ff9159238b600f3b095b508a85f376382447d78e5b11145774f9bf37c769ef5309def8cf5f3fbef9318fbc03d9713ff1215e686783de89057c05013aa7c9ec158bf02c50afb4ae5df6d792abab95bd18c88082cdbe1cdd0970d835c7852ac6d5f03152492969020dc",
    "389595bf922cebff3635785ee0e8ce329f668723ac60e73c83a453a60392ce7609d2ed8718de3197c13944bba81d5944ff822059e225c3860f799d0064fdb85f9db0d4f92048ffcd7f889950fe1e59455966ecb74cc5480ee96a40846971fd46388af20e532d50e991b719fc0baf094245a832ae057202476d693f15606f807e",
    "290babcc30c0303e48daba12a86a73c688b87523218e7349a6223e6ce05f5d750787569414fa7b389c085737c8584565b819363dfe994cb7508ed960ef2418be1f0cabddd0ee96bf672ac25f8c23dd0c150320936cd3e6f95eb43aa5da275c940ecf134202060e4be312e601c1d0a975b579a3bb7d782b65c4b281450501c3b5"
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
    subaccounts = accounts[1:args.num_producers]
    for i in subaccounts:
        for j in subaccounts:
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
        run(args.clultrain + 'create account ultrainio ' + a + ' ' + args.public_key)

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
        retry(args.clultrain + 'system newaccount --transfer ultrainio %s %s --stake-net "1000.0000 UGAS" --stake-cpu "1000.0000 UGAS" --buy-ram "1000.000 UGAS" ' % (accounts[i], args.public_key))
    sleep(15)


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
        retry(args.clultrain + 'system delegatebw --transfer ultrainio %s "500000.0000 UGAS"  "500000.0000 UGAS"'  % testacc)
        retry(args.clultrain + 'system buyram  ultrainio  %s  "50000.0000 UGAS"  '  % testacc)

def stepCreateinitAccounts():
    for a in initialAccounts:
        retry(args.clultrain + 'system newaccount --transfer ultrainio %s %s --stake-net "1000.0000 UGAS" --stake-cpu "1000.0000 UGAS" --buy-ram "1000.000 UGAS" ' % (a, args.initacc_pk))
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
    ('d', 'deletatetest',   stpDelegateTestAcc,         True,    "Conduct transfer test for user.111 and user.112 multi-agent resources"),
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
