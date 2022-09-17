rm -f ./main
rm -f ./read_simple
rm -f ./convert_trace
# just read simple file
#g++ -std=c++11  read_simple.c -w -g -o read_simple 



# for lirs
g++ -std=c++11  read_simple.c lirs.c -w -g -o lirs -fpermissive -mcmodel=medium


