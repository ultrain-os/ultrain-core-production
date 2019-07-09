#coding=utf-8

class Config():
    clultrain_path = '/root/workspace/ultrain-core/build/programs/clultrain/clultrain --wallet-url http://127.0.0.1:6666 '
    base_url = 'http://127.0.0.1:8888/v1/chain'
    get_account_info_url = base_url + '/get_account_info'
    get_table_url = base_url + '/get_table_records'
    get_info_url = base_url + '/get_chain_info'