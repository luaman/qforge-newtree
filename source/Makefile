#				*Defines*

SV_name=qw-server
SV_libs=

CL_SVGA_name=qw-client-svga
CL_SVGA_libs=-lvga

CL_X11_name=qw-client-x11
CL_X11_libs=-lX11 -lXext

CL_GLX_name=qw-client-glx
CL_GLX_libs=-lX11 -lXext -lGL

DIRECTORIES=
vpath %.a $(patsubst @%,%,$(DIRECTORIES)) /usr/lib

CPPFLAGS=-I . -I ../include -I /usr/X11R6/include -DELF -Did386=1 -MMD -Dstricmp=strcasecmp

gcc -I . -I ../include -DELF -Did386=1 -MMD -Dstricmp=strcasecmp  -c -o gl_draw.o gl_draw.c


CFLAGS=-Wall -Werror
CXXFLAGS=-Wall -Werror
LDFLAGS=-L/usr/X11R6/lib 

CC=gcc
CXX=g++

CFLAGS+=-g -O2
CXXFLAGS+=-g -O2


%.d:

#               *List Macros*

COM_sources=\
  net_chan.c \
  net_com.c \
  net_udp.c \
  pmove.c \
  pmovetst.c \
  zone.c \
  mdfour.c \
  mathlib.c \
  math.S \
  cvar.c \
  crc.c \
  common.c \
  cmd.c \
  model.c

COM_dependencies = $(patsubst %,%.d,$(basename $(COM_sources)))
COM_objects = $(patsubst %.d,%.o,$(COM_dependencies))

SV_sources=\
  pr_cmds.c \
  pr_edict.c \
  pr_exec.c \
  sv_init.c \
  sv_main.c \
  sv_misc.c \
  sv_model.c \
  sv_nchan.c \
  sv_ents.c \
  sv_send.c \
  sv_move.c \
  sv_phys.c \
  sv_user.c \
  sv_ccmds.c \
  world.c \
  worlda.S \
  sys_unix.c \
  sv_cvar.c

SV_dependencies = $(patsubst %,%.d,$(basename $(SV_sources)))
SV_objects = $(patsubst %.d,%.o,$(SV_dependencies))

CL_SW_sources=\
  d_edge.c \
  d_fill.c \
  d_init.c \
  d_modech.c \
  d_part.c \
  d_polyse.c \
  d_scan.c \
  d_sky.c \
  d_sprite.c \
  d_surf.c \
  d_vars.c \
  d_zpoint.c \
  draw.c \
  r_aclip.c \
  r_alias.c \
  r_bsp.c \
  r_draw.c \
  r_edge.c \
  r_efrag.c \
  r_light.c \
  r_main.c \
  r_misc.c \
  r_part.c \
  r_sky.c \
  r_sprite.c \
  r_surf.c \
  r_vars.c \
  r_view.c \
  screen.c \
  d_copy.S \
  d_draw.S \
  d_draw16.S \
  d_parta.S \
  d_polysa.S \
  d_scana.S \
  d_spr8.S \
  d_varsa.S \
  r_aclipa.S \
  r_aliasa.S \
  r_drawa.S \
  r_edgea.S \
  r_varsa.S \
  snd_mixa.S \
  surf16.S \
  surf8.S \
  sys_dosa.S

CL_SW_dependencies = $(patsubst %,%.d,$(basename $(CL_SW_sources)))
CL_SW_objects = $(patsubst %.d,%.o,$(CL_SW_dependencies))

CL_sources=\
  cl_cmd.c \
  cl_cvar.c \
  cl_demo.c \
  cl_ents.c \
  cl_input.c \
  cl_main.c \
  cl_math.S \
  cl_misc.c \
  cl_model.c \
  cl_parse.c \
  cl_pred.c \
  cl_tent.c \
  cl_cam.c \
  snd_linux.c \
  sys_linux.c \
  cd_linux.c \
  wad.c \
  snd_dma.c \
  snd_mem.c \
  snd_mix.c \
  skin.c \
  sbar.c \
  nonintel.c \
  menu.c \
  keys.c \
  console.c

CL_dependencies = $(patsubst %,%.d,$(basename $(CL_sources)))
CL_objects = $(patsubst %.d,%.o,$(CL_dependencies))

CL_SVGA_sources=\
  vid_svgalib.c

CL_SVGA_dependencies = $(patsubst %,%.d,$(basename $(CL_SVGA_sources)))
CL_SVGA_objects = $(patsubst %.d,%.o,$(CL_SVGA_dependencies))

CL_X11_sources=\
  vid_x11.c

CL_X11_dependencies = $(patsubst %,%.d,$(basename $(CL_X11_sources)))
CL_X11_objects = $(patsubst %.d,%.o,$(CL_X11_dependencies))

CL_GLX_sources=\
  gl_draw.c \
  gl_mesh.c \
  gl_model.c \
  gl_ngraph.c \
  gl_part.c \
  gl_refrag.c \
  gl_rlight.c \
  gl_rmain.c \
  gl_rmisc.c \
  gl_rsurf.c \
  gl_screen.c \
  gl_view.c \
  gl_warp.c \
  gl_vidlinuxglx.c

CL_GLX_dependencies = $(patsubst %,%.d,$(basename $(CL_GLX_sources)))
CL_GLX_objects = $(patsubst %.d,%.o,$(CL_GLX_dependencies))

#               *Explicit Rules*

all: $(SV_name) $(CL_SVGA_name) $(CL_X11_name) $(CL_GLX_name)

$(SV_name): $(COM_objects) $(SV_objects)
	$(CC) $(LDFLAGS) -o $@ $^ -lm $(SV_libs)

$(CL_SVGA_name): $(COM_objects) $(CL_objects) $(CL_SW_objects) $(CL_SVGA_objects)
	$(CC) $(LDFLAGS) -o $@ $^ -lm $(CL_SVGA_libs)

$(CL_X11_name): $(COM_objects) $(CL_objects) $(CL_SW_objects) $(CL_X11_objects)
	$(CC) $(LDFLAGS) -o $@ $^ -lm $(CL_X11_libs)

$(CL_GLX_objects): CPPFLAGS += -I/usr/include/GL

$(CL_GLX_name): $(COM_objects) $(CL_objects) $(CL_GL_objects) $(CL_GLX_objects)
	$(CC) $(LDFLAGS) -o $@ $^ -lm $(CL_GLX_libs)

clean:
	-rm -f *.[od] $(SV_name) $(CL_SVGA_name) $(CL_X11_name) $(CL_GLX_name)

#               *Individual File Dependencies*

-include $(COM_dependencies)
-include $(SV_dependencies)
-include $(CL_dependencies)
-include $(CL_GL_dependencies)
-include $(CL_SW_dependencies)
-include $(CL_SVGA_dependencies)
-include $(CL_X11_dependencies)
-include $(CL_GLX_dependencies)
