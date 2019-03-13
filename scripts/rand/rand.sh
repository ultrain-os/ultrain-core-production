#! /bin/bash
randDir="./"; clDir="../../build/programs/clultrain"
cvrf="./vrf_client"
random="234567" # TODO: add "vrf" prefix to all random for security
readarray -t ult_pkLst < pk.lst; readarray -t ult_skLst < sk.lst; readarray -t ult_accountLst < account.lst

wallet_url=" --wallet-url http://127.0.0.1:6666 "

# 1. rand.sh e

VOTER_NUM=${#ult_accountLst[@]}
case "$1" in
  "e") #vote
	i=0;j=1; VOTE_TIMES=200000; VOTER_NUM=16;
	while [ true ];do
                printf "times : "$j"\n"
		while [ $i -lt $VOTER_NUM ];do
			printf "\n\n"
			sk="$(perl $randDir/b58.pl ${ult_skLst[$i]})" #decode sk from base58 to hex
			printf "sk="$sk"\n"

			res=$($clDir/clultrain --wallet-url http://127.0.0.1:6666 push action utrio.rand query '' -p ${ult_accountLst[$i]})
			# TODO submit vote value removing duplicate voting transactoins
			echo "return:"$res
			random=$(echo $res | grep -oP "=> [0-9,]+" | cut -d',' -f 2)
			bcknum=$(echo $res | grep -oP "=> [0-9]+" | cut -d' ' -f 2)
			pk="$(perl $randDir/b58.pl ${ult_pkLst[$i]})"
			if [ $((${#random} % 2)) -eq 1 ]; then
				random=$random"0"
			fi
			echo "final seed: "$random
			if [ $random ]; then
				pout="$($cvrf -p $sk  $random)"
				IFS=';' read -ra data1 <<< "$pout"
				# echo "pout: "$pout
				# echo "public key: "$pk
				# echo "data1[1]: "${data1[1]}
				# echo "data1[2]: "${data1[2]}
				# echo "account: "${ult_accountLst[$i]}
				# echo "block number: "${bcknum}
				$clDir/clultrain --wallet-url http://127.0.0.1:6666 push action utrio.rand vote '["'"${data1[1]}"'", "'"${bcknum}"'"]' -p ${ult_accountLst[$i]}
			fi
			i=`expr $i + 1`
		done
		i=0
		sleep 5;
		j=`expr $j + 1`;
	done
    ;;
esac
