/* default time interval */
#define DEF_INTERVAL  1.0

enum BOOL_LITERALS {
  FALSE=0
  ,TRUE
};

struct mem_stats_str {
  double size;
  double resident;
};

typedef struct mem_stats_str* MemStats;

#define msSize(ms)	(ms->size)
#define msResident(ms)  (ms->resident)

#define maxof(x,y) ((x)>(y)?(x):(y))
