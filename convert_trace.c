
//#define _GNU_SOURCE
//
#include <asm/unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
//#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sched.h>
#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <map>

#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <atomic>
#include <hash_map>

#include <vector>
//#include <hash_map>
#include <unordered_map>

#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
//#include<fcntl.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <vector>

using namespace std;

#define MAX_PPN (64ULL << 18)
//#define LOCAL_CACHE_SIZE ( 5 ULL << 18)
#define LOCAL_CACHE_SIZE ( 3ULL << 18) // tpch q12
#define MONITER_WINDOW (LOCAL_CACHE_SIZE  * 2)



//#define MEMNUM                  (0x500000000ULL)  // forddr3 20G
#define MEMNUM                  (0xA00000000ULL)  // forddr4 40G
#define cfg_end                 (MEMNUM + (1ULL << 15))  // for ddr3
#define KERNEL_TRACE_ENTRY_ADDR ((cfg_end >> 20) + 32ULL + 32ULL)
#define KERNEL_TRACE_CONFIG_ENTRY_ADDR ((cfg_end >> 20) + 64 + 5248ULL)

#define MALLOC_TAG_ENTRY_ADDR ((cfg_end >> 20) + 64 + 5120ULL)
unsigned long long malloc_tag_addr = MALLOC_TAG_ENTRY_ADDR << 20; 
int topmc_tag = 0;

unsigned long long kernel_config_entry = KERNEL_TRACE_CONFIG_ENTRY_ADDR << 20;
#define KERNEL_TRACE_CONFIG_SIZE (1LLU)
#define KERNEL_TRACE_SEQ_NUM     (64LLU)
unsigned long long kernel_config_end =
        (KERNEL_TRACE_CONFIG_ENTRY_ADDR + KERNEL_TRACE_CONFIG_SIZE * KERNEL_TRACE_SEQ_NUM) << 20;
#define TAG_ACCESS_SIZE (4096LLU)
#define TAG_ACCESS_STEP (2048LLU)   //in Byte,
#define TAG_ACCESS_TIMES (2U)
#define TAG_MAX_POS (256U)

#define SET_PTE_TAG                     0
#define FREE_PTE_TAG                    1

#define DUMP_PAGE_TABLE_TAG             2
#define KERNEL_TRACE_END_TAG            3
#define SET_PT_ADDR_TAG                 5
#define FREE_PT_ADDR_TAG                6

unsigned long long tmp_addr[50000] = {0};
int tmp_addr_length = 0;
int tmp_addr_len[50000] = {0};


unsigned long long sync_tag_num = 0;
unsigned long long set_pte_num = 0;
unsigned long long set_pt_num = 0;
unsigned long long sync_tag_num_r = 0;
unsigned long long set_pte_num_r = 0;
unsigned long long set_pt_num_r = 0;



unsigned long long some_error_trace = 0;


unsigned long long free_pte_num = 0;
unsigned long long free_pte_num_r = 0;
unsigned long long free_pt_num = 0;

int is_kernel_tag_trace(unsigned long long addr) {
    return addr >= kernel_config_entry && addr < kernel_config_end;
}

int has_print = 0;
#define MAX_PRINT 70


int show_sync_tag_num = 10;

int fd,size;
int fd_w;

int write_ptr = 0;
//#define LENBUFF (1 << 26) 
#define LENBUFF (6 << 20)
//#define LENBUFF (48 << 20)
char bufferw[LENBUFF] = {1}; //16MB
char bufferr[LENBUFF] = {1};
unsigned int last_seq = 666;
unsigned int last_io_seq = 666;
int start_flag = 0;
unsigned long long all_timer = 0;
unsigned long long usedtrace = 0;
unsigned long finish_io_tag = 0;



void writefilea(){
//	printf("sizeof(bufferw) = %d\n",sizeof(bufferw) );
	write(fd_w,bufferw, sizeof(bufferw));
}
unsigned long long trace_id = 0;
unsigned long long real_trace_id = 0;
int error_flag = 0;
int tag_num = 0;

