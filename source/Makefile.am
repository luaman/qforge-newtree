## Process this file with automake to produce Makefile.in
#
#	Makefile.am
#
#	Automake-using build system for QuakeForge
#
#	Copyright (C) 2000 Jeff Teunissen <deek@quakeforge.net>
#
#	This Makefile is free software; you can redistribute it and/or
#	modify it under the terms of the GNU General Public License
#	as published by the Free Software Foundation; either version 2
#	of the License, or (at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
#	See the GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program; if not, write to:
#
#		Free Software Foundation, Inc.
#		59 Temple Place - Suite 330
#		Boston, MA  02111-1307, USA
#
#	$Id$
#

#
# Stuff that is common to both client and server
#
INCLUDES=	-I$(top_srcdir)/include $(GGI_CFLAGS) $(MGL_CFLAGS) $(SDL_CFLAGS) $(SVGA_CFLAGS) $(X_CFLAGS) $(GLX_CFLAGS) $(TDFXGL_CFLAGS)

bin_PROGRAMS =	@TARGETS@

EXTRA_PROGRAMS= qf-server \
		qf-client-mgl qf-client-ggi qf-client-sdl qf-client-svga qf-client-x11 \
		qf-client-3dfx qf-client-glx qf-client-sgl qf-client-wgl

noinst_LIBRARIES= libqfsys_cl.a libqfsys_sv.a libqfsnd.a libqfcd.a

if ASM_ARCH
math_ASM = math.S
endif
common_SOURCES= net_chan.c net_com.c net_udp.c pmove.c pmovetst.c zone.c \
		mdfour.c mathlib.c cvar.c crc.c cmd.c \
		qargs.c qendian.c quakefs.c quakeio.c msg.c sizebuf.c info.c \
		checksum.c link.c buildnum.c va.c com.c model.c model_brush.c $(math_ASM)

#
# Server builds
#
# ... System type
#
if SYSTYPE_WIN32
libqfsys_sv_a_SOURCES=	sys_win.c sv_sys_win.c fnmatch.c
else
libqfsys_sv_a_SOURCES=	sys_unix.c sv_sys_unix.c
endif
EXTRA_libqfsys_sv_a_SOURCES= sv_sys_unix.c sv_sys_win.c sys_win.c sys_unix.c fnmatch.c dirent.c

if ASM_ARCH
world_ASM=	worlda.S
endif

server_SOURCES=	pr_cmds.c pr_edict.c pr_exec.c pr_offs.c sv_init.c sv_main.c \
		sv_misc.c sv_model.c sv_nchan.c sv_ents.c sv_send.c sv_move.c \
		sv_phys.c sv_user.c sv_ccmds.c world.c sv_cvar.c \
		$(world_ASM)
		
qf_server_SOURCES=	$(common_SOURCES) $(server_SOURCES)
qf_server_LDADD=	-L. -lqfsys_sv $(NET_LIBS)
qf_server_DEPENDENCIES= libqfsys_sv.a

#
# Client builds
#
# ... System type
#
if SYSTYPE_WIN32
libqfsys_cl_a_SOURCES=	sys_win.c cl_sys_win.c fnmatch.c
else
libqfsys_cl_a_SOURCES=	sys_unix.c cl_sys_unix.c
endif
EXTRA_libqfsys_cl_a_SOURCES= cl_sys_unix.c cl_sys_win.c sys_win.c sys_unix.c fnmatch.c dirent.c

#
# ... Digital audio
#
if SNDTYPE_ALSA_0_5
libqfsnd_a_SOURCES=	snd_dma.c snd_mem.c snd_mix.c snd_alsa_0_5.c
endif
if SNDTYPE_ALSA_0_6
libqfsnd_a_SOURCES=	snd_dma.c snd_mem.c snd_mix.c snd_alsa_0_6.c
endif
if SNDTYPE_MME
libqfsnd_a_SOURCES = snd_dma.c snd_mem.c snd_mix.c snd_mme.c
endif
if SNDTYPE_OSS
libqfsnd_a_SOURCES = snd_dma.c snd_mem.c snd_mix.c snd_oss.c
endif
if SNDTYPE_SUN
libqfsnd_a_SOURCES = snd_dma.c snd_mem.c snd_mix.c snd_sun.c
endif
if SNDTYPE_WIN32
libqfsnd_a_SOURCES = snd_dma.c snd_mem.c snd_mix.c snd_win.c
endif
if SNDTYPE_NULL
libqfsnd_a_SOURCES = snd_null.c
endif
EXTRA_libqfsnd_a_SOURCES= snd_dma.c snd_mem.c snd_mix.c snd_alsa_0_5.c \
			snd_alsa_0_6.c snd_oss.c snd_sun.c snd_win.c snd_null.c

