from fabric.api import *

with open('hosts.txt') as f:
    env.hosts = f.read().splitlines()

env.user = 'root'
env.password = 'Uranus12#$'
env.warn_only = True

def deploy():
    put("deploy.tar")
    run("sh extra.sh")