int monitor = 0 ;


static unsigned long long mem_start = (18 * 1024 * 1024 + 2 * 1024 * 1024), mem_size = 4 * 1024 * 1024;  //4096
unsigned long last_ppn = 0;


/*******  evict analysis  ************/
unsigned long  duration_all = 0;
extern void record_filter_table( unsigned long p_addr, unsigned long tt );
extern void print_bitmap_and_reusedistance();
extern void init_struct();
extern int reuse_distance_analysis(unsigned long paddr, unsigned long tt);

extern void init_convert_struct();
extern void convert_trace(unsigned long paddr, unsigned long tt);
extern void print_ppntimes();
extern void print_left_ppn();

int write_trace_num = 0;
unsigned long store_file_byte = 0;


int page_inlist = 0;
int max_page_inlist = 0;

/*******************/



int  readfilea(){
	size = read(fd,bufferr, LENBUFF);
	unsigned long long       tmp;
	if(size == 0  )
	{
		printf("finish size =%d \n",size);
		return 0;
	}
	unsigned int seq_no,r_w;
	unsigned long paddr;
	unsigned long long timer;

	int i ;
	for (i = 0; i < size / 6; i++){

		trace_id ++;
				
		
		tmp = (unsigned long long)( *(unsigned long long*) (bufferr + i * 6));
		seq_no = (unsigned int) ((tmp >> 40) & 0xffU);
		timer  = (unsigned long long)((tmp >> 32) & 0xffULL);
		r_w    = (unsigned int)((tmp >> 31) & 0x1U);
		paddr   = (unsigned long)(tmp & 0x7fffffffUL);
		paddr   = (unsigned long)(paddr << 6);


		if (paddr >= (2ULL << 30)) {
        		paddr += (2ULL << 30);
    		}

		if (paddr == 0 && timer == 0){
			duration_all += 256;
	                continue ;
		}
		duration_all += timer;
		convert_trace( paddr, duration_all );
//		record_filter_table( paddr, duration_all );
		continue;

//		if(paddr != 0 && (paddr >> 12 == 0x71b5c4 || paddr >> 12 ==  0x712928) )
		if(paddr != 0)// && (paddr >> 12 == 0x71b5c4 || paddr >> 12 ==  0x712928) )
		 printf("trace_id = %lld seq_no = %d ,r_w = %d, paddr = %lx,  last_seq_no = %d \n",trace_id, seq_no, r_w, paddr, last_seq);
		 continue ;


//	paddr_m = p_trace->paddr >> 20;//in MB
//	mid_timer = p_trace->timer;
	//if(trace_id > 6644560 && trace_id < 6644578 + 20)
//		printf("seq_no = %d ,r_w = %d, paddr = %lx,  last_seq_no = %d \n",seq_no, r_w, paddr, last_seq);
//		printf("last_seq = %d\n",last_seq);
//		return 1;
//	if(trace_id % 10000000 == 0)
//		printf("trace_id = %lld seq_no = %d ,r_w = %d, paddr = %lx,  last_seq_no = %d \n",trace_id, seq_no, r_w, paddr, last_seq);
//		printf("start_flag = %d\n",start_flag);
		if(error_flag == 1){
//			printf("trace_id = %lld seq_no = %d ,r_w = %d, paddr = %lx,  last_seq_no = %d \n",trace_id, seq_no, r_w, paddr, last_seq);
//			error("");
//			error("seq error!\n");

		}	
		if( trace_id == 77653169 || trace_id == 77653170 || trace_id == 77653171 ){
			printf("trace_id = %lld seq_no = %d ,r_w = %d, paddr = %lx,  last_seq_no = %d \n",trace_id, seq_no, r_w, paddr, last_seq);

		}


		if(timer == 0){
			all_timer += 256;
			
		}
		else{
			if(paddr >= mem_start * 1024)
				monitor ++;

			unsigned long now_ppn = paddr >> 12;
			if(now_ppn > last_ppn){
				if(now_ppn > last_ppn + 1){
					;
					//error
//					printf("ERROR PPN now = 0x%lx , last = 0x%lx \n", now_ppn, last_ppn);
					
				}
				else{

				}
				last_ppn = now_ppn;
			}

			if(now_ppn == 0x723641)
				printf("id = %d, seq_no = %d ,r_w = %d, timer = %d, paddr = %lx, ppn = 0x%lx \n",  has_print,seq_no, r_w, timer, paddr, paddr>>12);
	
			if(now_ppn >= (8ULL << 18))
			 if(has_print < MAX_PRINT){
//ASPLOS
	//	                 printf("id = %d, seq_no = %d ,r_w = %d, timer = %d, paddr = %lx, ppn = 0x%lx \n",  has_print,seq_no, r_w, timer, paddr, paddr>>12);
		                 has_print ++;
		         }
			else {
	//			printf("id = %d, seq_no = %d ,r_w = %d, timer = %d, paddr = %lx ,ppn = 0x%lx\n",  has_print,seq_no, r_w, timer ,paddr , paddr >> 12);
				has_print ++;
			}
			all_timer += timer;
			usedtrace += 1;
		}
		if( paddr == 0 && timer == 0){
			
		}
		else	real_trace_id ++;

		if(start_flag == 0) {
			printf(" first  trace_id = %lld seq_no = %d ,r_w = %d, paddr = %lx,  last_seq_no = %d \n",trace_id, seq_no, r_w, paddr, last_seq);
			last_seq = seq_no;
			start_flag = 1;
		}
		else{
			if(error_flag == 0 &&  seq_no != (last_seq + 1) % 256 ){
//			printf("id = %lld,last_seq = %d, now_seq = %d\n",trace_id, last_seq,seq_no);
//				printf("trace_id = %lld seq_no = %d ,r_w = %d, paddr = %lx,  last_seq_no = %d \n",trace_id, seq_no, r_w, paddr, last_seq);
//				printf("&&&& error topmc_tag = %d\n",topmc_tag);
//				printf("error\n");
				error_flag = 1;
				last_seq = seq_no;
			}
			else{
				last_seq = seq_no;
				error_flag = 0;
			}
		}

		if(paddr == malloc_tag_addr + 64){
//			printf("***************** trace_id = %lld seq_no = %d ,r_w = %d, paddr = %lx,  last_seq_no = %d \n",trace_id, seq_no, r_w, paddr, last_seq);	
			topmc_tag ++;
		}

		if (is_kernel_tag_trace(paddr)){
			int kernel_trace_seq = (paddr - kernel_config_entry) / (KERNEL_TRACE_CONFIG_SIZE << 20);
			unsigned long long kernel_trace_seq_entry =
                    paddr - kernel_config_entry - kernel_trace_seq * (KERNEL_TRACE_CONFIG_SIZE << 20);
			if(show_sync_tag_num > 0){
//				printf("real_trace_id = %lld seq_no = %d ,r_w = %d, paddr = %lx,  sync_tag_num = %d \n",real_trace_id, seq_no, r_w, paddr, sync_tag_num);
				show_sync_tag_num --;
			}


			
			sync_tag_num ++;
            		int kernel_trace_tag = (kernel_trace_seq_entry) / TAG_ACCESS_SIZE;
			if(kernel_trace_tag == DUMP_PAGE_TABLE_TAG){
				printf("Start trace id = %ld\n",trace_id);
				printf("sync_tag_num = %llu,  set_pte_num = %llu, free_pte_num = %llu, set_pt_num = %llu, free_pt_num = %llu\n",sync_tag_num,set_pte_num, free_pte_num, set_pt_num, free_pt_num);
			}
			else if(kernel_trace_tag == SET_PTE_TAG){
				set_pte_num ++;
				if(r_w == 1) set_pte_num_r ++;
				else{
					printf("$$$$$$s set_pte_write error trace trace_id = %lld seq_no = %d ,r_w = %d, paddr = %lx,  last_seq_no = %d , ckernel_trace_tag = %d \n",trace_id, seq_no, r_w, paddr, last_seq, kernel_trace_tag);
					int tm20 = 0;
/*
					for(tm20 = 0; tm20 < tmp_addr_length; tm20 ++){
						if(tmp_addr[tm20] == paddr)
                                                     break;

					}
					if(tm20 == tmp_addr_length){
						tmp_addr[tm20] = paddr;
						tmp_addr_length ++;
                                                tmp_addr_len[tm20] = 1;
					}
					else{
						                                                tmp_addr_len[tm20] ++;
                                        }
*/
	//			       	printf(" trace_id = %lld seq_no = %d ,r_w = %d, paddr = %lx,  last_seq_no = %d \n",trace_id, seq_no, r_w, paddr, last_seq);
				}
			}
			else if(kernel_trace_tag == FREE_PTE_TAG){
				free_pte_num ++;
				if(r_w == 1)
				free_pte_num_r ++;
				else {
				printf("$$$$$$s free_pte_write error trace trace_id = %lld seq_no = %d ,r_w = %d, paddr = %lx,  last_seq_no = %d , ckernel_trace_tag = %d \n",trace_id, seq_no, r_w, paddr, last_seq, kernel_trace_tag);
/*					int tm20 = 0;
					for(tm20 = 0; tm20 < tmp_addr_length; tm20 ++){
						if(tmp_addr[tm20] == paddr)
							break;
					}
					if(tm20 == tmp_addr_length){
						//first
						tmp_addr[tm20] = paddr;
						tmp_addr_length ++;
						tmp_addr_len[tm20] = 1;
					}
					else{
						tmp_addr_len[tm20] ++;
					}
*/	//				printf(" trace_id = %lld seq_no = %d ,r_w = %d, paddr = %lx,  last_seq_no = %d \n",trace_id, seq_no, r_w, paddr, last_seq);
				}
			
			}
			else if(kernel_trace_tag == SET_PT_ADDR_TAG){
				set_pt_num ++;
			}
			else if(kernel_trace_tag == FREE_PT_ADDR_TAG){
				free_pt_num ++;
				
			}
			else if(kernel_trace_tag == KERNEL_TRACE_END_TAG){
				printf("KERNEL_TRACE_END_TAG KERNEL_TRACE_END_TAG KERNEL_TRACE_END_TAG find\n");

			}
//			else if(kernel_trace_tag == KERNEL_TRACE_END_TAG )
			else{ 
				int tm20 = 0;
				some_error_trace += 1;
				printf("***error trace trace_id = %lld seq_no = %d ,r_w = %d, paddr = %lx,  last_seq_no = %d , ckernel_trace_tag = %d \n",trace_id, seq_no, r_w, paddr, last_seq, kernel_trace_tag); 
			}	
//				fprintf(stdout, "UnIdentifitable kernel trace tag:addr=0x%llx, tag=%d\n",
//			         paddr,
//			         kernel_trace_tag);
		  }

		
	
	//if(paddr >= (29ULL << 30) && paddr <= ((29ULL << 30) + 640ULL)    ){
#if 0 
		if( (paddr - (30ULL << 30)) /  64 <= 255  ){
//		if( (paddr >=  (28ULL << 30)) ){
//		if( paddr == ((28ULL << 30) + 640ULL)  ){
			//&& paddr > ((29ULL << 30) + 0x100000)  ){
//			printf("trace_id = %lld seq_no = %d ,r_w = %d, paddr = %lx,  last_seq_no = %d \n",trace_id, seq_no, r_w, paddr, last_seq);
//			printf("trace_id = %lld seq_no = %d ,r_w = %d, paddr = %lx,  last_seq_no = %d ,tag_num = %d , offset = %d\n",trace_id, seq_no, r_w, paddr, last_seq, tag_num, (paddr - (28ULL << 30)) /  64);
			if (last_io_seq == 666){
				last_io_seq = ((paddr - (30ULL << 30)) /  64);
			}
			else{
				int now_io_seq = (paddr - (30ULL << 30)) /  64;
				if((now_io_seq )== (last_io_seq + 1) % 256){
				
				}
				else {
					printf("trace_id = %lld  now_io_seq = %d ,r_w = %d, paddr = %lx,  last_io_seq = %d \n",trace_id, now_io_seq, r_w, paddr, last_io_seq);
					printf("error\n");
				}
				last_io_seq = now_io_seq;


			}
			tag_num ++;
//		printf("seq_no = %d ,r_w = %d, paddr = %lx\n",seq_no,r_w, paddr);

		}
		else if( (paddr - (30ULL << 30)) /  64 == 256){
			finish_io_tag ++;

		}
#endif


//	if(trace_id > 6644560 && trace_id < 6644578 + 20)
//		printf("seq_no = %d ,r_w = %d, paddr = %lx\n",seq_no,r_w, paddr);
	}
	return 1;

}

