#! /bin/bash
randDir="./"; 
clDir="../../build/programs/clultrain"
cvrf="./vrf_client"
random="234567" # TODO: add "vrf" prefix to all random for security
readarray -t ult_pkLst < pk.lst; readarray -t ult_skLst < sk.lst; readarray -t ult_accountLst < account.lst

wallet_url=" --wallet-url http://127.0.0.1:6666 "

# 1. rand.sh e

VOTER_NUM=${#ult_accountLst[@]}
voted_data=()
res=""
case "$1" in
  "e") #vote
	i=0;
	j=1; 
	VOTE_TIMES=200000;
	CACHE_VOTE=80;
	VOTER_NUM=4;
	while [ true ];
	do
		printf "times : "$j"\n"
		res=$($clDir/clultrain --wallet-url http://127.0.0.1:6666 push action utrio.rand query '' -p ${ult_accountLst[$i]})
		while [ $i -lt $VOTER_NUM ];
		do
			if [ -z "$res" ]; then
				res=$($clDir/clultrain --wallet-url http://127.0.0.1:6666 push action utrio.rand query '' -p ${ult_accountLst[$i]})
			fi
			random=$(echo $res | grep -oP "=> [0-9,]+" | cut -d',' -f 2)
			bcknum=$(echo $res | grep -oP "=> [0-9]+" | cut -d' ' -f 2)
			if [ $((${#random} % 2)) -eq 1 ]; then
				random=$random"0"
			fi
			#echo "final seed: "$random
			if [ -n $random ]; then
				#pk="$(perl $randDir/b58.pl ${ult_pkLst[$i]})"
				sk="$(perl $randDir/b58.pl ${ult_skLst[$i]})" #decode sk from base58 to hex
				pout="$($cvrf -p $sk  $random)"
				IFS=';' read -ra data1 <<< "$pout"

				existing=false
				vote_data=${random}${ult_accountLst[$i]}
				echo "vote_data: "$vote_data
				for value in ${voted_data[@]};
				do
					if [ $value = ${vote_data} ]; then
						existing=true
					fi
				done

				if [ "$existing" = "false" ]; then
					voted_data=(${voted_data[@]} $vote_data)
					$clDir/clultrain --wallet-url http://127.0.0.1:6666 push action utrio.rand vote '["'"${data1[1]}"'", "'"${bcknum}"'"]' -p ${ult_accountLst[$i]}
				fi
				len=${#voted_data[*]}

				if [ ${len} -gt ${CACHE_VOTE} ]; then
					voted_data=("${voted_data[@]:1}")
				fi
				# echo "pout: "$pout
				# echo "public key: "$pk
				# echo "data1[1]: "${data1[1]}
				# echo "data1[2]: "${data1[2]}
				# echo "account: "${ult_accountLst[$i]}
				# echo "block number: "${bcknum}
			fi
			i=`expr $i + 1`
		done
		i=0
		sleep 7;
		j=`expr $j + 1`;
	done
    ;;
esac
