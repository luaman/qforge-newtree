# Generated automatically from Makefile.in by configure.
# Makefile.in generated automatically by automake 1.4 from Makefile.am

# Copyright (C) 1994, 1995-8, 1999 Free Software Foundation, Inc.
# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.


SHELL = /bin/sh

srcdir = .
top_srcdir = ..
prefix = /usr/local
exec_prefix = ${prefix}

bindir = ${exec_prefix}/bin
sbindir = ${exec_prefix}/sbin
libexecdir = ${exec_prefix}/libexec
datadir = ${prefix}/share
sysconfdir = ${prefix}/etc
sharedstatedir = ${prefix}/com
localstatedir = ${prefix}/var
libdir = ${exec_prefix}/lib
infodir = ${prefix}/info
mandir = ${prefix}/man
includedir = ${prefix}/include
oldincludedir = /usr/include

DESTDIR =

pkgdatadir = $(datadir)/quakeforge
pkglibdir = $(libdir)/quakeforge
pkgincludedir = $(includedir)/quakeforge

top_builddir = ..

ACLOCAL = aclocal
AUTOCONF = autoconf
AUTOMAKE = automake
AUTOHEADER = autoheader

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL} $(AM_INSTALL_PROGRAM_FLAGS)
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_SCRIPT = ${INSTALL_PROGRAM}
transform = s,x,x,

NORMAL_INSTALL = :
PRE_INSTALL = :
POST_INSTALL = :
NORMAL_UNINSTALL = :
PRE_UNINSTALL = :
POST_UNINSTALL = :
build_alias = i686-pc-linux-gnu
build_triplet = i686-pc-linux-gnu
host_alias = i686-pc-linux-gnu
host_triplet = i686-pc-linux-gnu
target_alias = i686-pc-linux-gnu
target_triplet = i686-pc-linux-gnu
ASM_ARCH = 
AWK = gawk
CC = gcc
CL_SYS_SRC = sys_linux.c
CPP = gcc -E
EXEEXT = 
HAVE_OPENGL = yes
HAVE_SVGA = yes
HAVE_X11 = yes
INTEL_ARCH = 
ISODATE = 2000-05-17
LIBS = -lm 
LN_S = ln -s
MAKEINFO = makeinfo
OBJEXT = o
OGL_CFLAGS = 
OGL_INCLUDES = 
OGL_LIBS = 
PACKAGE = quakeforge
QSG_VERSION = 1.0
QW_VERSION = 2.40
SVGA_CFLAGS = 
SVGA_LIBS =  -lvga
SV_SYS_SRC = sys_unix.c
TARGETS = qf-server qf-client-svga qf-client-x11 qf-client-glx
VERSION = 0.1.99pre0
X11_SHM_LIB = -lXext
X11_VID_SRC = vid_x11.c
X_CFLAGS =  -I/usr/X11R6/include
X_LIBS =  -L/usr/X11R6/lib
ac_configure_args = 
libdir = ${exec_prefix}/lib

bin_PROGRAMS = qf-server qf-client-svga qf-client-x11 qf-client-glx

EXTRA_PROGRAMS = qf-server qf-client-svga qf-client-x11 qf-client-glx

common_SOURCES = net_chan.c net_com.c net_udp.c pmove.c pmovetst.c zone.c 		mdfour.c mathlib.c math.S cvar.c crc.c common.c model.c cmd.c

server_SOURCES = pr_cmds.c pr_edict.c pr_exec.c sv_init.c sv_main.c sv_misc.c 		sv_model.c sv_nchan.c sv_ents.c sv_send.c sv_move.c sv_phys.c 		sv_user.c sv_ccmds.c world.c worlda.S sys_unix.c sv_cvar.c

