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

PROTOBUF_BUILD    = $(abspath $(BUILD_DIR)/protobuf)
PROTOBUF_SRC      = $(abspath $(BUILD_DIR)/generated-src)

proto:
	mkdir -p $(PROTOBUF_BUILD)
	protoc -I=$(PROTOBUF_SRC) --cpp_out=$(PROTOBUF_BUILD) $(PROTOBUF_SRC)/difftest-iotrace.proto --experimental_allow_proto3_optional