char filename[200] = "/mnt/ssd/tpcc-hmtt-5w.trace";
char filename_write[200] = "write.trace";

char *q = 0;


unsigned long access_pp[] = {1,2,3, 1, 2, 4, 5, 6, 7, 2, 3, 2};


int main(int argc, char* argv[]){
	int i;

	init_convert_struct();
	init_struct();
	/* Unit test for reuse distance */
	unsigned long tmp_ppn = 0;
	for (i = 0; i < 12; i ++){
//		tmp_ppn = i % 5 + 1;
//		tmp_ppn = access_pp[i];
		;
//		printf(" ppn = %lu, reuse = %d\n " , tmp_ppn, reuse_distance_analysis(tmp_ppn << 12, 2));
	}
		




	last_ppn = mem_start * 1024 / 4096;
	last_seq = 666;
	printf("last_seq = %d\n",last_seq);
	start_flag = 0;

	if(argc > 1){
		printf("%s\n",argv[1]);
		strcpy(filename,argv[1]);
		printf("%s\n",filename);
		printf("%s\n",argv[2]);
                strcpy(filename_write,argv[2]);
		printf("%s\n",filename_write);
	}
	for(i = 0; i < LENBUFF << 1; i ++){
		bufferw[i] = '1';
	}
	printf("pid = %d\n",getpid());

	fd=open(filename, O_RDWR |O_CREAT, 0777);
	fd_w = open(filename_write, O_RDWR |O_CREAT, 0777);

	start_flag = 0;
	
	while(readfilea() == 1){
	}
	print_left_ppn();
//	write(fd_w,bufferw, sizeof(bufferw));
	write(fd_w,bufferw, write_ptr);
	store_file_byte += write_ptr;

	printf("store_file_byte = %lu, write_trace_num = %d\n", store_file_byte, write_trace_num);

	printf("page_inlist = %d\n", max_page_inlist);
	print_ppntimes();

	
	close(fd_w);	
	close(fd);
//	print_bitmap_and_reusedistance();

	return 0;
}

