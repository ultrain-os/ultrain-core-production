#!/usr/local/bin/python2
import argparse
import os
import datetime
import time
import random
import copy
parser = argparse.ArgumentParser()
parser.add_argument('-m', '--masterchain', action='store_true', help="set current master chain")
parser.add_argument('-sub', '--subchain', type=str, help="set subchain name info")
args = parser.parse_args()
class Host(object):

	def __init__(self, ip, constainers):
		self.ip = ip
		self.containers = constainers

class Container(object):

	def __init__(self, id, name, ip, hostip):
		self.id = id
		self.name = name
		self.ip = ip
		self.hostip = hostip
		self.peers = []
		self.isleaf = False

class Node(object):

	def __init__(self, ip):
		self.id=ip

# global parameters
hosts={}
allIDs=[]
allNames=[]
allIPs=[]
allAccounts=[]
allPriKeys = []
allPubKeys =[]

pk_list = ["369c31f242bfc5093815511e4a4eda297f4b8772a7ff98f7806ce7a80ffffb35",
"b7e0a16fdca44d4ece1b14d8e7e6207402a6447115ca7d2d7edb08958e6d8ed5",
"4031a95071a092eca8646d3999c438bdfde368d4837770755af65a11b4520a48",
"2b43a3d8e0523a85141bbfca41006cf9abc47587d0ff3c46d257551ec05f1677",
"b9a55c3c661abd8c539b7a7c05c8176036f87aeeb6b117a138327cfdb374cc23",
"8f6e3b3336276138023617f0ae0e6fd0c37d27aa9995f9803fe78df4941dd3ec",
"3696fdfb2a9716ae4b1bca0d5b0a861a1e5d64562994aeb515eed49290c9f1c2",
"578451935c370d9c7fbcdd77e35a40e49bf0a5311e065035778ac27e6263b10d",
 ]

