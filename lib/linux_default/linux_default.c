#include "linux_default.h"

#define RRIP_BITS 2
#define INSERT_VALUE ((1 << RRIP_BITS) - 2)

#define RECLAIM_BATCH 32
#define BITS_PER_LONG 64


int local_cache_size = 0;
/* basic sturct*/
unsigned long vpn2ppn[MAX_PPN] = {0};
unsigned long ppn2vpn[MAX_PPN] = {0};


struct page_list ppn2page_list[ MAX_PPN ];


unsigned long linux_evict_using_page[2] = {0};
struct page_list linux_evict_head[2], linux_evict_tail[2];

struct pte_struct vpn2state[MAX_PPN] = {0};

void update_access_bit(unsigned long vpn, unsigned char flag){
	vpn2state[vpn].access_bit = flag;	
}

unsigned char get_access_bit(unsigned long ppn){
	return vpn2state[ppn].access_bit;
}


int ppn2groupid[MAX_PPN] = {0};

unsigned long hit_num_linux_default = 0, miss_num_linux_defaul = 0;

//struct lirs_entry *vpn2list_entry;
struct lirs_entry vpn2list_entry[ MAX_PPN ];
//
struct lirs_entry head_lru[ 1 << RRIP_BITS ];
struct lirs_entry tail_lru[ 1 << RRIP_BITS  ];
long long maintain_number[ 1 << RRIP_BITS ] = {0};


long long mem_using = 0;

int init_lirs_flag = 0;



unsigned long int_sqrt(unsigned long x)
{
	unsigned long b, m, y = 0;

	if (x <= 1)
		return x;

	m = 1UL << (BITS_PER_LONG - 2);
	while (m != 0) {
		b = y + m;
		y >>= 1;

		if (x >= b) {
			x -= b;
			y += m;
		}
		m >>= 2;
	}

	return y;
}

void list_del( struct page_list * page_list ){

    page_list->prev->next = page_list->next;
    page_list->next->prev = page_list->prev;

    page_list->next = NULL;
    page_list->prev = NULL;
}

static bool inactive_list_is_low(){
	unsigned long gb;
	unsigned long inactive_ratio;
	unsigned long inactive, active;
	inactive = linux_evict_using_page[0];
	active = linux_evict_using_page[1];

	gb = local_cache_size >> 18;
	if (gb)
		inactive_ratio = int_sqrt(10 * gb);
	else
		inactive_ratio = 1;
	return inactive * inactive_ratio < active;
}

