from fabric.api import *

with open('hosts-mongo.txt') as f:
    env.hosts = f.read().splitlines()

env.user = 'root'
env.password = 'Uranus12#$'
env.warn_only = True

def deploy():
    put("mongo.tar","/root/")
    run("tar -xvf mongo.tar")
    run("apt install libcurl3 libcurl-openssl1.0-dev")
