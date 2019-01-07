#! /usr/bin/python
import string
import os
import sys
savedstdout=sys.stdout
with open('out.txt', 'w+') as file:
    sys.stdout = file
buildpath=sys.argv[1]
cmdpath=buildpath+"/programs/nodultrain/nodultrain"
os.system(cmdpath)
