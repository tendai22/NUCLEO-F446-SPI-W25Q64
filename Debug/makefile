################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Drivers/STM32F4xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
NUCLEO-F446-SPI-W25Q64.elf \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
NUCLEO-F446-SPI-W25Q64.list \

OBJCOPY_BIN += \
NUCLEO-F446-SPI-W25Q64.bin \


# All Target
all: NUCLEO-F446-SPI-W25Q64.elf secondary-outputs

# Tool invocations
NUCLEO-F446-SPI-W25Q64.elf: $(OBJS) $(USER_OBJS) D:\tenda_000\STM32-Workplace\NUCLEO-F446-SPI-W25Q64\STM32F446RETX_FLASH.ld
	arm-none-eabi-gcc -o "NUCLEO-F446-SPI-W25Q64.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"D:\tenda_000\STM32-Workplace\NUCLEO-F446-SPI-W25Q64\STM32F446RETX_FLASH.ld" --specs=nosys.specs -Wl,-Map="NUCLEO-F446-SPI-W25Q64.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

NUCLEO-F446-SPI-W25Q64.list: $(EXECUTABLES)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "NUCLEO-F446-SPI-W25Q64.list"
	@echo 'Finished building: $@'
	@echo ' '

NUCLEO-F446-SPI-W25Q64.bin: $(EXECUTABLES)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "NUCLEO-F446-SPI-W25Q64.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) *
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_BIN)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
