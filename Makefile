#
# Quake Makefile for Linux 2.0
#
# Aug '98 by Zoid <zoid@idsoftware.com>
#
# ELF only
#

MOUNT_DIR=.

BUILD_DEBUG_DIR=debug-i386
BUILD_RELEASE_DIR=release-i386

EGCS=gcc
CC=$(EGCS)

BASE_CFLAGS=-Dstricmp=strcasecmp
RELEASE_CFLAGS=$(BASE_CFLAGS) -g -O6 -ffast-math -funroll-loops -fexpensive-optimizations
DEBUG_CFLAGS=$(BASE_CFLAGS) -g
LDFLAGS=-lm -ldl

DO_CC=$(CC) $(CFLAGS) -o $@ -c $<
DO_DEBUG_CC=$(CC) $(DEBUG_CFLAGS) -o $@ -c $<
DO_AS=$(CC) $(CFLAGS) -DELF -x assembler-with-cpp -o $@ -c $<

#############################################################################
# SETUP AND BUILD
#############################################################################

TARGETS=$(BUILDDIR)/bin/squake

build_debug:
	@-mkdir $(BUILD_DEBUG_DIR) \
		$(BUILD_DEBUG_DIR)/bin \
		$(BUILD_DEBUG_DIR)/squake
	$(MAKE) targets BUILDDIR=$(BUILD_DEBUG_DIR) CFLAGS="$(DEBUG_CFLAGS)"

