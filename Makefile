# Makefile for ManQuake
#
# https://github.com/slotzero/manquake

# executable
BIN_FILE	:= manquake

# directories
DEBUG_DIR	:= debug
RELEASE_DIR	:= release
OBJS_DIR	:= objs
BIN_DIR		:= bin

# compiler flags
CC		:= gcc
CFLAGS		:= -m32 -Dstricmp=strcasecmp -g -O6 -ffast-math -funroll-loops -fexpensive-optimizations
DEBUG_CFLAGS	:= -m32 -Dstricmp=strcasecmp -g
LDFLAGS		:= -lm -ldl
ASFLAGS		:= -DELF -x assembler-with-cpp

# target
TARGET		:= $(BUILD_DIR)/$(BIN_DIR)/$(BIN_FILE)

# object files
OBJS		:= $(addprefix $(BUILD_DIR)/$(OBJS_DIR)/, \
		   cmd.o common.o console.o crc.o cvar.o host.o host_cmd.o iplog.o banlog.o mathlib.o \
		   model.o net_dgrm.o net_loop.o net_main.o net_udp.o net_bsd.o pr_cmds.o pr_edict.o \
		   pr_exec.o r_main.o security.o sv_main.o sv_phys.o sv_move.o sv_user.o zone.o wad.o \
		   world.o sys_linux.o math.o worlda.o sys_a.o)

.PHONY: debug release build_debug build_release all

build_debug debug:
	@-mkdir -p $(DEBUG_DIR)/$(BIN_DIR) $(DEBUG_DIR)/$(OBJS_DIR)
	$(MAKE) $(DEBUG_DIR)/$(BIN_DIR)/$(BIN_FILE) CFLAGS:="$(DEBUG_CFLAGS)" BUILD_DIR:=$(DEBUG_DIR)
	@echo $@ complete

build_release release:
	@-mkdir -p $(RELEASE_DIR)/$(BIN_DIR) $(RELEASE_DIR)/$(OBJS_DIR)
	$(MAKE) $(RELEASE_DIR)/$(BIN_DIR)/$(BIN_FILE) BUILD_DIR:=$(RELEASE_DIR)
	strip $(RELEASE_DIR)/$(BIN_DIR)/$(BIN_FILE)
	@echo $@ complete

all: build_debug build_release

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

$(BUILD_DIR)/$(OBJS_DIR)/%.o: %.s
	$(CC) $(CFLAGS) $(ASFLAGS) -c $< -o $@

$(BUILD_DIR)/$(OBJS_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean clean-debug clean-release clean-build clean-clean

clean: clean-debug clean-release

clean-debug:
	$(MAKE) clean-build BUILD_DIR:=$(DEBUG_DIR)

clean-release:
	$(MAKE) clean-build BUILD_DIR:=$(RELEASE_DIR)

clean-build:
	$(RM) $(OBJS) $(TARGET)

clean-clean:
	-rm -rf $(RELEASE_DIR) $(DEBUG_DIR)
