#
# Quake Makefile for Linux 2.0
#
# Aug '98 by Zoid <zoid@idsoftware.com>
#
# ELF only
#

BASEVERSION=1.09
VERSION=$(BASEVERSION)$(GLIBC)

# RPM release number
RPM_RELEASE=5

ifneq (,$(findstring libc6,$(shell if [ -e /lib/libc.so.6 ];then echo libc6;fi)))
GLIBC=-glibc
else
GLIBC=
endif

ifneq (,$(findstring alpha,$(shell uname -m)))
ARCH=axp
else
ARCH=i386
endif
NOARCH=noarch

# MOUNT_DIR=/grog/Projects/WinQuake
MOUNT_DIR=.

MASTER_DIR=/grog/Projects/QuakeMaster
MESA_DIR=/usr/local/src/Mesa-2.6
TDFXGL_DIR = /home/zoid/3dfxgl

BUILD_DEBUG_DIR=debug$(ARCH)$(GLIBC)
BUILD_RELEASE_DIR=release$(ARCH)$(GLIBC)

# EGCS=/usr/local/egcs-1.1.2/bin/gcc
EGCS=gcc

CC=$(EGCS)

BASE_CFLAGS=-Dstricmp=strcasecmp
#RELEASE_CFLAGS=$(BASE_CFLAGS) -g -mpentiumpro -O6 -ffast-math -funroll-loops \
#	-fomit-frame-pointer -fexpensive-optimizations
RELEASE_CFLAGS=$(BASE_CFLAGS) -g -O6 -ffast-math -funroll-loops -fexpensive-optimizations
DEBUG_CFLAGS=$(BASE_CFLAGS) -g
LDFLAGS=-lm -ldl
SVGALDFLAGS=-lvga
XLDFLAGS=-L/usr/X11R6/lib -lX11 -lXext -lXxf86dga
XCFLAGS=-DX11

MESAGLLDFLAGS=-L/usr/X11/lib -L/usr/local/lib -L$(MESA_DIR)/lib -lMesaGL -lglide2x -lX11 -lXext -ldl
TDFXGLLDFLAGS=-L$(TDFXGL_DIR)/release$(ARCH)$(GLIBC) -l3dfxgl -lglide2x -ldl
GLLDFLAGS=-L/usr/X11/lib -L/usr/local/lib -lGL -lX11 -lXext -ldl -lXxf86dga -lXxf86vm -lm
GLCFLAGS=-DGLQUAKE -I$(MESA_DIR)/include -I/usr/include/glide

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
	$(CC) $(CFLAGS) -o $@ $(SQUAKE_OBJS) $(SVGALDFLAGS) $(LDFLAGS)

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
# RPM
#############################################################################

# Make RPMs.  You need to be root to make this work
RPMROOT=/usr/src/redhat
RPM = rpm
RPMFLAGS = -bb
INSTALLDIR = /usr/local/games/quake
TMPDIR = /var/tmp
RPMDIR = $(TMPDIR)/quake-$(VERSION)
BASERPMDIR = $(TMPDIR)/quake-$(BASEVERSION)

rpm: rpm-quake rpm-quake-data rpm-hipnotic rpm-rogue

rpm-quake: quake.spec \
		$(BUILD_RELEASE_DIR)/bin/squake \
		$(BUILD_RELEASE_DIR)/bin/quake.x11 \
		$(BUILD_RELEASE_DIR)/bin/glquake \
		$(BUILD_RELEASE_DIR)/bin/glquake.glx \
		$(BUILD_RELEASE_DIR)/bin/glquake.3dfxgl
	touch $(RPMROOT)/SOURCES/quake-$(VERSION).tar.gz
	if [ ! -d RPMS ];then mkdir RPMS;fi
	cp $(MOUNT_DIR)/quake.gif $(RPMROOT)/SOURCES/quake.gif

	# basic binaries rpm
	-mkdirhier $(RPMDIR)/$(INSTALLDIR)
	cp $(MOUNT_DIR)/docs/README $(RPMDIR)/$(INSTALLDIR)/.
	cp $(BUILD_RELEASE_DIR)/bin/squake $(RPMDIR)/$(INSTALLDIR)/squake
	strip $(RPMDIR)/$(INSTALLDIR)/squake
	cp $(BUILD_RELEASE_DIR)/bin/quake.x11 $(RPMDIR)/$(INSTALLDIR)/quake.x11
	strip $(RPMDIR)/$(INSTALLDIR)/quake.x11
	cp $(BUILD_RELEASE_DIR)/bin/glquake $(RPMDIR)/$(INSTALLDIR)/glquake
	strip $(RPMDIR)/$(INSTALLDIR)/glquake
	cp $(BUILD_RELEASE_DIR)/bin/glquake.glx $(RPMDIR)/$(INSTALLDIR)/glquake.glx
	strip $(RPMDIR)/$(INSTALLDIR)/glquake.glx
	cp $(BUILD_RELEASE_DIR)/bin/glquake.3dfxgl $(RPMDIR)/$(INSTALLDIR)/glquake.3dfxgl
	strip $(RPMDIR)/$(INSTALLDIR)/glquake.3dfxgl
	-mkdirhier $(RPMDIR)/usr/lib
	cp $(TDFXGL_DIR)/release$(ARCH)$(GLIBC)/lib3dfxgl.so $(RPMDIR)/usr/lib/lib3dfxgl.so
	cp $(MESA_DIR)/lib/libMesaGL.so.2.6 $(RPMDIR)/usr/lib/libMesaGL.so.2.6

	cp quake.spec $(RPMROOT)/SPECS/.
	cd $(RPMROOT)/SPECS; $(RPM) $(RPMFLAGS) quake.spec
	rm -rf $(RPMDIR)
	rm -f $(RPMROOT)/SOURCES/quake-$(VERSION).tar.gz

	mv $(RPMROOT)/RPMS/$(ARCH)/quake-$(VERSION)-$(RPM_RELEASE).$(ARCH).rpm RPMS/.