build_release:
	@-mkdir $(BUILD_RELEASE_DIR) \
		$(BUILD_RELEASE_DIR)/bin \
		$(BUILD_RELEASE_DIR)/squake
	$(MAKE) targets BUILDDIR=$(BUILD_RELEASE_DIR) CFLAGS="$(RELEASE_CFLAGS)"
	strip $(BUILD_RELEASE_DIR)/bin/*

all: build_debug build_release

targets: $(TARGETS)

#############################################################################
# SVGALIB Quake
#############################################################################

SQUAKE_OBJS = \
	$(BUILDDIR)/squake/cmd.o \
	$(BUILDDIR)/squake/common.o \
	$(BUILDDIR)/squake/console.o \
	$(BUILDDIR)/squake/crc.o \
	$(BUILDDIR)/squake/cvar.o \
	$(BUILDDIR)/squake/host.o \
	$(BUILDDIR)/squake/host_cmd.o \
	$(BUILDDIR)/squake/iplog.o \
	$(BUILDDIR)/squake/keys.o \
	$(BUILDDIR)/squake/mathlib.o \
	$(BUILDDIR)/squake/model.o \
	$(BUILDDIR)/squake/net_dgrm.o \
	$(BUILDDIR)/squake/net_loop.o \
	$(BUILDDIR)/squake/net_main.o \
	$(BUILDDIR)/squake/net_udp.o \
	$(BUILDDIR)/squake/net_bsd.o \
	$(BUILDDIR)/squake/pr_cmds.o \
	$(BUILDDIR)/squake/pr_edict.o \
	$(BUILDDIR)/squake/pr_exec.o \
	$(BUILDDIR)/squake/r_main.o \
	$(BUILDDIR)/squake/security.o \
	$(BUILDDIR)/squake/sv_main.o \
	$(BUILDDIR)/squake/sv_phys.o \
	$(BUILDDIR)/squake/sv_move.o \
	$(BUILDDIR)/squake/sv_user.o \
	$(BUILDDIR)/squake/zone.o \
	$(BUILDDIR)/squake/wad.o \
	$(BUILDDIR)/squake/world.o \
	$(BUILDDIR)/squake/sys_linux.o \
	$(BUILDDIR)/squake/math.o \
	$(BUILDDIR)/squake/worlda.o \
	$(BUILDDIR)/squake/sys_dosa.o

$(BUILDDIR)/bin/squake : $(SQUAKE_OBJS)
	$(CC) $(CFLAGS) -o $@ $(SQUAKE_OBJS) $(LDFLAGS)

####

$(BUILDDIR)/squake/cmd.o :      $(MOUNT_DIR)/cmd.c
	$(DO_CC)

$(BUILDDIR)/squake/common.o :   $(MOUNT_DIR)/common.c
	$(DO_DEBUG_CC)

$(BUILDDIR)/squake/console.o :  $(MOUNT_DIR)/console.c
	$(DO_CC)

$(BUILDDIR)/squake/crc.o :      $(MOUNT_DIR)/crc.c
	$(DO_CC)

$(BUILDDIR)/squake/cvar.o :     $(MOUNT_DIR)/cvar.c
	$(DO_CC)

$(BUILDDIR)/squake/host.o :     $(MOUNT_DIR)/host.c
	$(DO_CC)

$(BUILDDIR)/squake/host_cmd.o : $(MOUNT_DIR)/host_cmd.c
	$(DO_CC)

$(BUILDDIR)/squake/iplog.o :    $(MOUNT_DIR)/iplog.c
	$(DO_CC)

$(BUILDDIR)/squake/keys.o :     $(MOUNT_DIR)/keys.c
	$(DO_CC)

$(BUILDDIR)/squake/mathlib.o :  $(MOUNT_DIR)/mathlib.c
	$(DO_CC)

$(BUILDDIR)/squake/model.o :    $(MOUNT_DIR)/model.c
	$(DO_CC)

$(BUILDDIR)/squake/net_dgrm.o : $(MOUNT_DIR)/net_dgrm.c
	$(DO_CC)

$(BUILDDIR)/squake/net_loop.o : $(MOUNT_DIR)/net_loop.c
	$(DO_CC)

$(BUILDDIR)/squake/net_main.o : $(MOUNT_DIR)/net_main.c
	$(DO_CC)

$(BUILDDIR)/squake/net_udp.o :  $(MOUNT_DIR)/net_udp.c
	$(DO_CC)

$(BUILDDIR)/squake/net_bsd.o :  $(MOUNT_DIR)/net_bsd.c
	$(DO_CC)

$(BUILDDIR)/squake/pr_cmds.o :  $(MOUNT_DIR)/pr_cmds.c
	$(DO_CC)

$(BUILDDIR)/squake/pr_edict.o : $(MOUNT_DIR)/pr_edict.c
	$(DO_CC)

$(BUILDDIR)/squake/pr_exec.o :  $(MOUNT_DIR)/pr_exec.c
	$(DO_CC)

$(BUILDDIR)/squake/r_main.o :   $(MOUNT_DIR)/r_main.c
	$(DO_CC)

$(BUILDDIR)/squake/security.o : $(MOUNT_DIR)/security.c
	$(DO_CC)

$(BUILDDIR)/squake/sv_main.o :  $(MOUNT_DIR)/sv_main.c
	$(DO_CC)

$(BUILDDIR)/squake/sv_phys.o :  $(MOUNT_DIR)/sv_phys.c
	$(DO_CC)

$(BUILDDIR)/squake/sv_move.o :  $(MOUNT_DIR)/sv_move.c
	$(DO_CC)

$(BUILDDIR)/squake/sv_user.o :  $(MOUNT_DIR)/sv_user.c
	$(DO_CC)

$(BUILDDIR)/squake/zone.o :	$(MOUNT_DIR)/zone.c
	$(DO_CC)

$(BUILDDIR)/squake/wad.o :      $(MOUNT_DIR)/wad.c
	$(DO_CC)

$(BUILDDIR)/squake/world.o :    $(MOUNT_DIR)/world.c
	$(DO_CC)

$(BUILDDIR)/squake/sys_linux.o :$(MOUNT_DIR)/sys_linux.c
	$(DO_CC)

$(BUILDDIR)/squake/math.o :     $(MOUNT_DIR)/math.s
	$(DO_AS)

$(BUILDDIR)/squake/worlda.o :   $(MOUNT_DIR)/worlda.s
	$(DO_AS)

$(BUILDDIR)/squake/sys_dosa.o : $(MOUNT_DIR)/sys_dosa.s
	$(DO_AS)

#############################################################################
# MISC
#############################################################################

clean: clean-debug clean-release

clean-debug:
	$(MAKE) clean2 BUILDDIR=$(BUILD_DEBUG_DIR) CFLAGS="$(DEBUG_CFLAGS)"

clean-release:
	$(MAKE) clean2 BUILDDIR=$(BUILD_RELEASE_DIR) CFLAGS="$(DEBUG_CFLAGS)"

clean2:
	-rm -f $(SQUAKE_OBJS)
