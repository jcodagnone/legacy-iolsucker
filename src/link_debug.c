/* autogenerado */


const char *link_debug(int state) {
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
		r = "ST_TAG_A_OTHER";
	else if( state==7 ) 
		r = "ST_TAG_A_H";
	else if( state==8 ) 
		r = "ST_TAG_A_HR";
	else if( state==9 ) 
		r = "ST_TAG_A_HRE";
	else if( state==10 ) 
		r = "ST_TAG_A_HREF";
	else if( state==11 ) 
		r = "ST_TAG_A_HREF_EQ";
	else if( state==12 ) 
		r = "ST_TAG_A_HREF_EQ_READ";

	return r;
}
