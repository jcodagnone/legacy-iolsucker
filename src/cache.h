#ifndef ZC7324B1D9A08F10305B2F9CDA9597CD9
#define ZC7324B1D9A08F10305B2F9CDA9597CD9


typedef struct cache *cache_t;

cache_t cache_new(const char *dbpath);
void    cache_destroy(cache_t cdt);
int     cache_is_valid(cache_t cdt);
int     cache_add_file( cache_t cdt, const char *id, const char *file );
char *  cache_get_file( cache_t cdt, const char *id);
char *  cache_version(char *buf, unsigned nbuf);
#endif
