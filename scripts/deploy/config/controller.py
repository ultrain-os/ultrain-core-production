#!/usr/local/bin/python2
# -*- coding: utf8 -*-

import argparse
import os

parser = argparse.ArgumentParser()
parser.add_argument('action',help='start|stop|deploy|update')
parser.add_argument('-cd','--cleardata',help='clear history block data',action='store_true')
parser.add_argument('-cl','--clearlogs',help='clear history logs',action='store_true')
parser.add_argument('-bn',help='replay block height',type=int)
args = parser.parse_args()


if __name__ == "__main__":
	if args.action == "updatefile":
		os.system("fab -f fabfile4azure update")
	elif args.action == "start":
		if args.cleardata:
			os.system("fab -f fabfile4azure clearhostdata")
			print("cleardata")
		if args.clearlogs:
			os.system("fab -f fabfile4azure clearhostlog")
			print("clearlogs")
		os.system("python toplogy-h.py")
		os.system("fab -f fabfile4azure uploadconfig")
		os.system("fab -f fabfile4azure starthosts")
	elif args.action == "resume":
		if args.cleardata:
			os.system("fab -f fabfile4azure clearhostdata")
			print("cleardata")
		if args.clearlogs:
			os.system("fab -f fabfile4azure clearhostlog")
			print("clearlogs")
		os.system("fab -f fabfile4azure starthosts")
	elif args.action == "stop":
		os.system("fab -f fabfile4azure stophosts")
        elif args.action == "startmng":
                os.system("fab -f fabfile4azure startmng")
        elif args.action == "stopmng":
                os.system("fab -f fabfile4azure stopmng")
	elif args.action == "deploy":
		os.system("fab -f fabfile4azure deployfile")
        elif args.action == "urgent":
                os.system("fab -f fabfile4azure urgentmode")
        elif args.action == "upreplayconfig":
                print("upreplayconfig")
                os.system("fab -f fabfile4azure upreplayconfig")
        elif args.action == "replay":
                if args.clearlogs:
                        os.system("fab -f fabfile4azure clearhostlog")
                        print("clearlogs")
                print("replay")
                print(args.bn)
                cmd = "fab -f fabfile4azure replay:%s" % (args.bn)
                os.system(cmd)
