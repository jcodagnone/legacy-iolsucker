MAKE=make
export WANT_AUTOMAKE=1.7
export WANT_AUTOCONF=2.58

echo "*** Retrieving configure tests needed by configure.in"
aclocal$AMSUFFIX -I . -I admin -I admin/extra
echo "*** Scanning for include statements"
autoheader
echo "*** Building Makefile templates (step one)"
automake$AMSUFFIX -a
echo "*** Building Makefile templates (step two)"
autoconf
if grep "ac_kw foo" configure &>/dev/null; then perl -p -i -e "s/ac_kw foo/ac_kw int foo/" configure; fi
perl -pi -e 'if (/\[\/\$$\]\*. INSTALL=/) { print $$_ ; $$_ = "\"\") ;;\n"; }' configure
rm -f config.cache config.h
echo "*** Create date/time stamp"
touch stamp-h.in
echo "*** Finished"
echo "    Don't forget to run ./configure"
echo "    If you haven't done so in a while, run ./configure --help"
