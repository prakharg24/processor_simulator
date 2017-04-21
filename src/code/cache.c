/*
 * cache.c
 */


#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "cache.h"
#include "main.h"

/* cache configuration parameters */
static int cache_split = 0;
static int cache_usize = DEFAULT_CACHE_SIZE;
static int cache_isize = DEFAULT_CACHE_SIZE; 
static int cache_dsize = DEFAULT_CACHE_SIZE;
static int cache_block_size = DEFAULT_CACHE_BLOCK_SIZE;
static int words_per_block = DEFAULT_CACHE_BLOCK_SIZE / WORD_SIZE;
static int cache_assoc = DEFAULT_CACHE_ASSOC;
static int cache_writeback = DEFAULT_CACHE_WRITEBACK;
static int cache_writealloc = DEFAULT_CACHE_WRITEALLOC;

/* cache model data structures */
static Pcache icache;
static Pcache dcache;
static cache c1;
static cache c2;
static cache_stat cache_stat_inst;
static cache_stat cache_stat_data;

/************************************************************/
void set_cache_param(param, value)
	int param;
	int value;
{

	switch (param) {
	case CACHE_PARAM_BLOCK_SIZE:
		cache_block_size = value;
		words_per_block = value / WORD_SIZE;
		break;
	case CACHE_PARAM_USIZE:
		cache_split = FALSE;
		cache_usize = value;
		break;
	case CACHE_PARAM_ISIZE:
		cache_split = TRUE;
		cache_isize = value;
		break;
	case CACHE_PARAM_DSIZE:
		cache_split = TRUE;
		cache_dsize = value;
		break;
	case CACHE_PARAM_ASSOC:
		cache_assoc = value;
		break;
	case CACHE_PARAM_WRITEBACK:
		cache_writeback = TRUE;
		break;
	case CACHE_PARAM_WRITETHROUGH:
		cache_writeback = FALSE;
		break;
	case CACHE_PARAM_WRITEALLOC:
		cache_writealloc = TRUE;
		break;
	case CACHE_PARAM_NOWRITEALLOC:
		cache_writealloc = FALSE;
		break;
	default:
		printf("error set_cache_param: bad parameter value\n");
		exit(-1);
	}

}
/************************************************************/

/************************************************************/
void init_cache()
{

	/* initialize the cache, and cache statistics data structures */
	if(cache_split==0)
	{
		c1.size = cache_usize;
		c1.associativity = cache_assoc;
		c1.n_sets = cache_usize / (words_per_block * c1.associativity);
		c1.index_mask = WORD_SIZE * words_per_block * c1.n_sets - 1;
		c1.index_mask_offset = LOG2(WORD_SIZE) + LOG2(words_per_block) ;
		c1.LRU_head = (Pcache_line *)malloc(sizeof(Pcache_line)*c1.n_sets) ;
		c1.LRU_tail = (Pcache_line *)malloc(sizeof(Pcache_line)*c1.n_sets) ;
		c1.set_contents = (int *)malloc(sizeof(int)*c1.n_sets);
		for(int i=0;i<c1.n_sets;i++)
		{
			c1.set_contents[i] = 0;
		}

	}
	else
	{
		c1.size = cache_isize;
		c1.associativity = cache_assoc;
		c1.n_sets = cache_isize / (words_per_block * c1.associativity);
		c1.index_mask = WORD_SIZE * words_per_block * c1.n_sets - 1;
		c1.index_mask_offset = LOG2(WORD_SIZE) + LOG2(words_per_block) ;
		c1.LRU_head = (Pcache_line *)malloc(sizeof(Pcache_line)*c1.n_sets) ;
		c1.LRU_tail = (Pcache_line *)malloc(sizeof(Pcache_line)*c1.n_sets) ;
		c1.set_contents = (int *)malloc(sizeof(int)*c1.n_sets);
		for(int i=0;i<c1.n_sets;i++)
		{
			c1.set_contents[i] = 0;
		}
		c2.size = cache_dsize;
		c2.associativity = cache_assoc;
		c2.n_sets = cache_dsize / (words_per_block * c2.associativity);
		c2.index_mask = WORD_SIZE * words_per_block * c2.n_sets - 1;
		c2.index_mask_offset = LOG2(WORD_SIZE) + LOG2(words_per_block) ;
		c2.LRU_head = (Pcache_line *)malloc(sizeof(Pcache_line)*c2.n_sets) ;
		c2.LRU_tail = (Pcache_line *)malloc(sizeof(Pcache_line)*c2.n_sets) ;
		c2.set_contents = (int *)malloc(sizeof(int)*c2.n_sets);
		for(int i=0;i<c2.n_sets;i++)
		{
			c2.set_contents[i] = 0;
		}
	}
	



	cache_stat_inst.accesses = 0;		
	cache_stat_inst.misses = 0;		
	cache_stat_inst.replacements = 0;		
	cache_stat_inst.demand_fetches = 0;		
	cache_stat_inst.copies_back = 0;	

	cache_stat_data.accesses = 0;		
	cache_stat_data.misses = 0;		
	cache_stat_data.replacements = 0;		
	cache_stat_data.demand_fetches = 0;		
	cache_stat_data.copies_back = 0;	
}
/************************************************************/