struct record_struct{
	unsigned long timer;
	unsigned long bitmap;
};

struct record_struct new_ppn[MAX_PPN] = {0};

struct evict_transfer_entry_struct{
	unsigned long bitmap;
	unsigned long reuse_distance;
	int real_reuse_distance;
};

vector<struct evict_transfer_entry_struct> ppn2entry_struct[MAX_PPN];




void record_filter_table( unsigned long p_addr, unsigned long tt ){
	int ret = -1;
	unsigned long ppn = p_addr >> 12;
	unsigned long cache_line_offset = (p_addr & 0xfff) >> 6;
	unsigned long value = new_ppn[ppn].bitmap;
	int tmp_reuse_d  = reuse_distance_analysis(p_addr, tt);	

	if( duration_all - new_ppn[ppn].timer >= (1ULL << 20) && new_ppn[ppn].timer != 0){
		////extract and clear bitmap
		struct evict_transfer_entry_struct tmp;
		tmp.bitmap = new_ppn[ppn].bitmap;
		tmp.reuse_distance = duration_all - new_ppn[ppn].timer;
		tmp.real_reuse_distance = tmp_reuse_d;
//		ppn2entry_struct[ppn].push_back( tmp );
//		printf("ppn = 0x%lx reuseDistance = %d reuseTime = %lu bitmap = 0x%lx  \n", ppn, tmp_reuse_d, tmp.reuse_distance, new_ppn[ppn].bitmap);
		printf("0x%lx %d %lu 0x%lx\n", ppn, tmp_reuse_d, tmp.reuse_distance, new_ppn[ppn].bitmap);
		new_ppn[ppn].bitmap = 0;
	}
	/*
	if(new_ppn[ppn].bitmap == 0xFFFFFFFFFFFFFFFF){
		struct evict_transfer_entry_struct tmp;
		tmp.bitmap = new_ppn[ppn].bitmap;
                tmp.reuse_distance = duration_all - new_ppn[ppn].timer;
                ppn2entry_struct[ppn].push_back( tmp );
                new_ppn[ppn].bitmap = 0;
	}
	*/
	new_ppn[ppn].bitmap = ( new_ppn[ppn].bitmap | (1ULL << cache_line_offset) );
	new_ppn[ppn].timer = duration_all;
}

