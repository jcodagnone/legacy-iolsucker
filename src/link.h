#ifndef D4AFC5AB48A98F57BB9476499411ED651
#define D4AFC5AB48A98F57BB9476499411ED651

typedef void (*link_callback)(const char *link, const char *callback, void *);
typedef struct link_parserCDT * link_parser_t;


link_parser_t link_parser_new(void);
void link_parser_set_debug(link_parser_t parser, int b);
void link_parser_destroy(link_parser_t parser);
void link_parser_set_link_callback(link_parser_t parser,link_callback call,
                                    void *d );

int link_parser_proccess_char( link_parser_t parser, int c );

void link_parser_end(link_parser_t parser);

#endif
