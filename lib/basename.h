#ifndef _BASENAME_H_
#define _BASENAME_H_

#ifdef __cplusplus
extern "C" {
#endif

/* for GNU includes */
#ifndef basename
#define basename	basename_

/* basename()
 *
 * strip directory and suffix from filenames  
 */
const char *
basename_( const char *path );

#endif

#ifdef __cplusplus
}
#endif

#endif
