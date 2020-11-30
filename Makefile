CXX=g++

COMP_DATA:=$(shell date)

OUT_PATH=./bin/
OBJ_PATH=./obj/
DEBUG_PATH=debug/
RELEASE_PATH=release/
OUT_NAME=main

SRC_PATH=./ ./net/ ./log/
SRC_FILE=$(foreach SUB_DIR,$(SRC_PATH),$(wildcard $(SUB_DIR)*.cpp))
ALL_FILE=$(notdir $(SRC_FILE))
OBJ_FILE=$(patsubst %.cpp,%.o,$(ALL_FILE))

OBJ_DEBUG_FILE=$(addprefix $(OBJ_PATH)$(DEBUG_PATH),$(notdir $(OBJ_FILE)))
OUT_DEBUG_FILE=$(OUT_PATH)$(DEBUG_PATH)$(OUT_NAME)

OBJ_RELEASE_FILE=$(addprefix $(OBJ_PATH)$(RELEASE_PATH), $(notdir $(OBJ_FILE)))
OUT_RELEASE_FILE=$(OUT_PATH)$(RELEASE_PATH)$(OUT_NAME)

DEBUG_CXXFLAGS=-g -Wall -Wno-conversion-null -Wno-format-security -Werror -DCOMP_DATE='"$(COMP_DATA)"'
RELEASE_CXXFLAGS=-O2 -Wall -Wno-conversion-null -Wno-format-security -Werror -DCOMP_DATE='"$(COMP_DATA)"'

VPATH = ./ : ./net/ : ./log/


debug: pre_debug $(OUT_DEBUG_FILE)

pre_debug:
	-$(shell mkdir $(OBJ_PATH) -p)
	-$(shell mkdir $(OBJ_PATH)$(DEBUG_PATH) -p)
	-$(shell mkdir $(OUT_PATH) -p)
	-$(shell mkdir $(OUT_PATH)$(DEBUG_PATH) -p)

$(OUT_DEBUG_FILE) : $(OBJ_DEBUG_FILE)
	$(CXX) $(DEBUG_CXXFLAGS) $(addprefix $(OBJ_PATH)$(DEBUG_PATH), $(notdir $^)) -o $@ -lpthread

$(OBJ_PATH)$(DEBUG_PATH)%.o : %.cpp
	$(CXX) -c $(DEBUG_CXXFLAGS)  $< -o $@

release: pre_release $(OUT_RELEASE_FILE)

pre_release:
	-$(shell mkdir $(OBJ_PATH) -p)
	-$(shell mkdir $(OBJ_PATH)$(RELEASE_PATH) -p)

$(OBJ_RELEASE_FILE) : $(OBJ_RELEASE_FILE)
	$(CXX) $(RELEASE_CXXFLAGS) $(addprefix $(OBJ_PATH)$(DEBUG_PATH), $(notdir $^)) -o $@

$(OBJ_PATH)$(RELEASE_PATH)%.o : %.cpp
	$(CXX) -c $(RELEASE_CXXFLAGS) $< -o $@

clean:
	@echo "make clean"
	-$(shell rm $(OBJ_PATH) -rf )
	-$(shell rm $(OUT_PATH)$(DEBUG_PATH)$(OUT_NAME) -f)
	-$(shell rm $(OUT_PATH)$(RELEASE_PATH)$(OUT_NAME) -f)

info:
	$(info SRC_FILE:  $(SRC_FILE))
	$(info ALL_FILE:  $(ALL_FILE))
	$(info OBJ_FILE:  $(OBJ_FILE))
	$(info OBJ_DEBUG_FILE:  $(OBJ_DEBUG_FILE))
	$(info OUT_DEBUG_FILE:  $(OUT_DEBUG_FILE))