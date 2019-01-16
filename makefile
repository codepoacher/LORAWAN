#TARGET=MOTO-MCU

ccache_check = $(shell ccache -V 2>/dev/null)
ifneq ($(ccache_check),)
	CCACHE_T = ccache
endif
export CC 			= arm-none-eabi-gcc
export CPP 			= $(CCACHE_T) arm-none-eabi-g++
export AS 			= arm-none-eabi-gcc -x assembler-with-cpp 
export LD 			= arm-none-eabi-ld
export OBJCOPY 		= arm-none-eabi-objcopy

TOP = $(shell pwd)

				
#-ffunction-sections -fdata-sections 
# 

cur_date=$(shell date)

CPU = -mcpu=cortex-m0plus
FPU = #-mfpu=fpv4-sp-d16
FLOAT-ABI = #-mfloat-abi=hard

CUR=$(shell pwd)
CHIP = $(shell cat $(CUR)/build_conf/chip_conf | tr '\n' '\0')
HAL_CONF = $(CUR)/build_conf/hal_conf
SRC_SHELL = $(CUR)/build_tool/src_select.sh
HAL_SRC_CMD = $(SRC_SHELL) $(CHIP) $(HAL_CONF)

SRC_SUFFIX = c
COMM_SRC_CONF = $(CUR)/build_conf/src_conf
COMM_SRC_SHELL = $(CUR)/build_tool/comm_src_select.sh
COMM_SRC_CMD = $(COMM_SRC_SHELL) $(COMM_SRC_CONF)

ASM_SUFFIX = S
ASM_CMD = $(COMM_SRC_SHELL) $(COMM_SRC_CONF) $(ASM_SUFFIX)

HEAD_SUFFIX = h
HEAD_CONF = $(CUR)/build_conf/head_conf
HEAD_CMD = $(COMM_SRC_SHELL) $(HEAD_CONF) $(HEAD_SUFFIX)


ifeq ($(LINK_FILE),)
	LINK_FILE=m_system/arch/s32l083/linker/STM32L083V8Tx_FLASH.ld
endif

ifeq ($(TARGET),)
	TARGET=IA-WA100C
endif

MAP_FILE=stm32l083retx.map

C_DEFS = -DUSE_HAL_DRIVER -DSTM32L083xx -DREGION_CN470 -DNO_MAC_PRINTF -DLORAMAC_CLASSB_ENABLED

MCU = $(CPU) -mthumb -mthumb-interwork $(FPU) $(FLOAT-ABI)

#$(info $(HEAD_CMD))
INC_FLAGS = $(shell $(HEAD_CMD))	

export CFLAGS := -g $(MCU) -Os  -fsigned-char -ffunction-sections -fdata-sections  $(C_DEFS) -DARM_MATH_CM0PLUS \
	-ffunction-sections -fdata-sections $(INC_FLAGS) -MMD -MP -MF"$(@:%.o=%.cdep)" -MT"$(@)"

export CPPFLAGS := -g $(CPU) $(FPU) $(FLOAT-ABI) -Os  -fsigned-char -ffunction-sections -fdata-sections $(c_DEFS) \
	-ffunction-sections -fdata-sections $(INC_FLAGS) -fabi-version=0 -MMD -MP -MF"$(@:%.o=%.cdep)" -MT"$(@)"


ASFLAGS = -W -Wall

LDFLAGS = $(MCU) -fsigned-char -fmessage-length=0 -ffunction-sections -fdata-sections -T \
		"$(LINK_FILE)" -Xlinker --gc-sections -Wl,-Map,"$(MAP_FILE)" -lnosys --specs=nano.specs  -specs=rdimon.specs -lc -lm
LDLIB += -lc -lm

########################################################################
#C_SRC = $(shell find ./ -name '*.c' | grep -v erpc | grep -v mcu2mpu_client)
C_SRC = $(shell $(COMM_SRC_CMD) 2>/dev/null)
C_SRC += $(shell $(HAL_SRC_CMD) 2>/dev/null)
C_OBJ = $(C_SRC:%.c=%.o)
#C_DEP = $(C_SRC:%.c=%.cdep)

#CPP_SRC = $(shell find ./ -name '*.cpp' | grep -v erpc | grep -v mcu2mpu_client)
#CPP_OBJ = $(CPP_SRC:%.cpp=%.o)
#CPP_DEP = $(CPP_SRC:%.cpp=%.cdep)

#ASM_SRC = $(shell find ./ -name '*.S')
ASM_SRC = $(shell $(ASM_CMD) 2>/dev/null)
ASM_OBJ = $(ASM_SRC:%.S=%.o)
ASM_DEP = $(ASM_SRC:%.S=%.adep)

########################################################################
.PHONY: all clean print
#-ffunction-sections -fdata-sections 
#all:$(CPP_DEP)  $(C_DEP) $(ASM_DEP) $(CPP_OBJ) $(C_OBJ) $(ASM_OBJ)
all:  $(CPP_OBJ) $(C_OBJ) $(ASM_OBJ) 

	@echo -e "\033[31mlinking target $(TARGET).elf\033[0m"
	@$(CC) $(LDFLAGS) -o "$(TARGET).elf" $(ASM_OBJ) $(C_OBJ) $(CPP_OBJ) $(LDLIB)

	$(OBJCOPY) $(TARGET).elf $(TARGET).bin -Obinary
	$(OBJCOPY) $(TARGET).elf $(TARGET).hex -Oihex

########################################################################
$(C_OBJ):%.o:%.c
	@echo -e "\033[33mBuilding $<\033[0m" 
	@$(CC) $(CFLAGS) -c $< -o $@ 

########################################################################
$(CPP_OBJ):%.o:%.cpp
	@echo -e "\033[33mBuilding $<\033[0m"
	@$(CPP) $(CPPFLAGS) -c $< -o $@ 


########################################################################
$(ASM_OBJ):%.o:%.S
	@echo -e "\033[33mBuilding $<\033[0m"
	@$(AS) -c $< -o $@ $(ASFLAGS)

########################################################################
clean:
	rm -rf build
	@rm -rf $(TARGET).bin
	@rm -rf $(TARGET).elf
	@rm -rf $(TARGET).hex
	@rm -rf $(MAP_FILE)
	@for i in $(shell find ./ -name '*.o'); do if [ -e $${i} ];then rm $${i};fi;done
	@for i in $(shell find ./ -name '*.cdep'); do if [ -e $${i} ]; then rm $${i};fi;done
	@for i in $(shell find ./ -name '*.adep'); do if [ -e $${i} ]; then rm $${i};fi; done
	@rm -rf $(TARGET) $(TARGET).elf $(TARGET).hex

print:
	@echo "ccache_check = $(ccache_check)"
