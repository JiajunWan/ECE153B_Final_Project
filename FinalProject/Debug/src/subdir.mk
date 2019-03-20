################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/FinalProject.c \
../src/cr_startup_lpc407x_8x.c \
../src/crp.c \
../src/sysinit.c 

OBJS += \
./src/FinalProject.o \
./src/cr_startup_lpc407x_8x.o \
./src/crp.o \
./src/sysinit.o 

C_DEPS += \
./src/FinalProject.d \
./src/cr_startup_lpc407x_8x.d \
./src/crp.d \
./src/sysinit.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -D__MULTICORE_NONE -DDEBUG -D__CODE_RED -DCORE_M4 -D__USE_LPCOPEN -D__LPC407X_8X__ -I"/Users/alphajun/Documents/LPCXpresso_7.3.0/workspace/FinalProject/inc" -I"/Users/alphajun/Documents/LPCXpresso_7.3.0/workspace/lpc_board_ea_devkit_4088/inc" -I"/Users/alphajun/Documents/LPCXpresso_7.3.0/workspace/lpc_chip_40xx/inc" -Og -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fsingle-precision-constant -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


