#ifndef _IOL_H_
#define _IOL_H_

#include <list>
#include <queue>

struct materia
{	const char *name;
	unsigned short cod;
};


class IOL
{
public:
	IOL();
	~IOL();
	int login(const char *user, const char *pass);
	int logout();
	
	/**
	 *  resync a course
	 */
	int resync(const char *code);
	
	/**
	 * resync all courses
	 */
	int resync_all();
private:
	/**
	 * Get all the files for the current subject
	 */
	int get_file_list_recursive(const char *url, std::list<char *> *files,
	                            std::queue<char *> *pending);

	int get_file_list(std::list<char *> &l);
	int set_current_course(const char *course);
	unsigned load_courses(struct buff *page);
	int transfer_page( const char *url, unsigned flags, struct buff *);
	struct hidden *cdt;
};

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

