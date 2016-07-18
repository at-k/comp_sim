# -*- coding: utf-8 -*-

import sys
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import cm

import csv

def plot(data):
    grplabel = []
    xlabel = []
    y = []

    prev_bs = ""

    for item in data:
        label_tmp = ""
        if item[2] == "0":
            label_tmp += "C"
        else:
            label_tmp += "P"

        label_tmp += (str(int(item[4])/1024) + "kB")

        xlabel.append(label_tmp)

        if prev_bs != (str(int(item[3])/1024) + "kB"):
            prev_bs = (str(int(item[3])/1024) + "kB")
            grplabel.append(prev_bs)
            #xlabel.append([label_tmp])
            y.append([item[6]])
        else:
            #xlabel[len(xlabel) - 1].append(label_tmp)
            y[len(y) - 1].append(item[6])

    i = 0
    ic = 0
    for ydata, glabel in zip(y, grplabel):
        c = cm.hot(float(ic) / len(y))
        plt.bar( np.array(range(len(ydata))) + i, ydata, color = c, width = 0.5, label = glabel, align="center")
        i = i + len(ydata)
        ic = ic + 1

    #plt.bar(range(len(data)) ,y, align="center")
    plt.legend()
    plt.xticks(range(len(data)), xlabel)

    plt.show()


if __name__ == "__main__":
    # load data
    fname = sys.argv[1]
    file = open(fname, "r")
    reader = csv.reader(file)

    data_list = []
    data_wiki = []
    data_xls = []
    data_db = []
    data_jpg = []

    for row in reader:
        if len(row) > 1:
            buf = []
            for item in row:
                buf.append(item.split("=")[1].replace(" ",""))
            data_list.append(buf)

    for item in data_list:
        file_name = item[0]
        if file_name == "data/tgt/enwik8":
            data_wiki.append(item)
#        elif file_name == "data/tgt/kennedy.xls":
#            data_xls.append(item)
#        elif file_name == "data/tgt/osdb":
#            data_db.append(item)
#        elif file_name == "data/tgt/x-ray":
#            data_jpg.append(item)
#        else:
#            print("error: unknown file name\n")
#            sys.exit()

    for alg in [0,1]:
        plot( filter(lambda n:n[1] == str(alg), data_wiki) )
#        plot( filter(lambda n:n[alg] == 0, data_xls) )
#        plot( filter(lambda n:n[alg] == 0, data_db) )
#        plot( filter(lambda n:n[alg] == 0, data_jpg) )

