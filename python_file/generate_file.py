
#!/usr/bin/python
# -*- coding: UTF-8 -*-

import os,sys,re

#algo = "lru"

#algo = "arc"
#algo = "et"
algo = "lirs"

# python3 analysis_hit_rate_app.py 37

for i in range(37):
    cmd = "python3 analysis_hit_rate_app.py "+ str(i)
    cmd = cmd + " >> meili.txt"
    print(cmd)
    os.system(cmd)
    cmd = "echo '' >> meili.txt"
    os.system(cmd)
    os.system(cmd)


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
