# GPLv3
# by Mark Hill 2017-2018
RM    = rm -rf
MKDIR = mkdir -p
CMAKE = cmake
GCOV = gcov
SH = sh

# Fine to edit this section
BUILD_TYPE := Debug
PROJECT_NAME := calsat software
TEST_EXECUTABLE_NAME := calsat-test
TEST_MAIN_FILE := test/test.cc
SUBDIRS := telem orientation drivers
INCLUDE_ROOT := include
EIGEN_DIR := $(INCLUDE_ROOT)/eigen
INCLUDES := $(INCLUDE_ROOT) $(EIGEN_DIR)
# Not a good idea to do this since then #include paths make no sense
# INCLUDES = $(INCLUDE_ROOT) $(EIGEN_DIR) $(patsubst %,$(INCLUDE_ROOT)/%,$(SUBDIRS))
PROJECT_LIBS := m rt $(SUBDIRS)

COMM_RELEASE_FLAGS = -O2
COMM_TEST_FLAGS = -g -O0 --coverage
COMM_FLAGS = $(if $(filter Debug,$(BUILD_TYPE)), $(COMM_TEST_FLAGS), $(COMM_FLAGS))
CFLAGS = $(COMM_TEST_FLAGS)
CXXFLAGS = $(COMM_TEST_FLAGS)
GCOV_FLAGS =

# Don't edit this
TOOLCHAIN_PREFIX := toolchain-
TOOLCHAIN_INSTALL_PREFIX := install-
NATIVE_TOOLCHAIN_NAME := native
BUILD_DIR_BASE := build_
BUILD_DIR = $(BUILD_DIR_BASE)$(subst $(TOOLCHAIN_PREFIX),,$(TOOLCHAIN_NAME))
SCRIPTS_DIR := scripts
BUILD_MAKEFILE = $(BUILD_DIR)/Makefile
ROOT_MAKEFILE := Makefile
CONFIG = .config

# Don't edit this
ifneq ("","$(wildcard $(strip $(CONFIG)))")
include $(CONFIG)
else
TOOLCHAIN_NAME = $(TOOLCHAIN_PREFIX)$(NATIVE_TOOLCHAIN_NAME)
endif
ifdef TOOLCHAIN_NAME
ifneq ("$(TOOLCHAIN_NAME)","$(TOOLCHAIN_PREFIX)$(NATIVE_TOOLCHAIN_NAME)")
TOOLCHAIN_DIR = $(BUILD_DIR)/$(subst _,,.$(TOOLCHAIN_PREFIX))/$(TOOLCHAIN_NAME)
CMAKE_TOOLCHAIN_FILE = $(BUILD_DIR)/$(TOOLCHAIN_NAME).cmake
TOOLCHAIN_INSTALL_SCRIPTS_DIR = $(SCRIPTS_DIR)
TOOLCHAIN_INSTALL_SCRIPT = $(TOOLCHAIN_INSTALL_SCRIPTS_DIR)/$(TOOLCHAIN_INSTALL_PREFIX)$(TOOLCHAIN_NAME).sh
endif
endif
export BUILD_DIR TOOLCHAIN_NAME TOOLCHAIN_DIR CMAKE_TOOLCHAIN_FILE

EMPTY :=
SPACE := $(EMPTY) $(EMPTY)
define create-cmake-list
$(subst $(SPACE),;,$(strip $1))
endef
define newline :=
$(strip)
$(strip)
endef

SUPPORTED_BOARDS := $(subst .sh,,\
	$(subst $(TOOLCHAIN_INSTALL_PREFIX)$(TOOLCHAIN_PREFIX),,\
	$(foreach config,$(shell find $(TOOLCHAIN_INSTALL_SCRIPTS_DIR) \
	-name '$(TOOLCHAIN_INSTALL_PREFIX)$(TOOLCHAIN_PREFIX)*'),$(shell basename $(config)))))
SUPPORTED_BOARDS_CONFIG_TARGETS := $(patsubst %,%-config,$(SUPPORTED_BOARDS))

.PHONY: all install test clean distclean cscope cscope.files tags ctags ycm_config depends $(SUPPORTED_BOARDS_CONFIG_TARGETS) reallyclean listconfigs clangfile


all: $(BUILD_MAKEFILE)
	$(MAKE) -C $(BUILD_DIR)

# useful if you added new files and cmake hasn't seen them yet
# this is needed because the subdir cmakelists use globbing
depends: $(CONFIG)
	@ touch $(CONFIG)

