import threading
import numpy as np
import os
import sys
import httplib
import urllib
import random
import getopt

'''
usage:

python consensus_check.py logs_dir

    options:
        -v : post data to visual server
'''

const = {}
const['PHASE_FINAL'] = 0
const['PHASE_BA0'] = 1
const['PHASE_BA1'] = 2

const['PROPOSER'] = 1
const['VOTER'] = 2

const['HOST'] = '47.106.181.65'
const['PORT'] = 80
const['REQ_URL'] = 'http://' + const['HOST'] + ':' + str(const['PORT']) + '/api/block/add'
const['LOOP'] = True

g_timer = None
g_timer_interval = 10
g_block_list = []
g_block_id_index = 0
g_statistics_info = {
    'total_block_number': 0,
    'fork_block_number': 0,
    'fork_ratio': 0.0,
    'ba0_average_proposer_number': 0,
    'ba0_average_voter_number': 0,
    'ba1_average_voter_number': 0,
    'ba0_total_proposer_number': 0,
    'ba0_total_voter_number': 0,
    'ba1_total_voter_number': 0
}

class BaInfo:
    def __init__(self):
        self.proposer = []
        self.voter = []

    def to_dict(self):
        dict = {}
        dict['proposer'] = self.proposer
        dict['voter'] = self.voter
        return dict

class Block:
    def __init__(self):
        self.ba0 = BaInfo()
        self.ba1 = BaInfo()
        self.block_id = -1
        self.min_priority = -1
        self.timestamp = 0
        self.trx_num = random.randint(37000, 43000)
        self.block_hash = ''
        self.previous_block_hash = ''

    def to_dict(self):
        dict = {}
        dict['block_id'] = self.block_id
        dict['min_priority'] = self.min_priority
        dict['timestamp'] = self.timestamp
        dict['trx_num'] = self.trx_num
        dict['block_hash'] = self.block_hash
        dict['previous_block_hash'] = self.previous_block_hash
        self.ba0.proposer.sort()
        self.ba1.voter.sort()
        self.ba0.proposer.sort()
        self.ba1.voter.sort()
        dict['ba0'] = self.ba0.to_dict()
        dict['ba1'] = self.ba1.to_dict()
        return dict

class PhaseInfo:
    def __init__(self):
        self.block_id = -1
        self.phase = -1
        self.role = -1
        self.priority = 0
        self.hash = ''
        self.previous_block_hash = ''
        self.host_name = ''

class RoundInfo:
    def __init__(self, block_id, ba0, ba1, final):
        self.block_id = block_id
        self.ba0 = ba0
        self.ba1 = ba1
        self.final = final
        self.node_id = 0
    def is_complete(self):
        return self.final != None

class NodeIdManager:
    def __init__(self):
        self.next_id = 0
        self.host_name_node_id_dict = {}

    def get_node_id(self, host_name):
        if host_name in self.host_name_node_id_dict:
            return self.host_name_node_id_dict[host_name]
        self.host_name_node_id_dict[host_name] = self.next_id
        self.next_id += 1

g_node_id_manager = NodeIdManager()

