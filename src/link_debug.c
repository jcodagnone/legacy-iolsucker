/* autogenerado */


#ifdef GEN_LIN
static const char *link_debug(int state) {
	const char *r;

	if( state==0 ) 
		r = "ST_START";
	else if( state==1 ) 
		r = "ST_TAG";
	else if( state==2 ) 
		r = "ST_ISTAG_A";
	else if( state==3 ) 
		r = "ST_OTHERTAG";
	else if( state==4 ) 
		r = "ST_TAG_A";
	else if( state==5 ) 
		r = "ST_TAG_A_END";
	else if( state==6 ) 
		r = "ST_TAG_A_END_IS_SLASH";
	else if( state==7 ) 
		r = "ST_TAG_A_END_IS_SLASH_A";
	else if( state==8 ) 
		r = "ST_TAG_A_OTHER";
	else if( state==9 ) 
		r = "ST_TAG_A_H";
	else if( state==10 ) 
		r = "ST_TAG_A_HR";
	else if( state==11 ) 
		r = "ST_TAG_A_HRE";
	else if( state==12 ) 
		r = "ST_TAG_A_HREF";
	else if( state==13 ) 
		r = "ST_TAG_A_HREF_EQ";
	else if( state==14 ) 
		r = "ST_TAG_A_HREF_EQ_READ";
	else if( state==15 ) 
		r = "ST_TAG_A_END_IS_SLASH_A_OTHER";
	else if( state==16 ) 
		r = "ST_TAG_A_END_IS_A_AGAIN";

	return r;
}
#endif
#ifdef GEN_LIN
static const char * links_fnc_name(void *p) {
	const char *r="";

	if (p == &isspace) r="isspace";
	else if (p == &init_parser) r="init_parser";
	else if (p == &link_first_char) r="link_first_char";
	else if (p == &endlink) r="endlink";
	else if (p == &add_char) r="add_char";
	else if (p == &addcomment) r="addcomment";
	else if (p == &e_is_slash) r="e_is_slash";
	else if (p == &e_slash_a) r="e_slash_a";
	else if (p == &e_slash_a_other) r="e_slash_a_other";
	else if (p == &done_link) r="done_link";
	else if (p == &embeeded_goto_end) r="embeeded_goto_end";

	return r;
}
#endif
