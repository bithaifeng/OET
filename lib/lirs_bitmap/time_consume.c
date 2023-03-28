#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <vector>
#include <stdio.h>
#include <time.h>

#include <math.h>

#include "time_consume.h"


float gaussrand_NORMAL() {
	static float V1, V2, S;
	static int phase = 0;
	float X;


	if (phase == 0) {
		do {
			float U1 = (float) rand() / RAND_MAX;
			float U2 = (float) rand() / RAND_MAX;


			V1 = 2 * U1 - 1;
			V2 = 2 * U2 - 1;
			S = V1 * V1 + V2 * V2;
		} while (S >= 1 || S == 0);


		X = V1 * sqrt(-2 * log(S) / S);
	} else
		X = V2 * sqrt(-2 * log(S) / S);


	phase = 1 - phase;


	return X;
}


float gaussrand(float mean, float stdc) {
	return mean + gaussrand_NORMAL() * stdc;
}


#define LOCAL_ACCESS_TIME 55 // 80ns
#define KERNEL_OVERHEAD 10000
#define REMOTE_ACCESS_TIME (4000 + 10000) // 4us for RDMA , 5us for Flash , 6us for compressed memory
unsigned long local_access_num = 0, remote_access_num = 0;

unsigned long all_time_consume = 0;

int caculate_bit_num(unsigned long bitmap){
	int num = 0;
	while(bitmap!=0){
		num ++;
		bitmap = bitmap >> 1;
	}
	return num;
}

void local_hit_time( unsigned long bitmap ){
	int num = caculate_bit_num( bitmap );
	local_access_num += num;

	all_time_consume += ( num * LOCAL_ACCESS_TIME );	
}

void remote_hit_time(unsigned long bitmap){
	int num = caculate_bit_num( bitmap );
	local_access_num += (num - 1);
	remote_access_num += (1);
	all_time_consume += ( (num - 1) * LOCAL_ACCESS_TIME +  1 * REMOTE_ACCESS_TIME );
}

void print_all_time_summry( ){

	printf("local_access_num %ld remote_access_num  %ld\n", local_access_num, remote_access_num);

	//RDMA
	printf("RDMA %ld", local_access_num * LOCAL_ACCESS_TIME + remote_access_num * KERNEL_OVERHEAD + remote_access_num * (4000) );

	//Compressed memory
	long tmp_compress = 0;
	int i = 0;
	for(i = 0 ; i < remote_access_num; i++){
		tmp_compress += ( 6400 + gaussrand(0, 1) * 1000 );

	}
	printf("Compressed %ld", local_access_num * LOCAL_ACCESS_TIME + remote_access_num * KERNEL_OVERHEAD + tmp_compress );
	
	//Flash
	
	tmp_compress = 0;
        for(i = 0 ; i < remote_access_num; i++){
                tmp_compress += ( 5000 + gaussrand(0, 1) * 1000 );
        }
	printf("Flash %ld", local_access_num * LOCAL_ACCESS_TIME + remote_access_num * KERNEL_OVERHEAD + tmp_compress );
//	printf("AllTimeUse %ld\n", all_time_consume);
}

