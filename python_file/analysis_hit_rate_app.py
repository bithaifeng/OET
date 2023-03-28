
#!/usr/bin/python
# -*- coding: UTF-8 -*-

import os,sys,re

#algo = "lru"

#algo = "arc"
#algo = "et"
algo = "lirs"

algo_name = ["ARC","et", "lirs", "LRU","lirs_limit", "lirs_bitmap", "ARC_2C", "linux_default", "clock/new_result", "fifo"]
indir_name = [ "arc","et", "lirs", "lru","lirs_limit", "lirs_bitmap", "arc_2c", "linux_default", "clock", "fifo"]

algo_name = ["ARC", "fifo" ,"lirs" , "LRU", "lirs_bitmap", "clock"]
indir_name = ["arc", "fifo","lirs","lru", "lirs_bitmap", "clock"]

algo_name = [ "clock", "fifo", "LRU","ARC", "lirs" , "lirs_bitmap"]
indir_name = [ "clock", "fifo","lru", "arc", "lirs", "lirs_bitmap"]



real_name = [ "spark-graphx-cc ", "spark-graphx-pgrank ", "tpch-6g-q18 ", "tpch-6g-q12 ", "tpch-6g-q3 ", "kmeans ", "tpch-6g-q5 ", "npb-mg ", "tpch-6g-q21 ", "tpch-6g-q9 ", "spark-graphx-bfs ", "tpch-6g-q8 ", "tpch-6g-q11 ", "npb-cg ", "tpch-6g-q14 ", "tpch-6g-q4 ", "tpch-6g-q16 ", "npb-ft ", "tpch-6g-q15 ", "tpch-6g-q22 ", "npb-is ", "tpch-6g-q10 ", "tpch-6g-q13 ", "wtdbg2 ", "spark-graphx-lp ", "tpch-6g-q2 ", "qsort ", "tpch-6g-q20 ", "npb-lu ", "specjbb ", "tpch-6g-q1 ", "tpch-6g-q7 ", "tpch-6g-q6 ", "terasort-6g-1 ", "tpch-6g-q19 ", "hpl ", "tpch-6g-q17 "]

real_name = [ 
        "kmeans","npb-is","npb-lu","npb-cg","npb-mg","npb-ft","qsort","wtdbg2","specjbb","hpl",
        "spark-graphx-cc","spark-graphx-pgrank","spark-graphx-bfs","spark-graphx-lp",
        "terasort-6g-1","tpch-6g-q1","tpch-6g-q2","tpch-6g-q3","tpch-6g-q4","tpch-6g-q5","tpch-6g-q6","tpch-6g-q7","tpch-6g-q8","tpch-6g-q9","tpch-6g-q10","tpch-6g-q11","tpch-6g-q12","tpch-6g-q13","tpch-6g-q14","tpch-6g-q15","tpch-6g-q16","tpch-6g-q17","tpch-6g-q18","tpch-6g-q19","tpch-6g-q20","tpch-6g-q21","tpch-6g-q22"]

#print(int(sys.argv[1]), indir_name[ int(sys.argv[1]) ] )
#for fname in os.listdir("."):
id = -1
for real_n in real_name:
    id = id + 1
    if id != int(sys.argv[1]):
        continue
#    name = re.findall("log\.(.+?)\.trace*",fname)
    name = real_n
    name = name.replace(' ','')
    if len(name):
#        fin = open(fname,encoding='utf-8').readlines()
#        print(fin)
#        print(name[0])
#        for i in range(1,6):
        for j in range(0,6):
#        for i in range(int(sys.argv[2])  , int(sys.argv[2]) + 1):
            # 10 persent to 50 persent
#            for j in range(1):
#            for j in range(0,6):
             for i in range(1,6):
#            for j in range( int(sys.argv[1]) , int(sys.argv[1]) + 1):
#            for j in range( int(sys.argv[1]) , int(sys.argv[1]) + 1):
#            for j in range(9, 10):
#                filename = "/root/lhf/evict_analysis/OET/lib/" + algo_name[j] + "/result." + name[0] + "." \
                filename = "/root/lhf/evict_analysis/OET/lib/" + algo_name[j] + "/result." + name + "." \
                        + indir_name[j] + "." + str(i * 10) + "percent"
#                print(filename)
#                print( name[0] + "." + str(i * 10) + "percent",)

#                print(filename)
                fin = open(filename,encoding='utf-8').readlines()
                for line in fin:
                    if "miss_num" in line:
                        list_tmp = line.split()
                        # hit rate
#                        print(list_tmp)
#                        print( filename , float( list_tmp[2].replace(",","") ) / float( list_tmp[-1]) )
#                         print(name,"\",\"", end='')
#                        print(name)
                        if i < 5:
                            print(float( list_tmp[2].replace(",","") ) / float( list_tmp[-1]),"\t ", end='' )
                        else:
                            print(float( list_tmp[2].replace(",","") ) / float( list_tmp[-1]))
#            print()
    print(name)

#            break
            

#print(int(sys.argv[1]), indir_name[ int(sys.argv[1]) ] , int(sys.argv[2]) * 10 ,"%")


#        break
#        line = fin[-1].split()
#        res = int(line[-2])+int(line[-4])

#        print (res)
#        for i in range(1,6):
#            cmd = "./" + algo + " /mnt/ssd/lhf/simpletrace/" + name[0] + ".simpletrace " + str( int(res * i / 10) ) + " > result." \
#                + name[0] + "."+ algo + "." + str( i * 10 ) + "percent;"
#            print(cmd)
#        print(name[0])

 #       print(res)
#        break
