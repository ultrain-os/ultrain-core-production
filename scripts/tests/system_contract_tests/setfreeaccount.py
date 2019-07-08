#coding=utf-8
import unittest
import requests
import json
import sys
sys.path.append("..")
from config import *
from general_func import *

class SetFreeAccount(unittest.TestCase):
    name = "root"
    free_num = 0

    #获取账户免费可创建数量
    def getInitFreeAccountNum( self ):
        j = json.loads(requests.get(Config.get_table_url,data = json.dumps({"code":"ultrainio","scope":"ultrainio","table":"freeacc","json":"true"})).text)
        for i in range(0,len(j["rows"])):
            if self.name == j["rows"][i]["owner"]:
                self.free_num = int(j["rows"][i]["acc_num"])

    #设置账户免费可创建数量
    def setInitFreeAccountNum( self ):
        cmd_exec(Config.clultrain_path + '''push action ultrainio  setfreeacc  '["%s", "%d"]' -f -p ultrainio ''' % ( self.name, self.free_num))

    # 测试设置的参数是否正确
    def testSetFreeAccount( self ):
        cmd_exec(Config.clultrain_path + '''push action ultrainio  setfreeacc  '["%s", "%d"]' -p ultrainio ''' % ( self.name, self.free_num+1))
        sleep(20)
        freeaccount = 0
        j = json.loads(requests.get(Config.get_table_url,data = json.dumps({"code":"ultrainio","scope":"ultrainio","table":"freeacc","json":"true"})).text)
        for i in range(0,len(j["rows"])):
            if self.name == j["rows"][i]["owner"]:
                freeaccount = int(j["rows"][i]["acc_num"])
        self.assertEqual( freeaccount, self.free_num + 1 )


    def setUp( self ):
        print('\n====SetFreeAccount init====')
        self.getInitFreeAccountNum()

    def tearDown( self ):
        print('\n====SetFreeAccount destroy====')
        self.setInitFreeAccountNum()