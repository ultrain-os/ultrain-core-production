from fabric.api import *
from fabric.contrib.files import exists

with open('hosts.txt') as f:
    env.hosts = f.read().splitlines()
env.user = 'root'
env.password = 'Uranus12#$'
env.warn_only = True

def deployall():
#    run('mkdir log')
#    put("/root/logrotate.sh","/root/")
#    run('chmod +x /root/logrotate.sh')
#    put("/root/runlogr.sh","/root/")
#    run('chmod +x /root/runlogr.sh')
    put("/root/runultrain-h.sh","/root/")
    run('chmod +x /root/runultrain-h.sh')

def updatefile():
    put("nodultrain","/root/")
    run('chmod +x /root/nodultrain')
    put("sideChainService.js","/root/ultrainmng/src/")
    run('chmod +x /root/ultrainmng/src/')

#@parallel
def deployfile():
    #run("mkdir -p /root/config")
    #put("/root/uploadconfig.sh","/root/config")
    put("nodultrain","/root/")
    run('chmod +x /root/nodultrain')
    put("wssultrain","/root/")
    run('chmod +x /root/wssultrain')
    run("rm -rf /root/ultrainmng")
    put("ultrainmng.tar","/root/")
    run("tar xf ultrainmng.tar")
    run("chmod +x /root/ultrainmng/tool/_runultrain.sh")
    run("chmod +x /root/ultrainmng/tool/_runworldstate.sh")
    #put("ultrainmng","/root/")
    #put("/root/restartdockers.sh","/root/")
    #run('chmod +x /root/restartdockers.sh')
    #put("/root/startdockers.sh","/root/")
    #run('chmod +x /root/startdockers.sh')
    #run('mkdir log')
    #put("/root/logrotate.sh","/root/")
    #run('chmod +x /root/logrotate.sh')
    #put("/root/runlogr.sh","/root/")
    #run('chmod +x /root/runlogr.sh')
    #put("/root/runultrain-h.sh","/root/")
    #run('chmod +x /root/runultrain-h.sh')
    #put("/root/runeos.sh","/root/")
    #put("/root/stopNod.sh","/root/")
    #put("/root/installsoft.sh","/root/")
    #run('chmod +x /root/installsoft.sh')
    #run('/root/installsoft.sh')
    #run('brctl stp docker_gwbridge on')
    #run('brctl stp docker0 on')
    #put('/root/libstdc++.so.6.0.25','/usr/lib/x86_64-linux-gnu/')
    #run('rm /usr/lib/x86_64-linux-gnu/libstdc++.so.6')
    #run('ln /usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.25 /usr/lib/x86_64-linux-gnu/libstdc++.so.6')
    #run('cp /usr/share/zoneinfo/Asia/Shanghai /etc/localtime')

@parallel
def clearhostlog():
    with cd('/root/log'):
         run('rm -rf *')

@parallel
def clearhostdata():
    run('rm -rf /root/.local/share/ultrainio/nodultrain/data/')
    run('rm -rf /root/.local/share/ultrainio/wssultrain/data/worldstate')

@parallel
def clearhostenv():
    clearhostlog()
    clearhostdata()

@parallel
def starthosts():
    run("/root/runultrain-h.sh && sleep 1")
    run("/root/runlogr.sh && sleep 1")

    #run("pm2 start /root/ultrainmng/src/sideChainService.js && sleep 1")

@parallel
def startmng():
    run("pm2 start /root/ultrainmng/src/sideChainService.js && sleep 1")

@parallel
def upgrademng():
    run("rm -rf /root/ultrainmng")
    put("ultrainmng.tar","/root/")
    run("tar xf ultrainmng.tar")
    run("chmod +x /root/ultrainmng/tool/_runultrain.sh")
    run("chmod +x /root/ultrainmng/tool/_runworldstate.sh")
    put("ultrainmng/seedconfig.json", "/root/.local/share/ultrainio/ultrainmng/config/seedconfig.json")
    put("ultrainmng/config.ini", "/root/.local/share/ultrainio/ultrainmng/config/config.ini")
    run("pm2 restart /root/ultrainmng/src/sideChainService.js && sleep 1")

