#coding=utf-8
import unittest
import time
import requests
import json
import sys
sys.path.append("..")
from config import *
from general_func import *
class RegDelProd(unittest.TestCase):
    _name = "root1"
    _producer_key = "8f6e3b3336276138023617f0ae0e6fd0c37d27aa9995f9803fe78df4941dd3ec"
    _bls_key = "8a4e44c5240b45cec1665cbea836887fc96aa6eb5515a0e83f257aa593bb0721cd8b56af9d1f6a8b11bbff1f901186274771d3a7a40b23ae661aad247f1705b300"
    _is_found = False
    def regProducer( self ):
        cmd_exec(Config.clultrain_path + '''  system regproducer %s %s %s %s "" "ultrainio" -u ''' % ( self._name, self._producer_key, self._bls_key, self._name ))
        time.sleep( 30 )
        self._is_found = False
        j = json.loads(requests.get(Config.get_table_url,data = json.dumps({"code":"ultrainio","scope":"ultrainio","table":"disableprods","json":"true"})).text)
        for i in range(0,len(j["rows"])):
            if self._name == j["rows"][i]["owner"]:
                self.assertEqual(self._producer_key, j["rows"][i]["producer_key"])
                self.assertEqual(self._bls_key, j["rows"][i]["bls_key"])
                self._is_found = True
        self.assertTrue(self._is_found)
    def delegateProducer( self ):
        cons_staked = 42100
        cmd_exec(Config.clultrain_path + '''  system  delegatecons  ultrainio  %s "%d UGAS" -p ultrainio@active  ''' % ( self._name, cons_staked ))
        time.sleep( 30 )
        self._is_found = False
        j = json.loads(requests.get(Config.get_table_url,data = json.dumps({"code":"ultrainio","scope":"ultrainio","table":"producers","json":"true"})).text)
        for i in range(0,len(j["rows"])):
            if self._name == j["rows"][i]["owner"]:
                self.assertEqual(self._producer_key, j["rows"][i]["producer_key"])
                self.assertEqual(self._bls_key, j["rows"][i]["bls_key"])
                self.assertEqual(cons_staked*10000, int(j["rows"][i]["total_cons_staked"]))
                self._is_found = True
        self.assertTrue(self._is_found)

    def undelegateProducer( self ):
        cmd_exec(Config.clultrain_path + '''  system  undelegatecons  ultrainio  %s -p ultrainio@active  ''' % ( self._name ))
        time.sleep( 30 )
        self._is_found = False
        j = json.loads(requests.get(Config.get_table_url,data = json.dumps({"code":"ultrainio","scope":"ultrainio","table":"producers","json":"true"})).text)
        for i in range(0,len(j["rows"])):
            if self._name == j["rows"][i]["owner"]:
                self._is_found = True
        self.assertFalse(self._is_found)
        j = json.loads(requests.get(Config.get_table_url,data = json.dumps({"code":"ultrainio","scope":"ultrainio","table":"disableprods","json":"true"})).text)
        for i in range(0,len(j["rows"])):
            if self._name == j["rows"][i]["owner"]:
                self.assertEqual(self._producer_key, j["rows"][i]["producer_key"])
                self.assertEqual(self._bls_key, j["rows"][i]["bls_key"])
                self._is_found = True
        self.assertTrue(self._is_found)
    def test_producer( self ):
        self.regProducer()
        self.delegateProducer()
        self.undelegateProducer()
    def setUp( self ):
        print('\n====RegDelProd init====')
        j = json.loads(requests.get(Config.get_table_url,data = json.dumps({"code":"ultrainio","scope":"ultrainio","table":"producers","json":"true"})).text)
        for i in range(0,len(j["rows"])):
            if self._name == j["rows"][i]["owner"]:
                cmd_exec(Config.clultrain_path + '''  system  undelegatecons  ultrainio  %s -p ultrainio@active  ''' % ( self._name ))
                time.sleep( 20 )

    def tearDown( self ):
        print('\n====RegDelProd destroy====')
