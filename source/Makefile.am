## Process this file with automake to produce Makefile.in

INCLUDES=	@X_INCLUDES@ @OPENGL_INCLUDES@ -I$(top_srcdir)/include

bin_PROGRAMS =	@TARGETS@

EXTRA_PROGRAMS= qf-server qf-client-ggi qf-client-svga qf-client-x11 \
		qf-client-glx

common_SOURCES= net_chan.c net_com.c net_udp.c pmove.c pmovetst.c zone.c \
		mdfour.c mathlib.c math.S cvar.c crc.c common.c model.c cmd.c
server_SOURCES=	pr_cmds.c pr_edict.c pr_exec.c sv_init.c sv_main.c sv_misc.c \
		sv_model.c sv_nchan.c sv_ents.c sv_send.c sv_move.c sv_phys.c \
		sv_user.c sv_ccmds.c world.c worlda.S sys_unix.c sv_cvar.c
client_SOURCES= cl_cmd.c cl_cvar.c cl_demo.c cl_ents.c cl_input.c cl_main.c \
		cl_math.S cl_misc.c cl_parse.c cl_pred.c cl_tent.c cl_cam.c \
		snd_linux.c sys_linux.c cd_linux.c r_view.c wad.c snd_dma.c \
		snd_mem.c snd_mix.c snd_mixa.S skin.c sbar.c nonintel.c \
		menu.c keys.c console.c sys_x86.S
soft_SOURCES=	cl_model.c cl_trans.c d_edge.c d_fill.c d_init.c d_modech.c \
		d_part.c d_polyse.c d_scan.c d_sky.c d_sprite.c d_surf.c \
		d_vars.c d_zpoint.c draw.c r_aclip.c r_alias.c r_bsp.c \
		r_draw.c r_edge.c r_efrag.c r_light.c r_main.c r_misc.c \
		r_part.c r_sky.c r_sprite.c r_surf.c r_vars.c sw_view.c \
		screen.c \
		d_copy.S d_draw.S d_draw16.S d_parta.S d_polysa.S d_scana.S \
		d_spr8.S d_varsa.S r_aclipa.S r_aliasa.S r_drawa.S r_edgea.S \
		r_varsa.S surf16.S surf8.S
ogl_SOURCES=	gl_draw.c gl_mesh.c gl_model.c gl_ngraph.c gl_part.c \
		gl_refrag.c gl_rlight.c gl_rmain.c gl_rmisc.c gl_rsurf.c \
		gl_screen.c gl_trans.c gl_view.c gl_warp.c

ggi_SOURCES=	vid_ggi.c
svga_SOURCES=	vid_svgalib.c
x11_SOURCES=	vid_x11.c
glx_SOURCES=	vid_glx.c

qf_server_SOURCES=	$(common_SOURCES) $(server_SOURCES)
qf_client_ggi_SOURCES=	$(common_SOURCES) $(client_SOURCES) $(soft_SOURCES) $(ggi_SOURCES)
qf_client_svga_SOURCES= $(common_SOURCES) $(client_SOURCES) $(soft_SOURCES) $(svga_SOURCES)
qf_client_x11_SOURCES=	$(common_SOURCES) $(client_SOURCES) $(soft_SOURCES) $(x11_SOURCES)
qf_client_glx_SOURCES=	$(common_SOURCES) $(client_SOURCES) $(ogl_SOURCES) $(glx_SOURCES)

qf_client_svga_LDADD=	@SVGA_LIBS@
qf_client_x11_LDADD=	@X_PRE_LIBS@ @X_LIBS@ @X_EXTRA_LIBS@ -lX11 @X_SHM_LIB@
qf_client_glx_LDADD=	@OPENGL_LIBS@
qf_client_ggi_LDADD=	@GGI_LIBS@
