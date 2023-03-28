
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
storage = ["RDMA","Compressed","Flash"]


print(int(sys.argv[1]), indir_name[ int(sys.argv[1]) ] )
for fname in os.listdir("."):
    name = re.findall("log\.(.+?)\.trace*",fname)
    if len(name):
#        fin = open(fname,encoding='utf-8').readlines()
#        print(fin)
#        print(name[0])
        for i in range(1,6):
            # 10 persent to 50 persent
#            for j in range(1):
#            for j in range(5,6):
            for j in range( int(sys.argv[1]) , int(sys.argv[1]) + 1):
#            for j in range(9, 10):
                filename = "/root/lhf/evict_analysis/OET/lib/" + algo_name[j] + "/result." + name[0] + "." \
                        + indir_name[j] + "." + str(i * 10) + "percent"
#                print(filename)
#                print( name[0] + "." + str(i * 10) + "percent",)

#                print(filename)
                fin = open(filename,encoding='utf-8').readlines()
                for line in fin:
                    if "RDMA" in line:
                        list_tmp = line.split()
                        # hit rate
                        # ['RDMA', '1489654332615Compressed', '1499456415591Flash', '1493739499367page', 'numbers[access', 'times],', '25974', '[1],', '44973', '[2-4],', '334408', '[>=5]']
                        RDMA_time = list_tmp[1].replace("Compressed", "")
                        Compressed = list_tmp[2].replace("Flash", "")
                        Flash = list_tmp[3].replace("page", "")
                        if sys.argv[2] == '0':
                            print(RDMA_time)
                        elif sys.argv[2] == '1':
                            print(Compressed)
                        elif sys.argv[2] == '2':
                            print(Flash)
#                        print(RDMA_time,Compressed, Flash)

#                        print( filename , float( list_tmp[2].replace(",","") ) / float( list_tmp[-1]) )
#                        print(float( list_tmp[2].replace(",","") ) / float( list_tmp[-1]) )

#            break
            

print(int(sys.argv[1]), indir_name[ int(sys.argv[1]) ] , storage[ int(sys.argv[2]) ])


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
