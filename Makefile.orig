#######################################################################
#   Copyright [2014] [Cisco Systems, Inc.]
# 
#   Licensed under the Apache License, Version 2.0 (the \"License\");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
# 
#       http://www.apache.org/licenses/LICENSE-2.0
# 
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an \"AS IS\" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#######################################################################

#
include $(CCSP_ROOT_DIR)/arch/ccsp_common.mk

#
#	Set up include directories
#

CFLAGS += $(addprefix -I, $(INCPATH))


CFLAGS += -Wall
LDFLAGS   += -lccsp_common


source_files := $(call add_files_from_src,,'*.c')
obj_files := $(addprefix $(ComponentBuildDir)/, $(source_files:%.c=%.o))


target := $(ComponentBuildDir)/log_agent

-include $(obj_files:.o=.d)

INCPATH += $(CCSP_ROOT_DIR)/../generic/rdk_logger/include/
CFLAGS += -DFEATURE_SUPPORT_RDKLOG
$(target): $(obj_files)


#
#	Build targets
#
all: $(target)

.PHONY: all clean

clean:
	rm -Rf $(ComponentBuildDir)


install_targets += $(wildcard $(ComponentArchCfgDir)/*)
install_targets += $(wildcard $(ComponentBoardCfgDir)/*)
install_targets += $(wildcard $(ComponentBoardScriptsDir)/*)
# scripts directories from arch, arch-board, arch-board-customer
install_targets += $(wildcard $(ComponentArchScriptsDir)/*)
install_targets += $(wildcard $(ComponentBoardScriptsDir)/*)
install_targets += $(wildcard $(ComponentBoardCustomScriptsDir)/*)


install:
	@echo "Installing Log Agent Installables"
	@install -d -m 0755 $(CCSP_OUT_DIR)/logagent
	@install -m 0755 $(target) $(CCSP_OUT_DIR)/logagent
	@cp -f arch/intel_usg/boards/rdkb_arm/config/comcast/LogAgent.xml $(CCSP_OUT_DIR)/logagent
	@cp -f arch/intel_usg/boards/arm_shared/config/msg_daemon.cfg $(CCSP_OUT_DIR)/logagent




#
# include custom post makefile, if exists
#
ifneq ($(findstring $(CCSP_CMPNT_BUILD_CUSTOM_MK_POST), $(wildcard $(ComponentBoardDir)/*.mk)), )
    include $(ComponentBoardDir)/$(CCSP_CMPNT_BUILD_CUSTOM_MK_POST)
endif