client_SOURCES = cl_cmd.c cl_cvar.c cl_demo.c cl_ents.c cl_input.c cl_main.c 		cl_math.S cl_misc.c cl_parse.c cl_pred.c cl_tent.c cl_cam.c 		snd_linux.c sys_linux.c cd_linux.c r_view.c wad.c snd_dma.c 		snd_mem.c snd_mix.c snd_mixa.S skin.c sbar.c nonintel.c 		menu.c keys.c console.c sys_x86.S

soft_SOURCES = cl_model.c cl_trans.c d_edge.c d_fill.c d_init.c d_modech.c 		d_part.c d_polyse.c d_scan.c d_sky.c d_sprite.c d_surf.c 		d_vars.c d_zpoint.c draw.c r_aclip.c r_alias.c r_bsp.c 		r_draw.c r_edge.c r_efrag.c r_light.c r_main.c r_misc.c 		r_part.c r_sky.c r_sprite.c r_surf.c r_vars.c sw_view.c 		screen.c 		d_copy.S d_draw.S d_draw16.S d_parta.S d_polysa.S d_scana.S 		d_spr8.S d_varsa.S r_aclipa.S r_aliasa.S r_drawa.S r_edgea.S 		r_varsa.S surf16.S surf8.S

ogl_SOURCES = gl_draw.c gl_mesh.c gl_model.c gl_ngraph.c gl_part.c 		gl_refrag.c gl_rlight.c gl_rmain.c gl_rmisc.c gl_rsurf.c 		gl_screen.c gl_trans.c gl_view.c gl_warp.c


svga_SOURCES = vid_svgalib.c
glx_SOURCES = vid_glx.c
x11_SOURCES = vid_x11.c

qf_server_SOURCES = $(common_SOURCES) $(server_SOURCES)
qf_client_svga_SOURCES = $(common_SOURCES) $(client_SOURCES) $(soft_SOURCES) $(svga_SOURCES)
qf_client_x11_SOURCES = $(common_SOURCES) $(client_SOURCES) $(soft_SOURCES) $(x11_SOURCES)
qf_client_glx_SOURCES = $(common_SOURCES) $(client_SOURCES) $(ogl_SOURCES) $(glx_SOURCES)

qf_client_svga_LDADD =  -lvga
qf_client_x11_LDADD =  -lSM -lICE  -L/usr/X11R6/lib  -lX11 -lXext
qf_client_glx_LDADD = @OPENGL_LIBS@
mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
CONFIG_HEADER = ../include/config.h
CONFIG_CLEAN_FILES = 
bin_PROGRAMS =  qf-server qf-client-svga qf-client-x11 qf-client-glx
PROGRAMS =  $(bin_PROGRAMS)


