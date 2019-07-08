#coding=utf-8
import unittest
import subprocess
import time
import requests
import json
import sys
sys.path.append("..")
from config import Config

class SetGlobalExtenData(unittest.TestCase):
    d = { 1:0, 2:0, 6:0}
    #获取初始系统参数
    def getInitglobalparams( self ):
        j = json.loads(requests.get(Config.get_table_url,data = json.dumps({"code":"ultrainio","scope":"ultrainio","table":"global","json":"true"})).text)
        for v in j["rows"][0]["table_extension"]:
            if int(v["key"]) in self.d.keys():
                print(v["key"],self.d[v["key"]],int(v["value"]))
                self.d[int(v["key"])] = int(v["value"])

    #还原初始参数
    def setInitglobalparams( self ):
        for key in self.d:
            execcommand = Config.clultrain_path + '''push action ultrainio  setglobalextendata  '["%d", "%d"]' -f -p ultrainio ''' % ( key, self.d[key])
            print( execcommand )
            subprocess.call( execcommand, shell=True )

    # 测试设置的参数是否正确
    def test_setglobalextendata( self ):
        for key in self.d:
            execcommand = Config.clultrain_path + '''push action ultrainio  setglobalextendata  '["%d", "%d"]' -p ultrainio ''' % ( key, self.d[key] + 1 )
            print( execcommand )
            subprocess.call( execcommand, shell=True )
        #time.sleep(15)
        j = json.loads(requests.get(Config.get_table_url,data = json.dumps({"code":"ultrainio","scope":"ultrainio","table":"global","json":"true"})).text)
        for v in j["rows"][0]["table_extension"]:
            if int(v["key"]) in self.d.keys():
                print(v["key"],self.d[v["key"]],int(v["value"]))
                self.assertEqual( int(v["value"]) , self.d[v["key"]] + 1 )


    def setUp( self ):
        print('\n====SetGlobalExtenData init====')
        self.getInitglobalparams()

    def tearDown( self ):
        print('\n====SetGlobalExtenData destroy====')
        self.setInitglobalparams()
