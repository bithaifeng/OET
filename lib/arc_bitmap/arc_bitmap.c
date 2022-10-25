#include "arc_bitmap.h"


int local_cache_size = 0;
/* basic sturct*/
unsigned long vpn2ppn[MAX_PPN] = {0};
unsigned long ppn2vpn[MAX_PPN] = {0};

int page_inlist[4] = {0}; // 0-LRU, 1-ghost LRU, 2,FRU, 3-ghost-FRU
int p = 0;


unsigned long hit_num_arc = 0, miss_num_arc = 0;

//struct lirs_entry *vpn2list_entry;
struct lirs_entry vpn2list_entry[ MAX_PPN ];
//
struct lirs_entry head_lru[4];
struct lirs_entry tail_lru[4];


long long mem_using = 0;

int init_lirs_flag = 0;

/***********/
//record bitmap to 
struct bit2vpn{
        unsigned long vpn;
        unsigned long last_access_time;
};
unordered_map<unsigned long, struct bit2vpn> bitmap2struct;

#define BITMAP_TIME_INTERVAL (1ULL << 20)

int last_access_num[5][5] = {0};
int last_access_num_overtime[5][5] = {0};
int first_access_time = 0;
unsigned long last_vpn = 0;

int check_and_return(unsigned long vpn, unsigned long bitmap, unsigned long current_time){
        int ret = 0;
        if(bitmap2struct[bitmap].vpn == 0){
                // first insert
                ;
                bitmap2struct[bitmap].vpn = vpn;
                bitmap2struct[bitmap].last_access_time = current_time;
                first_access_time ++;
                ret = -1;

        }
        else{
                unsigned long tmp_vpn = bitmap2struct[bitmap].vpn;
                last_vpn = tmp_vpn;
                // second insert
                //check time first
                if( current_time - bitmap2struct[bitmap].last_access_time >=  BITMAP_TIME_INTERVAL){
                        //update time
                        bitmap2struct[bitmap].last_access_time = current_time;

                        ret = -1;
			if( vpn2list_entry[tmp_vpn].status != -1){
                                if( vpn2list_entry[vpn].status != -1 ){
                                        last_access_num_overtime[ vpn2list_entry[tmp_vpn].status ][ vpn2list_entry[vpn].status ]   ++;
                                }
                                else{
                                        last_access_num_overtime[ vpn2list_entry[tmp_vpn].status ][4] ++;
                                }

                        }
                        else{
                                if( vpn2list_entry[vpn].status != -1 ){
                                        last_access_num_overtime[4][ vpn2list_entry[vpn].status ] ++;
                                }
                                else{
                                        last_access_num_overtime[4][4] ++;
                                }
                        }
                }
                else{

                        // find previous similar bitmap's vpn
                        // do some  speculate
			if( vpn2list_entry[tmp_vpn].status != -1){
                                ret = vpn2list_entry[tmp_vpn].status;
                                if( vpn2list_entry[vpn].status != -1 ){
                                        last_access_num[ vpn2list_entry[tmp_vpn].status ][ vpn2list_entry[vpn].status ]   ++;
                                }
                                else{
                                        last_access_num[ vpn2list_entry[tmp_vpn].status ][4] ++;
                                }
                        }
                        else{
                                ret = 4;
                                if( vpn2list_entry[vpn].status != -1 ){
                                        last_access_num[4][ vpn2list_entry[vpn].status ] ++;
                                }
                                else{
                                        last_access_num[4][4] ++;
                                }

                        }

                }

                bitmap2struct[bitmap].vpn = vpn;
        }
        return ret;
}

void print_bitmap_analysis(){
        int i,j;
        int sum = 0;
        printf("first_access_time = %d\n",first_access_time);
        printf(" In time:\n");
        for(i = 0; i < 5; i++){
                for( j = 0; j < 5; j++ ){
                        sum += last_access_num[i][j];
                }
                printf("i = %d, num = %d ; ", i, sum); sum = 0;

                for(j = 0; j < 5; j++){
                        printf(" [%d,%d] ~ %d",i,j,  last_access_num[i][j] );
                }
                printf("\n");
        }
        printf(" Overtime:\n");
        for(i = 0; i < 5; i++){
                for( j = 0; j < 5; j++ ){
                        sum += last_access_num_overtime[i][j];
                }
                printf("i = %d, num = %d ; ", i, sum); sum = 0;


                for(j = 0; j < 5; j++){
                        printf(" [%d,%d] ~ %d",i,j,  last_access_num_overtime[i][j] );
                }
                printf("\n");
        }
}


