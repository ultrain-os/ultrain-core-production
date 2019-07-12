#coding=utf-8
import unittest
import subprocess
import time
import requests
import json
import sys
import datetime
sys.path.append("..")
from config import *
from general_func import *

class Multichain(unittest.TestCase):
    name = "tester.11"
    pk = "UTR7aBDriMhNuHaySQknDGS4auB8RXAE1oQp3ZE9wo51PrUYauox1"
    sk = "5JajpWwvGem8TLfxkTDDnxiMC11VaLioaFCX56d4mYxwyBLPhy6"

    accounts = [
        ".111",
        ".112",
        ".113",
        ".114",
        ".115",
    ]
    pk_list = [
        "b7e0a16fdca44d4ece1b14d8e7e6207402a6447115ca7d2d7edb08958e6d8ed5",
        "4031a95071a092eca8646d3999c438bdfde368d4837770755af65a11b4520a48",
        "2b43a3d8e0523a85141bbfca41006cf9abc47587d0ff3c46d257551ec05f1677",
        "b9a55c3c661abd8c539b7a7c05c8176036f87aeeb6b117a138327cfdb374cc23",
        "8f6e3b3336276138023617f0ae0e6fd0c37d27aa9995f9803fe78df4941dd3ec",
    ]

    bls_pk_list = [
        "3377255e474a9aa9caa13034d540200914970f98574538d5370d4958fc30a52c6cc2a41bf47b5828483c63e5d0b3d407ab4354c631fc0a57506df9a675d0c1c100",
        "4d07f24635343f83d99ec2c95a5596d0a830c040e55c9d4273a1bc891501d66ac0b3e1b2ac6639c32f279bdafb8f1f6ee4c71dcb07e066d00e4d3fb4524f066901",
        "20d392f9813d86534b8e9547a8f83902c1254ae82a5f59f1cffcaf57cf93fb43dacb03fe12863c9912b0321faadbb4f2d5e353dff4913d1f8b56390d16b94a1701",
        "4f53d674913b202d35b1e864adc35f133e457b78cf572ce3aee8fa95d296e52141e64c4cfc6895217bd63ffdc2263434cf72a89c54d5ca1f98bd1b45c6d671b300",
        "8a4e44c5240b45cec1665cbea836887fc96aa6eb5515a0e83f257aa593bb0721cd8b56af9d1f6a8b11bbff1f901186274771d3a7a40b23ae661aad247f1705b300",
    ]

    account_pk_list = [
        "UTR6r3BNrssrD9F5jJo17aszYN3S7mrYcCYqXGDgaEB1WPgF9LVfe",
        "UTR6BThE2vci6Jqq5YyfYCy8mHMdk14Dfx9n5RWfkL8jcMDW7PJ5b",
        "UTR5kNr7rCgkkkyqXXZh51ZAHm8QbvhhX1KHH6wT4zzVMi7w8miZz",
        "UTR7bggm3iyTDmZMHfsnZ96N8wQnGmam6x97uuTYtxgSLWR3zyCix",
        "UTR8YCCmCEAM4GGqhSwQTB4SwP3J4sJTs721cAjZ6AXHr55qbig29",
    ]

    account_sk_list = [
        "5JfSeQ3Jr8RyL7HThWsgctLmovDHGhBCnm7e6dQo3rV3TQ73oHB",
        "5KXoGjfRKb6qiJUR29jDPrt9DpUXPTMS1gpJpoSH8iN6axak89g",
        "5KSfov4mshaB2pqd74jT7FZFSRitpFnuoqw1jpy2WSB42BpyzD9",
        "5JYFkZtDzXM3xvvh8DaT6oYNx4KSpRNh16cb9ZGFin9KuUgaoW7",
        "5HryvWgTq235oBMcEpWZdbJKhzZvfz2s7iWWKcWdj1eV7trbe2e",
    ]

    #创建账号
    def create_account( self ):
        execcommand = Config.clultrain_path + 'create account ultrainio %s %s %s ' % (self.name, self.pk, self.pk)
        cmd_exec( execcommand )

    #销毁账号
    def destroy_account( self ):
        execcommand = Config.clultrain_path + 'push action ultrainio  delaccount \'["%s"]\' --delay-sec 30 -p ultrainio' % ( self.name )
        cmd_exec( execcommand )

    def import_key( self ):
        execcommand = Config.clultrain_path + 'wallet import --private-key %s' % ( self.sk )
        cmd_exec( execcommand )

    def create_sidechain( self ):
        execcommand = Config.clultrain_path + 'push action ultrainio regchaintype \'{"type_id":"1", "min_producer_num":"4", "max_producer_num":"30", "sched_step":"4", "consensus_period":"10"}\'  -p ultrainio@active'
        cmd_exec( execcommand )

        for chain in Config.chain_name:
            j = json.loads(requests.get(Config.get_table_url, data = json.dumps({"code":"ultrainio","scope":"ultrainio","table":"chains","json":"true","table_key":chain})).text)
            if len(j["rows"]) == 1:
                print('\n chain %s has been created' %chain)
            else:
                execcommand = Config.clultrain_path + 'push action ultrainio regsubchain \'{"chain_name":"%s", "chain_type":"1", "genesis_producer_pubkey":"369c31f242bfc5093815511e4a4eda297f4b8772a7ff98f7806ce7a80ffffb35"}\' -p ultrainio@active' % (chain)
                cmd_exec( execcommand )
        sleep(11)

    def clear_sidechain( self ):
        for chain in Config.chain_name:
            execcommand = Config.clultrain_path + 'push action ultrainio clearchain \'{"chain_name":"%s", "users_only":"0"}\' -p ultrainio@active' % (chain)
            cmd_exec( execcommand )
        sleep(10)

    def create_producer_account( self ):
        for chain in Config.chain_name:
            for i in range(5):
                account_name = chain + self.accounts[i]
                j = json.loads(requests.get(Config.get_account_info_url,data = json.dumps({"account_name":account_name})).text)
                if "account_name" in j:
                    print('\naccount %s has be created' %account_name)
                elif "error" in j:
                    execcommand = Config.clultrain_path + 'create account ultrainio %s %s %s ' % (account_name, self.account_pk_list[i], self.account_pk_list[i])
                    cmd_exec( execcommand )
                    cmd_exec( Config.clultrain_path + 'wallet import --private-key %s' % ( self.account_sk_list[i]))
                    print('\ncreate account %s' %account_name)
        sleep(20)

    def reg_sidechain_producer( self ):
        for chain in Config.chain_name:
            for i in range(5):
                account_name = chain + self.accounts[i]
                execcommand = Config.clultrain_path + 'transfer ultrainio %s "10.0000 UGAS"' %account_name
                cmd_exec( execcommand )
        sleep(20)
        for chain in Config.chain_name:
            for i in range(5):
                account_name = chain + self.accounts[i]
                for location in Config.chain_name:
                    execcommand = Config.clultrain_path + 'system empoweruser %s %s %s %s 1' % (account_name, location, self.account_pk_list[i], self.account_pk_list[i])
                    cmd_exec( execcommand )
        sleep(20)
        for chain in Config.chain_name:
            for i in range(5):
                account_name = chain + self.accounts[i]
                execcommand = Config.clultrain_path + 'system regproducer %s %s %s %s https://user.%s.com %s -p ultrainio@active -u' % (account_name, self.pk_list[i], self.bls_pk_list[i], account_name, account_name, chain)
                cmd_exec( execcommand )
        sleep(20)
        for chain in Config.chain_name:
            for i in range(5):
                account_name = chain + self.accounts[i]
                execcommand = Config.clultrain_path + 'system delegatecons ultrainio %s "42000.0000 UGAS" -p ultrainio@active' %account_name
                cmd_exec( execcommand )
        sleep(10)

    def unreg_sidechain_producer( self ):
        for chain in Config.chain_name:
            for i in range(5):
                account_name = chain + self.accounts[i]
                execcommand = Config.clultrain_path + 'system undelegatecons ultrainio %s -p ultrainio@active' %account_name
                cmd_exec( execcommand )
        sleep(15)
        for chain in Config.chain_name:
            for i in range(5):
                account_name = chain + self.accounts[i]
                execcommand = Config.clultrain_path + 'push action ultrainio claimrewards \'{"producer":"%s"}\' -p %s@active' % (account_name, account_name)
                cmd_exec( execcommand )
        sleep(10)

    def empower_user( self ):
        for a in Config.chain_name:
            execcommand = Config.clultrain_path + 'system empoweruser %s %s %s %s 1' % (self.name, a, self.pk, self.pk)
            cmd_exec( execcommand )

    def resource_lease( self ):
        execcommand = Config.clultrain_path + 'system resourcelease ultrainio %s 80 1 %s' % (self.name, Config.chain_name[0])
        cmd_exec( execcommand )
        sleep(10)

    def transfer_to_bank( self, src = "master", chain_index = 0):
        if src == "master":
            execcommand = Config.clultrain_path + 'transfer %s.111 utrio.bank "10.0000 UGAS" %s' % (Config.chain_name[chain_index], Config.chain_name[chain_index])
            cmd_exec( execcommand )
        else:
            execcommand = Config.clultrain_path + '--url %s transfer %s.111 utrio.bank "10.0000 UGAS" ultrainio' % (Config.node_url[chain_index], Config.chain_name[chain_index])
            cmd_exec( execcommand )
        #wait enough time for lws sync
        sleep(25)

    def move_producer( self, prod_base, producer_index, src_chain_index, dest_chain_index):
        execcommand = Config.clultrain_path + 'push action ultrainio moveprod \'{"producer":"%s", "producerkey":%s, "blskey":%s, "from_disable":"0", "from_chain":"%s", "to_disable":"0", "to_chain":"%s"}\' -p ultrainio@active' % ((prod_base + self.accounts[producer_index]), self.pk_list[producer_index], self.bls_pk_list[producer_index], Config.chain_name[src_chain_index], Config.chain_name[dest_chain_index])
        cmd_exec( execcommand )
        sleep(25)

    def enable_schedule( self ):
        execcommand = Config.clultrain_path + 'push action ultrainio setsched \'{"is_enabled":"1", "sched_period":"60", "expire_time":"15"}\' -p ultrainio@active'
        cmd_exec( execcommand )
        sleep(10)

    def disable_schedule( self ):
        execcommand = Config.clultrain_path + 'push action ultrainio setsched \'{"is_enabled":"0", "sched_period":"60", "expire_time":"15"}\' -p ultrainio@active'
        cmd_exec( execcommand )
        sleep(10)

    # 测试轻客户端同步
    def test_lwc_sync( self ):
        # test account sync
        print('\n====Account sync====')
        self.empower_user()
        j = json.loads(requests.get(Config.get_account_info_url,data = json.dumps({"account_name":self.name})).text)
        self.assertIn("account_name", j)
        self.assertEqual( j["account_name"] , self.name )
        self.assertEqual( j["ram_quota"], 0 )
        self.assertEqual( j["updateable"], True )
        self.assertEqual( j["privileged"], False )
        self.assertEqual( j["permissions"][0]["required_auth"]["keys"][0]["key"] , self.pk )
        self.assertEqual( j["permissions"][1]["required_auth"]["keys"][0]["key"] , self.pk )
        #leave enough time for light weight client transaction sync
        sleep( 20 )
        for url in Config.node_url:
            get_sidechain_account_url = url + 'v1/chain/get_account_info'
            j = json.loads(requests.get(get_sidechain_account_url, data = json.dumps({"account_name":self.name})).text)
            self.assertIn("account_name", j)
            self.assertEqual( j["account_name"] , self.name )
            self.assertEqual( j["ram_quota"], 0 )
            self.assertEqual( j["updateable"], True )
            self.assertEqual( j["privileged"], False )
            self.assertEqual( j["permissions"][0]["required_auth"]["keys"][0]["key"] , self.pk )
            self.assertEqual( j["permissions"][1]["required_auth"]["keys"][0]["key"] , self.pk )

        # test resource lease on side chain
        print('\n====Resource sync====')
        self.resource_lease()
        j = json.loads(requests.get(Config.get_account_info_url,data = json.dumps({"account_name":self.name})).text)
        self.assertIn("account_name", j)
        self.assertEqual( j["account_name"] , self.name )
        found = False
        for location in j["chain_resource"]:
            if location["chain_name"] == Config.chain_name[0]:
                found = True
                self.assertEqual( location["lease_num"], 80 )
                d_end = datetime.datetime.strptime(location["end_time"][:-4], "%Y-%m-%dT%H:%M:%S")
                d_start = datetime.datetime.strptime(location["start_time"][:-4], "%Y-%m-%dT%H:%M:%S")
                delta = d_end - d_start
                self.assertGreaterEqual( delta.days, 1)
                break
        self.assertTrue(found)
        get_sidechain_account_url = Config.node_url[0] + 'v1/chain/get_account_info'
        j = json.loads(requests.get(get_sidechain_account_url, data = json.dumps({"account_name":self.name})).text)
        self.assertIn("account_name", j)
        self.assertEqual( j["account_name"] , self.name )
        self.assertGreater( j["ram_quota"],  100)

        # test token sync from master to sidechain
        print('\n====Token sync from master to sidechain====')
        amount = '0.0000 UGAS'
        get_sidechain_account_url = Config.node_url[0] + 'v1/chain/get_account_info'
        j = json.loads(requests.get(get_sidechain_account_url, data = json.dumps({"account_name":(Config.chain_name[0]+'.111')})).text)
        self.assertIn("account_name", j)
        self.assertEqual( j["account_name"] , self.name )
        if "core_liquid_balance" in j:
            amount = j["core_liquid_balance"]

        self.transfer_to_bank("master", 0)
        get_sidechain_account_url = Config.node_url[0] + 'v1/chain/get_account_info'
        j = json.loads(requests.get(get_sidechain_account_url, data = json.dumps({"account_name":(Config.chain_name[0]+'.111')})).text)
        self.assertIn("core_liquid_balance", j)
        self.assertGreater(j["core_liquid_balance"], amount)

        # test token sync from sidechain to master
        print('\n====Token sync from sidechain to master====')
        self.transfer_to_bank(Config.chain_name[0], 0)
        get_sidechain_account_url = Config.node_url[0] + 'v1/chain/get_account_info'
        j = json.loads(requests.get(get_sidechain_account_url, data = json.dumps({"account_name":(Config.chain_name[0]+'.111')})).text)
        self.assertIn("account_name", j)
        if "core_liquid_balance" in j:
            self.assertEqual(j["core_liquid_balance"], amount)
        else:
            self.assertEqual(amount, '0.0000 UGAS')

        #test committee sync
        print('\n====Committee sync====')
        producer_from_chain = Config.chain_name[0]
        self.move_producer(producer_from_chain, 0, 0, 1)
        producer = producer_from_chain + self.accounts[0]
        j = json.loads(requests.get(Config.get_table_url, data = json.dumps({"code":"ultrainio","scope":Config.chain_name[0],"table":"producers","json":"true","table_key":producer})).text)
        self.assertEqual(len(j["rows"]), 0)
        j = json.loads(requests.get(Config.get_table_url, data = json.dumps({"code":"ultrainio","scope":Config.chain_name[1],"table":"producers","json":"true","table_key":producer})).text)
        self.assertEqual(len(j["rows"]), 1)
        get_sidechain1_table_url = Config.node_url[0] + 'v1/chain/get_table_records'
        j = json.loads(requests.get(get_sidechain1_table_url, data = json.dumps({"code":"ultrainio","scope":"ultrainio","table":"producers","json":"true","table_key":producer})).text)
        self.assertEqual(len(j["rows"]), 0)
        get_sidechain2_table_url = Config.node_url[1] + 'v1/chain/get_table_records'
        j = json.loads(requests.get(get_sidechain2_table_url, data = json.dumps({"code":"ultrainio","scope":"ultrainio","table":"producers","json":"true","table_key":producer})).text)
        self.assertEqual(len(j["rows"]), 1)
        self.move_producer(producer_from_chain, 0, 1, 0)
        j = json.loads(requests.get(Config.get_table_url, data = json.dumps({"code":"ultrainio","scope":Config.chain_name[0],"table":"producers","json":"true","table_key":producer})).text)
        self.assertEqual(len(j["rows"]), 1)
        j = json.loads(requests.get(Config.get_table_url, data = json.dumps({"code":"ultrainio","scope":Config.chain_name[1],"table":"producers","json":"true","table_key":producer})).text)
        self.assertEqual(len(j["rows"]), 0)
        j = json.loads(requests.get(get_sidechain1_table_url, data = json.dumps({"code":"ultrainio","scope":"ultrainio","table":"producers","json":"true","table_key":producer})).text)
        self.assertEqual(len(j["rows"]), 1)
        j = json.loads(requests.get(get_sidechain2_table_url, data = json.dumps({"code":"ultrainio","scope":"ultrainio","table":"producers","json":"true","table_key":producer})).text)
        self.assertEqual(len(j["rows"]), 0)

    def setUp( self ):
        print('\n====Multi chain test init====')
        self.create_sidechain()
        self.create_account()
        self.create_producer_account()
        self.reg_sidechain_producer()
        self.import_key()
        self.enable_schedule()
        sleep(20)

    def tearDown( self ):
        print('\n====Multi chain test destroy====')
        self.disable_schedule()
        self.unreg_sidechain_producer()
        self.clear_sidechain()
        self.destroy_account()
