## Process this file with automake to produce Makefile.in
AUTOMAKE_OPTIONS = foreign

SUBDIRS = include source doc

EXTRA_DIST = README.WIN newtree.dsw ChangeLog \
	RPM/build_rpm.in RPM/quakeforge.spec.in \
	tools/gas2masm/Makefile tools/gas2masm/gas2masm.c \
	tools/gas2masm/gas2masm.dsp tools/gas2masm/gas2masm.dsw \
	tools/gas2masm/gas2masm.mak tools/gas2masm/gas2masm.mdp \
	tools/zpak

changelog::
	-touch ChangeLog
	-tools/cvs2cl/cvs2cl.pl -b
	-rm -f ChangeLog.bak

dist-zip: distdir
	-chmod -R a+r $(distdir)
	ZIP="-r9ql" zip $(distdir).zip $(distdir)
	-rm -rf $(distdir)

dist-bz2: distdir
	-chmod -R a+r $(distdir)
	BZIP2="-9" $(TAR) Ichof $(distdir).tar.bz2 $(distdir)
	-rm -rf $(distdir)

dist-all-local: distdir
	-chmod -R a+r $(distdir)
	GZIP=$(GZIP_ENV) $(TAR) chozf $(distdir).tar.gz $(distdir)
	BZIP2="-9" $(TAR) Ichof $(distdir).tar.bz2 $(distdir)
	ZIP="-r9ql" zip $(distdir).zip $(distdir)
	-rm -rf $(distdir)
