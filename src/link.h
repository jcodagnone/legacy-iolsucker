#ifndef D4AFC5AB48A98F57BB9476499411ED651
#define D4AFC5AB48A98F57BB9476499411ED651

typedef void (*link_callback)(const unsigned char *link,
                              const unsigned char *callback, void *);
typedef struct link_parserCDT * link_parser_t;

/** creates a parser object */
link_parser_t link_parser_new(void);

/** destroys a parser object */
void link_parser_destroy(link_parser_t parser);

/** enable printing debug information */
void link_parser_set_debug(link_parser_t parser, int b);

/** 
 * set the callback for the function that is called 
 * every time a link is found
 */
void link_parser_set_link_callback(link_parser_t parser,link_callback call,
                                    void *d );

/** feed the state machine with one character */
int link_parser_process_char( link_parser_t parser, int c );

void link_parser_end(link_parser_t parser);

#endif
