#ifndef _IOL_H_
#define _IOL_H_

typedef struct iolCDT *iol_t;

enum iol_settings {
	IOL_REPOSITORY,	/**< sets the file repository dir. data: path for dir */
	IOL_PROXY_HOST, /**< sets the host proxy setting. data: <host[:port]> */
	IOL_PROXY_USER, /**< stes the user proxy setting. data: <user[:pass]> */
	IOL_MAX
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
int   iol_resync(iol_t iol, const char *code);

/** resync all the courses that the server says we have */
int   iol_resync_all(iol_t iol);

/** change the setting */
int   iol_set(iol_t iol, enum iol_settings, void *data);


/**
 * errors
 */
enum errors
{	E_OK,		/**< no error */
	E_INVAL,	/**< invalid arguments */
	E_ALOGED,	/**< already logged */
	E_NLOGED,	/**< not logued */
	E_NETWORK,	/**< network error */
	E_MEMORY,	/**< need to take some Memorol 500 pills :) */
	E_FS,		/**< error in the filesystem I/O */
	E_MAXERROR	
};

#endif

