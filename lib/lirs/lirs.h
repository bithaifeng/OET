

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


struct page_list{
	        unsigned long ppn;
		        struct page_list *prev;
			        struct page_list *next;
};


struct lirs_entry{
	unsigned long vpn,ppn;
	int status; //LIR or HIR // -1 miss no init or dont in stack S, 0 means LIR, 1 means HIR , 2 means non-resident block
	int in_local; // 1 means in local, 0 means not in local.
	struct lirs_entry *prev, *next;
};

extern struct lirs_entry vpn2list_entry[ MAX_PPN ];
extern struct lirs_entry ppn2list_entry_stackQ[ MAX_PPN ];


//extern void check_pn_lirs(unsigned long page_number);
extern void check_pn_lirs(unsigned long page_number, unsigned long bitmap, unsigned long last_access_time);
extern void init_lirs(int cache_size);



extern int local_cache_size;

extern void print_analysis_lirs();
