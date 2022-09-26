#include "et.h"

#define RRIP_BITS 3
#define INSERT_VALUE ((1 << RRIP_BITS) - 2)



int local_cache_size = 0;
/* basic sturct*/
unsigned long vpn2ppn[MAX_PPN] = {0};
unsigned long ppn2vpn[MAX_PPN] = {0};

int ppn2groupid[MAX_PPN] = {0};

unsigned long hit_num_et = 0, miss_num_et = 0;

//struct lirs_entry *vpn2list_entry;
struct lirs_entry vpn2list_entry[ MAX_PPN ];
//
struct lirs_entry head_lru[ 1 << RRIP_BITS ];
struct lirs_entry tail_lru[ 1 << RRIP_BITS  ];
long long maintain_number[ 1 << RRIP_BITS ] = {0};


long long mem_using = 0;

int init_lirs_flag = 0;



void init_et( int cache_size){
	int i= 0;
	printf("local cache size = %d\n", cache_size);
	local_cache_size = cache_size;
	for(i = 0 ; i < MAX_PPN; i++){
		vpn2list_entry[i].vpn = i;
		vpn2list_entry[i].ppn = local_cache_size;
		vpn2list_entry[i].status = -1; 
		vpn2list_entry[i].in_local = 0;
		vpn2list_entry[i].prev = NULL;
		vpn2list_entry[i].next = NULL;
		ppn2groupid[i] = (1ULL << RRIP_BITS);
	}
	for(i = 0 ; i < (1 << RRIP_BITS); i++){
		head_lru[i].prev = NULL;
		head_lru[i].next = &tail_lru[i];
		head_lru[i].ppn = 99999999999999;
		head_lru[i].vpn = 99999999999999;

		tail_lru[i].prev = &head_lru[i];
		tail_lru[i].next = NULL;
		
		tail_lru[i].ppn = 88888888888888;
		tail_lru[i].vpn = 88888888888888;
	}
	//set bound ppn
        for(i = 0 ; i < MAX_PPN; i++)
                vpn2ppn[i] = local_cache_size;
        for(i = 0 ; i < local_cache_size; i++)
                ppn2vpn[i] = MAX_PPN;
}

int check_group_id(unsigned long ppn){
	int id = 0;
	id = ppn2groupid[ppn];		
	return id;
}

void decrease_rrip(unsigned long vpn){
	maintain_number[ ppn2groupid[vpn] ] --;
	ppn2groupid[vpn] --;
}


void delete_page_list( struct lirs_entry * page_list ){
        page_list->prev->next = page_list->next;
        page_list->next->prev = page_list->prev;

        page_list->next = NULL;
        page_list->prev = NULL;
}

void insert_page(unsigned long vpn, int id){
	ppn2groupid[vpn] = id;

	head_lru[id].next->prev =  &vpn2list_entry[vpn];
	
	vpn2list_entry[vpn].next = head_lru[id].next;	
	head_lru[id].next = &vpn2list_entry[vpn];

	vpn2list_entry[vpn].prev = &head_lru[id];
	maintain_number[id] ++;
//	mem_using ++;
}

void change_vnn_group(unsigned long vpn, int target_id){
	delete_page_list( &vpn2list_entry[vpn] );
	insert_page( vpn,  target_id);
//	mem_using --;
}
#define lru_0

void check_and_update_page(unsigned long vpn){
	int now_group_id = check_group_id( vpn );
	if(now_group_id == ((1ULL << RRIP_BITS)) ){
		//new page, insert page to list with RRIP value 
		printf("error in check_group_id\n");
//		insert_page(ppn, (1ULL << RRIP_BITS) - 2 );
		insert_page(vpn, INSERT_VALUE );
		return ;	
	}

	if(now_group_id == 0){
		//skip
#if 1
		//update lru list
		delete_page_list( &vpn2list_entry[vpn] );
		maintain_number[ ppn2groupid[vpn] ] --;
//	        mem_using --;
		insert_page(vpn, 0 );
#endif

		return ;
	}
	if( now_group_id > 0 && now_group_id < (1ULL << RRIP_BITS) ){
	//decrease its RRIP value
		decrease_rrip(vpn);
		change_vnn_group(vpn, ppn2groupid[vpn] );
	}
	else{
		printf(" error group_id = %d\n ", now_group_id);
	}
}


