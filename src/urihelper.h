#ifndef Z87FA161D94157FA5814CD54D6BDD51FA
#define Z87FA161D94157FA5814CD54D6BDD51FA


/* backport de ar.com.leak.iolsucker.impl.http.util.URIHelper
 */
typedef struct urihelper * urihelper_t;

urihelper_t urihelper_new(const char *name);
void urihelper_destroy(urihelper_t c);


/**
 * @return Ej. si uri = pepe.asp?a=1&b=2, retornaria pepe.asp
 */
const char * urihelper_getbase(urihelper_t cdt);

/**
 * @param param parametro
 * @return <code>true</code> si está presente determinado
 * parámetro
 */
int urihelper_has_param(urihelper_t cdt, const char *s);

const char *urihelper_get_param(urihelper_t cdt, const char *s);
unsigned urihelper_size(urihelper_t cdt);


#endif

