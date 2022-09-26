#include "lirs_bitmap.h"
#include <unordered_map>

int local_cache_size = 0;
/* basic sturct*/
unsigned long vpn2ppn[MAX_PPN] = {0};
unsigned long ppn2vpn[MAX_PPN] = {0};


unsigned long hit_num_lirs = 0, miss_num_lirs = 0;
unsigned long lirs_num[2] = {0};
unsigned long lirs_num_real[2] = {0};

//struct lirs_entry *vpn2list_entry;
struct lirs_entry vpn2list_entry[ MAX_PPN ];
struct lirs_entry ppn2list_entry_stackQ[ MAX_PPN ];
//
struct lirs_entry head_lirs[2];
struct lirs_entry tail_lirs[2];
long long mem_using = 0;

int init_lirs_flag = 0;


extern void delete_page_lirs_stackS( struct lirs_entry * page_list );
extern void stack_pruning();


/***********/
//record bitmap to 
struct bit2vpn{
	unsigned long vpn;
	unsigned long last_access_time;	
};
unordered_map<unsigned long, struct bit2vpn> bitmap2struct;

unsigned long check_and_return(unsigned long vpn, unsigned long bitmap){
	unsigned long ret = 0;
	if(bitmap2struct[bitmap])
	
}


/*************/


void init_lirs( int cache_size){
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
	for(i = 0 ; i < local_cache_size; i++){
		ppn2list_entry_stackQ[i].vpn = i;
		ppn2list_entry_stackQ[i].ppn = i;
                ppn2list_entry_stackQ[i].status = -1; 
                ppn2list_entry_stackQ[i].in_local = 0;
                ppn2list_entry_stackQ[i].prev = NULL;
                ppn2list_entry_stackQ[i].next = NULL;
        }
	for(i = 0 ; i < 2; i++){
		head_lirs[i].prev = NULL;
		head_lirs[i].next = &tail_lirs[i];
		head_lirs[i].ppn = 99999999999999;
		head_lirs[i].vpn = 99999999999999;

		tail_lirs[i].prev = &head_lirs[i];
		tail_lirs[i].next = NULL;
		
		tail_lirs[i].ppn = 88888888888888;
		tail_lirs[i].vpn = 88888888888888;

	}
	//set bound ppn
        for(i = 0 ; i < MAX_PPN; i++)
                vpn2ppn[i] = local_cache_size;
        for(i = 0 ; i < local_cache_size; i++)
                ppn2vpn[i] = MAX_PPN;
}



void delete_page_lirs_stackQ( struct lirs_entry * page_list ){
        page_list->prev->next = page_list->next;
        page_list->next->prev = page_list->prev;

        page_list->next = NULL;
        page_list->prev = NULL;
	lirs_num_real[1] --;
//	lirs_num[1] --;
}

unsigned long get_ppn_from_stackQ(){
	unsigned long ppn = 0, vpn = 0;
	struct lirs_entry* tmp;
	tmp = tail_lirs[1].prev;
	if(tmp->prev == NULL){
//		printf("stack Q is empty\n");
		return local_cache_size;
	}
	ppn = tmp->ppn;
//	vpn = vpn2ppn[ppn];
	vpn = ppn2vpn[ppn];

//	delete_page_lirs_stackS(tmp);
	delete_page_lirs_stackQ(tmp);
//	// check is the origin ppn~vpn in stack S?
	if( vpn2list_entry[ vpn ].status == -1 )
	{
		; // no action
	}
	else if (vpn2list_entry[ vpn ].status == 1){
		vpn2list_entry[ vpn ].status = 2;
	}
	else{
		printf("error in get_ppn_from stackQ, status = %d\n", vpn2list_entry[ vpn ].status);
		;
	}
	return ppn;
}

unsigned long get_ppn_from_stackS(){
	unsigned long ppn = 0;
	struct lirs_entry * tmp;
	tmp = tail_lirs[0].prev;
	if(tmp->prev == NULL){
		printf("Error in get page frome stack S\n");
		return 0;
	}
	tmp->status = -1;

//	delete_page_lirs(tmp);
	delete_page_lirs_stackS( tmp );


	ppn = tmp->ppn;
	stack_pruning();
	return ppn;
}

