#ifndef Z4BAFF45AF259E1BF90C41C12CDDFDC3B
#define Z4BAFF45AF259E1BF90C41C12CDDFDC3B

typedef struct stringsetCDT *stringset_t;

typedef enum  stringset_error {
	E_STRINGSET_OK,         /** no error */
	E_STRINGSET_NOTFOUND,   /** string is not included */
	E_STRINGSET_EXISTS,     /** already exists */
	E_STRINGSET_NOMEM,      /** needs more memory! */
	E_STRINGSET_INVALID     /** invalid argument */
} stringset_error_t;
/**
 * creates a set of strings 
 */
stringset_t stringset_new(void);
stringset_t stringset_destroy(stringset_t set);
int stringset_is_valid(stringset_t set);

stringset_error_t stringset_add(stringset_t set, const char *string);
stringset_error_t stringset_look(stringset_t set, const char *string);
stringset_error_t stringset_remove(stringset_t set, const char *string);
void stringset_list(stringset_t set, int (*fn)(const char *file, void *data), 
                    void *data );

#endif
