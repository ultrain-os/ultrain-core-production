#! /usr/bin/python
import string
import os
import sys
savedstdout=sys.stdout
with open('out.txt', 'w+') as file:
    sys.stdout = file
buildpath=sys.argv[1]
oscmd="rm -rf  ~/ultrainio-wallet/default.wallet"
os.system(oscmd)
cmdpath=buildpath+"/programs/nodultrain/nodultrain"
os.system(cmdpath)
