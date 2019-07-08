#coding=utf-8
import unittest
import time
import requests
import json
import sys
sys.path.append("..")
from config import *
class BuyResourceLease(unittest.TestCase):

    name = name
    #购买资源套餐
    def buy_resource_lease( self ):
        clultrain = '/root/workspace/ultrain-core/build/programs/clultrain/clultrain --wallet-url http://127.0.0.1:6666 '
        return subprocess.call(clultrain + 'system resourcelease ultrainio  root  1 1  "ultrainio"', shell=True)
    # 测试购买资源
    def test_buy_resource_lease( self ):
        self.assertEqual( buy_resource_lease(), 0 )
        time.sleep( 20 )
        j = json.loads(requests.get("http://127.0.0.1:8888/v1/chain/get_account_info",data = json.dumps({"account_name":recacc})).text)
        assert (sellram_before-j["ram_quota"]) == 1024,'sellram account:'+recacc+' sell_ram:'+str(sellram_before-j["ram_quota"])+'!='+str(1024)

    def getaccresinfo( self, ip,acclist,nofindacc,leasenum,days,reslist):
        for a in acclist:
            j = json.loads(requests.get("http://"+ip+"/v1/chain/get_account_info",data = json.dumps({"account_name":a})).text)
            if ("account_name" in j):
                continue
            nofindacc.append(a)
        num = 1;
        for a in acclist:
            j = json.loads(requests.get("http://"+ip+"/v1/chain/get_table_records",data = json.dumps({"code":"ultrainio","scope":"ultrainio","table":"reslease","json":"true","limit":1000,"table_key":a})).text)
            if len(j["rows"])== 0 :
                reslist.append(a)
                continue
            if (a == j["rows"][0]["owner"]) and (leasenum == j["rows"][0]["lease_num"]):
                print "-----"+str(num);
                print "expect days:"+str(days);
                startBlockNum = j["rows"][0]["start_block_height"]
                endBlockNum = j["rows"][0]["end_block_height"]
                lease_days = (endBlockNum - startBlockNum)/(6*60*24)
                print "actual days:"+str(lease_days);
                print "-----"+str(num);
                num = num +1;
                if lease_days == days :
                    continue
            reslist.append(a)
    def setUp( self ):
        print('\n====BuyResourceLease init====')
        self.create_account()

    def tearDown( self ):
        print('\n====BuyResourceLease destroy====')
        self.destroy_account()