/************************************************************/


void do_work(cache *c2,unsigned addr,unsigned access_type,cache_stat *stat)
{
	stat->accesses++;
	unsigned index = (addr & c2->index_mask) >> c2->index_mask_offset;
	unsigned tag = addr >> (c2->index_mask_offset + LOG2(c2->n_sets)) ;
	int flag = -1;
	Pcache_line temp = c2->LRU_head[index];
	for(int i=0;i<c2->set_contents[index];i++)
	{
		if(temp->tag==tag)
		{
			flag = 1;
			break;
		}
		temp = temp->LRU_next;
	}
	if(flag==-1)
	{

		
		stat->misses++;
		if(cache_writealloc==0 && access_type==1)
		{
			stat->copies_back++;                    // assume in write - back without allocate block is not read
			return;
		}
		stat->demand_fetches++;
		if(c2->set_contents[index]<c2->associativity)
		{

			Pcache_line new = (Pcache_line)malloc(sizeof(cache_line));
			c2->set_contents[index]++;
			new->tag = tag;
			if(access_type==2 || (cache_writeback==0))
			{
				new->dirty = 0;
			}
			else
			{
				new->dirty = access_type;
			}
			insert(&c2->LRU_head[index],&c2->LRU_tail[index],new);
			if(access_type==1 && cache_writeback==0)
			{
				stat->copies_back++;
			}
			
		}
		else
		{
			if(c2->LRU_tail[index]->dirty==1)
			{
				stat->copies_back++;
			}
			delete(&c2->LRU_head[index],&c2->LRU_tail[index],c2->LRU_tail[index]);
			Pcache_line new = (Pcache_line)malloc(sizeof(cache_line));
			new->tag = tag;
			if(access_type==2 || (cache_writeback==0))
			{
				new->dirty = 0;
			}
			else
			{
				new->dirty = access_type;
			}		
			insert(&c2->LRU_head[index],&c2->LRU_tail[index],new);
			if(access_type==1 && cache_writeback==0)
			{
				stat->copies_back++;
			}
			stat->replacements++;
		}

	}
	else
	{
		Pcache_line new = (Pcache_line)malloc(sizeof(cache_line));
		new->tag = tag;
		if(access_type==1)
		{
			if(cache_writeback==0)
			{
				new->dirty = 0;
				stat->copies_back++;
			}
			else
				new->dirty = 1;
		}
		else
		{
			new->dirty = temp->dirty;
		}
		delete(&c2->LRU_head[index],&c2->LRU_tail[index],temp);
		insert(&c2->LRU_head[index],&c2->LRU_tail[index],new);
	}	
}




void perform_access(addr, access_type)
	unsigned addr, access_type;
{

	//printf("sdfs\n");

	if(cache_split==0)
	{
		if(access_type==0 || access_type==1)
		{
			do_work(&c1,addr,access_type,&cache_stat_data);
		}
		else
		{
			do_work(&c1,addr,access_type,&cache_stat_inst);
		}
	}
	else
	{
		if(access_type==0 || access_type==1)
		{
			do_work(&c2,addr,access_type,&cache_stat_data);
		}
		else
		{
			do_work(&c1,addr,access_type,&cache_stat_inst);
		}

	}

}