static void insert_to_page_list_linux_evict(unsigned long ppn, int active_flag){
	linux_evict_head[active_flag].next->prev = &ppn2page_list[ppn];
	ppn2page_list[ppn].next = linux_evict_head[active_flag].next;
	linux_evict_head[active_flag].next = &ppn2page_list[ppn];
	ppn2page_list[ppn].prev = &linux_evict_head[active_flag];
	linux_evict_using_page[active_flag] ++;
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

void transfer_page_to_inactive(){
	//scan first and move pages to inactive page list
//	scan_update_page_list(1);
//	move_page_to_inactive();	

	int i = 0;
	struct page_list *tmp, *scan_ptr;
        tmp = &linux_evict_tail[1];
        tmp = tmp -> prev;
	scan_ptr = tmp;
	unsigned long ppn = 0;
	int find_old_page = 0;
	for(i = 0; i < RECLAIM_BATCH * 2;i++){
		//
		ppn = scan_ptr->ppn;
		tmp = scan_ptr;
/*
		scan_ptr = scan_ptr->prev;
                list_del( tmp );
                linux_evict_using_page[1] --;
                insert_to_page_list_linux_evict( ppn , 0 );
*/
		if(get_access_bit(ppn) == 1){
			update_access_bit(ppn, 0);
			scan_ptr = scan_ptr->prev;
			list_del(tmp);
			linux_evict_using_page[1] --;
			insert_to_page_list_linux_evict(ppn, 1);
		}
		else{
			find_old_page ++;
			scan_ptr = scan_ptr->prev;
			list_del( tmp );
			linux_evict_using_page[1] --;
			insert_to_page_list_linux_evict( ppn , 0 );
		}
	}
//	if(find_old_page == 0){
//		move_page_to_inactive();
//	}
}

unsigned long select_and_return_free_ppn_linux_default(unsigned long vpn){
	unsigned long ppn = 0, tmp_vpn;
	if(mem_using < local_cache_size){
		mem_using ++;
		return (mem_using - 1);
	}

	struct page_list * tmp, *find_ptr, *scan_ptr, *first_find;
	find_ptr = NULL;
	tmp = linux_evict_tail[0].prev;
	scan_ptr = tmp;
	first_find = linux_evict_tail[0].prev;

	int i = 0;
	
	for(i = 0; i < RECLAIM_BATCH * 4; i++){
//		ppn = tmp->ppn;
		if(scan_ptr->prev == NULL){
			if(find_ptr == NULL){
				printf("error, no page in inactive list\n");
				return 0;
			}
			return find_ptr->ppn;
		}
		ppn = scan_ptr->ppn;
		tmp = scan_ptr;
		if(get_access_bit(ppn) == 1){
			update_access_bit(ppn, 0);
			scan_ptr = scan_ptr->prev;
			list_del( tmp );
                        linux_evict_using_page[0] --;
#ifdef LRU
			insert_to_page_list_linux_evict(ppn, 0);
#else
			insert_to_page_list_linux_evict(ppn, 1);
#endif
		}
		else{
			if(find_ptr == NULL){
				scan_ptr = scan_ptr->prev;
				list_del( tmp );
				linux_evict_using_page[0] --;
				vpn2ppn[ ppn2vpn[ tmp->ppn ] ] = local_cache_size;
			        ppn2vpn[ tmp->ppn ] = MAX_PPN;
				find_ptr = tmp;
				break;
			}
		}
//		tmp = tmp->next;
	}
	if(find_ptr == NULL){
		printf("scan %d pages, but no find a page whose access bit equals to 0\n", RECLAIM_BATCH * 4);
		tmp = first_find;
		list_del( tmp );
		linux_evict_using_page[0] --;
		vpn2ppn[ ppn2vpn[ tmp->ppn ] ] = local_cache_size;
                ppn2vpn[ tmp->ppn ] = MAX_PPN;
                find_ptr = tmp;
	}


#ifndef LRU
	//check acvtive page list first and decide whether to move page to inactive list
	if( inactive_list_is_low() ){
		//update active page list and transfer pages from active to inactive
		transfer_page_to_inactive();
	}
#endif


//	list_del( tmp );
//	linux_evict_using_page[0] --;
//	vpn2ppn[ ppn2vpn[ tmp->ppn ] ] = LOCAL_PAGE_SIZE;
  //      ppn2vpn[ tmp->ppn ] = USING_PAGE_SIZE;
//	return tmp->ppn;

//        vpn2ppn[ ppn2vpn[ppn] ] = local_cache_size;
//	ppn2vpn[ ppn ] = MAX_PPN;
//	mem_using --;

	vpn2ppn[ ppn2vpn[ find_ptr->ppn ] ] = local_cache_size;
	ppn2vpn[ find_ptr->ppn ] = MAX_PPN;
	return find_ptr->ppn;
//	return ppn;
}



void linux_evict_init(int cache_size){

	int i= 0;
        printf("local cache size = %d\n", cache_size);
        local_cache_size = cache_size;
	for(i = 0; i < 2; i++){
		linux_evict_head[i].next = &linux_evict_tail[i];
		linux_evict_head[i].prev = NULL;
		linux_evict_tail[i].next = NULL;
		linux_evict_tail[i].prev = &linux_evict_head[i];
	}
	for(i = 0; i < MAX_PPN; i++){
//                ppn2groupid[i] = (1ULL << RRIP_BITS);
                ppn2page_list[i].ppn = i;
                ppn2page_list[i].prev = NULL;
                ppn2page_list[i].next = NULL;
        }

	//set bound ppn
        for(i = 0 ; i < MAX_PPN; i++)
	{
                vpn2ppn[i] = local_cache_size;
		update_access_bit(i, 0);
	}
        for(i = 0 ; i < local_cache_size; i++)
                ppn2vpn[i] = MAX_PPN;



}


void check_pn_linux_default( unsigned long page_number ){
	unsigned long ppn, vpn;
	vpn = page_number;
	ppn = vpn2ppn[vpn];
//	printf("vpn = %lu\n", vpn);

	if( vpn2ppn[vpn] == local_cache_size){
		// miis , get page from stack list Q	
		ppn = select_and_return_free_ppn_linux_default(vpn);
		ppn2vpn[ppn] = vpn;
//		vpn2list_entry[vpn].ppn = ppn;
                vpn2ppn[vpn] = ppn;
		if(mem_using <= local_cache_size && init_lirs_flag == 0){
			// init
			if(mem_using == local_cache_size)
				init_lirs_flag = 1;
//			vpn2list_entry[vpn].vpn = vpn;
//			vpn2list_entry[vpn].ppn = ppn;
			update_access_bit(ppn, 0);
			insert_to_page_list_linux_evict(ppn, 0);
			return ;
		}
		update_access_bit(ppn, 0);
		insert_to_page_list_linux_evict(ppn, 0);
		miss_num_linux_defaul ++;

	}
	else{
		//update access bit
		update_access_bit(ppn, 1);
		hit_num_linux_default ++;	
	}
}

void print_analysis_lirs(){
	int i = 0;
//	printf("hit num = %lu, miss num = %lu\n", hit_num_et, miss_num_et);

	printf("active list  = %lu, inactive list = %lu\n", linux_evict_using_page[1], linux_evict_using_page[0]);
	printf("miss_num = %lu, hit_num = %lu, sum = %lu\n", miss_num_linux_defaul, hit_num_linux_default, miss_num_linux_defaul + hit_num_linux_default );
	printf("miss rate = %lf\n", (double)miss_num_linux_defaul / (double)(miss_num_linux_defaul + hit_num_linux_default));

	printf("mem_using = %d\n", mem_using);
}


