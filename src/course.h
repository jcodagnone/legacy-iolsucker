#ifndef Z39A8E160E826019532A53D9883FD7B00
#define Z39A8E160E826019532A53D9883FD7B00

#include "iol.h"

enum course_type {
	CT_COURSE,
	CT_DEPART
};

struct course 
{       char *code;	/**< code name: eg: "21.71" */
	char *name;     /**< human name: Base de datos I */
	enum course_type type; 
	enum resync_flags flags;
};

typedef struct course_t *course_t;
typedef void (* course_callback_t)(const struct course *course, void *data);

course_t     course_new(void);
int          course_is_valid(course_t c);
void         course_destroy(course_t c);

/**
 * carga la lista de cursos parseando el contenido de page
 */
int course_load_from_page(course_t c, struct buff *page);

/**
 * corre callback por cada curso que está cargado
 */
void course_foreach_run( course_t c, course_callback_t callback,void *data);

/**
 * busca la materia con codigo `code'. Si no la encuentra, retorna NULL.
 */
struct course *
course_get_by_name( course_t c, const char *code );

/** 
 * code (codigo de materia) está formado por caracteres validos? 
 * metodo estatico
 */
int course_name_is_valid( const char *code );


/** 
 * metodo estatico: obtiene las capacidades de alguna materia, parseando 
 * el contenido de la página page.
 * metodo statico
 */
enum resync_flags course_get_capabilities_from_page( struct buff *page);


#endif