DEFS = -DHAVE_CONFIG_H -I. -I$(srcdir) -I../include
CPPFLAGS = 
LDFLAGS = 
X_EXTRA_LIBS = 
X_PRE_LIBS =  -lSM -lICE
qf_server_OBJECTS =  net_chan.$(OBJEXT) net_com.$(OBJEXT) \
net_udp.$(OBJEXT) pmove.$(OBJEXT) pmovetst.$(OBJEXT) zone.$(OBJEXT) \
mdfour.$(OBJEXT) mathlib.$(OBJEXT) math.$(OBJEXT) cvar.$(OBJEXT) \
crc.$(OBJEXT) common.$(OBJEXT) model.$(OBJEXT) cmd.$(OBJEXT) \
pr_cmds.$(OBJEXT) pr_edict.$(OBJEXT) pr_exec.$(OBJEXT) \
sv_init.$(OBJEXT) sv_main.$(OBJEXT) sv_misc.$(OBJEXT) \
sv_model.$(OBJEXT) sv_nchan.$(OBJEXT) sv_ents.$(OBJEXT) \
sv_send.$(OBJEXT) sv_move.$(OBJEXT) sv_phys.$(OBJEXT) sv_user.$(OBJEXT) \
sv_ccmds.$(OBJEXT) world.$(OBJEXT) worlda.$(OBJEXT) sys_unix.$(OBJEXT) \
sv_cvar.$(OBJEXT)
qf_server_LDADD = $(LDADD)
qf_server_DEPENDENCIES = 
qf_server_LDFLAGS = 
qf_client_svga_OBJECTS =  net_chan.$(OBJEXT) net_com.$(OBJEXT) \
net_udp.$(OBJEXT) pmove.$(OBJEXT) pmovetst.$(OBJEXT) zone.$(OBJEXT) \
mdfour.$(OBJEXT) mathlib.$(OBJEXT) math.$(OBJEXT) cvar.$(OBJEXT) \
crc.$(OBJEXT) common.$(OBJEXT) model.$(OBJEXT) cmd.$(OBJEXT) \
cl_cmd.$(OBJEXT) cl_cvar.$(OBJEXT) cl_demo.$(OBJEXT) cl_ents.$(OBJEXT) \
cl_input.$(OBJEXT) cl_main.$(OBJEXT) cl_math.$(OBJEXT) \
cl_misc.$(OBJEXT) cl_parse.$(OBJEXT) cl_pred.$(OBJEXT) \
cl_tent.$(OBJEXT) cl_cam.$(OBJEXT) snd_linux.$(OBJEXT) \
sys_linux.$(OBJEXT) cd_linux.$(OBJEXT) r_view.$(OBJEXT) wad.$(OBJEXT) \
snd_dma.$(OBJEXT) snd_mem.$(OBJEXT) snd_mix.$(OBJEXT) \
snd_mixa.$(OBJEXT) skin.$(OBJEXT) sbar.$(OBJEXT) nonintel.$(OBJEXT) \
menu.$(OBJEXT) keys.$(OBJEXT) console.$(OBJEXT) sys_x86.$(OBJEXT) \
cl_model.$(OBJEXT) cl_trans.$(OBJEXT) d_edge.$(OBJEXT) d_fill.$(OBJEXT) \
d_init.$(OBJEXT) d_modech.$(OBJEXT) d_part.$(OBJEXT) d_polyse.$(OBJEXT) \
d_scan.$(OBJEXT) d_sky.$(OBJEXT) d_sprite.$(OBJEXT) d_surf.$(OBJEXT) \
d_vars.$(OBJEXT) d_zpoint.$(OBJEXT) draw.$(OBJEXT) r_aclip.$(OBJEXT) \
r_alias.$(OBJEXT) r_bsp.$(OBJEXT) r_draw.$(OBJEXT) r_edge.$(OBJEXT) \
r_efrag.$(OBJEXT) r_light.$(OBJEXT) r_main.$(OBJEXT) r_misc.$(OBJEXT) \
r_part.$(OBJEXT) r_sky.$(OBJEXT) r_sprite.$(OBJEXT) r_surf.$(OBJEXT) \
r_vars.$(OBJEXT) sw_view.$(OBJEXT) screen.$(OBJEXT) d_copy.$(OBJEXT) \
d_draw.$(OBJEXT) d_draw16.$(OBJEXT) d_parta.$(OBJEXT) \
d_polysa.$(OBJEXT) d_scana.$(OBJEXT) d_spr8.$(OBJEXT) d_varsa.$(OBJEXT) \
r_aclipa.$(OBJEXT) r_aliasa.$(OBJEXT) r_drawa.$(OBJEXT) \
r_edgea.$(OBJEXT) r_varsa.$(OBJEXT) surf16.$(OBJEXT) surf8.$(OBJEXT) \
vid_svgalib.$(OBJEXT)
qf_client_svga_DEPENDENCIES = 
qf_client_svga_LDFLAGS = 
qf_client_x11_OBJECTS =  net_chan.$(OBJEXT) net_com.$(OBJEXT) \
net_udp.$(OBJEXT) pmove.$(OBJEXT) pmovetst.$(OBJEXT) zone.$(OBJEXT) \
mdfour.$(OBJEXT) mathlib.$(OBJEXT) math.$(OBJEXT) cvar.$(OBJEXT) \
crc.$(OBJEXT) common.$(OBJEXT) model.$(OBJEXT) cmd.$(OBJEXT) \
cl_cmd.$(OBJEXT) cl_cvar.$(OBJEXT) cl_demo.$(OBJEXT) cl_ents.$(OBJEXT) \
cl_input.$(OBJEXT) cl_main.$(OBJEXT) cl_math.$(OBJEXT) \
cl_misc.$(OBJEXT) cl_parse.$(OBJEXT) cl_pred.$(OBJEXT) \
cl_tent.$(OBJEXT) cl_cam.$(OBJEXT) snd_linux.$(OBJEXT) \
sys_linux.$(OBJEXT) cd_linux.$(OBJEXT) r_view.$(OBJEXT) wad.$(OBJEXT) \
snd_dma.$(OBJEXT) snd_mem.$(OBJEXT) snd_mix.$(OBJEXT) \
snd_mixa.$(OBJEXT) skin.$(OBJEXT) sbar.$(OBJEXT) nonintel.$(OBJEXT) \
menu.$(OBJEXT) keys.$(OBJEXT) console.$(OBJEXT) sys_x86.$(OBJEXT) \
cl_model.$(OBJEXT) cl_trans.$(OBJEXT) d_edge.$(OBJEXT) d_fill.$(OBJEXT) \
d_init.$(OBJEXT) d_modech.$(OBJEXT) d_part.$(OBJEXT) d_polyse.$(OBJEXT) \
d_scan.$(OBJEXT) d_sky.$(OBJEXT) d_sprite.$(OBJEXT) d_surf.$(OBJEXT) \
d_vars.$(OBJEXT) d_zpoint.$(OBJEXT) draw.$(OBJEXT) r_aclip.$(OBJEXT) \
r_alias.$(OBJEXT) r_bsp.$(OBJEXT) r_draw.$(OBJEXT) r_edge.$(OBJEXT) \
r_efrag.$(OBJEXT) r_light.$(OBJEXT) r_main.$(OBJEXT) r_misc.$(OBJEXT) \
r_part.$(OBJEXT) r_sky.$(OBJEXT) r_sprite.$(OBJEXT) r_surf.$(OBJEXT) \
r_vars.$(OBJEXT) sw_view.$(OBJEXT) screen.$(OBJEXT) d_copy.$(OBJEXT) \
d_draw.$(OBJEXT) d_draw16.$(OBJEXT) d_parta.$(OBJEXT) \
d_polysa.$(OBJEXT) d_scana.$(OBJEXT) d_spr8.$(OBJEXT) d_varsa.$(OBJEXT) \
r_aclipa.$(OBJEXT) r_aliasa.$(OBJEXT) r_drawa.$(OBJEXT) \
r_edgea.$(OBJEXT) r_varsa.$(OBJEXT) surf16.$(OBJEXT) surf8.$(OBJEXT) \
vid_x11.$(OBJEXT)
qf_client_x11_DEPENDENCIES = 
qf_client_x11_LDFLAGS = 
qf_client_glx_OBJECTS =  net_chan.$(OBJEXT) net_com.$(OBJEXT) \
net_udp.$(OBJEXT) pmove.$(OBJEXT) pmovetst.$(OBJEXT) zone.$(OBJEXT) \
mdfour.$(OBJEXT) mathlib.$(OBJEXT) math.$(OBJEXT) cvar.$(OBJEXT) \
crc.$(OBJEXT) common.$(OBJEXT) model.$(OBJEXT) cmd.$(OBJEXT) \
cl_cmd.$(OBJEXT) cl_cvar.$(OBJEXT) cl_demo.$(OBJEXT) cl_ents.$(OBJEXT) \
cl_input.$(OBJEXT) cl_main.$(OBJEXT) cl_math.$(OBJEXT) \
cl_misc.$(OBJEXT) cl_parse.$(OBJEXT) cl_pred.$(OBJEXT) \
cl_tent.$(OBJEXT) cl_cam.$(OBJEXT) snd_linux.$(OBJEXT) \
sys_linux.$(OBJEXT) cd_linux.$(OBJEXT) r_view.$(OBJEXT) wad.$(OBJEXT) \
snd_dma.$(OBJEXT) snd_mem.$(OBJEXT) snd_mix.$(OBJEXT) \
snd_mixa.$(OBJEXT) skin.$(OBJEXT) sbar.$(OBJEXT) nonintel.$(OBJEXT) \
menu.$(OBJEXT) keys.$(OBJEXT) console.$(OBJEXT) sys_x86.$(OBJEXT) \
gl_draw.$(OBJEXT) gl_mesh.$(OBJEXT) gl_model.$(OBJEXT) \
gl_ngraph.$(OBJEXT) gl_part.$(OBJEXT) gl_refrag.$(OBJEXT) \
gl_rlight.$(OBJEXT) gl_rmain.$(OBJEXT) gl_rmisc.$(OBJEXT) \
gl_rsurf.$(OBJEXT) gl_screen.$(OBJEXT) gl_trans.$(OBJEXT) \
gl_view.$(OBJEXT) gl_warp.$(OBJEXT) vid_glx.$(OBJEXT)
qf_client_glx_DEPENDENCIES = 
qf_client_glx_LDFLAGS = 
CFLAGS = -g -O2 -Wall -Werror
COMPILE = $(CC) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS)
CCLD = $(CC)
LINK = $(CCLD) $(AM_CFLAGS) $(CFLAGS) $(LDFLAGS) -o $@
DIST_COMMON =  Makefile.am Makefile.in


