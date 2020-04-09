
# build targets
TARGET_EXEC ?= main.exe
TARGET_OUT ?= a.out

# folder structure
BUILD_DIR ?= ./out
SRC_DIRS ?= ./src
INC_DIR ?= ./inc
VX_DIR ?= ./VxWorks

# find all source files
SRCS := $(shell find $(SRC_DIRS) -name '*.c')

#generate object file for each source file
TMP_OBJS := $(subst $(SRC_DIRS), $(BUILD_DIR), $(SRCS))
OBJS := $(TMP_OBJS:.c=.o)
# generate dependency file for each object
DEPS := $(OBJS:.o=.d)

# find all header files in include folders
INCS := $(shell find $(INC_DIR) -name '*.h')
VX :=   $(shell find $(VX_DIR) -name '*.h')
INC_FLAGS := $(addprefix -I ,$(INC_DIR)) $(addprefix -I , $(VX_DIR))

CFLAGS ?= $(INC_FLAGS) -MMD -MP -Wall -I. -Itarget_h -iquote -D_GNU_SOURCE -D_REENTRANT

# .exe build target
$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# generate object files for each source file
$(BUILD_DIR)/%.o: $(SRC_DIRS)/%.c $(INCS)
	$(MKDIR_P) $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@ -L. -lv2lin -lpthread

# builds a.out file for debugging
debug: $(OBJS)
	$(CC) $(OBJS) -g -o $(BUILD_DIR)/$(TARGET_OUT) $(LDFLAGS)

# build and runs executable
run: $(TARGET_EXEC)
	./$(TARGET_EXEC)

# debugs makefile, prints lists of identified sources, objects and headers
debug_make:
	@echo $(SRCS)
	@echo $(OBJS)
	@echo $(INCS)
	@echo $(VX)
	@echo $(INC_FLAGS)

# when in doubt clean
.PHONY: clean

# deletes generated files
clean:
	$(RM) -r $(BUILD_DIR)/*.o
	$(RM) -r $(BUILD_DIR)/*.d
	$(RM) -r $(TARGET_EXEC)
	$(RM) -r $(BUILD_DIR)/$(TARGET_OUT)



-include $(DEPS)

MKDIR_P ?= mkdir -p
