#!/usr/bin/awk  -f
#
# Copyright (c) 2003 Juan F. Codagnone <juam@users.sourceforge.net>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
BEGIN {
	print "#include \"iol.h\""
	print ""
	print "const char *";
	print "iol_strerror(int code)"
	print "{	const char *r;"
	print ""
	print "\tswitch(code)"
	print "\t{"
}
/^\tE_/{
	gsub(",","",$1)
	gsub("\"","\\\"")
	printf("\t\tcase %s:\n",$1)
	printf("\t\t\tr = \"")
	if($3)
	{	for(i=3; i<NF; i++)
		{	printf("%s",$i)
			if( i!=NF-1)
				printf(" ");
		}
	}
	else
		printf("%s",$1);
	printf("\";\n\t\t\tbreak;\n");
	
}
END {
	printf("\t\tdefault:\n\t\t\tr = \"\";\n\t}\n\n");
	printf("\treturn r;\n}\n");
}
