#ifndef _IOL_H_
#define _IOL_H_

typedef struct iolCDT *iol_t;

iol_t iol_new(void);
void  iol_destroy(iol_t iol);
int   iol_login(iol_t iol, const char *user, const char *pass);
int   iol_logout(iol_t iol);
int   iol_resync(iol_t iol, const char *code);
int   iol_resync_all(iol_t iol);
int   iol_set_repository(iol_t iol, const char *path);;

/*
int get_file_list_recursive(const char *url, std::list<char *> *files,
	                            std::queue<char *> *pending);

	int get_file_list(std::list<char *> &l);
	int set_current_course(const char *course);
	unsigned load_courses(struct buff *page);
	int transfer_page( const char *url, unsigned flags, struct buff *);
	struct hidden *cdt;
};
*/
enum errors
{	E_OK,		/* no error */
	E_INVAL,	/* invalid arguments */
	E_ALOGED,	/* already logged */
	E_NLOGED,	/* not logued */
	E_NETWORK,	/* network error */
	E_MEMORY,	/* need to take some Memorol 500 pills :) */
	E_FS,		/* error in the filesystem I/O */
	E_MAXERROR	
};

#endif

