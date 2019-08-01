import unittest
import subprocess
import time
import requests
import json
import sys
import string
sys.path.append("..")
from config import Config
from general_func import *

class MultiSig(unittest.TestCase):
    jack_acc = "jack"
    jack_pk = "UTR75rwpPD8YSTuT2vfVH1DgxiCve4YusCBKwEExZtHJHoVAjQj4a"
    jack_sk = "5KE91hYFAraYuK7tqJaknPPBNUbvsToZgNCLAXpK8cnJ4W5czKN"
    alice_acc = "alice"
    alice_pk = "UTR5566tjgd54VQVXbxSndBjTq3Xcg4UnvqGuTiR7uExaGHx3XLwY"
    alice_sk = "5KCWGqGiNJWVadCTTLUEf6RJZMyBHqNpEvG1MzL4Y93p7xEbT3Y"
    bob_acc = "bob"
    bob_pk = "UTR5Jq3BunAwFi7s7woBqa8dwcYGMbffuNyBEEwEyuwgGpnQamqWa"
    bob_sk = "5JaX93VhPa3grcH6y2n3T6TX5RYJTyqeutyLfyJ74METu3MysfL"

    name = "tom"
    pk = "UTR5nriDjfZWS8kNCZXKWLBPz9cMN7BA2YdgL89tDokmLAyQD4vto"
    sk = "5Jwc9cRfsFYDeK3Wt8qtDZ2x5fU63HaB3uJf7qrbvmoRV34Z8iQ"

    def create_account( self ):
        execcommand = Config.clultrain_path + 'create account ultrainio %s %s %s ' % (self.name, self.pk, self.pk)
        cmd_exec( execcommand )
        execcommand = Config.clultrain_path + 'create account ultrainio %s %s %s ' % (self.jack_acc, self.jack_pk, self.jack_pk)
        cmd_exec( execcommand )
        execcommand = Config.clultrain_path + 'create account ultrainio %s %s %s ' % (self.alice_acc, self.alice_pk, self.alice_pk)
        cmd_exec( execcommand )
        execcommand = Config.clultrain_path + 'create account ultrainio %s %s %s ' % (self.bob_acc, self.bob_pk, self.bob_pk)
        cmd_exec( execcommand )
        execcommand = Config.clultrain_path + 'create account ultrainio utriomsig %s %s ' % (self.pk, self.pk)
        cmd_exec( execcommand )
        sleep(20)
        execcommand = Config.clultrain_path + 'transfer ultrainio %s "5.2000 UGAS"' %self.jack_acc
        cmd_exec( execcommand )
        execcommand = Config.clultrain_path + 'transfer ultrainio utriomsig "5.0000 UGAS"'
        cmd_exec( execcommand )
        execcommand = Config.clultrain_path + 'transfer ultrainio %s "5.0000 UGAS"' %self.alice_acc
        cmd_exec( execcommand )
        execcommand = Config.clultrain_path + 'transfer ultrainio %s "5.0000 UGAS"' %self.bob_acc
        cmd_exec( execcommand )
        execcommand = Config.clultrain_path + 'transfer ultrainio %s "5.0000 UGAS"' %self.name
        cmd_exec( execcommand )
        execcommand = Config.clultrain_path + 'system resourcelease ultrainio tom 1 1 ultrainio'
        cmd_exec( execcommand )
        sleep(15)

    def destroy_account( self ):
        execcommand = Config.clultrain_path + 'push action ultrainio  delaccount \'["%s"]\' --delay-sec 30 -p ultrainio' % ( self.bob_acc )
        cmd_exec( execcommand )
        execcommand = Config.clultrain_path + 'push action ultrainio  delaccount \'["%s"]\' --delay-sec 30 -p ultrainio' % ( self.alice_acc )
        cmd_exec( execcommand )
        execcommand = Config.clultrain_path + 'push action ultrainio  delaccount \'["%s"]\' --delay-sec 30 -p ultrainio' % ( self.jack_acc )
        cmd_exec( execcommand )
        execcommand = Config.clultrain_path + 'push action ultrainio  delaccount \'["%s"]\' --delay-sec 30 -p ultrainio' % ( self.name )
        cmd_exec( execcommand )
        sleep(10)

    def import_key( self ):
        execcommand = Config.clultrain_path + 'wallet import --private-key %s' %self.sk
        cmd_exec( execcommand )
        execcommand = Config.clultrain_path + 'wallet import --private-key %s' %self.jack_sk
        cmd_exec( execcommand )
        execcommand = Config.clultrain_path + 'wallet import --private-key %s' %self.alice_sk
        cmd_exec( execcommand )
        execcommand = Config.clultrain_path + 'wallet import --private-key %s' %self.bob_sk
        cmd_exec( execcommand )

    def auth_permission( self ):
        execcommand = Config.clultrain_path + 'set account permission %s owner  \'{"threshold":2,"keys":[],"accounts":[{"permission":{"actor":"%s","permission":"owner"},"weight":1},{"permission":{"actor":"%s","permission":"owner"},"weight":1}],"waits":[]}\' -p %s@owner' %(self.jack_acc, self.alice_acc, self.bob_acc, self.jack_acc)
        cmd_exec( execcommand )
        sleep(15)

    def un_auth_permission( self ):
        execcommand = Config.clultrain_path + 'set account permission jack owner  %s -p jack@owner' %self.jack_pk
        cmd_exec( execcommand )
        sleep(15)

    def test_multi_sig( self ):
        tom_token = 0
        jack_token = 0
        j = json.loads(requests.get(Config.get_account_info_url,data = json.dumps({"account_name":"tom"})).text)
        self.assertIn("account_name", j)
        if "core_liquid_balance" in j:
            point_index = j["core_liquid_balance"].find('.')
            tom_token = int(j["core_liquid_balance"][:point_index])
        j = json.loads(requests.get(Config.get_account_info_url,data = json.dumps({"account_name":"jack"})).text)
        self.assertIn("account_name", j)
        self.assertIn("core_liquid_balance", j)
        point_index = j["core_liquid_balance"].find('.')
        jack_token = int(j["core_liquid_balance"][:point_index])

        execcommand = Config.clultrain_path + 'multisig propose nojack \'[{"actor":"alice","permission":"owner"},{"actor":"bob","permission":"owner"}]\' \'[{"actor":"jack","permission":"owner"}]\' utrio.token transfer \'{"from":"jack","to":"tom","quantity":"5.0000 UGAS","memo":"test multisig"}\'  -p utriomsig'
        cmd_exec( execcommand )
        sleep(20)
        j = json.loads(requests.get(Config.get_table_url, data = json.dumps({"code":"utrio.msig","scope":"utriomsig","table":"proposal","json":"true","table_key":"nojack"})).text)
        self.assertEqual(len(j["rows"]), 1)
        execcommand = Config.clultrain_path + 'multisig approve utriomsig nojack \'{"actor":"alice","permission":"owner"}\' -p alice@owner'
        cmd_exec( execcommand )
        sleep(30)
        execcommand = Config.clultrain_path + 'multisig exec utriomsig nojack -p tom'
        cmd_exec( execcommand )
        sleep(30)
        j = json.loads(requests.get(Config.get_table_url, data = json.dumps({"code":"utrio.msig","scope":"utriomsig","table":"proposal","json":"true","table_key":"nojack"})).text)
        self.assertEqual(len(j["rows"]), 1)
        execcommand = Config.clultrain_path + 'multisig approve utriomsig nojack \'{"actor":"bob","permission":"owner"}\' -p bob@owner'
        cmd_exec( execcommand )
        sleep(30)
        execcommand = Config.clultrain_path + 'multisig exec utriomsig nojack -p tom'
        cmd_exec( execcommand )
        sleep(30)
        j = json.loads(requests.get(Config.get_table_url, data = json.dumps({"code":"utrio.msig","scope":"utriomsig","table":"proposal","json":"true","table_key":"nojack"})).text)
        self.assertEqual(len(j["rows"]), 0)
        jack_new_token = 0
        tom_new_token = 0
        j = json.loads(requests.get(Config.get_account_info_url,data = json.dumps({"account_name":"jack"})).text)
        self.assertIn("account_name", j)
        if "core_liquid_balance" in j:
            point_index = j["core_liquid_balance"].find('.')
            jack_new_token = int(j["core_liquid_balance"][:point_index])
        else:
            jack_new_token = 0
        j = json.loads(requests.get(Config.get_account_info_url,data = json.dumps({"account_name":"tom"})).text)
        self.assertIn("account_name", j)
        self.assertIn("core_liquid_balance", j)
        point_index = j["core_liquid_balance"].find('.')
        tom_new_token = int(j["core_liquid_balance"][:point_index])

        #self.assertEqual(jack_new_token, jack_token - 5)
        self.assertEqual(tom_new_token, tom_token + 5)

    def setUp( self ):
        print('\n====MultiSig init====')
        self.create_account()
        self.import_key()
        self.auth_permission()

    def tearDown( self ):
        print('\n====MultiSig destroy====')
        #self.destroy_account()
        #self.un_auth_permission() 
