#include "lru.h"
#include "time_consume.h"


int local_cache_size = 0;
/* basic sturct*/
unsigned long vpn2ppn[MAX_PPN] = {0};
unsigned long ppn2vpn[MAX_PPN] = {0};


unsigned long hit_num_lru = 0, miss_num_lru = 0;

//struct lirs_entry *vpn2list_entry;
struct lirs_entry vpn2list_entry[ MAX_PPN ];
//
struct lirs_entry head_lru;
struct lirs_entry tail_lru;
long long mem_using = 0;

int init_lirs_flag = 0;


extern void delete_page_lirs_stackS( struct lirs_entry * page_list );
extern void stack_pruning();

void init_lru( int cache_size){
	int i= 0;
	printf("local cache size = %d\n", cache_size);
	local_cache_size = cache_size;
	for(i = 0 ; i < MAX_PPN; i++){
		vpn2list_entry[i].vpn = i;
		vpn2list_entry[i].status = -1; 
		vpn2list_entry[i].in_local = 0;
		vpn2list_entry[i].prev = NULL;
		vpn2list_entry[i].next = NULL;
	}
		head_lru.prev = NULL;
		head_lru.next = &tail_lru;
		head_lru.ppn = 99999999999999;
		head_lru.vpn = 99999999999999;

		tail_lru.prev = &head_lru;
		tail_lru.next = NULL;
		
		tail_lru.ppn = 88888888888888;
		tail_lru.vpn = 88888888888888;

	//set bound ppn
        for(i = 0 ; i < MAX_PPN; i++)
                vpn2ppn[i] = local_cache_size;
        for(i = 0 ; i < local_cache_size; i++)
                ppn2vpn[i] = MAX_PPN;
}



void delete_page_list( struct lirs_entry * page_list ){
        page_list->prev->next = page_list->next;
        page_list->next->prev = page_list->prev;

        page_list->next = NULL;
        page_list->prev = NULL;
//	mem_using ++;
}

void insert_page(unsigned long vpn){

	head_lru.next->prev =  &vpn2list_entry[vpn];

	
	vpn2list_entry[vpn].next = head_lru.next;	
	head_lru.next = &vpn2list_entry[vpn];

	vpn2list_entry[vpn].prev = &head_lru;
//	mem_using ++;
}

unsigned long select_and_return_free_ppn_lru(unsigned long vpn){
	unsigned long ppn = 0, tmp_vpn;
	if(mem_using < local_cache_size){
		mem_using ++;
		return (mem_using - 1);
	}

//	ppn = tail_lru.prev->ppn;
	vpn = tail_lru.prev->vpn;
	delete_page_list( tail_lru.prev );

	//clear origin PTE corresponding to ppn.
	
	ppn2vpn[ vpn2ppn[vpn] ] = MAX_PPN;
	vpn2ppn[ vpn ]  = local_cache_size;
//	vpn2ppn[ ppn2vpn[ppn] ] = local_cache_size;
//	ppn2vpn[ppn] = MAX_PPN;
	return ppn;
}



//void check_pn_lru( unsigned long page_number ){
void check_pn_lru( unsigned long page_number, unsigned long bitmap, unsigned long last_access_time ){
	unsigned long ppn, vpn;
	vpn = page_number;
	ppn = vpn2ppn[vpn];


	if( vpn2ppn[vpn] == local_cache_size){
		remote_hit_time( bitmap );


		// miis , get page from stack list Q	
		ppn = select_and_return_free_ppn_lru(vpn);
		ppn2vpn[ppn] = vpn;
//		vpn2list_entry[vpn].ppn = ppn;
                vpn2ppn[vpn] = ppn;
		if(mem_using <= local_cache_size && init_lirs_flag == 0){
			// init
			if(mem_using == local_cache_size)
				init_lirs_flag = 1;
//			vpn2list_entry[vpn].vpn = vpn;
//			vpn2list_entry[vpn].ppn = ppn;
			insert_page(vpn);
			return ;
		}
		insert_page(vpn);

		miss_num_lru ++;

	}
	else{
		local_hit_time( bitmap );

		// hit situation
		delete_page_list( &vpn2list_entry[vpn] );
		insert_page(vpn);
		hit_num_lru ++;	
	}
}

void print_analysis_lirs(){
//	printf("hit num = %lu, miss num = %lu\n", hit_num_lru, miss_num_lru);

	printf("miss_num = %lu, hit_num = %lu, sum = %lu\n", miss_num_lru, hit_num_lru, miss_num_lru + hit_num_lru );
	printf("mem_using = %d\n", mem_using);
	print_all_time_summry();
}


