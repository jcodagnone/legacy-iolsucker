grep '@fextract@'|
sed 's/[(]/ /g'|
awk '
BEGIN {
	print "#ifdef GEN_LIN"
	print "static const char * links_fnc_name(void *p) {"
	print "\tconst char *r=\"\";\n"
}
{	
	print "\t" el"if (p == &"$1") r=\""$1"\";"
	el="else "
}
END {
	print "\n\treturn r;\n}"
	print "#endif"
}'
