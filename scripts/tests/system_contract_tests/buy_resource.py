#coding=utf-8
import unittest
import time
import requests
import json
import sys
sys.path.append("..")
from config import *
from general_func import *
class BuyResourceLease(unittest.TestCase):
    _name = "root3"
    _is_bought = False
    _res_num = 0
    _res_day = 0
    _modify_blockheight = 0
    _is_found = False

    # 测试购买资源
    def test_buy_resource_lease( self ):
        j = json.loads(requests.get(Config.get_info_url).text)
        if (int(j["head_block_num"]) - self._modify_blockheight) < 360 :
            return
        if self._is_bought:
            cmd_exec(Config.clultrain_path + ''' system resourcelease ultrainio  %s  1 0  "ultrainio" ''' % ( self._name))
            self._res_num += 1
        else:
            cmd_exec(Config.clultrain_path + ''' system resourcelease ultrainio  %s  1 1  "ultrainio" ''' % ( self._name))
            self._res_day = 1
            self._res_num = 1
        time.sleep( 20 )
        j = json.loads(requests.get(Config.get_table_url,data = json.dumps({"code":"ultrainio","scope":"ultrainio","table":"reslease","json":"true"})).text)
        for i in range(0,len(j["rows"])):
            if self._name == j["rows"][i]["owner"]:
                self.assertEqual(self._res_num, int(j["rows"][i]["lease_num"]))
                self.assertEqual(self._res_day, int(j["rows"][i]["end_block_height"] - j["rows"][i]["start_block_height"])/8640)
                self._is_found = True
        self.assertTrue(self._is_found)

    def getInitResInfo( self ):
        j = json.loads(requests.get(Config.get_table_url,data = json.dumps({"code":"ultrainio","scope":"ultrainio","table":"reslease","json":"true"})).text)
        for i in range(0,len(j["rows"])):
            if self._name == j["rows"][i]["owner"]:
                self._res_num = int(j["rows"][i]["lease_num"])
                self._is_bought = True
                self._res_day = (int(j["rows"][i]["end_block_height"]) - int(j["rows"][i]["start_block_height"]))/8640
                self._modify_blockheight = int(j["rows"][i]["modify_block_height"])
                break

    def setUp( self ):
        print('\n====BuyResourceLease init====')
        self.getInitResInfo()

    def tearDown( self ):
        print('\n====BuyResourceLease destroy====')
