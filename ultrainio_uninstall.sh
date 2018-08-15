#! /bin/bash

binaries=(clultrain
          nodultrain
          ultrainio-kultraind
          ultrainio-abigen
          ultrainio-applesdemo
          ultrainio-launcher
          ultrainio-s2wasm
          ultrainio-wast2wasm)

if [ -d "/usr/local/ultrainio" ]; then
   printf "\tDo you wish to remove this install? (requires sudo)\n"
   select yn in "Yes" "No"; do
      case $yn in
         [Yy]* )
            if [ "$(id -u)" -ne 0 ]; then
               printf "\n\tThis requires sudo, please run ./ultrainio_uninstall.sh with sudo\n\n"
               exit -1
            fi

            pushd /usr/local &> /dev/null
            rm -rf ultrainio
            pushd bin &> /dev/null
            for binary in ${binaries[@]}; do
               rm ${binary}
            done
            popd &> /dev/null
            break;;
         [Nn]* ) 
            printf "\tAborting uninstall\n\n"
            exit -1;;
      esac
   done
fi