#define RD_INTERVAL 5000
#define RT_INTERVAL 100000

int distribute_rd[2000] = {0};
int distribute_rt[2000] = {0};
int exceed_rd = 0;
int exceed_rt = 0;


void print_bitmap_and_reusedistance(){
	unsigned long i;
	unsigned long j;
	int max_access_time = 0;
	for(i = 0 ; i < MAX_PPN; i++){
		if( ppn2entry_struct[i].size() > 1 ){
			printf("PPN = 0x%lx, access_times = %d\n", i, ppn2entry_struct[i].size() );;
		}
		else{
			continue;
		}

		for(j = 0; j < ppn2entry_struct[i].size() ; j++){
			printf(" Tag = %d , reuse_distance = %ld real_reuse_distance = %d  bitmap = 0x%lx \n", ppn2entry_struct[i][j].real_reuse_distance / 10000,
			      ppn2entry_struct[i][j].reuse_distance, 
				ppn2entry_struct[i][j].real_reuse_distance,
				ppn2entry_struct[i][j].bitmap );	
			int id = 0;
			if( ppn2entry_struct[i][j].real_reuse_distance / RD_INTERVAL >= 2000){
				id = 1999;
				exceed_rd ++;
			}
			else id = ppn2entry_struct[i][j].real_reuse_distance / RD_INTERVAL;
			
			distribute_rd[id] ++;
			
			if( ppn2entry_struct[i][j].reuse_distance / RT_INTERVAL >= 2000 ){
				id = 1999;
				exceed_rd ++;
			}
			else id = ppn2entry_struct[i][j].reuse_distance / RT_INTERVAL;
			distribute_rt[id] ++;
		}
		if(max_access_time < ppn2entry_struct[i].size())
			max_access_time = ppn2entry_struct[i].size();
	}
	printf("max_access_time = %d\n", max_access_time);

	
	for(i = 0; i < 2000 ; i ++){
		if(distribute_rd[i] != 0)
		printf("rd = %lu~%lu num = %d\n", i * RD_INTERVAL, (i + 1) * RD_INTERVAL, distribute_rd[i]);
	}

	for(i = 0; i < 2000 ; i ++){
		if(distribute_rt[i] != 0)
		printf("rt = %lu~%lu num = %d\n", i * RT_INTERVAL, (i + 1) * RT_INTERVAL, distribute_rt[i]);
	}
	
}

