################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/startup.c \
../ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/system.c 

CREF += \
demoElectronicNoseAllSensor4.1.cref 

C_DEPS += \
./ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/startup.d \
./ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/system.d 

OBJS += \
./ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/startup.o \
./ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/system.o 

MAP += \
demoElectronicNoseAllSensor4.1.map 


# Each subdirectory must supply rules for building sources it contributes
ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/%.o: ../ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/%.c
	@echo 'Building file: $<'
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mlittle-endian -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Os -ffunction-sections -fdata-sections -fno-strict-aliasing -fmessage-length=0 -funsigned-char -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Waggregate-return -Wno-parentheses-equality -Wfloat-equal -g3 -std=c99 -fshort-enums -fno-unroll-loops -I"E:\\MCU_project\\demoElectronicNoseAllSensor4.1\\ra_gen" -I"." -I"E:\\MCU_project\\demoElectronicNoseAllSensor4.1\\ra_cfg\\fsp_cfg\\bsp" -I"E:\\MCU_project\\demoElectronicNoseAllSensor4.1\\ra_cfg\\fsp_cfg" -I"E:\\MCU_project\\demoElectronicNoseAllSensor4.1\\src" -I"E:\\MCU_project\\demoElectronicNoseAllSensor4.1\\ra\\fsp\\inc" -I"E:\\MCU_project\\demoElectronicNoseAllSensor4.1\\ra\\fsp\\inc\\api" -I"E:\\MCU_project\\demoElectronicNoseAllSensor4.1\\ra\\fsp\\inc\\instances" -I"E:\\MCU_project\\demoElectronicNoseAllSensor4.1\\ra\\arm\\CMSIS_6\\CMSIS\\Core\\Include" -D_RENESAS_RA_ -D_RA_CORE=CM33 -D_RA_ORDINAL=1 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -x c "$<" -c -o "$@")
	@clang --target=arm-none-eabi @"$@.in"