DISTFILES = $(DIST_COMMON) $(SOURCES) $(HEADERS) $(TEXINFOS) $(EXTRA_DIST)

TAR = gtar
GZIP_ENV = --best
DEP_FILES =  .deps/cd_linux.P .deps/cl_cam.P .deps/cl_cmd.P \
.deps/cl_cvar.P .deps/cl_demo.P .deps/cl_ents.P .deps/cl_input.P \
.deps/cl_main.P .deps/cl_math.P .deps/cl_misc.P .deps/cl_model.P \
.deps/cl_parse.P .deps/cl_pred.P .deps/cl_tent.P .deps/cl_trans.P \
.deps/cmd.P .deps/common.P .deps/console.P .deps/crc.P .deps/cvar.P \
.deps/d_copy.P .deps/d_draw.P .deps/d_draw16.P .deps/d_edge.P \
.deps/d_fill.P .deps/d_init.P .deps/d_modech.P .deps/d_part.P \
.deps/d_parta.P .deps/d_polysa.P .deps/d_polyse.P .deps/d_scan.P \
.deps/d_scana.P .deps/d_sky.P .deps/d_spr8.P .deps/d_sprite.P \
.deps/d_surf.P .deps/d_vars.P .deps/d_varsa.P .deps/d_zpoint.P \
.deps/draw.P .deps/gl_draw.P .deps/gl_mesh.P .deps/gl_model.P \
.deps/gl_ngraph.P .deps/gl_part.P .deps/gl_refrag.P .deps/gl_rlight.P \
.deps/gl_rmain.P .deps/gl_rmisc.P .deps/gl_rsurf.P .deps/gl_screen.P \
.deps/gl_trans.P .deps/gl_view.P .deps/gl_warp.P .deps/keys.P \
.deps/math.P .deps/mathlib.P .deps/mdfour.P .deps/menu.P .deps/model.P \
.deps/net_chan.P .deps/net_com.P .deps/net_udp.P .deps/nonintel.P \
.deps/pmove.P .deps/pmovetst.P .deps/pr_cmds.P .deps/pr_edict.P \
.deps/pr_exec.P .deps/r_aclip.P .deps/r_aclipa.P .deps/r_alias.P \
.deps/r_aliasa.P .deps/r_bsp.P .deps/r_draw.P .deps/r_drawa.P \
.deps/r_edge.P .deps/r_edgea.P .deps/r_efrag.P .deps/r_light.P \
.deps/r_main.P .deps/r_misc.P .deps/r_part.P .deps/r_sky.P \
.deps/r_sprite.P .deps/r_surf.P .deps/r_vars.P .deps/r_varsa.P \
.deps/r_view.P .deps/sbar.P .deps/screen.P .deps/skin.P .deps/snd_dma.P \
.deps/snd_linux.P .deps/snd_mem.P .deps/snd_mix.P .deps/snd_mixa.P \
.deps/surf16.P .deps/surf8.P .deps/sv_ccmds.P .deps/sv_cvar.P \
.deps/sv_ents.P .deps/sv_init.P .deps/sv_main.P .deps/sv_misc.P \
.deps/sv_model.P .deps/sv_move.P .deps/sv_nchan.P .deps/sv_phys.P \
.deps/sv_send.P .deps/sv_user.P .deps/sw_view.P .deps/sys_linux.P \
.deps/sys_unix.P .deps/sys_x86.P .deps/vid_glx.P .deps/vid_svgalib.P \
.deps/vid_x11.P .deps/wad.P .deps/world.P .deps/worlda.P .deps/zone.P
SOURCES = $(qf_server_SOURCES) $(qf_client_svga_SOURCES) $(qf_client_x11_SOURCES) $(qf_client_glx_SOURCES)
OBJECTS = $(qf_server_OBJECTS) $(qf_client_svga_OBJECTS) $(qf_client_x11_OBJECTS) $(qf_client_glx_OBJECTS)