sk_list = ["5079f570cde7801c70a19fb2c7e292d09923218f2684c8a1121c2da7a02a5dc3369c31f242bfc5093815511e4a4eda297f4b8772a7ff98f7806ce7a80ffffb35",
           "97d157bf7fdedfeaef46216e246cc83f9c25574b49bacf83a1862ef0d82233ecb7e0a16fdca44d4ece1b14d8e7e6207402a6447115ca7d2d7edb08958e6d8ed5",
           "b5b3b423eed6fd6f255fe1e4570e3a9f9878a36870b584b1d231ca778ad95ede4031a95071a092eca8646d3999c438bdfde368d4837770755af65a11b4520a48",
           "5cfdb237515f969620f857658baac5485266f1c143fee80208e0da9935bbfaae2b43a3d8e0523a85141bbfca41006cf9abc47587d0ff3c46d257551ec05f1677",
           "6b8ed821703f369f1a6b7c767bf076b093a17a5f839ad66d77d808f86a68dd7db9a55c3c661abd8c539b7a7c05c8176036f87aeeb6b117a138327cfdb374cc23",
           "fe512a00e3c5d22cc8a849a0713df76733ed730bd66cb85b08dee5e7d6e7b15a8f6e3b3336276138023617f0ae0e6fd0c37d27aa9995f9803fe78df4941dd3ec",
           "ab0c2e8ea8ba037f169dbb384605b227016c617d48495fd78250b2809dfa06203696fdfb2a9716ae4b1bca0d5b0a861a1e5d64562994aeb515eed49290c9f1c2",
           "0bccefe21fa6beb22335e1b200c8a4fdd9d250527d643f33bb4d080d6941ff1a578451935c370d9c7fbcdd77e35a40e49bf0a5311e065035778ac27e6263b10d",
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

accounts = [
"genesis",
".111",
".112",
".113",
".114",
".115",
".121",
".122",
]
dockerinfo = "dockerinfo"
def select_sort(lists):
    count = len(lists)
    for i in range(0, count):
        min = i
        for j in range(i + 1, count):
            if int(lists[min].name[len(lists[min].name)-1::len(lists[min].name)]) > int(lists[j].name[len(lists[j].name)-1::len(lists[j].name)]):
                min = j
        lists[min], lists[i] = lists[i], lists[min]
    return lists
# load containers' parameters in hosts
def load_parameters():
    global hosts,allIDs,allNames,allIPs,allAccounts,allPriKeys,allPubKeys
    print(dockerinfo)
    with open('config/IPs/'+dockerinfo+'.txt') as f:
       elements = f.read().splitlines()
    IDs = elements[0::3]
    Names = elements[1::3]
    IPs = elements[2::3]

    containers = []
    for i in range(0,len(IDs)):
        container = Container(IDs[i],Names[i],IPs[i],dockerinfo)
        containers.append(container)
    hosts[dockerinfo] = select_sort(containers)

    adjustaccounts = ["genesis",]
    if args.masterchain:
        for a in accounts[1:]:
            adjustaccounts.append("master"+a)
    elif args.subchain:
        for a in accounts[1:]:
            adjustaccounts.append("user"+"."+args.subchain+a)
    else:
        for a in accounts[1:]:
            adjustaccounts.append("user"+a)
    allAccounts = adjustaccounts
    allIDs = allIDs + IDs
    allNames = allNames + Names
    allIPs = allIPs + IPs
    allPriKeys = sk_list 
    allPubKeys = pk_list 

    print(allIDs)
    print(allNames)
    print(allIPs)

def write_config_file():
        insert_genesis_time()	
        hostip = "config"
        os.system("rm -rf config/" + hostip)
        os.system("mkdir config/" + hostip)
        index_key = 0
        for con in hosts[dockerinfo]:
            fname = "config/%s/%s.ini" % (hostip, con.id)
            print(fname)
            if not os.path.isfile(fname):  # file does not exist
                        cmd = "cp template.txt " + fname
                        os.system(cmd)
            if con.name[len(con.name)-1::len(con.name)] != '7':
                insert_keys(fname, index_key)
            if con.name[len(con.name)-1::len(con.name)] == '7':
                insert_non_producing(fname)
            update_ultrainmng_config(fname)
            print(hostip,con.ip,con.id)
            insert_peer(fname,hosts[dockerinfo][0].ip)
            index_key+=1
        os.system("rm -f  template.txt")

def readfile(fname):
	fileold = open(fname, "r")
	content = fileold.readlines()
	fileold.close()
	return content

def writefile(fname,content):
	filenew = open(fname, "w")
	filenew.writelines(content)
	filenew.close()

def insert_genesis_time():
	fname_orig = "template_orig.txt"
	fname = "template.txt"
	cmd = "cp %s %s" % (fname_orig, fname)
	os.system(cmd)
	content = readfile(fname)
	after_time = (datetime.datetime.now()+datetime.timedelta(minutes=2)).strftime("%Y-%m-%d %H:%M") + ":00"
	newcontent = "genesis-time = %s\n" % after_time
	index_line = content.index("#insert_genesis-time\n")
	content.insert(index_line+1,newcontent)
	writefile(fname,content)

def insert_leader_sk(fname):
	content = readfile(fname)
	newcontent = "my-account-as-committee = %s\nmy-sk-as-committee = %s\n" % ("genesis", \
				"5079f570cde7801c70a19fb2c7e292d09923218f2684c8a1121c2da7a02a5dc3369c31f242bfc5093815511e4a4eda297f4b8772a7ff98f7806ce7a80ffffb35\n")
	index_line = content.index("#insert_my_keys\n")
	content.insert(index_line+1, newcontent)
	writefile(fname,content)


# update ultrainmng  config
def update_ultrainmng_config(fname):
    content = readfile(fname)
    newcontent = "";
    if args.masterchain:
        newcontent = "subchainHttpEndpoint = http://172.16.10.4:8877\n"
    if args.subchain == "11" :
        newcontent = "subchainHttpEndpoint = http://172.16.10.5:8888\n"
    elif args.subchain == "12" :
        newcontent = "subchainHttpEndpoint = http://172.16.10.5:8899\n"
    index_line = content.index("#ultrainmng_subchainHttpEndpoint\n")
    content.insert(index_line+1, newcontent)
    writefile(fname,content)

def insert_non_producing(fname):
	content = readfile(fname)
	newcontent = "is-non-producing-node = 1\nhttp-server-address = 0.0.0.0:8888\nhttp-alias = 172.16.10.5:8888\n"
        if args.masterchain:
            newcontent = "is-non-producing-node = 1\nhttp-server-address = 0.0.0.0:8888\nhttp-alias = 172.16.10.4:8877\n"
        if args.subchain == "11" :
            newcontent = "is-non-producing-node = 1\nhttp-server-address = 0.0.0.0:8888\nhttp-alias = 172.16.10.5:8888\n"
        elif args.subchain == "12" :
            newcontent = "is-non-producing-node = 1\nhttp-server-address = 0.0.0.0:8888\nhttp-alias = 172.16.10.5:8899\n"
	print(newcontent)
	index_line = content.index("#insert_if_producing-node\n")
	content.insert(index_line+1, newcontent)
	writefile(fname,content)

def insert_peer(fname,peer):
	content = readfile(fname)
        newcontent = "p2p-peer-address = %s:20122\nrpos-p2p-peer-address = %s:20123\n" % (peer,peer)
	print(newcontent)
	index_line = content.index("#insert_peers\n")
	content.insert(index_line+1, newcontent)
	writefile(fname,content)

def insert_keys(fname,index_key):
	content = readfile(fname)
	newcontent = "my-account-as-committee = %s\nmy-sk-as-committee = %s\nmy-sk-as-account = %s\n" % \
				 (allAccounts[index_key],allPriKeys[index_key],account_sk_list[index_key])
	index_line = content.index("#insert_my_keys\n")
	content.insert(index_line+1, newcontent)
	writefile(fname, content)

load_parameters()
write_config_file()

