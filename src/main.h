#ifndef Z379A95B12D99D055293635656DD491C6
#define Z379A95B12D99D055293635656DD491C6

/* magic numbers 
 */
enum {
	MAX_USERNAME = 16,
	MAX_PASSWORD = 128,
	MAX_CONFIGFILE = 512,
	MAX_REPOSITORY = MAX_CONFIGFILE
};

/**
 * command line options
 */
struct opt {
	char username[MAX_USERNAME];
	char password[MAX_PASSWORD];
	char configfile[MAX_CONFIGFILE];
	char repository[MAX_REPOSITORY];
	char *proxy;
	char *proxy_user;
	char *proxy_type;
	char *server;
	int dry;
	int verbose;
	int fancy;
	int forum;
	int wait;
	int xenofobe;
	int no_cache;
};

/** parse command line options */
int parseOptions( int argc, char * const * argv, struct opt *opt);

/** free a struct opt structure */
void free_options( struct opt *opt );

#endif
