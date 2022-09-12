rm -f ./main
rm -f ./read_simple
rm -f ./convert_trace
g++ -std=c++11  convert_trace.c -w -g -o convert_trace
g++ -std=c++11  read_simple.c -w -g -o read_simple 
