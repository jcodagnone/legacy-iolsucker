#include "iol.h"

const char *
iol_strerror(int code)
{	const char *r;

	switch(code)
	{
		case E_INVAL:
			r = "invalid arguments";
			break;
		case E_ALOGED:
			r = "already logged";
			break;
		case E_NLOGED:
			r = "not logued";
			break;
		case E_LOGINTUPLE:
			r = "invalid login tuple";
			break;
		case E_NETWORK:
			r = "network error";
			break;
		case E_MEMORY:
			r = "need to take some Memorol 500 pills :)";
			break;
		case E_FS:
			r = "error in the filesystem I/O";
			break;
		case E_USERCANCEL:
			r = "user canceled operation";
			break;
		case E_MAXERROR:
			r = "E_MAXERROR";
			break;
		default:
			r = "";
	}

	return r;
}
