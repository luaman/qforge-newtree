#				*Defines*

EXE_name=qw-server
EXE_libs=

DIRECTORIES=
vpath %.a $(patsubst @%,%,$(DIRECTORIES)) /usr/lib

CPPFLAGS=-I . -I ../include -MMD
CFLAGS=-Wall #-Werror
CXXFLAGS=-Wall #-Werror
LDFLAGS=

CC=gcc
CXX=g++

CFLAGS+=-g -O2
CXXFLAGS+=-g -O2


%.d:

#               *List Macros*

EXE_sources=\
  pr_cmds.c \
  pr_edict.c \
  pr_exec.c \
  sv_init.c \
  sv_main.c \
  sv_nchan.c \
  sv_ents.c \
  sv_send.c \
  sv_move.c \
  sv_phys.c \
  sv_user.c \
  sv_ccmds.c \
  world.c \
  sys_unix.c \
  model.c \
  cmd.c \
  common.c \
  crc.c \
  cvar.c \
  mathlib.c \
  md4.c \
  zone.c \
  pmove.c \
  pmovetst.c \
  net_chan.c \
  net_udp.c 


EXE_dependencies = $(patsubst %,%.d,$(basename $(EXE_sources)))
EXE_objects = $(patsubst %.d,%.o,$(EXE_dependencies))


#               *Explicit Rules*

$(EXE_name): $(EXE_objects) $(EXE_libs)
	$(CXX) $(LDFLAGS) -o $@ $^ -lm -ldl -lpthread

clean:
	-rm -f *.[od] $(EXE_name)

#               *Individual File Dependencies*

-include $(EXE_dependencies)