unsigned long select_and_return_free_ppn_lirs(unsigned long vpn){
	unsigned long ppn = 0, tmp_vpn;
	if(mem_using < local_cache_size){
		mem_using ++;
		return (mem_using - 1);
	}

	if(  lirs_num_real[1] >= local_cache_size / 100 ){
	ppn = get_ppn_from_stackQ();
	if(ppn != local_cache_size){
		lirs_num[1] --;
		//check and update stackS
		tmp_vpn = ppn2vpn[ppn];
		//check whether the page is in stack S?
		if(vpn2list_entry[tmp_vpn].status == -1){
			; // no action, because the page is not in stack S
		}
		else{
			//the page is in stack S, change its state to non-resident block
			vpn2list_entry[tmp_vpn].status = 2;
		}
		vpn2ppn[ ppn2vpn[ppn] ] = local_cache_size;
	        ppn2vpn[ppn] = MAX_PPN;


		return ppn;
	}
	}
	else{
	//there is no free page in stack Q, we need get new page from tail of stack S.
	ppn = get_ppn_from_stackS();

	lirs_num[0] --;

	//clear origin PTE corresponding to ppn.
	vpn2ppn[ ppn2vpn[ppn] ] = local_cache_size;
	ppn2vpn[ppn] = MAX_PPN;
	return ppn;
	}
}


void insert_stack_s(unsigned long vpn){
	head_lirs[ 0 ].next->prev = &vpn2list_entry[vpn];
	vpn2list_entry[vpn].next = head_lirs[0].next;
	
	head_lirs[0].next = &vpn2list_entry[vpn];
	vpn2list_entry[vpn].prev = &head_lirs[0];
	lirs_num_real[0] ++;
}

void delete_page_lirs_stackS( struct lirs_entry * page_list ){
        page_list->prev->next = page_list->next;
        page_list->next->prev = page_list->prev;

        page_list->next = NULL;
        page_list->prev = NULL;
	lirs_num_real[0] --;
//	lirs_num[0] --;

}


void insert_page_lirs_tail( unsigned long ppn, int group_id ){
	tail_lirs[group_id].prev->next = &ppn2list_entry_stackQ[ppn];
        ppn2list_entry_stackQ[ppn].prev = tail_lirs[group_id].prev;

        tail_lirs[group_id].prev = &ppn2list_entry_stackQ[ppn];
        ppn2list_entry_stackQ[ppn].next = &tail_lirs[group_id];
	lirs_num_real[1] ++;
//	lirs_num[1] ++;
}
void insert_page_lirs( unsigned long ppn, int group_id ){
	head_lirs[group_id].next->prev = &ppn2list_entry_stackQ[ppn];
        ppn2list_entry_stackQ[ppn].next = head_lirs[group_id].next;

        head_lirs[group_id].next = &ppn2list_entry_stackQ[ppn];
        ppn2list_entry_stackQ[ppn].prev = &head_lirs[group_id];
	lirs_num_real[1] ++;
//	lirs_num[1] ++;
}


void stack_pruning(){
	struct lirs_entry * tmp, *delete_entry;
	tmp = tail_lirs[0].prev;
	while(1){
		if(tmp->prev == NULL){
			// meet LRS head
			return ;
		}
		if(tmp->status != 0){
			// change to next and delete the block
			tmp->status = -1;
			delete_entry = tmp;
			tmp = tmp->prev;
//			delete_page_lirs( delete_entry );
			delete_page_lirs_stackS( delete_entry );
		}
		else{
			// the bottom block is LRS block, return it.
			return ;
		}
	}
}