#
# ... CD audio
#
if CDTYPE_WIN32
libqfcd_a_SOURCES = cd_win.c
endif
if CDTYPE_LINUX
libqfcd_a_SOURCES = cd_linux.c
endif
if CDTYPE_NULL
libqfcd_a_SOURCES = cd_null.c
endif
EXTRA_libqfcd_a_SOURCES = cd_audio.c cd_win.c cd_linux.c cd_null.c

CLIENT_LIBS=	-L. -lqfsys_cl -lqfsnd -lqfcd $(SOUND_LIBS) $(NET_LIBS)

if ASM_ARCH
client_ASM=	snd_mixa.S cl_math.S sys_x86.S
endif

client_SOURCES= cl_cmd.c cl_cvar.c cl_demo.c cl_ents.c cl_input.c cl_main.c \
		cl_misc.c cl_parse.c cl_pred.c cl_tent.c cl_cam.c teamplay.c \
		r_view.c wad.c model_alias.c model_sprite.c \
		console.c keys.c menu.c nonintel.c skin.c sbar.c \
		cl_slist.c $(client_ASM) joystick.c

#
# Software-rendering clients
#
# ... Common stuff
#
if ASM_ARCH
soft_ASM=	d_draw.S d_draw16.S d_parta.S d_polysa.S d_scana.S d_spr8.S \
		d_varsa.S r_aclipa.S r_aliasa.S r_drawa.S r_edgea.S r_varsa.S \
		surf16.S surf8.S
endif

soft_SOURCES=	cl_trans.c d_edge.c d_fill.c d_init.c d_modech.c \
		d_part.c d_polyse.c d_scan.c d_sky.c d_sprite.c d_surf.c \
		d_vars.c d_zpoint.c draw.c r_aclip.c r_alias.c r_bsp.c \
		r_draw.c r_edge.c r_efrag.c r_light.c r_main.c r_misc.c \
		r_part.c r_sky.c r_sprite.c r_surf.c r_vars.c sw_view.c \
		screen.c $(soft_ASM) sw_model_alias.c sw_model_brush.c \
		sw_model_sprite.c

#
# ... GGI
#
ggi_SOURCES=		vid_ggi.c

qf_client_ggi_SOURCES=	$(common_SOURCES) $(client_SOURCES) $(soft_SOURCES) $(ggi_SOURCES)
qf_client_ggi_LDADD=	$(GGI_LIBS) $(CLIENT_LIBS)
qf_client_ggi_DEPENDENCIES=libqfsys_cl.a libqfsnd.a libqfcd.a

#
# ... SciTech MGL
#
mgl_SOURCES=		vid_mgl.c in_win.c

qf_client_mgl_SOURCES=	$(common_SOURCES) $(client_SOURCES) $(soft_SOURCES) $(mgl_SOURCES)
qf_client_mgl_LDADD=	$(MGL_LIBS) $(CLIENT_LIBS)
qf_client_mgl_DEPENDENCIES=libqfsys_cl.a libqfsnd.a libqfcd.a

#
# ... Sam Lantinga's Simple DirectMedia Layer, version 1.0 and higher
#
sdl_SOURCES=		vid_sdl.c

qf_client_sdl_SOURCES=	$(common_SOURCES) $(client_SOURCES) $(soft_SOURCES) $(sdl_SOURCES)
qf_client_sdl_LDADD=	$(SDL_LIBS) $(CLIENT_LIBS)
qf_client_sdl_DEPENDENCIES=libqfsys_cl.a libqfsnd.a libqfcd.a

#
# ... Linux SVGAlib
#
svga_SOURCES=		d_copy.S vid_svgalib.c in_svgalib.c

