SRC_DIR = src
BUILD_DIR = build
OUTPUT_DIR = bin
INCLUDE_DIR = include
HEADERS_DIR = src/headers
LIB_DIR = lib

OBJECTS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(wildcard $(SRC_DIR)/*.cpp))
DEPS := $(OBJECTS:.o=.d)

CPP = g++
CPP_FLAGS = -I$(INCLUDE_DIR) -I$(HEADERS_DIR) -MMD -MP -std=c++14 -g
LINKER_FLAGS = -L$(LIB_DIR)

all : $(OBJECTS)
	$(CPP) -o $(OUTPUT_DIR)/main.exe $(OBJECTS) $(LINKER_FLAGS)

run :$(OBJECTS)
	$(CPP) -o $(OUTPUT_DIR)/main.exe $(OBJECTS) $(LINKER_FLAGS)
	$(OUTPUT_DIR)/main.exe

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.cpp
	$(CPP) -o $@ -c $< $(CPP_FLAGS)

.PHONY : clean
clean : 
	del $(BUILD_DIR)\*.o $(BUILD_DIR)\*.d
	del $(OUTPUT_DIR)\*.exe


-include $(DEPS)