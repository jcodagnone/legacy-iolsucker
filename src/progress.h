#ifndef Z03F9D58DD2E35AB2A85D93790F6FA55D
#define Z03F9D58DD2E35AB2A85D93790F6FA55D

struct progress * 
new_progress_callback(void *callback);

void 
destroy_progress_callback(struct progress *progress);

int 
dot_progress_callback(struct progress *p, double dltotal, double dlnow,
                      double ultotal, double ulnow);

int 
bar_progress_callback( struct progress *p, double dltotal, double dlnow,
                       double ultotal, double ulnow);

#endif