$(BUILD_MAKEFILE): $(CONFIG) $(CMAKE_TOOLCHAIN_FILE) $(ROOT_MAKEFILE) | $(BUILD_DIR)
	(cd $(BUILD_DIR) && \
		$(CMAKE) -DSUBDIRS="$(call create-cmake-list,$(SUBDIRS))" \
		-DINCLUDES="$(call create-cmake-list,$(INCLUDES))" \
		-DPROJECT_LIBS="$(call create-cmake-list,$(PROJECT_LIBS))" \
		-DTEST_EXECUTABLE_NAME="$(TEST_EXECUTABLE_NAME)" \
		-DTEST_MAIN_FILE="$(TEST_MAIN_FILE)" \
		-DPROJECT_NAME="$(PROJECT_NAME)" \
		-DCFLAGS="$(CFLAGS)" \
		-DCXXFLAGS="$(CXXFLAGS)" \
		-DCMAKE_BUILD_TYPE="$(BUILD_TYPE)" \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		-DCMAKE_TOOLCHAIN_FILE=$(subst $(BUILD_DIR)/%,%,$(CMAKE_TOOLCHAIN_FILE_)) ..)

$(CMAKE_TOOLCHAIN_FILE): $(CONFIG) | $(BUILD_DIR)
	@ (sh $(TOOLCHAIN_INSTALL_SCRIPT))

$(BUILD_DIR): $(CONFIG)
	$(MKDIR) $@

$(CONFIG):
	$(info no config detected.$(newline)$(subst targets:,targets:$(newline),$(shell $(MAKE) listconfigs))$(newline))
	@- printf "using native-config as default\n"
	@- $(MAKE) $(NATIVE_TOOLCHAIN_NAME)-config

listconfigs:
	@- printf "build a configuration using \"make <config>\". "
	@- printf "available config targets:\n\t$(SUPPORTED_BOARDS_CONFIG_TARGETS)\n"

$(SUPPORTED_BOARDS_CONFIG_TARGETS):
	@ printf "TOOLCHAIN_NAME = $(TOOLCHAIN_PREFIX)$(subst -config,,$@)\n" > $(CONFIG)

install:

test: install
	@ $(BUILD_DIR)/$(TEST_EXECUTABLE_NAME)

coverage:
	@ cd $(BUILD_DIR) && $(GCOV) $(GCOV_FLAGS) $(abspath $(shell find $(BUILD_DIR) -name '*.gcno'))

clean:
	@ (cd $(BUILD_DIR) && $(MAKE) clean)

distclean:
	@- $(RM) $(BUILD_DIR)
	@- $(RM) $(CONFIG)

reallyclean:
	@- $(RM) build*
	@- $(RM) $(CONFIG)
	@- $(RM) cscope*
	@- $(RM) tags
	@- $(RM) .ycm_extra_conf.py
	@- $(RM) compile_commands.json



cscope.files: FIND = find
cscope.files: EXCLUDE_DIRS =
cscope cscope.files: CSCOPE = cscope
ctags tags cscope cscople.files ycm_config: CURDIR = $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
ctags tags: CTAGS = ctags
ycm_config: YCM_GEN_CONFIG = $(HOME)/.vim/bundle/YCM-Generator/config_gen.py

cscope: cscope.files
	$(CSCOPE) -b -q -k

# needs fixing
cscope.files: $(CONFIG)
	$(FIND) $(CURDIR) $(patsubst %,-path "$(CURDIR)/%*" -prune -o,$(EXCLUDE_DIRS)) \
		-path "$(CURDIR)/$(BUILD_DIR_BASE)*"  '!' -path "$(CURDIR)/$(BUILD_DIR)*" -prune -a -name "*.*" -o \
		-name "*.[chxsS]" -o \
		-name "*.cpp" -o \
		-name "*.cc" -o \
		-name "*.hpp" > $(CURDIR)/cscope.files

ctags tags:
	$(CTAGS) --recurse --exclude=$(SCRIPTS_DIR) --exclude=$(BUILD_DIR_BASE)* --exclude="*.js" --languages=C --languages=+C++ --totals $(CURDIR)

COMPILE_COMMANDS_FILE := compile_commands.json
$(COMPILE_COMMANDS_FILE): $(CONFIG) | $(BUILD_DIR)
	ln -srf $(BUILD_DIR)/$(COMPILE_COMMANDS_FILE) $(COMPILE_COMMANDS_FILE)

# pseudo-target for compile_commands.json
# used because the alternative is long and complicated to type
clangfile: $(COMPILE_COMMANDS_FILE)

ycm_config: $(CONFIG) $(COMPILE_COMMANDS_FILE)
	@ $(YCM_GEN_CONFIG) -f $(CURDIR)


help:
	@- less README.md