QUAKEDATADIR=$(TMPDIR)/quake-data-$(BASEVERSION)
rpm-quake-data: quake-data.spec
	# data rpm
	touch $(RPMROOT)/SOURCES/quake-$(BASEVERSION)-data.tar.gz

	-mkdirhier $(QUAKEDATADIR)/$(INSTALLDIR)/id1
	cp $(MASTER_DIR)/id1/pak0.pak $(QUAKEDATADIR)/$(INSTALLDIR)/id1/.
	cp $(MASTER_DIR)/id1/pak1.pak $(QUAKEDATADIR)/$(INSTALLDIR)/id1/.
	cp $(MOUNT_DIR)/docs/README $(QUAKEDATADIR)/$(INSTALLDIR)/.
	cp $(MOUNT_DIR)/data/comexp.txt $(QUAKEDATADIR)/$(INSTALLDIR)/.
	cp $(MOUNT_DIR)/data/help.txt $(QUAKEDATADIR)/$(INSTALLDIR)/.
	cp $(MOUNT_DIR)/data/licinfo.txt $(QUAKEDATADIR)/$(INSTALLDIR)/.
	cp $(MOUNT_DIR)/data/manual.txt $(QUAKEDATADIR)/$(INSTALLDIR)/.
	cp $(MOUNT_DIR)/data/readme.txt $(QUAKEDATADIR)/$(INSTALLDIR)/.
	cp $(MOUNT_DIR)/data/rlicnse.txt $(QUAKEDATADIR)/$(INSTALLDIR)/.
	cp $(MOUNT_DIR)/data/slicnse.txt $(QUAKEDATADIR)/$(INSTALLDIR)/.
	cp $(MOUNT_DIR)/data/techinfo.txt $(QUAKEDATADIR)/$(INSTALLDIR)/.
	cp quake-data.spec $(RPMROOT)/SPECS/.
	cd $(RPMROOT)/SPECS; $(RPM) $(RPMFLAGS) quake-data.spec
	rm -rf $(QUAKEDATADIR)
	rm -f $(RPMROOT)/SOURCES/quake-$(BASEVERSION)-data.tar.gz

	mv $(RPMROOT)/RPMS/$(NOARCH)/quake-data-$(BASEVERSION)-$(RPM_RELEASE).$(NOARCH).rpm RPMS/.

RPMHIPNOTICDIR=$(TMPDIR)/quake-hipnotic-$(BASEVERSION)
rpm-hipnotic: quake-hipnotic.spec
	touch $(RPMROOT)/SOURCES/quake-hipnotic-$(BASEVERSION).tar.gz
	if [ ! -d RPMS ];then mkdir RPMS;fi
	cp $(MOUNT_DIR)/quake.gif $(RPMROOT)/SOURCES/quake.gif
	-mkdirhier $(RPMHIPNOTICDIR)/$(INSTALLDIR)/hipnotic/docs
	cp $(MASTER_DIR)/hipnotic/pak0.pak $(RPMHIPNOTICDIR)/$(INSTALLDIR)/hipnotic/.
	cp $(MASTER_DIR)/hipnotic/config.cfg $(RPMHIPNOTICDIR)/$(INSTALLDIR)/hipnotic/.
	cp $(MASTER_DIR)/hipnotic/docs/manual.doc $(RPMHIPNOTICDIR)/$(INSTALLDIR)/hipnotic/docs/.
	cp $(MASTER_DIR)/hipnotic/docs/manual.htm $(RPMHIPNOTICDIR)/$(INSTALLDIR)/hipnotic/docs/.
	cp $(MASTER_DIR)/hipnotic/docs/manual.txt $(RPMHIPNOTICDIR)/$(INSTALLDIR)/hipnotic/docs/.
	cp $(MASTER_DIR)/hipnotic/docs/readme.doc $(RPMHIPNOTICDIR)/$(INSTALLDIR)/hipnotic/docs/.
	cp $(MASTER_DIR)/hipnotic/docs/readme.htm $(RPMHIPNOTICDIR)/$(INSTALLDIR)/hipnotic/docs/.
	cp $(MASTER_DIR)/hipnotic/docs/readme.txt $(RPMHIPNOTICDIR)/$(INSTALLDIR)/hipnotic/docs/.
	cp quake-hipnotic.spec $(RPMROOT)/SPECS/.
	cd $(RPMROOT)/SPECS; $(RPM) $(RPMFLAGS) quake-hipnotic.spec
	rm -rf $(RPMHIPNOTICDIR)
	rm -f $(RPMROOT)/SOURCES/quake-hipnotic-$(BASEVERSION).tar.gz

	mv $(RPMROOT)/RPMS/$(NOARCH)/quake-hipnotic-$(BASEVERSION)-$(RPM_RELEASE).$(NOARCH).rpm RPMS/.

