#!/usr/bin/env python
#coding=utf-8
from pymongo import MongoClient
import json
import requests
import os
from email.header import Header
from email.mime.text import MIMEText
import smtplib
import time

from settings import *
logFile = open("output.log", 'a')

logFile.write('\n\n' + '*' * 80)
logFile.write("\n current time:%s" % time.strftime('%Y.%m.%d %H:%M:%S ',time.localtime(time.time())))

class MyMongoDB(object):
    def __init__(self, mongoinfo):
        try:
            #建立和数据库系统的连接,指定host及port参数
            self.conn = MongoClient(mongoinfo["ip"], mongoinfo["port"])        
        except Exception as e:
            print(e)
        self.db = self.conn[mongoinfo["db_name"]]
        #连接mydb数据库,账号密码认证
        self.db.authenticate(mongoinfo["account"], mongoinfo["passward"])
        #连接表
        self.my_set = self.db[mongoinfo["set_name"]]

    def insert(self,dic):
        print("inser...")
        #self.my_set.insert(dic)

    def update(self,dic,newdic):
        print("update...")
        #self.my_set.update(dic,newdic)

    def delete(self,dic):
        print("delete...")
        #self.my_set.remove(dic)

    def dbfind(self,dic):
        print("find...")
        data = self.my_set.find(dic)
        # for result in data:
        #     print(result)
        #     print(result["block_num"],result["block"]["proposer"])
        return data
def readfile(fname):
	fileold = open(fname, "r")
	content = fileold.readlines()
	fileold.close()
	return content

def writefile(fname,content):
	filenew = open(fname, "w")
	filenew.writelines(content)
	filenew.close()


def sendEmail(msg,successFlag):
    """
    邮件通知
    :param str: email content
    :return:
    """
    receiver = "yanhuichao@ultrain.io"#"#,suyu@ultrain.io"

    try:
        sender = "739884701@qq.com"
        subject = '测试网矿工出块奖励检查'
        status = "--告警"
        if successFlag == True :
            status = "--正常"
        subject = subject+status;
        username = "739884701"
        password = "cfiawwwcqltbbcic"
        host = "smtp.qq.com"
        s = "{0}".format(msg)

        msg = MIMEText(s, 'plain', 'utf-8')  # 中文需参数‘utf-8’，单字节字符不需要
        msg['Subject'] = Header(subject, 'utf-8')
        msg['From'] = sender
        msg['To'] = receiver

        smtp = smtplib.SMTP_SSL()
        smtp.connect(host)
        smtp.login(username, password)
        smtp.sendmail(sender, receiver.split(","), msg.as_string())
        smtp.quit()
        print(" email send success")
    except Exception as e:
        print(" email error", e)

