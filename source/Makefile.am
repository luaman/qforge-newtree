## Process this file with automake to produce Makefile.in

INCLUDES=	-I$(top_srcdir)/include $(MGL_CFLAGS) $(GGI_CFLAGS) $(SVGA_CFLAGS) $(X_CFLAGS) $(GLX_CFLAGS)

bin_PROGRAMS =	@TARGETS@

EXTRA_PROGRAMS= qf-server qf-client-mgl qf-client-ggi qf-client-svga \
		qf-client-x11 qf-client-glx qf-client-wgl

noinst_LIBRARIES = libqfsys_cl.a libqfsys_sv.a libqfsnd.a libqfcd.a

if SYSTYPE_WIN32
libqfsys_cl_a_SOURCES = sys_win.c cl_sys_win.c fnmatch.c
libqfsys_sv_a_SOURCES = sys_win.c sv_sys_win.c fnmatch.c
else
libqfsys_cl_a_SOURCES = sys_unix.c cl_sys_unix.c
libqfsys_sv_a_SOURCES = sys_unix.c sv_sys_unix.c
endif

if ASM_ARCH
MATH_ASM = math.S
WORLDA_ASM = worlda.S 
CL_ASM = snd_mixa.S cl_math.S sys_x86.S
SOFT_ASM = d_draw.S d_draw16.S d_parta.S d_polysa.S d_scana.S \
	d_spr8.S d_varsa.S r_aclipa.S r_aliasa.S r_drawa.S r_edgea.S \
	r_varsa.S surf16.S surf8.S
endif

EXTRA_libqfsys_cl_a_SOURCES = cl_sys_unix.c cl_sys_win.c sys_win.c sys_unix.c fnmatch.c dirent.c
EXTRA_libqfsys_sv_a_SOURCES = sv_sys_unix.c sv_sys_win.c sys_win.c sys_unix.c fnmatch.c dirent.c

if SNDTYPE_WIN32
libqfsnd_a_SOURCES = snd_win.c snd_dma.c snd_mem.c snd_mix.c 
else
if SNDTYPE_OSS
libqfsnd_a_SOURCES = snd_oss.c snd_dma.c snd_mem.c snd_mix.c 
else
if SNDTYPE_ALSA
libqfsnd_a_SOURCES = snd_alsa.c snd_dma.c snd_mem.c snd_mix.c 
else
if SNDTYPE_SUN
libqfsnd_a_SOURCES = snd_sun.c snd_dma.c snd_mem.c snd_mix.c 
else
libqfsnd_a_SOURCES = snd_null.c
endif
endif
endif
endif
EXTRA_libqfsnd_a_SOURCES = snd_dma.c snd_win.c snd_oss.c snd_alsa.c snd_null.c

if CDTYPE_WIN32
libqfcd_a_SOURCES = cd_win.c
endif
if CDTYPE_LINUX
libqfcd_a_SOURCES = cd_linux.c
endif
if CDTYPE_NULL
libqfcd_a_SOURCES = cd_null.c
endif
EXTRA_libqfcd_a_SOURCES = cd_win.c cd_linux.c cd_null.c

common_SOURCES= net_chan.c net_com.c net_udp.c pmove.c pmovetst.c zone.c \
		mdfour.c mathlib.c cvar.c crc.c model.c cmd.c \
		qargs.c qendian.c quakefs.c quakeio.c msg.c sizebuf.c info.c \
		checksum.c link.c buildnum.c va.c com.c $(MATH_ASM)
server_SOURCES=	pr_cmds.c pr_edict.c pr_exec.c sv_init.c sv_main.c sv_misc.c \
		sv_model.c sv_nchan.c sv_ents.c sv_send.c sv_move.c sv_phys.c \
		sv_user.c sv_ccmds.c world.c sv_cvar.c $(WORLDA_ASM)
client_SOURCES= cl_cmd.c cl_cvar.c cl_demo.c cl_ents.c cl_input.c cl_main.c \
		cl_misc.c cl_parse.c cl_pred.c cl_tent.c cl_cam.c \
		r_view.c wad.c \
		skin.c sbar.c nonintel.c menu.c keys.c console.c \
                cl_slist.c $(CL_ASM)
