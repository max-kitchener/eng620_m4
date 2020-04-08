CC = gcc

TARGET_EXEC ?= main.exe
TARGET_OUT ?= a.out

BUILD_DIR ?= ./out
SRC_DIRS ?= ./src
INC_DIR ?= ./inc
VX_DIR ?= ./VxWorks

SRCS := $(shell find $(SRC_DIRS) -name '*.c')
TMP_OBJS := $(subst $(SRC_DIRS), $(BUILD_DIR), $(SRCS))
OBJS := $(TMP_OBJS:.c=.o)
DEPS := $(OBJS:.o=.d)

INCS := $(shell find $(INC_DIR) -name '*.h')
VX :=   $(shell find $(VX_DIR) -name '*.h')
INC_FLAGS := $(addprefix -I ,$(INC_DIR)) $(addprefix -I , $(VX_DIR))

CFLAGS ?= $(INC_FLAGS) -MMD -MP -Wall -I. -Itarget_h -iquote -D_GNU_SOURCE -D_REENTRANT


$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# c source
$(BUILD_DIR)/%.o: $(SRC_DIRS)/%.c $(INCS)
	$(MKDIR_P) $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@ -L. -lv2lin -lpthread

.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)/*.o
	$(RM) -r $(BUILD_DIR)/*.d
	$(RM) -r $(TARGET_EXEC)
	$(RM) -r $(BUILD_DIR)/$(TARGET_OUT)

debug: $(OBJS)
	$(CC) $(OBJS) -o $(BUILD_DIR)/$(TARGET_OUT) $(LDFLAGS)

debug_make:
	@echo $(SRCS)
	@echo $(OBJS)
	@echo $(INCS)
	@echo $(VX)
	@echo $(INC_FLAGS)

run:
	./$(TARGET_EXEC)

-include $(DEPS)

MKDIR_P ?= mkdir -p