void rotate_rrip_group(){
	int i = 0;
	for(i = (1 << RRIP_BITS) - 1; i > 0; i --){
		maintain_number[i] = maintain_number[i - 1];
		
//		head[i].prev = head[i - 1].prev;
		head_lru[i].next = head_lru[i - 1].next;
		head_lru[i - 1].next->prev = &head_lru[i];

		tail_lru[i].prev = tail_lru[i - 1].prev;
		tail_lru[i].prev->next = &tail_lru[i];
//		tail[i].next = tail[i - 1].next;

		//clear 
		head_lru[i - 1].next = &tail_lru[i - 1];
		tail_lru[i - 1].prev = &head_lru[i - 1];
		maintain_number[i - 1] = 0;


		//increase
		struct lirs_entry *tmp;
		tmp = &head_lru[i];
		tmp = tmp->next;			
		while( tmp->next != NULL ){
			ppn2groupid[ tmp->vpn ] ++;
			if( ppn2groupid[ tmp->vpn ] >= (1ULL << RRIP_BITS) )
				printf("error groupid = %d\n", ppn2groupid[ tmp->vpn ]);
			tmp = tmp->next;
		}
	}
	maintain_number[0] = 0;
}

unsigned long get_tail_and_delete(int group_id){
//	unsigned long ppn;
	struct lirs_entry *tmp_list_entry;
        unsigned long ppn;
        unsigned long tmp_vpn;
	if(maintain_number[group_id] == 0){
		printf("error, groupid = %d \n", group_id);
		return local_cache_size;
	}

	tmp_list_entry = tail_lru[group_id].prev;	
	delete_page_list( tmp_list_entry );
	tmp_vpn = tmp_list_entry->vpn;
	ppn2groupid[tmp_vpn] = (1 << RRIP_BITS);

	ppn = vpn2ppn[tmp_vpn];

	//clear ppn mapping
	vpn2ppn[ tmp_vpn ] = local_cache_size;
	ppn2vpn[ ppn ] = MAX_PPN;


	maintain_number[group_id] --;

	if(maintain_number[group_id] < 0 ){
//		flag_p ++;
		printf("error in get_tail_and_delete, group id = %d, num = %ld\n", group_id , maintain_number[group_id]);
	}

	return ppn;
}

unsigned long select_and_return_free_ppn_et(unsigned long vpn){
	unsigned long ppn = 0, tmp_vpn;
	if(mem_using < local_cache_size){
		mem_using ++;
		return (mem_using - 1);
	}
	while(maintain_number[(1 << RRIP_BITS) - 1] == 0){
		//decrement RRIP for each group
		rotate_rrip_group();
	}

	struct lirs_entry *tmp_entry;
	ppn = get_tail_and_delete( (1 << RRIP_BITS) - 1 );
	if (ppn == local_cache_size){
		printf("Error!\n");
		return 0;
	}
        vpn2ppn[ ppn2vpn[ppn] ] = local_cache_size;
	ppn2vpn[ ppn ] = MAX_PPN;
//	mem_using --;

	return ppn;
}




void check_pn_et( unsigned long page_number ){
	unsigned long ppn, vpn;
	vpn = page_number;
	ppn = vpn2ppn[vpn];
//	printf("vpn = %lu\n", vpn);

	if( vpn2ppn[vpn] == local_cache_size){
		// miis , get page from stack list Q	
		ppn = select_and_return_free_ppn_et(vpn);
		ppn2vpn[ppn] = vpn;
//		vpn2list_entry[vpn].ppn = ppn;
                vpn2ppn[vpn] = ppn;
		if(mem_using <= local_cache_size && init_lirs_flag == 0){
			// init
			if(mem_using == local_cache_size)
				init_lirs_flag = 1;
//			vpn2list_entry[vpn].vpn = vpn;
//			vpn2list_entry[vpn].ppn = ppn;
			insert_page(vpn, INSERT_VALUE );
			return ;
		}
		insert_page(vpn, INSERT_VALUE );
		miss_num_et ++;

	}
	else{
		// hit situation
		check_and_update_page( vpn );
		hit_num_et ++;	
	}
}

void print_analysis_lirs(){
	int i = 0;
	for(i = 0; i < ((1ULL << RRIP_BITS)); i++){
                printf("maintains[%d] = %ld\n", i,maintain_number[i]);
        }
//	printf("hit num = %lu, miss num = %lu\n", hit_num_et, miss_num_et);

	printf("miss_num = %lu, hit_num = %lu, sum = %lu\n", miss_num_et, hit_num_et, miss_num_et + hit_num_et );
	printf("mem_using = %d\n", mem_using);
}


