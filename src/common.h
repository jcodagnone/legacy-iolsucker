#ifndef Z037D1D81C4C4C6B154A6785012648886
#define Z037D1D81C4C4C6B154A6785012648886

/**
 * Ej: 
 *    http://www.a.com/aa.html
 *    http://www.a.com/ab.html
 *    http://www.a.com/ac.html
 *  Returns -> http://www.a.com/
 */
char *
get_common_startpath(GSList *list);

/* this should not be here, but.... */
struct buff 
{ 	char *data;
	size_t size;
};



#endif
