
/*
 * cache.h
 */





/* structure definitions */
typedef struct cache_line_ {
  unsigned tag;
  int dirty;

  struct cache_line_ *LRU_next;
  struct cache_line_ *LRU_prev;
} cache_line, *Pcache_line;

typedef struct cache_ {
  int exist;
  int size;			/* cache size */
  int associativity;		/* cache associativity */
  int n_sets;			/* number of cache sets */
  unsigned index_mask;		/* mask to find cache index */
  int index_mask_offset;	/* number of zero bits in mask */
  Pcache_line *LRU_head;	/* head of LRU list for each set */
  Pcache_line *LRU_tail;	/* tail of LRU list for each set */
  int *set_contents;		/* number of valid entries in set */
  int contents;			/* number of valid entries in cache */
  int write_back;
} cache, *Pcache;

typedef struct cache_stat_ {
  int accesses;			/* number of memory references */
  int misses;			/* number of cache misses */
  int replacements;		/* number of misses that cause replacments */
  int demand_fetches;		/* number of fetches */
  int copies_back;		/* number of write backs */
} cache_stat, *Pcache_stat;


/* function prototypes */
void set_cache_param();
void init_cache();
void perform_access();
void flush();
void delete();
void insert();
void dump_settings();
void print_stats();
void i_init(int exis,int size, int assoc,int bsize,int wb);
void d_init(int exis,int size, int assoc,int bsize,int wb);


/* macros */
#define LOG2(x) ((int) rint((log((double) (x))) / (log(2.0))))
