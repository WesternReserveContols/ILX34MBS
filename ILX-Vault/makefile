################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM0/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/subdir.mk
-include Drivers/STM32F0xx_HAL_Driver/Src/subdir.mk
-include Core/thread/subdir.mk
-include Core/serial/subdir.mk
-include Core/mutex/subdir.mk
-include Core/io/subdir.mk
-include Core/context/subdir.mk
-include Core/can/subdir.mk
-include Core/_ILX34-MBS/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/DSC/subdir.mk
-include Core/Src/subdir.mk
-include Core/subdir.mk
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

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := ASC-ILX Scratch
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
ASC-ILX\ Scratch.elf \

MAP_FILES += \
ASC-ILX\ Scratch.map \

SIZE_OUTPUT += \
application.size.stdout \

OBJDUMP_LIST += \
ASC-ILX\ Scratch.list \

OBJCOPY_HEX += \
ASC-ILX\ Scratch.hex \


# All Target
all: main-build

# Main-build Target
main-build: ASC-ILX\ Scratch.elf secondary-outputs

# Tool invocations
ASC-ILX\ Scratch.elf ASC-ILX\ Scratch.map: $(OBJS) $(USER_OBJS) C:\Users\14407\OneDrive\WRC\ILX-\ ProSoft\India\ SW\ Dev\Working\ FW\ Project\ILX34MBS_DIR\STM32F072CBTX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "ASC-ILX Scratch.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m0 -T"C:\Users\14407\OneDrive\WRC\ILX- ProSoft\India SW Dev\Working FW Project\ILX34MBS_DIR\STM32F072CBTX_FLASH.ld" --specs=nosys.specs -Wl,-Map="ASC-ILX Scratch.map" -Wl,--gc-sections -static --specs=nano.specs -mfloat-abi=soft -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

application.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

ASC-ILX\ Scratch.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "ASC-ILX Scratch.list"
	@echo 'Finished building: $@'
	@echo ' '

ASC-ILX\ Scratch.hex: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O ihex $(EXECUTABLES) "ASC-ILX Scratch.hex"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) ASC-ILX\ Scratch.elf ASC-ILX\ Scratch.hex ASC-ILX\ Scratch.list ASC-ILX\ Scratch.map ASC-ILX\\ Scratch.elf application.size.stdout
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_HEX)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