/*************/




void init_arc( int cache_size){
	int i= 0;
//	p = cache_size / 2;
	p = 1;
	printf("local cache size = %d\n", cache_size);
	local_cache_size = cache_size;
	for(i = 0 ; i < MAX_PPN; i++){
		vpn2list_entry[i].vpn = i;
		vpn2list_entry[i].status = -1; 
		vpn2list_entry[i].prev = NULL;
		vpn2list_entry[i].next = NULL;
	}
	for(i = 0; i < 4; i ++){
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



void delete_page_list( struct lirs_entry * page_list ){
//void delete_page_list( u ){
        page_list->prev->next = page_list->next;
        page_list->next->prev = page_list->prev;

        page_list->next = NULL;
        page_list->prev = NULL;
	page_inlist[ page_list->status ] --;
//	vpn2list_entry[vpn].status = -1;
	page_list->status = -1;

//	mem_using ++;
}

void insert_page(unsigned long vpn, int id){
	head_lru[id].next->prev =  &vpn2list_entry[vpn];
	
	vpn2list_entry[vpn].next = head_lru[id].next;	
	head_lru[id].next = &vpn2list_entry[vpn];

	vpn2list_entry[vpn].prev = &head_lru[id];
	vpn2list_entry[vpn].status = id;

	page_inlist[id] ++;
//	mem_using ++;
}

void insert_page_last_vpn(unsigned long vpn, int id, unsigned long samebitmap_vpn){
	vpn2list_entry[samebitmap_vpn].prev->next = &vpn2list_entry[vpn];
        vpn2list_entry[vpn].next = &vpn2list_entry[samebitmap_vpn];

        vpn2list_entry[vpn].prev = vpn2list_entry[samebitmap_vpn].prev;
        vpn2list_entry[samebitmap_vpn].prev = &vpn2list_entry[vpn];
        vpn2list_entry[vpn].status = id;
	page_inlist[id] ++;
}



unsigned long get_new_ppn(unsigned long vpn){
	struct lirs_entry *tmp_list_entry;
	unsigned long ppn;
	unsigned long tmp_vpn;
	//check p 
        if(page_inlist[0] <= p && page_inlist[2] != 0){
                // get free page from fru list
                //move the page from fru to ghost fru
                tmp_list_entry = tail_lru[2].prev;
                delete_page_list( tmp_list_entry );
                tmp_vpn = tmp_list_entry->vpn;
                insert_page(tmp_vpn, 3);
		if(page_inlist[3] + page_inlist[2] > local_cache_size){
			tmp_list_entry = tail_lru[3].prev;
			delete_page_list( tmp_list_entry );
		}

                ppn = vpn2ppn[tmp_vpn];
                //clear the mapping of vpn, get free page
                ppn2vpn[ vpn2ppn[tmp_vpn] ] = MAX_PPN;
                vpn2ppn[tmp_vpn] = local_cache_size;

        }
        else if (page_inlist[0] > p){
                //get page from lru list
                tmp_list_entry = tail_lru[0].prev;
                delete_page_list( tmp_list_entry );
                tmp_vpn = tmp_list_entry->vpn;
                insert_page(tmp_vpn, 1);

                ppn = vpn2ppn[tmp_vpn];
                 //clear the mapping of vpn, get free page
                ppn2vpn[ vpn2ppn[tmp_vpn] ] = MAX_PPN;
                vpn2ppn[tmp_vpn] = local_cache_size;
                //insert new page to lru list
		
		if(page_inlist[1] + page_inlist[0] > local_cache_size){
                        tmp_list_entry = tail_lru[1].prev;
                        delete_page_list( tmp_list_entry );
                }

        }	
	return ppn;
}

unsigned long select_and_return_free_ppn_arc(unsigned long vpn, unsigned long previous_id){
	unsigned long ppn = 0, tmp_vpn;
	struct lirs_entry *tmp_list_entry;

	if(mem_using < local_cache_size){
		mem_using ++;
		return (mem_using - 1);
	}

	if( vpn2list_entry[vpn].status ==  1){
		// the page has been in lru list, but does not exist in local, gdb ok
		//
/*
		if(previous_id == 0){
			delete_page_list( &vpn2list_entry[last_vpn] );
			insert_page( last_vpn, 2);

			delete_page_list( &vpn2list_entry[vpn] );
	                insert_page( vpn, 2);
		}
*/
		if(previous_id == 2){
			delete_page_list( &vpn2list_entry[vpn] );
			insert_page_last_vpn( vpn, 2, last_vpn);
		}
		else
		{
		delete_page_list( &vpn2list_entry[vpn] );	
		insert_page( vpn, 2);
		}
		ppn = get_new_ppn(vpn);
		p = p + 1;
		if (p > local_cache_size)
			p = local_cache_size;
		// add p, p = p + 1
	}
	else if ( vpn2list_entry[vpn].status ==  3 ){
		// the page has been in FRU list, but does not exist in local FRU, gdb ok
		if(previous_id == 2){
			delete_page_list( &vpn2list_entry[vpn] );
			insert_page_last_vpn( vpn, 2, last_vpn);
		}
		else{
		delete_page_list( &vpn2list_entry[vpn] );
                insert_page( vpn, 2);
		}
		ppn = get_new_ppn(vpn);
		p = p - 1;
		if(p == 0) p = 1;
	}
	else if ( vpn2list_entry[vpn].status ==  -1 ){

		/*
		if(previous_id == 2){
			ppn = get_new_ppn(vpn);
                        insert_page_last_vpn(vpn, 2, last_vpn);
			return ppn;
		}
		*/
		// the page first arrive
		// get free page in LRU or FRU
		if(page_inlist[0] + page_inlist[1] == local_cache_size){
			//reach c
			if(page_inlist[0] == local_cache_size){
				//remove page from LRU list
				tmp_list_entry = tail_lru[0].prev;
				delete_page_list( tmp_list_entry );
				tmp_vpn = tmp_list_entry->vpn;
				//get new ppn
				ppn = vpn2ppn[tmp_vpn];

				//clear pgtable
				ppn2vpn[ppn] = MAX_PPN;
				vpn2ppn[tmp_vpn] = local_cache_size;

				if(previous_id == 2){
					insert_page_last_vpn(vpn , 2, last_vpn);
				}
				else if(previous_id == 0)
				insert_page_last_vpn(vpn , 0, last_vpn);
				else
				insert_page(vpn , 0);
			}
			else if (page_inlist[1] != 0){
				// clear ghost lru list first
				tmp_list_entry = tail_lru[1].prev;
				delete_page_list( tmp_list_entry );

				ppn = get_new_ppn(vpn);

				if(previous_id == 2){
//                                        insert_page_last_vpn(vpn , 2, last_vpn);
                                        insert_page(vpn , 2);
					p = p - 1;
					if (p == 0)
						p = 1;
                                }
                                else
				insert_page(vpn, 0);

#if 0
				//check p 
				if(page_inlist[0] <= p && page_inlist[2] != 0){
					// get free page from fru list
					//move the page from lru to ghost lru
					tmp_list_entry = tail_lru[2].prev;
	                                delete_page_list( tmp_list_entry );
	                                tmp_vpn = tmp_list_entry.vpn;	
					insert_page(tmp_vpn, 3);

					ppn = vpn2ppn[tmp_vpn];
					//clear the mapping of vpn, get free page
					ppn2vpn[ vpn2ppn[tmp_vpn] ] = MAX_PPN;				
					vpn2ppn[tmp_vpn] = local_cache_size;

					//insert new page to lru list
					insert_page(vpn, 0);
				}
				else if (page_inlist[0] > p){
					//get page from lru list
					tmp_list_entry = tail_lru[0].prev;
					delete_page_list( tmp_list_entry );
					tmp_vpn = tmp_list_entry.vpn;
					insert_page(tmp_vpn, 1);

					ppn = vpn2ppn[tmp_vpn];
					 //clear the mapping of vpn, get free page
					ppn2vpn[ vpn2ppn[tmp_vpn] ] = MAX_PPN;
					vpn2ppn[tmp_vpn] = local_cache_size;
					//insert new page to lru list
					insert_page(vpn, 0);
				}
#endif
			}
		}
		else{
			//page_inlist[0] + page_inlist[1] < local_cache_size// get free ppn first
			// gdb ok
			ppn = get_new_ppn(vpn);
			insert_page(vpn, 0);
#if 0
			if(page_inlist[0] <= p && page_inlist[2] != 0){
				//move the page from fru to ghost fru
				tmp_list_entry = tail_lru[2].prev;
				delete_page_list( tmp_list_entry );
				tmp_vpn = tmp_list_entry.vpn;
				insert_page(tmp_vpn, 3);
				ppn = vpn2ppn[tmp_vpn];
				ppn2vpn[ vpn2ppn[tmp_vpn] ] = MAX_PPN; vpn2ppn[tmp_vpn] = local_cache_size;
				insert_page(vpn, 0);
			}
			else if (page_inlist[0] > p){
				//get page from lru list
				tmp_list_entry = tail_lru[0].prev;
				delete_page_list( tmp_list_entry );
				tmp_vpn = tmp_list_entry.vpn;
				insert_page(tmp_vpn, 1);
				ppn = vpn2ppn[tmp_vpn];
				 //clear the mapping of vpn, get free page
				ppn2vpn[ vpn2ppn[tmp_vpn] ] = MAX_PPN; vpn2ppn[tmp_vpn] = local_cache_size;
				insert_page(vpn, 0);
			}
#endif
		}
	}
	else{
		printf("error, vpn2list_entry[vpn].status = %d\n", vpn2list_entry[vpn].status);
	}
//	
//	ppn2vpn[ vpn2ppn[vpn] ] = MAX_PPN;
//	vpn2ppn[ vpn ]  = local_cache_size;
	return ppn;
}



void check_pn_arc( unsigned long page_number ,unsigned long bitmap, unsigned long last_access_time){
	unsigned long ppn, vpn;
	vpn = page_number;
	ppn = vpn2ppn[vpn];
	struct lirs_entry *tmp_list_entry;

	int previous_id = 0;

	previous_id = check_and_return(page_number, bitmap, last_access_time);	

	if( vpn2ppn[vpn] == local_cache_size){
		// miis , get page from stack list Q	
		ppn = select_and_return_free_ppn_arc(vpn, previous_id);
		ppn2vpn[ppn] = vpn;
                vpn2ppn[vpn] = ppn;
		if(mem_using <= local_cache_size && init_lirs_flag == 0){
			// init
			if(mem_using == local_cache_size)
				init_lirs_flag = 1;
//			vpn2list_entry[vpn].vpn = vpn;
//			vpn2list_entry[vpn].ppn = ppn;
			vpn2list_entry[vpn].status = 0;
			insert_page(vpn, 0);
			return ;
		}
		miss_num_arc ++;

	}
	else{
		// hit situation
		// check whether hit in LRU or FRU
		if ( vpn2list_entry[vpn].status == 0 ){
			// LRU list hit
			// delete the ptr from LRU list and insert the ptr to FRU

			if(previous_id == 2){
				delete_page_list( &vpn2list_entry[vpn] );
				insert_page_last_vpn(vpn, 2, last_vpn);
			}else{

			delete_page_list( &vpn2list_entry[vpn] );
			insert_page(vpn, 2);
			}

			if(page_inlist[3] + page_inlist[2] > local_cache_size){
	                        tmp_list_entry = tail_lru[3].prev;
	                        delete_page_list( tmp_list_entry );
	                }
		}
		else if ( vpn2list_entry[vpn].status == 2 ){
			// FRU list hit
			// just extract the prt and insert it to the head of FRU
			//
			
			if( previous_id  == 2){
				delete_page_list( &vpn2list_entry[vpn] );
				insert_page_last_vpn(vpn, 2, last_vpn);
//				insert_page(vpn, 2);
			}
			else
			{
			delete_page_list( &vpn2list_entry[vpn] );
			insert_page(vpn, 2);
			}
		
		}
		else{
			printf("some error happen.,vpn2list_entry[vpn].status = %d \n", vpn2list_entry[vpn].status);
		}
//		delete_page_list( &vpn2list_entry[vpn] );
//		insert_page(vpn);
		hit_num_arc ++;	
	}
}

void print_analysis_lirs(){
//	printf("hit num = %lu, miss num = %lu\n", hit_num_arc, miss_num_arc);

	printf("miss_num = %lu, hit_num = %lu, sum = %lu\n", miss_num_arc, hit_num_arc, miss_num_arc + hit_num_arc );
	printf("mem_using = %d, p(local lru list'length) = %d\n", mem_using, p);
	print_bitmap_analysis();
}


