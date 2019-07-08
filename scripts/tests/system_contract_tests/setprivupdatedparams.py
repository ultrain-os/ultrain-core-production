#coding=utf-8
import unittest
import requests
import json
import sys
sys.path.append("..")
from config import *
from general_func import *

class SetPrivUpdatedPrams(unittest.TestCase):
    name = "root"
    privileged = False
    updateable = True

    #获取账户特权和是否可更新参数
    def getInitSetPrivUpdatedPrams( self ):
        j = json.loads(requests.get(Config.get_account_info_url,data = json.dumps({"account_name":self.name})).text)
        self.assertEqual( j["account_name"] , self.name )
        self.assertEqual( j["privileged"], self.privileged )
        self.assertEqual( j["updateable"], self.updateable )

    #还原账户特权和是否可更新参数
    def setInitSetPrivUpdatedPrams( self ):
        cmd_exec(Config.clultrain_path + '''push action ultrainio  setpriv  '["%s", "%d"]' -f -p ultrainio ''' % ( self.name, self.privileged))
        cmd_exec(Config.clultrain_path + '''push action ultrainio  setupdateabled  '["%s", "%d"]' -f -p ultrainio ''' % ( self.name, self.updateable))

    # 测试设置的参数是否正确
    def test_SetPrivUpdatedPrams( self ):
        cmd_exec(Config.clultrain_path + '''push action ultrainio  setpriv  '["%s", "%d"]' -p ultrainio ''' % ( self.name, not self.privileged))
        cmd_exec(Config.clultrain_path + '''push action ultrainio  setupdateabled  '["%s", "%d"]' -p ultrainio ''' % ( self.name, not self.updateable))
        sleep(20)
        j = json.loads(requests.get(Config.get_account_info_url,data = json.dumps({"account_name":self.name})).text)
        self.assertEqual( j["privileged"], not self.privileged )
        self.assertEqual( j["updateable"], not self.updateable )


    def setUp( self ):
        print('\n====SetPrivUpdatedPrams init====')
        self.getInitSetPrivUpdatedPrams()

    def tearDown( self ):
        print('\n====SetPrivUpdatedPrams destroy====')
        self.setInitSetPrivUpdatedPrams()
