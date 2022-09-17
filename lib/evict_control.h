
#define MAX_PPN (64ULL << 18)

extern int local_cache_size;

struct page_list{
	unsigned long ppn;
	struct page_list *prev;
	struct page_list *next;
};