int ppn2index[MAX_PPN] = {0};

//unsigned long local_cache[LOCAL_CACHE_SIZE * 2 ] = {0};
unsigned long local_cache[ MONITER_WINDOW ] = {0};

void init_struct(){
	unsigned long i = 0;
	for(i = 0 ; i < MAX_PPN; i ++){
		ppn2index[i] = -1;
	}
}

void update_local_cache_and_index(int index){
	unsigned long ppn_latest = local_cache[index];
	int i = 0;
	for(i = index; i > 0; i --){
		local_cache[i] = local_cache[i - 1];
		unsigned long tmp_ppn = local_cache[i - 1];
		ppn2index[tmp_ppn] = i;
	}
	ppn2index[ ppn_latest ] = 0;
	local_cache[ 0 ] = ppn_latest;
}

void backward_moniter_window(){
	int i = 0;
	unsigned long tmp_ppn;
	int last_i = 0;	
	unsigned long store_ppn = 0;
	for( i = MONITER_WINDOW - 1; i > 0; i--){
		if( local_cache[i - 1] == 0){
			// no need to backward
			continue;
		}
		//clear the last cache only!
		tmp_ppn = local_cache[i];
		if(i == MONITER_WINDOW - 1 )
	                ppn2index[ tmp_ppn ] = -1;
		local_cache[i] = local_cache[i - 1];
                ppn2index[ local_cache[i - 1] ] = i; //update index
	}
}


