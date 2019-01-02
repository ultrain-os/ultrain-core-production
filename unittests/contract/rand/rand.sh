#! /bin/bash
# bash run.sh -d; bash run.sh -e

randDir="./"; clDir="../../../build/programs/clultrain"
cvrf="./vrf_client"

msg="234567" # TODO: add "vrf" prefix to all msg for security
# read without trailing newline
# https://stackoverflow.com/questions/41721847/readarray-t-option-in-bash-default-behavior

cd $randDir
readarray -t ult_pkLst < pk.lst; readarray -t ult_skLst < sk.lst; readarray -t ult_accountLst < account.lst
# echo "${msgLst[*]}"

wallet_url="--wallet-url http://127.0.0.1:6666"

# 1. rand.sh c
# 2. rand.sh r
# 3. rand.sh e

VOTER_NUM=${#ult_accountLst[@]}
case "$1" in
  "c") # create account
	i=0
	while [ $i -lt $VOTER_NUM ]
	do
	    $clDir/clultrain --wallet-url http://127.0.0.1:6666 wallet import  --private-key ${ult_skLst[$i]}
	    sleep 1
	    $clDir/clultrain --wallet-url http://127.0.0.1:6666 create account ultrainio ${ult_accountLst[$i]} ${ult_pkLst[$i]} -u -p ultrainio
		sleep 1
		$clDir/clultrain --wallet-url http://127.0.0.1:6666 transfer ultrainio ${ult_accountLst[$i]} "300000.0000 UGAS" -p ultrainio
		sleep 1
		$clDir/clultrain --wallet-url http://127.0.0.1:6666 system resourcelease ${ult_accountLst[$i]} ${ult_accountLst[$i]} 10 100
		i=`expr $i + 1`
		printf "\n\n"
	done
	sleep 10
	;;
  "r")
	#4 candidates
	VOTER_NUM=16; i=0
	while [ $i -lt $VOTER_NUM ]
	do
		printf "\n\n"
		#$clDir/clultrain --wallet-url http://127.0.0.1:6666 push action utrio.rand removeCandidate '' -p ${ult_accountLst[$i]};
		sleep 1 #remove, in case already registed


		$clDir/clultrain --wallet-url http://127.0.0.1:6666 transfer ${ult_accountLst[$i]} utrio.rand "2.0000 UGAS" "as candidate" -p ${ult_accountLst[$i]};
		sleep 1

		i=`expr $i + 1`
		printf "\n\n"
	done
	sleep 10
	;;
  "e") #vote
	# delegate permission to send bonus
	# ./clultrain set account permission abc123 active '{"threshold":1,"keys": [{"key":"UTR6FjKthwDnnh6o2T9n4DDb19X1vsrafJibEiCPDX6YP1QmZS7Bu","weight": 1}],"accounts": [{"permission":{"actor":"abc123","permission":"utrio.code"},"weight":1}]}' owner -p abc123

	i=0;j=1; VOTE_TIMES=20; VOTER_NUM=16;
	while [ $j -lt $VOTE_TIMES ];do
                printf "times : "$j"\n"
		while [ $i -lt $VOTER_NUM ];do
			printf "\n\n"
			sk="$(perl $randDir/b58.pl ${ult_skLst[$i]})" #decode sk from base58 to hex
			printf "sk="$sk"\n"
			# query latest rand
			msg=$($clDir/clultrain --wallet-url http://127.0.0.1:6666 push action utrio.rand query '' -p ${ult_accountLst[$i]} | grep -oP "Rand: [0-9]+" | cut -d' ' -f 2)
			echo "query msg: "$msg
			if [ $((${#msg} % 2)) -eq 1 ]; then
				msg=$msg"0"
			fi
			pout="$($cvrf -p $sk  $msg)"
			echo "pout: "$pout
			IFS=';' read -ra data1 <<< "$pout"
			echo "data1[1]: "${data1[1]}
			echo "data1[2]: "${data1[2]}
			echo "account: "${ult_accountLst[$i]}
			# cannot parse $ in '', only in ""
			$clDir/clultrain --wallet-url http://127.0.0.1:6666 push action utrio.rand vote '["'"${data1[1]}"'" "'"${data1[2]}"'"]' -p ${ult_accountLst[$i]}
			i=`expr $i + 1`
			sleep 0.5
		done
		i=0
		sleep 20;
		j=`expr $j + 1`;
	done
    ;;
esac

