#ifndef Z379A95B12D99D055293635656DD491C6
#define Z379A95B12D99D055293635656DD491C6


enum {
	MAX_USERNAME = 16,
	MAX_PASSWORD = 128,
	MAX_CONFIGFILE = 512,
	MAX_REPOSITORY = MAX_CONFIGFILE
};

struct opt {
	char username[MAX_USERNAME];
	char password[MAX_PASSWORD];
	char configfile[MAX_CONFIGFILE];
	char repository[MAX_REPOSITORY];
	char *proxy;
	char *proxy_user;
	int dry;
	char *proxy_type;
};

int
parseOptions( int argc, char * const * argv, struct opt *opt);

void
free_options( struct opt *opt );

#endif