soft_SOURCES=	cl_model.c cl_trans.c d_edge.c d_fill.c d_init.c d_modech.c \
		d_part.c d_polyse.c d_scan.c d_sky.c d_sprite.c d_surf.c \
		d_vars.c d_zpoint.c draw.c r_aclip.c r_alias.c r_bsp.c \
		r_draw.c r_edge.c r_efrag.c r_light.c r_main.c r_misc.c \
		r_part.c r_sky.c r_sprite.c r_surf.c r_vars.c sw_view.c \
		screen.c $(SOFT_ASM)
ogl_SOURCES=	gl_draw.c gl_mesh.c gl_model.c gl_ngraph.c gl_part.c \
		gl_refrag.c gl_rlight.c gl_rmain.c gl_rmisc.c gl_rsurf.c \
		gl_screen.c gl_trans.c gl_view.c gl_warp.c

mgl_SOURCES=	vid_mgl.c in_win.c
ggi_SOURCES=	vid_ggi.c
svga_SOURCES=	d_copy.S vid_svgalib.c in_svgalib.c
x11_SOURCES=	vid_x11.c in_x11.c context_x11.c dga_check.c
glx_SOURCES=	vid_glx.c in_x11.c context_x11.c dga_check.c
wgl_SOURCES=	vid_wgl.c

qf_server_SOURCES=	$(common_SOURCES) $(server_SOURCES)
qf_client_mgl_SOURCES=	$(common_SOURCES) $(client_SOURCES) $(soft_SOURCES) $(mgl_SOURCES)
qf_client_ggi_SOURCES=	$(common_SOURCES) $(client_SOURCES) $(soft_SOURCES) $(ggi_SOURCES)
qf_client_svga_SOURCES= $(common_SOURCES) $(client_SOURCES) $(soft_SOURCES) $(svga_SOURCES)
qf_client_x11_SOURCES=	$(common_SOURCES) $(client_SOURCES) $(soft_SOURCES) $(x11_SOURCES)
qf_client_glx_SOURCES=	$(common_SOURCES) $(client_SOURCES) $(ogl_SOURCES) $(glx_SOURCES)
qf_client_wgl_SOURCES=	$(common_SOURCES) $(client_SOURCES) $(ogl_SOURCES) $(wgl_SOURCES)

CLIENT_LIBS = -L. -lqfsys_cl -lqfsnd -lqfcd $(SOUND_LIBS) $(NET_LIBS)

qf_server_LDADD=	-L. -lqfsys_sv $(NET_LIBS)
qf_client_mgl_LDADD=	$(MGL_LIBS) $(CLIENT_LIBS)
qf_client_ggi_LDADD=	$(GGI_LIBS) $(CLIENT_LIBS)
qf_client_svga_LDADD=	$(SVGA_LIBS) $(CLIENT_LIBS)
qf_client_x11_LDADD=	$(X_PRE_LIBS) $(VIDMODE_LIBS) $(X_LIBS) -lX11 $(X_EXTRA_LIBS) $(X_SHM_LIB) $(CLIENT_LIBS)
qf_client_glx_LDADD=	$(GLX_LIBS) $(X_PRE_LIBS) $(VIDMODE_LIBS) $(X_LIBS) -lX11 $(X_EXTRA_LIBS) $(CLIENT_LIBS)
qf_client_wgl_LDADD=	$(CLIENT_LIBS)

qf_server_DEPENDENCIES=libqfsys_sv.a
qf_client_mgl_DEPENDENCIES=libqfsys_cl.a libqfsnd.a libqfcd.a
qf_client_ggi_DEPENDENCIES=libqfsys_cl.a libqfsnd.a libqfcd.a
qf_client_svga_DEPENDENCIES=libqfsys_cl.a libqfsnd.a libqfcd.a
qf_client_x11_DEPENDENCIES=libqfsys_cl.a libqfsnd.a libqfcd.a
qf_client_glx_DEPENDENCIES=libqfsys_cl.a libqfsnd.a libqfcd.a
qf_client_glx_DEPENDENCIES=libqfsys_cl.a libqfsnd.a libqfcd.a

EXTRA_DIST = makefile.winc makefile.wingl makefile.wins \
	qw_client.dsp qw_server.dsp