RPMROGUEDIR=$(TMPDIR)/quake-rogue-$(BASEVERSION)
rpm-rogue: quake-rogue.spec
	touch $(RPMROOT)/SOURCES/quake-rogue-$(BASEVERSION).tar.gz
	if [ ! -d RPMS ];then mkdir RPMS;fi
	cp $(MOUNT_DIR)/quake.gif $(RPMROOT)/SOURCES/quake.gif
	-mkdirhier $(RPMROGUEDIR)/$(INSTALLDIR)/rogue/docs
	cp $(MASTER_DIR)/rogue/pak0.pak $(RPMROGUEDIR)/$(INSTALLDIR)/rogue/.
	cp $(MASTER_DIR)/rogue/docs/manual.doc $(RPMROGUEDIR)/$(INSTALLDIR)/rogue/docs/.
	cp $(MASTER_DIR)/rogue/docs/manual.htm $(RPMROGUEDIR)/$(INSTALLDIR)/rogue/docs/.
	cp $(MASTER_DIR)/rogue/docs/manual.txt $(RPMROGUEDIR)/$(INSTALLDIR)/rogue/docs/.
	cp $(MASTER_DIR)/rogue/docs/readme.doc $(RPMROGUEDIR)/$(INSTALLDIR)/rogue/docs/.
	cp $(MASTER_DIR)/rogue/docs/readme.htm $(RPMROGUEDIR)/$(INSTALLDIR)/rogue/docs/.
	cp $(MASTER_DIR)/rogue/docs/readme.txt $(RPMROGUEDIR)/$(INSTALLDIR)/rogue/docs/.
	cp $(MASTER_DIR)/rogue/docs/ctf.doc $(RPMROGUEDIR)/$(INSTALLDIR)/rogue/docs/.
	cp $(MASTER_DIR)/rogue/docs/ctf.htm $(RPMROGUEDIR)/$(INSTALLDIR)/rogue/docs/.
	cp $(MASTER_DIR)/rogue/docs/ctf.txt $(RPMROGUEDIR)/$(INSTALLDIR)/rogue/docs/.
	cp quake-rogue.spec $(RPMROOT)/SPECS/.
	cd $(RPMROOT)/SPECS; $(RPM) $(RPMFLAGS) quake-rogue.spec
	rm -rf $(RPMROGUEDIR)
	rm -f $(RPMROOT)/SOURCES/quake-rogue-$(BASEVERSION).tar.gz

	mv $(RPMROOT)/RPMS/$(NOARCH)/quake-rogue-$(BASEVERSION)-$(RPM_RELEASE).$(NOARCH).rpm RPMS/.

quake.spec : $(MOUNT_DIR)/quake.spec.sh
	sh $< $(VERSION) $(RPM_RELEASE) $(INSTALLDIR) > $@

quake-data.spec : $(MOUNT_DIR)/quake-data.spec.sh
	sh $< $(BASEVERSION) $(RPM_RELEASE) $(INSTALLDIR) > $@

quake-hipnotic.spec : $(MOUNT_DIR)/quake-hipnotic.spec.sh
	sh $< $(BASEVERSION) $(RPM_RELEASE) $(INSTALLDIR) > $@

quake-rogue.spec : $(MOUNT_DIR)/quake-rogue.spec.sh
	sh $< $(BASEVERSION) $(RPM_RELEASE) $(INSTALLDIR) > $@

#############################################################################
# MISC
#############################################################################

clean: clean-debug clean-release
	rm -f squake.spec glquake.spec quake.x11.spec

clean-debug:
	$(MAKE) clean2 BUILDDIR=$(BUILD_DEBUG_DIR) CFLAGS="$(DEBUG_CFLAGS)"

clean-release:
	$(MAKE) clean2 BUILDDIR=$(BUILD_RELEASE_DIR) CFLAGS="$(DEBUG_CFLAGS)"

clean2:
	-rm -f $(SQUAKE_OBJS)
