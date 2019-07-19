#coding=utf-8
import unittest
import subprocess
import time
import requests
import json
import sys
sys.path.append("..")
from config import Config

class CreateAccount(unittest.TestCase):
    #createaccount = CreateAccount( "test12345", "UTR6r3BNrssrD9F5jJo17aszYN3S7mrYcCYqXGDgaEB1WPgF9LVfe")
    name = "test111112"
    pk = "UTR6r3BNrssrD9F5jJo17aszYN3S7mrYcCYqXGDgaEB1WPgF9LVfe"
    #创建账号
    def create_account( self ):
        execcommand = Config.clultrain_path + 'create account ultrainio %s %s %s ' % (self.name, self.pk, self.pk)
        print( execcommand )
        return subprocess.call( execcommand, shell=True )

    #销毁账号
    def destroy_account( self ):
        execcommand = Config.clultrain_path + 'push action ultrainio  delaccount \'["%s"]\' --delay-sec 30 -p ultrainio' % ( self.name )
        print( execcommand )
        return subprocess.call( execcommand, shell=True )

    # 测试账号是否正确
    def test_create_account( self ):
        time.sleep( 20 )
        j = json.loads(requests.get(Config.get_account_info_url,data = json.dumps({"account_name":self.name})).text)
        self.assertIn( "account_name" , j )
        self.assertEqual( j["account_name"] , self.name )
        self.assertEqual( j["ram_quota"], 0 )
        self.assertEqual( j["updateable"], True )
        self.assertEqual( j["privileged"], False )
        self.assertEqual( j["permissions"][0]["required_auth"]["keys"][0]["key"] , self.pk )
        self.assertEqual( j["permissions"][1]["required_auth"]["keys"][0]["key"] , self.pk )

    def setUp( self ):
        print('\n====CreateAccount init====')
        self.create_account()

    def tearDown( self ):
        print('\n====CreateAccount destroy====')
        self.destroy_account()
