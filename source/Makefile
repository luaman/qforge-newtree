#				*Defines*

SV_name=qw-server
SV_libs=

CL_SVGA_name=qw-client-svga
SV_libs=

CL_X11_name=qw-client-x11
SV_libs=

DIRECTORIES=
vpath %.a $(patsubst @%,%,$(DIRECTORIES)) /usr/lib

CPPFLAGS=-I . -I ../include -DELF -Did386=1 -MMD -Dstricmp=strcasecmp -DSERVERONLY
CFLAGS=-Wall -Werror
CXXFLAGS=-Wall -Werror
LDFLAGS=

CC=gcc
CXX=g++

CFLAGS+=-g -O2
CXXFLAGS+=-g -O2


%.d:

#               *List Macros*

SV_sources=\
  pr_cmds.c \
  pr_edict.c \
  pr_exec.c \
  sv_init.c \
  sv_main.c \
  sv_misc.c \
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
  model.c \
  cmd.c \
  common.c \
  crc.c \
  cvar.c \
  sv_cvar.c \
  mathlib.c \
  math.S \
  mdfour.c \
  zone.c \
  pmove.c \
  pmovetst.c \
  net_chan.c \
  net_com.c \
  net_udp.c

SV_dependencies = $(patsubst %,%.d,$(basename $(SV_sources)))
SV_objects = $(patsubst %.d,%.o,$(SV_dependencies))

CL_sources=\
  cl_demo.c \
  cl_ents.c \
  cl_input.c \
  cl_main.c \
  cl_parse.c \
  cl_pred.c \
  cl_tent.c \
  cl_cam.c \
  cmd.c \
  common.c \
  console.c \
  crc.c \
  cvar.c \
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
  keys.c \
  mathlib.c \
  mdfour.c \
  menu.c \
  model.c \
  net_chan.c \
  net_com.c \
  net_udp.c \
  nonintel.c \
  pmove.c \
  pmovetst.c \
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
  sbar.c \
  screen.c \
  skin.c \
  snd_dma.c \
  snd_mem.c \
  snd_mix.c \
  view.c \
  wad.c \
  zone.c \
  cd_linux.c \
  sys_linux.c \
  snd_linux.c

CL_dependencies = $(patsubst %,%.d,$(basename $(CL_sources)))
CL_objects = $(patsubst %.d,%.o,$(CL_dependencies))

CL_SVGA_sources=\
  vid_svgalib.c

CL_SVGA_dependencies = $(patsubst %,%.d,$(basename $(CL_SVGA_sources)))
CL_SVGA_objects = $(patsubst %.d,%.o,$(CL_SVGA_dependencies))

CL_X11_sources=\
  vid_x.c

CL_X11_dependencies = $(patsubst %,%.d,$(basename $(CL_X11_sources)))
CL_X11_objects = $(patsubst %.d,%.o,$(CL_X11_dependencies))


#               *Explicit Rules*

all: $(SV_name) #$(CL_SVGA_name) $(CL_X11_name)

$(SV_name): $(SV_objects) $(SV_libs)
	$(CC) $(LDFLAGS) -o $@ $^ -lm -ldl -lpthread

$(CL_SVGA_name): $(CL_objects) $(CL_SVGA_objects) $(CL_SVGA_libs)
	$(CC) $(LDFLAGS) -o $@ $^ -lm -ldl -lpthread

$(CL_X11_name): $(CL_objects) $(CL_X11_objects) $(CL_X11_libs)
	$(CC) $(LDFLAGS) -o $@ $^ -lm -ldl -lpthread

clean:
	-rm -f *.[od] $(SV_name)

#               *Individual File Dependencies*

-include $(SV_dependencies)
-include $(CL_dependencies)
-include $(CL_SVGA_dependencies)
-include $(CL_X11_dependencies)