qf_client_svga_SOURCES= $(common_SOURCES) $(client_SOURCES) $(soft_SOURCES) $(svga_SOURCES)
qf_client_svga_LDADD=	$(SVGA_LIBS) $(CLIENT_LIBS)
qf_client_svga_DEPENDENCIES=libqfsys_cl.a libqfsnd.a libqfcd.a

#
# ... X11
#
x11_SOURCES=		vid_x11.c in_x11.c context_x11.c dga_check.c

qf_client_x11_SOURCES=	$(common_SOURCES) $(client_SOURCES) $(soft_SOURCES) $(x11_SOURCES)
qf_client_x11_LDADD=	$(X_PRE_LIBS) $(VIDMODE_LIBS) $(DGA_LIBS) $(X_LIBS) -lX11 $(X_EXTRA_LIBS) $(X_SHM_LIB) $(CLIENT_LIBS)
qf_client_x11_DEPENDENCIES=libqfsys_cl.a libqfsnd.a libqfcd.a


#
# OpenGL-using clients
#
# ... Common stuff
#
ogl_SOURCES=	gl_draw.c gl_mesh.c gl_ngraph.c gl_part.c \
		gl_refrag.c gl_rlight.c gl_rmain.c gl_rmisc.c gl_rsurf.c \
		gl_screen.c gl_trans.c gl_view.c gl_warp.c gl_model_alias.c \
		gl_model_brush.c gl_model_fullbright.c gl_model_sprite.c

#
# ... 3Dfx Voodoo 1 and 2 SVGAlib-based console GL
#
tdfx_SOURCES=	vid_3dfxsvga.c in_svgalib.c

qf_client_3dfx_SOURCES=	$(common_SOURCES) $(client_SOURCES) $(ogl_SOURCES) $(tdfx_SOURCES)
qf_client_3dfx_LDADD=	$(TDFXGL_LIBS) $(SVGA_LIBS) $(CLIENT_LIBS) $(DL_LIBS)
qf_client_3dfx_DEPENDENCIES=libqfsys_cl.a libqfsnd.a libqfcd.a

#
# ... OpenGL in X Window
#
glx_SOURCES=	vid_glx.c in_x11.c context_x11.c dga_check.c

qf_client_glx_SOURCES=	$(common_SOURCES) $(client_SOURCES) $(ogl_SOURCES) $(glx_SOURCES)
qf_client_glx_LDADD=	$(GLX_LIBS) $(X_PRE_LIBS) $(VIDMODE_LIBS) $(DGA_LIBS) $(X_LIBS) -lX11 $(X_EXTRA_LIBS) $(CLIENT_LIBS) $(DL_LIBS)
qf_client_glx_DEPENDENCIES=libqfsys_cl.a libqfsnd.a libqfcd.a

#
# ... Sam Lantinga's Simple DirectMedia Layer, version 1.1 and higher, in GL mode
#
sgl_SOURCES=	vid_sgl.c

qf_client_sgl_SOURCES=	$(common_SOURCES) $(client_SOURCES) $(ogl_SOURCES) $(sgl_SOURCES)
qf_client_sgl_LDADD=	$(SDL_LIBS) $(GLX_LIBS) $(CLIENT_LIBS) $(DL_LIBS)
qf_client_sgl_DEPENDENCIES=libqfsys_cl.a libqfsnd.a libqfcd.a

#
# SGI/Microsoft WGL (Windows OpenGL)
#
wgl_SOURCES=	vid_wgl.c

qf_client_wgl_SOURCES=	$(common_SOURCES) $(client_SOURCES) $(ogl_SOURCES) $(wgl_SOURCES)
qf_client_wgl_LDADD=	$(CLIENT_LIBS)
qf_client_wgl_DEPENDENCIES=libqfsys_cl.a libqfsnd.a libqfcd.a

#
# Stuff that doesn't get linked into an executable NEEDS to be mentioned here,
# or it won't be distributed with 'make dist'
#
EXTRA_DIST=	makefile.win \
		qf-server.mak qw_server.dsp \
		qf-client-sdl.mak qf-client-win.mak qw_client.dsp \
		qf-client-sgl.mak qf-client-wgl.mak qf-client-win.mak