/************************************************************/

/************************************************************/

void flush_work(cache *c2,cache_stat *stat)
{
	for(int i=0;i<c2->n_sets;i++)
	{
		Pcache_line temp = c2->LRU_head[i];
		for(int j=0;j<c2->set_contents[i];j++)
		{
			if(temp->dirty==1)
			{
				stat->copies_back++;
			}
			temp = temp->LRU_next;
		}
	}
}



void flush()
{


	if(cache_split==0)
	{
		flush_work(&c1,&cache_stat_data);
	}
	else
	{
		flush_work(&c1,&cache_stat_inst);
		flush_work(&c2,&cache_stat_data);
	}


	/* flush the cache */

}
/************************************************************/

/************************************************************/
void delete(head, tail, item)
	Pcache_line *head, *tail;
	Pcache_line item;
{
	if (item->LRU_prev) {
		item->LRU_prev->LRU_next = item->LRU_next;
	} else {
		/* item at head */

		*head = item->LRU_next;

	}

	if (item->LRU_next) {
		item->LRU_next->LRU_prev = item->LRU_prev;
	} else {
		/* item at tail */
		*tail = item->LRU_prev;

	}

}
/************************************************************/

/************************************************************/
/* inserts at the head of the list */
void insert(head, tail, item)
	Pcache_line *head, *tail;
	Pcache_line item;
{
	item->LRU_next = *head;
	item->LRU_prev = (Pcache_line)NULL;

	if (item->LRU_next)
		item->LRU_next->LRU_prev = item;
	else
		*tail = item;

	*head = item;
}
/************************************************************/

/************************************************************/
void dump_settings()
{
	printf("*** CACHE SETTINGS ***\n");
	if (cache_split) {
		printf("  Split I- D-cache\n");
		printf("  I-cache size: \t%d\n", cache_isize);
		printf("  D-cache size: \t%d\n", cache_dsize);
	} else {
		printf("  Unified I- D-cache\n");
		printf("  Size: \t%d\n", cache_usize);
	}
	printf("  Associativity: \t%d\n", cache_assoc);
	printf("  Block size: \t%d\n", cache_block_size);
	printf("  Write policy: \t%s\n", 
	 cache_writeback ? "WRITE BACK" : "WRITE THROUGH");
	printf("  Allocation policy: \t%s\n",
	 cache_writealloc ? "WRITE ALLOCATE" : "WRITE NO ALLOCATE");
}
/************************************************************/

/************************************************************/
void print_stats()
{
	printf("\n*** CACHE STATISTICS ***\n");

	printf(" INSTRUCTIONS\n");
	printf("  accesses:  %d\n", cache_stat_inst.accesses);
	printf("  misses:    %d\n", cache_stat_inst.misses);
	if (!cache_stat_inst.accesses)
		printf("  miss rate: 0 (0)\n"); 
	else
		printf("  miss rate: %2.4f (hit rate %2.4f)\n", 
	 (float)cache_stat_inst.misses / (float)cache_stat_inst.accesses,
	 1.0 - (float)cache_stat_inst.misses / (float)cache_stat_inst.accesses);
	printf("  replace:   %d\n", cache_stat_inst.replacements);

	printf(" DATA\n");
	printf("  accesses:  %d\n", cache_stat_data.accesses);
	printf("  misses:    %d\n", cache_stat_data.misses);
	if (!cache_stat_data.accesses)
		printf("  miss rate: 0 (0)\n"); 
	else
		printf("  miss rate: %2.4f (hit rate %2.4f)\n", 
	 (float)cache_stat_data.misses / (float)cache_stat_data.accesses,
	 1.0 - (float)cache_stat_data.misses / (float)cache_stat_data.accesses);
	printf("  replace:   %d\n", cache_stat_data.replacements);

	printf(" TRAFFIC (in words)\n");
	printf("  demand fetch:  %d\n", 4 * (cache_stat_inst.demand_fetches + 
	 cache_stat_data.demand_fetches));
	printf("  copies back:   %d\n", 4 * (cache_stat_inst.copies_back +
	 cache_stat_data.copies_back));
}
/************************************************************/
