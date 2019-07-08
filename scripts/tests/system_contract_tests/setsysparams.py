#coding=utf-8
import unittest
import subprocess
import time
import requests
import json
import sys
sys.path.append("..")
from config import Config

class SetsysparamsTest(unittest.TestCase):
    system = '''push action ultrainio setsysparams '{"params":{"chain_type": "0", "max_ram_size":"32212254720",        "min_activated_stake":420000000,"min_committee_member_number":4,        "block_reward_vec":[{"consensus_period":10,"reward":"10000"},{"consensus_period":2,"reward":"2000"}],        "max_resources_number":10000, "newaccount_fee":2000, "chain_name":"11", "worldstate_interval":1000,"resource_fee":35068,"table_extension":[[1,"10000"], [2, "12"], [3, "1"], [4, "true"], [5, "50"], [6, "2"]]}}' -p ultrainio"'''
    max_ram_size = 0
    min_activated_stake = 0
    newaccount_fee = 0
    worldstate_interval = 0
    resource_fee = 0
    init_global_data
    #获取初始系统参数
    def getInitsysparams( self ):
        j = json.loads(requests.get(Config.get_table_url,data = json.dumps({"code":"ultrainio","scope":"ultrainio","table":"global"})).text)
        max_ram_size = j["rows"][0]["max_ram_size"] + 1
        min_activated_stake = j["rows"][0]["min_activated_stake"] + 1
        newaccount_fee = j["rows"][0]["newaccount_fee"] + 1
        worldstate_interval = j["rows"][0]["worldstate_interval"] + 1
        resource_fee = j["rows"][0]["resource_fee"] + 1
        min_activated_stake = j["rows"][0]["min_activated_stake"] + 1
        init_global_data = j

    #还原初始参数
    def setInitsysparams( self ):
        execcommand = Config.clultrain_path + 'push action ultrainio  delaccount \'["%s"]\' --delay-sec 30 -p ultrainio' % ( self.name )
        print( execcommand )
        return subprocess.call( execcommand, shell=True )

    # 测试账号是否正确
    def test_create_account( self ):
        #time.sleep( 20 )
        j = json.loads(requests.get(Config.get_account_info_url,data = json.dumps({"account_name":self.name})).text)
        self.assertEqual( j["account_name"] , self.name )
        self.assertEqual( j["ram_quota"], 0 )
        self.assertEqual( j["updateable"], True )
        self.assertEqual( j["privileged"], False )
        self.assertEqual( j["permissions"][0]["required_auth"]["keys"][0]["key"] , self.pk )
        self.assertEqual( j["permissions"][1]["required_auth"]["keys"][0]["key"] , self.pk )

    def setUp( self ):
        print('\n====CreateAccount init====')
        self.getInitsysparams()

    def tearDown( self ):
        print('\n====CreateAccount destroy====')
        self.destroy_account()