all: all-redirect
.SUFFIXES:
.SUFFIXES: .S .c .o .obj .s
$(srcdir)/Makefile.in: Makefile.am $(top_srcdir)/configure.in $(ACLOCAL_M4) 
	cd $(top_srcdir) && $(AUTOMAKE) --gnu source/Makefile

Makefile: $(srcdir)/Makefile.in  $(top_builddir)/config.status $(BUILT_SOURCES)
	cd $(top_builddir) \
	  && CONFIG_FILES=$(subdir)/$@ CONFIG_HEADERS= $(SHELL) ./config.status


mostlyclean-binPROGRAMS:

clean-binPROGRAMS:
	-test -z "$(bin_PROGRAMS)" || rm -f $(bin_PROGRAMS)

distclean-binPROGRAMS:

maintainer-clean-binPROGRAMS:

install-binPROGRAMS: $(bin_PROGRAMS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(bindir)
	@list='$(bin_PROGRAMS)'; for p in $$list; do \
	  if test -f $$p; then \
	    echo "  $(INSTALL_PROGRAM) $$p $(DESTDIR)$(bindir)/`echo $$p|sed 's/$(EXEEXT)$$//'|sed '$(transform)'|sed 's/$$/$(EXEEXT)/'`"; \
	     $(INSTALL_PROGRAM) $$p $(DESTDIR)$(bindir)/`echo $$p|sed 's/$(EXEEXT)$$//'|sed '$(transform)'|sed 's/$$/$(EXEEXT)/'`; \
	  else :; fi; \
	done

uninstall-binPROGRAMS:
	@$(NORMAL_UNINSTALL)
	list='$(bin_PROGRAMS)'; for p in $$list; do \
	  rm -f $(DESTDIR)$(bindir)/`echo $$p|sed 's/$(EXEEXT)$$//'|sed '$(transform)'|sed 's/$$/$(EXEEXT)/'`; \
	done

# FIXME: We should only use cygpath when building on Windows,
# and only if it is available.
.c.obj:
	$(COMPILE) -c `cygpath -w $<`

.s.o:
	$(COMPILE) -c $<

.S.o:
	$(COMPILE) -c $<

mostlyclean-compile:
	-rm -f *.o core *.core
	-rm -f *.$(OBJEXT)

clean-compile:

distclean-compile:
	-rm -f *.tab.c

maintainer-clean-compile:

qf-server$(EXEEXT): $(qf_server_OBJECTS) $(qf_server_DEPENDENCIES)
	@rm -f qf-server$(EXEEXT)
	$(LINK) $(qf_server_LDFLAGS) $(qf_server_OBJECTS) $(qf_server_LDADD) $(LIBS)

qf-client-svga$(EXEEXT): $(qf_client_svga_OBJECTS) $(qf_client_svga_DEPENDENCIES)
	@rm -f qf-client-svga$(EXEEXT)
	$(LINK) $(qf_client_svga_LDFLAGS) $(qf_client_svga_OBJECTS) $(qf_client_svga_LDADD) $(LIBS)

qf-client-x11$(EXEEXT): $(qf_client_x11_OBJECTS) $(qf_client_x11_DEPENDENCIES)
	@rm -f qf-client-x11$(EXEEXT)
	$(LINK) $(qf_client_x11_LDFLAGS) $(qf_client_x11_OBJECTS) $(qf_client_x11_LDADD) $(LIBS)

qf-client-glx$(EXEEXT): $(qf_client_glx_OBJECTS) $(qf_client_glx_DEPENDENCIES)
	@rm -f qf-client-glx$(EXEEXT)
	$(LINK) $(qf_client_glx_LDFLAGS) $(qf_client_glx_OBJECTS) $(qf_client_glx_LDADD) $(LIBS)

tags: TAGS

ID: $(HEADERS) $(SOURCES) $(LISP)
	list='$(SOURCES) $(HEADERS)'; \
	unique=`for i in $$list; do echo $$i; done | \
	  awk '    { files[$$0] = 1; } \
	       END { for (i in files) print i; }'`; \
	here=`pwd` && cd $(srcdir) \
	  && mkid -f$$here/ID $$unique $(LISP)

TAGS:  $(HEADERS) $(SOURCES)  $(TAGS_DEPENDENCIES) $(LISP)
	tags=; \
	here=`pwd`; \
	list='$(SOURCES) $(HEADERS)'; \
	unique=`for i in $$list; do echo $$i; done | \
	  awk '    { files[$$0] = 1; } \
	       END { for (i in files) print i; }'`; \
	test -z "$(ETAGS_ARGS)$$unique$(LISP)$$tags" \
	  || (cd $(srcdir) && etags $(ETAGS_ARGS) $$tags  $$unique $(LISP) -o $$here/TAGS)

mostlyclean-tags:

clean-tags:

distclean-tags:
	-rm -f TAGS ID

maintainer-clean-tags:

distdir = $(top_builddir)/$(PACKAGE)-$(VERSION)/$(subdir)

subdir = source

distdir: $(DISTFILES)
	here=`cd $(top_builddir) && pwd`; \
	top_distdir=`cd $(top_distdir) && pwd`; \
	distdir=`cd $(distdir) && pwd`; \
	cd $(top_srcdir) \
	  && $(AUTOMAKE) --include-deps --build-dir=$$here --srcdir-name=$(top_srcdir) --output-dir=$$top_distdir --gnu source/Makefile
	@for file in $(DISTFILES); do \
	  d=$(srcdir); \
	  if test -d $$d/$$file; then \
	    cp -pr $$d/$$file $(distdir)/$$file; \
	  else \
	    test -f $(distdir)/$$file \
	    || ln $$d/$$file $(distdir)/$$file 2> /dev/null \
	    || cp -p $$d/$$file $(distdir)/$$file || :; \
	  fi; \
	done

DEPS_MAGIC := $(shell mkdir .deps > /dev/null 2>&1 || :)

-include $(DEP_FILES)

mostlyclean-depend:

clean-depend:

distclean-depend:
	-rm -rf .deps

maintainer-clean-depend:

%.o: %.c
	@echo '$(COMPILE) -c $<'; \
	$(COMPILE) -Wp,-MD,.deps/$(*F).pp -c $<
	@-cp .deps/$(*F).pp .deps/$(*F).P; \
	tr ' ' '\012' < .deps/$(*F).pp \
	  | sed -e 's/^\\$$//' -e '/^$$/ d' -e '/:$$/ d' -e 's/$$/ :/' \
	    >> .deps/$(*F).P; \
	rm .deps/$(*F).pp

%.lo: %.c
	@echo '$(LTCOMPILE) -c $<'; \
	$(LTCOMPILE) -Wp,-MD,.deps/$(*F).pp -c $<
	@-sed -e 's/^\([^:]*\)\.o[ 	]*:/\1.lo \1.o :/' \
	  < .deps/$(*F).pp > .deps/$(*F).P; \
	tr ' ' '\012' < .deps/$(*F).pp \
	  | sed -e 's/^\\$$//' -e '/^$$/ d' -e '/:$$/ d' -e 's/$$/ :/' \
	    >> .deps/$(*F).P; \
	rm -f .deps/$(*F).pp
info-am:
info: info-am
dvi-am:
dvi: dvi-am
check-am: all-am
check: check-am
installcheck-am:
installcheck: installcheck-am
install-exec-am: install-binPROGRAMS
install-exec: install-exec-am

install-data-am:
install-data: install-data-am

install-am: all-am
	@$(MAKE) $(AM_MAKEFLAGS) install-exec-am install-data-am
install: install-am
uninstall-am: uninstall-binPROGRAMS
uninstall: uninstall-am
all-am: Makefile $(PROGRAMS)
all-redirect: all-am
install-strip:
	$(MAKE) $(AM_MAKEFLAGS) AM_INSTALL_PROGRAM_FLAGS=-s install
installdirs:
	$(mkinstalldirs)  $(DESTDIR)$(bindir)


mostlyclean-generic:

clean-generic:

distclean-generic:
	-rm -f Makefile $(CONFIG_CLEAN_FILES)
	-rm -f config.cache config.log stamp-h stamp-h[0-9]*

maintainer-clean-generic:
mostlyclean-am:  mostlyclean-binPROGRAMS mostlyclean-compile \
		mostlyclean-tags mostlyclean-depend mostlyclean-generic

mostlyclean: mostlyclean-am

clean-am:  clean-binPROGRAMS clean-compile clean-tags clean-depend \
		clean-generic mostlyclean-am

clean: clean-am

distclean-am:  distclean-binPROGRAMS distclean-compile distclean-tags \
		distclean-depend distclean-generic clean-am

distclean: distclean-am

maintainer-clean-am:  maintainer-clean-binPROGRAMS \
		maintainer-clean-compile maintainer-clean-tags \
		maintainer-clean-depend maintainer-clean-generic \
		distclean-am
	@echo "This command is intended for maintainers to use;"
	@echo "it deletes files that may require special tools to rebuild."

maintainer-clean: maintainer-clean-am

.PHONY: mostlyclean-binPROGRAMS distclean-binPROGRAMS clean-binPROGRAMS \
maintainer-clean-binPROGRAMS uninstall-binPROGRAMS install-binPROGRAMS \
mostlyclean-compile distclean-compile clean-compile \
maintainer-clean-compile tags mostlyclean-tags distclean-tags \
clean-tags maintainer-clean-tags distdir mostlyclean-depend \
distclean-depend clean-depend maintainer-clean-depend info-am info \
dvi-am dvi check check-am installcheck-am installcheck install-exec-am \
install-exec install-data-am install-data install-am install \
uninstall-am uninstall all-redirect all-am all installdirs \
mostlyclean-generic distclean-generic clean-generic \
maintainer-clean-generic clean mostlyclean distclean maintainer-clean


# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