class RewardsTest(object):
    def __init__(self):
        #配置文件快照
        self.cfgDict = {}
        #链上信息
        self.chainProdDict = {}
        #记录文件快照和链上信息差值
        self.masterChainBlock = {}

        #mongo信息
        self.mongoChainBlock = {}
        self.chainEmptyNum = {}
        self.chainProduceNum = {}
        self.sendMsgInfo = ""
        self.isException = False
    def ReadConfigInfo( self ):
        if not os.path.isfile("record.json") :
            return
        load_dict = None
        with open("record.json",'r') as load_f:
            load_dict = json.load(load_f)
            json_str = json.dumps(load_dict)
            print(json_str)
            logFile.write('\n record.json:'+json_str+'\n')
        self.cfgDict = load_dict

    def WriteConfigInfo( self ):
        if os.path.isfile("record.json") :
            os.system("cp record.json  record_bak.json")
        #dumps 将数据转换成字符串
        json_str = json.dumps(self.chainProdDict)
        print(json_str)
        print(type(json_str))
        # #load 将字符串转换成字典
        # new_dict = json.loads(json_str)
        # print(new_dict)
        # print(type(new_dict))
        with open("record.json","w") as f:
            json.dump(self.chainProdDict,f)
            print("write success...")

    def GetProducerList( self ):
        self.chainProdDict["time"] = time.time()
        self.chainProdDict["chain"] = {}
        for chain_name,url in testNetsideHttp.items():
            chainInfoJson = json.loads(requests.get(testNetmasterHttp["ultrainio"]+"/v1/chain/get_table_records",data = json.dumps({"code":"ultrainio","scope":"ultrainio","table":"chains","table_key":chain_name,"json":"true"})).text)

            producerInfoJson = json.loads(requests.get(testNetmasterHttp["ultrainio"]+"/v1/chain/get_producers",data = json.dumps({"all_chain":"false","chain_name":chain_name,"json":"true"})).text)

            self.chainProdDict["chain"][chain_name] = {"confirmblock":chainInfoJson["rows"][0]["confirmed_block_number"],"prod_size":chainInfoJson["rows"][0]["committee_num"],"producers":producerInfoJson}
        json_str = json.dumps(self.chainProdDict)
        print(json_str)
        logFile.write('\n GetProducerList:'+json_str+'\n')

    def chainSnapshotSub( self ):
        masterChainBlock = {}
        for chain_name in testNetsideHttp.keys():
            for prodInfo in self.chainProdDict["chain"][chain_name]["producers"]["rows"]:
                producer = prodInfo["prod_detail"]["owner"]
                masterChainBlock[producer] = {}
                masterChainBlock[producer]["unpaid_balance"] = prodInfo["prod_detail"]["unpaid_balance"]
                masterChainBlock[producer]["block_num"] = prodInfo["prod_detail"]["total_produce_block"]
                masterChainBlock[producer]["chain_name"] = chain_name

        for chain_name in testNetsideHttp.keys():
            for prodInfo in self.cfgDict["chain"][chain_name]["producers"]["rows"]:
                producer = prodInfo["prod_detail"]["owner"]
                if producer in masterChainBlock.keys():
                    masterChainBlock[producer]["unpaid_balance"] = masterChainBlock[producer]["unpaid_balance"] - prodInfo["prod_detail"]["unpaid_balance"]
                    masterChainBlock[producer]["block_num"] = masterChainBlock[producer]["block_num"] - prodInfo["prod_detail"]["total_produce_block"]
        print( "........ chainSnapshotSub  .........")
        json_str = json.dumps(masterChainBlock)
        print(json_str)
        logFile.write('\n ........ chainSnapshotSub  .........\n'+json_str+'\n')
        self.sendMsgInfo += "\n 开始检测时间:%s (上次快照时间)" % time.strftime('%Y.%m.%d %H:%M:%S ',time.localtime(self.cfgDict["time"]))
        self.sendMsgInfo += "\n 结束检测时间:" + time.strftime('%Y.%m.%d %H:%M:%S ',time.localtime(self.chainProdDict["time"]))
        self.sendMsgInfo += "\n 根据时间间隔应当出块数:%d   跟实际出块数可能会有误差，可能已出块有提交块头，但未确认" % ((int(self.chainProdDict["time"]) - int(self.cfgDict["time"]))/10)
        allowProduceBlock = {}
        for chain_name in testNetsideHttp.keys():
            chain_block_sub = int(self.chainProdDict["time"] - self.cfgDict["time"])/10
            prod_size = self.chainProdDict["chain"][chain_name]["prod_size"]
            allowProduceBlock[chain_name] = {}
            allowProduceBlock[chain_name]["max_size"] = chain_block_sub/prod_size*1.2
            allowProduceBlock[chain_name]["min_size"] = chain_block_sub/prod_size*0.8
            self.sendMsgInfo += "\n 链名:%s, 矿工数量:%d, 平均出块数量:%d " %( chain_name, prod_size, chain_block_sub/prod_size)

        self.sendMsgInfo += "\n 统计矿工出块详情:\n"
        for key,info in masterChainBlock.items():
            print(info["block_num"])
            self.sendMsgInfo += "    矿工:%s ,所在链:%s,出块数量:%d  待发奖励:%.4f UGAS " %(key,info["chain_name"], info["block_num"], info["unpaid_balance"]/10000)
            if info["block_num"] > allowProduceBlock[info["chain_name"]]["max_size"]:
                self.sendMsgInfo += "该矿工出块超出平均值20%，请及时关注\n"
                self.isException = True
            elif info["block_num"] < allowProduceBlock[info["chain_name"]]["min_size"]:
                self.sendMsgInfo += "该矿工出块低于平均值20%，请及时关注\n"
                self.isException = True
            else:
                self.sendMsgInfo += "\n"

        self.masterChainBlock = masterChainBlock

    def getMongoBlockList( self ):
        for chain_name,url in testNetsideHttp.items():
            mongo = MyMongoDB( mongochain[chain_name] )
            startblock = self.cfgDict["chain"][chain_name]["confirmblock"]
            endblock = self.chainProdDict["chain"][chain_name]["confirmblock"]
            print( " getMongoBlockList chain_name:%s startblock:%d,endblock:%d" %(chain_name, startblock, endblock))
            mongoList = mongo.dbfind({"block_num":{"$gte": startblock+1, "$lte": endblock}})
            for info in mongoList:
                if not chain_name in self.chainEmptyNum.keys():
                    self.chainEmptyNum[chain_name] = 0
                if not chain_name in self.chainProduceNum.keys():
                    self.chainProduceNum[chain_name] = 0
                if info["block"]["proposer"] == "utrio.empty":
                    self.chainEmptyNum[chain_name] += 1
                self.chainProduceNum[chain_name] += 1
                if info["block"]["proposer"] in self.mongoChainBlock.keys():
                    self.mongoChainBlock[info["block"]["proposer"]] += 1
                else:
                    self.mongoChainBlock[info["block"]["proposer"]] = 1
        json_str = json.dumps(self.mongoChainBlock)
        print(json_str)
        print(self.mongoChainBlock)

    def compareProduceBlock( self ):
        masterChainBlock = self.masterChainBlock
        mongoChainBlock = self.mongoChainBlock
        self.sendMsgInfo += "\n\n 检查侧链mongo和主链统计出块是否一致及奖励是否正确:\n"
        produceErrorFlag = False
        for producer in masterChainBlock.keys():
            if not producer in mongoChainBlock.keys():
                mongoChainBlock[producer] = 0
            print("mongo producer:%s,number:%d" %(producer,mongoChainBlock[producer]))
            if masterChainBlock[producer]["block_num"] != mongoChainBlock[producer]:
                self.isException = True
                produceErrorFlag = True
                self.sendMsgInfo += " 出块不一致 矿工:%s, 主链统计出块:%d, mongo统计出块:%d \n"%( producer,masterChainBlock[producer]["block_num"], mongoChainBlock[producer] )
                print(" compareProduceBlock error producer:%s,masterblock:%d,mogoblock:%d"%( producer,masterChainBlock[producer]["block_num"],mongoChainBlock[producer] ))
            if masterChainBlock[producer]["block_num"] *10000*0.99 != masterChainBlock[producer]["unpaid_balance"]:
                self.isException = True
                produceErrorFlag = True
                print(" compareProduceBlock unpaidbalance error producer:%s,masterblock:%d,mogoblock:%d"%( producer,masterChainBlock[producer]["block_num"],masterChainBlock[producer]["unpaid_balance"] ))
                self.sendMsgInfo += " 奖励比例不一致 矿工:%s,主链块数:%d,奖励值:%d \n"%( producer,masterChainBlock[producer]["block_num"],masterChainBlock[producer]["unpaid_balance"] )
        if not produceErrorFlag :
            self.sendMsgInfo += "    侧链mongo和主链统计出块数量相等 奖励正确\n"
    def getProduceEmptyInfo( self ):
        self.sendMsgInfo += "\n\n 检查侧链出空块数量:"
        for chain_name,num in self.chainEmptyNum.items():
            self.sendMsgInfo += "\n    链名:%s,总出块数量:%d,出空块数量:%d" % (chain_name,self.chainProduceNum[chain_name],num)

def main():
    rewardTest = RewardsTest()

    #读配置文件快照
    rewardTest.ReadConfigInfo()
    print("\n\n ........ ................\n\n")
    #查询链上信息
    rewardTest.GetProducerList()
    #记录到配置文件
    #rewardTest.WriteConfigInfo()
    if not rewardTest.cfgDict:
        logFile.write('\n reward.json have not data\n')
        exit(0)
    print("\n\n ........ ................\n\n")
    #记录文件快照和链上信息差值
    rewardTest.chainSnapshotSub()
    print("\n\n ........ ................\n\n")
    #查询mongo信息
    rewardTest.getMongoBlockList()
    print("\n\n ........ ................\n\n")
    #比较出块是否一致 奖励是否正确 #检测出块数量是否异常
    rewardTest.compareProduceBlock()
    
    #检测出空块的数量
    rewardTest.getProduceEmptyInfo()
    #发送检测报告
    sendEmail( rewardTest.sendMsgInfo, not rewardTest.isException )
    
    #WriteConfigInfo(chainProdDict)


    

if __name__ == "__main__":
    main()