class FileInfo:
    def __init__(self, name):
        self.name = name
        self.end = False
        self.start_block_id = -1
        self.fd = open(self.name, 'r')

    def is_end(self):
        return self.end

    def get_a_round(self, block_id):
        if self.end == True:
            return None
        round_info = RoundInfo(block_id, None, None, None)
        while True:
            line = self.fd.readline()
            if line == '':
                print ('%s reach the end' % (self.name))
                self.end = True
                return None
            phase_info = self.parse_one_line(line)
            if phase_info == None:
                continue
            if block_id == -1:
                round_info.block_id = phase_info.block_id
                block_id = phase_info.block_id
            if phase_info.block_id > block_id:
                c_line_num = self.fd.tell()
                self.fd.seek(c_line_num - 1)
                return None
            elif phase_info.block_id < block_id:
                continue
            else:
                if phase_info.phase == const['PHASE_BA0']:
                    round_info.ba0 = phase_info
                elif phase_info.phase == const['PHASE_BA1']:
                    round_info.ba1 = phase_info
                else:
                    round_info.final = phase_info
            if round_info.is_complete():
                round_info.node_id = g_node_id_manager.get_node_id(round_info.final.host_name)
                return round_info

    def get_start_block_id(self):
        if self.start_block_id != -1:
            return self.start_block_id
        round_info = self.get_a_round(-1)
        self.fd.seek(0)
        if round_info != None:
            self.start_block_id = round_info.block_id
            return round_info.block_id
        return -1

    def close(self):
        self.fd.close()

    def parse_one_line(self, line):
        index = line.find('checkpoint:')
        if index == -1:
            return None
        #print line
        phase_info = PhaseInfo()
        phase_info.block_id = int(self.get(line, 'block_id:'))
        phase_info.phase = int(self.get(line, 'phase:'))
        phase_info.role = int(self.get(line, 'role:'))
        phase_info.priority = np.uint64(self.get(line, 'priority:'))
        phase_info.hash = self.get(line, 'txs_hash:')
        phase_info.host_name = self.get(line, 'host_name:')
        phase_info.previous_block_hash = self.get(line, 'block_hash_previous:')
        return phase_info

    def get(self, line, key):
        index = line.find(key)
        if index == -1:
            return None
        start_index = len(key) + index
        if start_index == -1:
            return None
        end_index = line.find(';', start_index)
        value = line[start_index:end_index]
        return value.strip()

def close(file_info_list):
    for fileInfo in file_info_list:
        fileInfo.close()

def do_statistics():
    global g_statistics_info
    g_statistics_info['fork_ratio'] = float(g_statistics_info['fork_block_number']) / g_statistics_info['total_block_number']
    g_statistics_info['ba0_average_proposer_number'] = float(g_statistics_info['ba0_total_proposer_number']) / g_statistics_info['total_block_number']
    g_statistics_info['ba0_average_voter_number'] = float(g_statistics_info['ba0_total_voter_number']) / g_statistics_info['total_block_number']
    g_statistics_info['ba1_average_voter_number'] = float(g_statistics_info['ba1_total_voter_number']) / g_statistics_info['total_block_number']
    del g_statistics_info['ba0_total_proposer_number']
    del g_statistics_info['ba0_total_voter_number']
    del g_statistics_info['ba1_total_voter_number']
    return g_statistics_info

def parse(path):
    file_info_list = []
    if os.path.isdir(path) and os.path.exists(path):
        files = os.listdir(path)
        for f in files:
            abs_file = os.path.join(path, f)
            print abs_file
            if os.path.isfile(abs_file):
                fileInfo = FileInfo(abs_file)
                file_info_list.append(fileInfo)
    return file_info_list

def get_min_start_block_id(file_info_list):
    block_id = -1
    for fileInfo in file_info_list:
        this_block_id = fileInfo.get_start_block_id()
        if this_block_id == -1:
            continue
        if block_id == -1:
            block_id = this_block_id
        if this_block_id < block_id:
            block_id = this_block_id
    return block_id

def get_a_round(file_info_list, block_id):
    round_info_list = []
    for fileInfo in file_info_list:
        if block_id < fileInfo.get_start_block_id():
            continue
        round_info = fileInfo.get_a_round(block_id)
        if round_info != None:
            round_info_list.append(round_info)
    return round_info_list