int insert_and_check( unsigned long paddr ){
	int ret = -1;
	unsigned long ppn = paddr >> 12;
	int reuse_distance = 0;
//	printf("accessing ppn = %lu\n", ppn);;

	// check whether the ppn in local cache.
	if( ppn2index[ ppn ] == -1 ){
		// the ppn not exists in local or moniter_window
		backward_moniter_window();


		local_cache[0] = ppn;
		ppn2index[ppn] = 0;
#if 0
		int i = 0;
                for(i = 0; i < MONITER_WINDOW;i++){
                        printf("local id = %d, ppn = %lu\n", i , local_cache[i]);
                }
                for( i = 1 ; i <=100; i++ ){
                        if( ppn2index[i] != -1){
                                printf("ppn = %d, in local offset = %d\n",i, ppn2index[i]);
                        }  
                }
#endif
		return -1; // means infinite reuse distance

	}
	else{
		// the ppn exists in local
		reuse_distance = ppn2index[ ppn ];
		// move the ppn to the head and move the others ppn
		update_local_cache_and_index( ppn2index[ ppn ] );
#if 0
		int i = 0;
                for(i = 0; i < MONITER_WINDOW;i++){
                        printf("local id = %d, ppn = %lu\n", i , local_cache[i]);
                }
                for( i = 1 ; i <=100; i++ ){
                        if( ppn2index[i] != -1){
                                printf("ppn = %d, in local offset = %d\n", i,ppn2index[i]);
                        }  
                }
#endif

		return reuse_distance;
	}
}




int reuse_distance_analysis(unsigned long paddr, unsigned long tt){
	int ret = - 1;
	ret = insert_and_check(paddr);		
	return ret ;

}

struct tmp_list_track{
	unsigned long ppn;
	unsigned long bitmap;
	unsigned long last_access_time;
	struct tmp_list_track *prev, *next;
	int access_num;
};

struct tmp_list_track ppn2struct[MAX_PPN] = {0};
struct tmp_list_track head_list,tail_list;

void init_convert_struct(){
	head_list.prev = NULL;
	head_list.next = &tail_list;
	tail_list.prev = &head_list;
	tail_list.next = NULL;
	tail_list.ppn = 0;
	head_list.ppn = 0;
	int i = 0;
	for(i = 0; i < MAX_PPN ; i++){
		ppn2struct[i].ppn = i;
		ppn2struct[i].prev = NULL;
		ppn2struct[i].next = NULL;
	}
}

void clear_ppn(unsigned long ppn){
	ppn2struct[ppn].next->prev = ppn2struct[ppn].prev;
	ppn2struct[ppn].prev->next = ppn2struct[ppn].next;
	ppn2struct[ppn].prev = NULL;
        ppn2struct[ppn].next = NULL;
	page_inlist --;
}

void insert_ppn(unsigned long ppn){
	ppn2struct[ppn].prev = &head_list;
	ppn2struct[ppn].next = head_list.next;
	
	head_list.next->prev = &ppn2struct[ppn];
	head_list.next = &ppn2struct[ppn];
	page_inlist ++;
	if(page_inlist > max_page_inlist)
		max_page_inlist = page_inlist;
}