void check_pn_lirs( unsigned long page_number ,unsigned long bitmap){
	unsigned long ppn, vpn;
	vpn = page_number;
	ppn = vpn2ppn[vpn];
//	printf("bitmap = 0x%lx\n", bitmap);


	if( vpn2ppn[vpn] == local_cache_size){

		if(  bitmap == 0xFFFFFFFFFFFFFFFF && init_lirs_flag == 1 ){
			if (vpn2list_entry[vpn].status == -1){
                        //new page add it to stack S.
                        	insert_stack_s(vpn);
	                        vpn2list_entry[vpn].status = 2; //first page to stack S, set it to HIR
				miss_num_lirs ++;
				return ;
			}	
		}



		// miis , get page from stack list Q	
		ppn = select_and_return_free_ppn_lirs(vpn);
		ppn2vpn[ppn] = vpn;
                vpn2ppn[vpn] = ppn;
		if(mem_using <= local_cache_size && init_lirs_flag == 0){
			// init
			if(mem_using == local_cache_size)
				init_lirs_flag = 1;
			//init state, add it to head of stack S
			vpn2list_entry[vpn].status = 0;
			vpn2list_entry[vpn].vpn = vpn;
			vpn2list_entry[vpn].ppn = ppn;
			lirs_num[0] ++;
			insert_stack_s(vpn);
			return ;
		}

		if( vpn2list_entry[vpn].status == 2 ){
			vpn2list_entry[vpn].status = 0;
			delete_page_lirs_stackS( &vpn2list_entry[vpn] );
			insert_stack_s(vpn);

			lirs_num[0] ++;

			//move tail of stack S to tail of stack Q.
			unsigned long tmp_ppn = tail_lirs[0].prev->ppn;
			struct lirs_entry *tmp_tail;
			tmp_tail = tail_lirs[0].prev;
			//change its state to -1
			tmp_tail->status = -1;

			

//			delete_page_lirs( tmp_tail );
			delete_page_lirs_stackS( tmp_tail );
			lirs_num[0] --;

			insert_page_lirs_tail( tmp_ppn, 1 );  // insert to stack Q			
			lirs_num[1] ++;

			stack_pruning();

			vpn2list_entry[vpn].vpn = vpn;
			vpn2list_entry[vpn].ppn = ppn;
		}
		else if (vpn2list_entry[vpn].status == -1){
			//new page add it to stack S.
			insert_stack_s(vpn);
			vpn2list_entry[vpn].status = 1; //first page to stack S, set it to HIR
			vpn2list_entry[vpn].vpn = vpn;
			vpn2list_entry[vpn].ppn = ppn;

			//also should add it to stack Q, because its a HIR block
			insert_page_lirs( ppn, 1);
			lirs_num[1] ++;
		}
		else{
			printf("error!, status = %d\n ", vpn2list_entry[vpn].status);
		}
		vpn2list_entry[ vpn ].in_local = 1;
		miss_num_lirs ++;
	}
	else{
		// hit situation
		//first check the page status, LIR or HIR page
		if( vpn2list_entry[vpn].status == 0 ){
			//move the page to head of stack S
//			delete_page_lirs( &vpn2list_entry[vpn] );
			delete_page_lirs_stackS( &vpn2list_entry[vpn] );
                        insert_stack_s(vpn);
			stack_pruning();			
		}
		else if( vpn2list_entry[vpn].status == 1 ){
			//change the page state and remove the page from Stack Q.
			vpn2list_entry[vpn].status = 0;
//			delete_page_lirs( &vpn2list_entry[vpn] );
			delete_page_lirs_stackS( &vpn2list_entry[vpn] );
			insert_stack_s(vpn);
			lirs_num[0] ++;
			
			
//			delete_page_lirs( &ppn2list_entry_stackQ[ppn] );
			delete_page_lirs_stackQ( &ppn2list_entry_stackQ[ppn] );


			lirs_num[1] --;
		}
		else if( vpn2list_entry[vpn].status == -1){
			vpn2list_entry[vpn].status = 1;
			insert_stack_s(vpn); // its a new HRS page, add the page to stack S
			
			//move the page to the head of stack Q
//			delete_page_lirs( &ppn2list_entry_stackQ[ppn] );
			delete_page_lirs_stackQ( &ppn2list_entry_stackQ[ppn] );
			insert_page_lirs( ppn, 1  );
		}		
		else{
			printf("Error hit in page, state: [%d], vpn = %lu, ppn = %lu\n", vpn2list_entry[vpn], vpn, ppn);
		}
		hit_num_lirs ++;	
	}
}

void print_analysis_lirs(){
//	printf("hit num = %lu, miss num = %lu\n", hit_num_lirs, miss_num_lirs);

	printf("stackS num = %lu, stackQ num = %lu, all = %lu\n", lirs_num[0], lirs_num[1], lirs_num[0] + lirs_num[1]);        
	printf("real stackS [%lu], stackQ [%lu] \n", lirs_num_real[0], lirs_num_real[1]);
	printf("miss_num = %lu, hit_num = %lu, sum = %lu\n", miss_num_lirs, hit_num_lirs, miss_num_lirs + hit_num_lirs );
	printf("rate = %lf\n", double(miss_num_lirs)/ (double)( miss_num_lirs + hit_num_lirs ));
	hit_num_lirs = 0;
	miss_num_lirs = 0;
}


