#ifndef _IOL_H_
#define _IOL_H_

typedef struct iolCDT *iol_t;

enum iol_settings {
	IOL_REPOSITORY,	/**< sets the file repository dir. data: path for dir */
	IOL_PROXY_TYPE, /**< sets the type of the proxy. data: <http|sock5>*/
	IOL_PROXY_HOST, /**< sets the host proxy setting. data: <host[:port]> */
	IOL_PROXY_USER, /**< stes the user proxy setting. data: <user[:pass]> */
	IOL_DRY,	/**< dry run? default FALSE. data: (int *) */
	IOL_VERBOSE,	/**< verbose information. default FALSE. data: (int *)*/
	IOL_FANCY_NAMES,/**< course fancy names? default FALSE. data: (int *)*/
	IOL_MAX
};

enum resync_flags {
	IOL_RF_FILE  = 1<<0,	/**< resync files */	
	IOL_RF_NEWS  = 1<<1,	/**< resync the news */
	IOL_RF_FORUM = 1<<2,	/**< resync the forum */
	IOL_RF_MAX
};

/** creates a new IOL object */
iol_t iol_new(void);

/** destroy an existant IOL object. can destroy NULL */
void  iol_destroy(iol_t iol);

/** login in to IOL as usernemae @user */
int   iol_login(iol_t iol, const char *user, const char *pass);

/** logout from  a logged-in session */
int   iol_logout(iol_t iol);

/** resync all the files of the curse code @code */
int   iol_resync(iol_t iol, const char *code, enum resync_flags flags);

/** resync all the courses that the server says we have */
int   iol_resync_all(iol_t iol, enum resync_flags flags);

/** change the setting */
int   iol_set(iol_t iol, enum iol_settings, void *data);

const char *iol_get_network_error(iol_t iol);

const char *iol_strerror(int code);

int iol_get_new_novedades( iol_t iol );

/**
 * errors
 */
enum errors
{	E_OK,		/**< no error */
	E_INVAL,	/**< invalid arguments */
	E_ALOGED,	/**< already logged */
	E_NLOGED,	/**< not logued */
	E_LOGINTUPLE,	/**< invalid login tuple */
	E_NETWORK,	/**< network error */
	E_MEMORY,	/**< need to take some Memorol 500 pills :) */
	E_FS,		/**< error in the filesystem I/O */
	E_MAXERROR	
};

#endif

