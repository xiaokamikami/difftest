#***************************************************************************************
# Copyright (c) 2024 Beijing Institute of Open Source Chip (BOSC)
# Copyright (c) 2020-2024 Institute of Computing Technology, Chinese Academy of Sciences
# Copyright (c) 2020-2021 Peng Cheng Laboratory
#
# DiffTest is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#          http://license.coscl.org.cn/MulanPSL2
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
#
# See the Mulan PSL v2 for more details.
#***************************************************************************************
CXX = g++
CXXFLAGS = -std=c++11 -w -pedantic -O3
LDFLAGS = -lz -lzstd -ldl

SIMDIFF_CSRC_DIRS = \
    $(abspath ./src/test/csrc/simdifftest/) \
    $(abspath ./src/test/csrc/difftest/) \
    $(abspath ./src/test/csrc/common/) \
    $(abspath ./config/) \
    $(abspath ../build/generated-src/) \
    $(abspath ../build/)

SIMDIFF_BUILD_DIR = $(abspath ../build/simv-compile)

SIMDIFF_CXXFILES = $(shell find $(SIMDIFF_CSRC_DIRS) -name "*.cpp")
SIMDIFF_INCFLAGS = $(foreach dir,$(SIMDIFF_CSRC_DIRS),-I$(dir))

CXXFLAGS += +DCONFIG_SIMDIFFTEST $(SIM_CXXFLAGS)

OBJS = $(patsubst $(SIMDIFF_CXXFILES)%.cpp,$(SIMDIFF_BUILD_DIR)%.o,$(SIMDIFF_CXXFILES))  

PROM = simdifftest
$(PROM): $(OBJS)  
	$(CXX) $(CXXFLAGS) -g -o $@ $^ $(LDFLAGS)  

$(SIMDIFF_BUILD_DIR)%.o: %.cpp  
	$(CXX) $(CXXFLAGS) $(SIMDIFF_INCFLAGS) -c $< -g -o $@

.PHONY: dirs
dirs:  
	$(shell mkdir -p $(SIMDIFF_BUILD_DIR))  
simdiff:dirs $(PROM)