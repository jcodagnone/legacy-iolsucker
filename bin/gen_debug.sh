grep '^\/\*extract\*\/'|
sed 's/,//g'|
awk '
	BEGIN{i=0
	print "/* autogenerado */\n\n"
	print "#ifdef GEN_LIN"
	print "static const char *link_debug(int state) {" 
	print "\tconst char *r;"
	print "";
}
{	if( i != 0 )
		printf "\telse "
	else
		printf "\t"
	print "\if( state==" i++ " ) \n\t\tr = \""$2"\";"
}
	END {
		print "";
		print "\treturn r;\n}"
		print "#endif"
}'
