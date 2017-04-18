#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# 圧縮データの格納効率評価用簡易シミュレータ
#

import math
import sys
from numpy.random import *
from optparse import OptionParser

# default parameter definition
class GlobalParam:
    def __init__(self):
        self.loop_count = 10
        self.data_type = 0
        self.page_size = 32
        self.lp_size = 16
        self.buf_size = 2
        self.ran_seed = 0
    def printParam(self):
        print "loop:{0:d}, dtype:{1:d}, psize:{2:d}, lpsize:{3:d}, bsize:{4:d}, rseed:{5:d}".format(
            self.loop_count,
            self.data_type,
            self.page_size,
            self.lp_size,
            self.buf_size,
            self.ran_seed)

class IOGenerator:
    def __init__(self, type, max, ran_seed):
        self.type = type
        self.max = max
        if ran_seed != 0:
            seed(ran_seed)
        self.avg = float(self.max) / 2
        self.div = float(self.max) / 4

        if self.type == 1:
            self.avg = float(self.max) / 2
            self.div = float(self.max) / 4
        elif self.type == 2:
            self.avg = 3*float(self.max) / 4
            self.div = float(self.max) / 4
        elif self.type == 3:
            self.avg = 7*float(self.max) / 8
            self.div = float(self.max) / 4
        else:
            self.type = 0

    def nextio(self):
        length = 0
        if self.type == 0:
            length = randint(1,self.max+1)
        elif self.type == 1 or self.type == 2 or self.type == 3:
            while True:
                length = int( round( normal(self.avg, self.div) ) )
                if length >=1 and length <= self.max:
                    break
        return Data(length)

class Data:
    def __init__(self, length):
        self.len = length

class Buffer:
    def __init__(self, num):
        self.num  = num
        self.cur_buf = 0
        self.buf_list = [0] * num
        self.data_len = 0

    def add_data(self, data):
        if self.is_full():
            return False

        self.buf_list[self.cur_buf] = data
        self.data_len += data.len
        self.cur_buf = self.cur_buf + 1

        return True

    def is_full(self):
        if self.cur_buf >= self.num:
            return True
        return False

    def clear(self):
        self.cur_buf = 0
        self.data_len = 0

def fifo_dst(buf_list, fm_page_size):
    dst_size = 0
    pad_size = 0
    tmp = 0

    for data in buf_list:
        if tmp + data.len >= fm_page_size:
            pad_size += (fm_page_size - tmp)
            dst_size += fm_page_size
            tmp = data.len
        else:
            tmp += data.len
        #print data.len,tmp,pad_size,dst_size

    if tmp != 0:
        pad_size += (fm_page_size - tmp)
        dst_size += fm_page_size

    #print pad_size,dst_size
    return {'pad' : pad_size, 'dst' : dst_size}

def sort_dst(buf_list, fm_page_size):
    check_list = [0] * len(buf_list)
    checked_num = 0

    dst_size = 0
    pad_size = 0
    #print len(buf_list)
    while checked_num != len(buf_list):
        #tmp = []
        tmp_size = 0
        for i in range(0,len(buf_list)):
            if check_list[i] == 0 and (tmp_size + buf_list[i].len) < fm_page_size:
                #tmp.append(i)
                tmp_size += buf_list[i].len
                check_list[i] = 1
                checked_num += 1

        pad_size += (fm_page_size - tmp_size)
        dst_size += fm_page_size

    return {'pad' : pad_size, 'dst' : dst_size}

# main
if __name__ == '__main__':

    # argument
    parser = OptionParser()

    parser.add_option("-c", "--count",     type="int", dest="loop_count", help="simulation loop count")
    parser.add_option("-d", "--data_type", type="int", dest="data_type",  help="data type")
    parser.add_option("-p", "--page_size", type="int", dest="page_size",  help="fm page size")
    parser.add_option("-l", "--lp_size",   type="int", dest="lp_size",    help="lp size")
    parser.add_option("-b", "--buf_size",  type="int", dest="buf_size",   help="buffer size")
    parser.add_option("-s", "--ran_seed",  type="int", dest="ran_seed",   help="random seed")

    (options, args) = parser.parse_args()

    # initialize param
    global_param = GlobalParam()
    if options.loop_count != None:
        global_param.loop_count = options.loop_count
    if options.data_type != None:
        global_param.data_type = options.data_type
    if options.page_size != None:
        global_param.page_size = options.page_size
    if options.lp_size != None:
        global_param.lp_size = options.lp_size
    if options.buf_size != None:
        global_param.buf_size = options.buf_size
    if options.ran_seed != None:
        global_param.ran_seed = options.ran_seed

    if global_param.buf_size > global_param.loop_count:
        print "error, buf size is smaller than loop count"
        sys.exit(0)

    # initialize iogen
    iogen = IOGenerator( global_param.data_type, global_param.lp_size, global_param.ran_seed )

    # initialize buffer
    buf = Buffer( global_param.buf_size )

    # initialize result
    result = [0] * 3
    for i in range(0,3):
        result[i] = {'dst' : 0, 'pad' : 0}

    # print parameter
    global_param.printParam();

    # main loop
    remain_io = global_param.loop_count
    ttl_data_size = 0
    while remain_io > 0:
        data = iogen.nextio()
        remain_io = remain_io - 1
        #print data.len

        buf.add_data(data)
        if buf.is_full():
            result_tmp = fifo_dst(buf.buf_list, global_param.page_size)
            result[0]['dst'] += result_tmp['dst']
            result[0]['pad'] += result_tmp['pad']

            result_tmp = sort_dst(buf.buf_list, global_param.page_size)
            result[1]['dst'] += result_tmp['dst']
            result[1]['pad'] += result_tmp['pad']

            pad_tmp = 0
            if buf.data_len % global_param.page_size != 0:
                pad_tmp = global_param.page_size - buf.data_len % global_param.page_size
            result[2]['dst'] += (buf.data_len + pad_tmp)
            result[2]['pad'] += pad_tmp

            #print "clear {0:d}".format(buf.data_len)
            ttl_data_size += buf.data_len
            buf.clear()

    # output result
    for i in range(0,3):
        if result[i]['dst'] - result[i]['pad'] != ttl_data_size:
            print "error, total data length is not matched"
        if result[i]['dst'] == 0:
            print 'error, total destage data size is zero'
        print "{0:>3d}, {1:>4d}, {2:>4d}, {3:>5.2f}".format(
                i, result[i]['dst'], result[i]['pad'],
                100*float(result[i]['pad']) / result[i]['dst'] )


