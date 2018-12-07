#!/usr/bin/env python
import json
import requests

url = "http://127.0.0.1:8888/v1/chain/register_event"
#content = [{"ref_block_num":21453,"ref_block_prefix":3165644999,"expiration":"2017-12-08T10:28:49","scope":["initb","initc"],"read_scope":[],"messages":[{"code":"currency","type":"transfer","authorization":[{"account":"initb","permission":"active"}],"data":"000000008093dd74000000000094dd74e803000000000000"}],"signatures":[]}, ["UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"], ""]

content = {"account":"hello","post_url":"http://127.0.0.1:3000/v1/listen_event"}
json_content = json.dumps(content)
print json_content
r=requests.post(url,data=json_content)
print r.text