@parallel
def stophosts():
    run('killall -2 nodultrain')
    run('killall -2 wssultrain')
    run("killall logrotate.sh")
    run("pm2 stop sideChainService")

@parallel
def getIPs():
    run("for i in `docker ps | grep uranus | awk '{print $1}'`; \
            do echo $i; \
            docker inspect $i -f '{{.Config.Hostname}}';\
            docker inspect $i -f '{{.NetworkSettings.Networks.globalnet.IPAddress}}';\
            done > %s.txt" % env.host)
    get("%s.txt" % env.host,"IPs")

#@parallel
def startdockers():
    run("/root/startdockers.sh 2")

#@parallel
def restartdockers():
    run("/root/restartdockers.sh")

@parallel
def stopdockers():
    run("/root/stopdockers.sh")

@parallel
def startnods():
    run("/root/startNod.sh")
    run("/root/runlogr.sh && sleep 1")

@parallel
def stopnods():
    run("/root/stopNod.sh")
    run("killall logrotate.sh")

@parallel
def clearnodsdata():
    run("/root/cleardata.sh")

@parallel
def impconfig():
    run("rm /root/config/*.ini")
    put("config/%s/*" % env.host,"/root/config/")
    put("/root/config/uploadconfig.sh","/root/config/")
    run('chmod +x /root/config/uploadconfig.sh')
    run("/root/config/uploadconfig.sh")

@parallel
def uploadconfig():
    run("mkdir -p /root/.local/share/ultrainio/nodultrain/config")
    put("config/%s/config.ini" % env.host, "/root/.local/share/ultrainio/nodultrain/config")
    run("mkdir -p /root/.local/share/ultrainio/wssultrain/config")
    put("wssconfig.ini", "/root/.local/share/ultrainio/wssultrain/config/config.ini")
    run("mkdir -p /root/.local/share/ultrainio/ultrainmng/config")
    put("ultrainmng/config.ini", "/root/.local/share/ultrainio/ultrainmng/config/config.ini")
    put("ultrainmng/seedconfig.json", "/root/.local/share/ultrainio/ultrainmng/config/seedconfig.json")

@parallel
def joinswarm():
    run("docker swarm leave")
    put("/root/joinswarm.sh","/root/")
    run('chmod +x /root/joinswarm.sh')
    run("/root/joinswarm.sh")

def getlogs():
    get("/root/log/2018*","/root/logs")

def rename():
    run("mv /root/startEOS.sh /root/startNod.sh")
    run("mv /root/stopEOS.sh /root/stopNod.sh")

@parallel
def startlogr():
    run("/root/runlogr.sh && sleep 1")

@parallel
def stoplogr():
    run("killall logrotate.sh")

def setmss():
     run("sysctl net.ipv4.tcp_app_win=31")

def settcpwin():
     run("sysctl net.ipv4.tcp_app_win=31")
     run("sysctl net.core.wmem_max=4194304")
     run("sysctl net.core.rmem_max=4194304")
     run("sysctl -p")

def unsettcpwin():
     run("sysctl net.core.wmem_max=212992")
     run("sysctl net.core.rmem_max=212992")
     run("sysctl -p")

def getmss():
     run("sysctl -a|grep tcp_app_win")

def turnoffloadoff():
     run("ethtool -K eth0 tso off")
     run("ethtool -K eth0 gso off")
     run("ethtool -K eth0 gro off")

def turnoffloadon():
     run("ethtool -K ens5 tso on")
     run("ethtool -K ens5 gso on")
     run("ethtool -K ens5 gro on")

#@parallel
def installdep():
    run("apt update && apt install -y nodejs npm")
    run("npm install pm2 -g")

@parallel
def urgentmode():
    put("urgent.sh", "/root/")
    run('chmod +x /root/urgent.sh')
    run("/root/urgent.sh")

@parallel
def upreplayconfig():
    print("upload replay config");
    run("rm -f /root/replayultrain.sh")
    put("replayultrain.sh","/root/")
    run('chmod +x /root/replayultrain.sh')

@parallel
def replay(bn):
    print("replay to %s" % (bn))
    run("/root/replayultrain.sh %s && sleep 1" % (bn))
    run("/root/runlogr.sh && sleep 1")