def round_info_list_2_block(round_info_list):
    if len(round_info_list) == 0:
        return None
    global g_statistics_info
    g_statistics_info['total_block_number'] += 1
    hash_value_map = {}
    block = Block()
    min_priority_round_info = round_info_list[0]
    for round_info in round_info_list:
        if round_info.ba0 != None:
            if min_priority_round_info.ba0 != None and round_info.ba0.priority < min_priority_round_info.ba0.priority:
                min_priority_round_info = round_info
            if round_info.ba0.role == const['PROPOSER']:
                block.ba0.proposer.append(round_info.node_id)
            elif round_info.ba0.role == const['VOTER']:
                block.ba0.voter.append(round_info.node_id)
        if round_info.ba1 != None and round_info.ba1.role == const['VOTER']:
            block.ba1.voter.append(round_info.node_id)
        if hash_value_map.has_key(round_info.final.hash):
            hash_value_map[round_info.final.hash] += 1
        else:
            hash_value_map[round_info.final.hash] = 1
    if len(hash_value_map) > 1:
        g_statistics_info['fork_block_number'] += 1
        for hash, number in hash_value_map.items():
            print ('block_id : %d, hash : %s, number: %d'% (min_priority_round_info.block_id, hash, number))
    g_statistics_info['ba0_total_proposer_number'] += len(block.ba0.proposer)
    g_statistics_info['ba0_total_voter_number'] += len(block.ba0.voter)
    g_statistics_info['ba1_total_voter_number'] += len(block.ba1.voter)
    block.block_id = min_priority_round_info.block_id
    block.block_hash = min_priority_round_info.final.hash
    block.previous_block_hash = min_priority_round_info.final.previous_block_hash
    #block.block_hash = round_info_list[0].final.hash
    #block.previous_block_hash = round_info_list[0].final.previous_block_hash
    block.min_priority = min_priority_round_info.node_id
    return block

def post_block(block_dict):
    print ('post data : %s' % block_dict)
    headers = {'Content-Type': 'application/x-www-form-urlencoded'}
    conn = httplib.HTTPConnection(const['HOST'], const['PORT'])
    data_urlencode = urllib.urlencode(block_dict)
    try:
        conn.request(method='POST', url=const['REQ_URL'], body=data_urlencode, headers=headers)
        response = conn.getresponse()
        res= response.read()
        print res
    except Exception as err:
        print err
    finally:
        conn.close()

def time_fun():
    global g_timer
    global g_block_list
    global g_block_id_index
    global g_timer_interval
    block = Block()
    if const['LOOP'] == False:
        if g_block_id_index >= len(g_block_list):
            g_timer.cancel()
            return
        block = g_block_list[g_block_id_index]
    else:
        block = g_block_list[g_block_id_index % len(g_block_list)]
        block.block_id = g_block_id_index
    post_block(block.to_dict())
    g_block_id_index += 1
    g_timer = threading.Timer(g_timer_interval, time_fun)
    g_timer.start()

def do_visual(visual):
    if visual == True:
        print ('start post data to %s:%d' % (const['HOST'], const['PORT']))
        global g_timer
        global g_timer_interval
        g_timer = threading.Timer(g_timer_interval, time_fun)
        g_timer.start()

def check():
    global g_block_list
    index = 1
    while index < len(g_block_list):
        assert(g_block_list[index].previous_block_hash == g_block_list[index - 1].block_hash)
        index += 1


def main():
    global g_block_list
    if len(sys.argv) < 2:
        return
    options, args = getopt.getopt(sys.argv[1:], 'v')
    visual = False
    for name, value in options:
        print name, value
        if name == '-v':
            visual = True
    print args

    file_info_list = parse(args[0])
    start_block_id = get_min_start_block_id(file_info_list)
    print ('start block id : %d' % (start_block_id))
    if start_block_id == -1:
        print 'do not has any block'
        return
    while True:
        round_info_list = get_a_round(file_info_list, start_block_id)
        if round_info_list == []:
            print 'all has any block, end'
            break
        block = round_info_list_2_block(round_info_list)
        g_block_list.append(block)
        start_block_id += 1;
        print block.to_dict()
    # close all file
    close(file_info_list)
    print do_statistics()
    #check()
    do_visual(visual)

if __name__ == '__main__':
    main()