void copy_and_write(unsigned long ppn, unsigned long bitmap, unsigned long last_access_time  ){
//	strcpy(bufferw + write_ptr,  &ppn);
	* (unsigned long *)(bufferw + write_ptr) = ppn;
	write_ptr += sizeof(ppn);
//	strcpy(bufferw + write_ptr,  &bitmap);

	* (unsigned long *)(bufferw + write_ptr) = bitmap;
	write_ptr += sizeof(bitmap);

//	strcpy(bufferw + write_ptr,  &last_access_time);
	* (unsigned long *)(bufferw + write_ptr) = last_access_time;
	write_ptr += sizeof(last_access_time);

	if(write_ptr + 24 >= LENBUFF ){
//		write(fd_w,bufferw, sizeof(bufferw));
		write(fd_w,bufferw, write_ptr);
		store_file_byte += write_ptr;
		write_ptr = 0;
	}
}


void check_and_insert(unsigned long ppn){
	if(ppn2struct[ppn].prev != NULL && ppn2struct[ppn].next != NULL){
		//update it
		clear_ppn(ppn);
		insert_ppn(ppn);
	}
	else{
		//insert
		insert_ppn(ppn);
	}

}

void check_list_tail(){
	struct tmp_list_track tmp = *tail_list.prev;
	unsigned long ppn;
	while( tmp.prev != NULL ){
		if( duration_all - tmp.last_access_time >= (1ULL << 20) ){
			//delete and printf
//			ppn = tail_list.ppn;
			ppn = tmp.ppn;
			tmp = *tmp.prev;
			clear_ppn( ppn );
//			printf("ppn 0x%lx bitmap 0x%lx time %lu\n",ppn, ppn2struct[ppn].bitmap, ppn2struct[ppn].last_access_time);
//			printf("0x%lx 0x%lx %lu\n",ppn, ppn2struct[ppn].bitmap, ppn2struct[ppn].last_access_time);
			copy_and_write(ppn,  ppn2struct[ppn].bitmap, ppn2struct[ppn].last_access_time);
			write_trace_num ++;
			ppn2struct[ppn].access_num ++;
		}
		else{
			break;
		}
	}
}

void print_left_ppn(){
	struct tmp_list_track tmp = *tail_list.prev;
	unsigned long ppn;
	while( tmp.prev != NULL ){
		ppn = tmp.ppn;
                tmp = *tmp.prev;
                clear_ppn( ppn );
//		printf("0x%lx 0x%lx %lu\n",ppn, ppn2struct[ppn].bitmap, ppn2struct[ppn].last_access_time);
		copy_and_write(ppn,  ppn2struct[ppn].bitmap, ppn2struct[ppn].last_access_time);
		write_trace_num ++;
                ppn2struct[ppn].access_num ++;
	}

}


void convert_trace(unsigned long paddr, unsigned long tt){
	        int ret = -1;
        unsigned long ppn = paddr >> 12;
        unsigned long cache_line_offset = (paddr & 0xfff) >> 6;
	ppn2struct[ppn].bitmap = ( ppn2struct[ppn].bitmap | (1ULL << cache_line_offset) );
	ppn2struct[ppn].last_access_time = duration_all;

	check_and_insert( ppn );
	check_list_tail();	
}

void print_ppntimes(){
	int access1num = 0;
	int access2num = 0;
	int access5num = 0;
	int i = 0 ;
	for(i = 0; i < MAX_PPN; i++){
		if(ppn2struct[i].access_num >= 5){
			access5num ++;
		}
		else if( ppn2struct[i].access_num >= 2 ){
			access2num ++;
		}
		else if( ppn2struct[i].access_num == 1 )
			access1num ++;
	}
	printf("page numbers[access times], %d [1], %d [2-4], %d [>=5]\n", access1num, access2num, access5num);
}




