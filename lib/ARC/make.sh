rm -f ./main
rm -f ./read_simple
rm -f ./convert_trace
# just read simple file
#g++ -std=c++11  read_simple.c -w -g -o read_simple 



# for lirs
g++ -std=c++11  read_simple.c arc.c time_consume.c -w -g -o arc -fpermissive -mcmodel=medium


