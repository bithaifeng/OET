import os,sys,re


algo= "arc_2c"

algo = "linux_default"
#algo = "et"


algo = "clock"
#algo = "fifo"
algo = "arc"
algo = "lru"
algo = "lirs"
algo = "lirs_bitmap"


for fname in os.listdir("."):
    name = re.findall("log\.(.+?)\.trace*",fname)
    if len(name):
        fin = open(fname,encoding='utf-8').readlines()
        line = fin[-1].split()
        res = int(line[-2])+int(line[-4])

#        print (res)
        for i in range(1,6):
            cmd = "./" + algo + " /mnt/ssd/lhf/simpletrace/" + name[0] + ".simpletrace " + str( int(res * i / 10) ) + " > result." \
                + name[0] + "."+ algo + "." + str( i * 10 ) + "percent;"
            print(cmd)
#        print(name[0])

 #       print(res)
#        break
