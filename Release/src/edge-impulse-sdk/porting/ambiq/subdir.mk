################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/edge-impulse-sdk/porting/ambiq/debug_log.cpp \
../src/edge-impulse-sdk/porting/ambiq/ei_classifier_porting.cpp 

CREF += \
demoElectronicNoseAllSensor4.1.cref 

OBJS += \
./src/edge-impulse-sdk/porting/ambiq/debug_log.o \
./src/edge-impulse-sdk/porting/ambiq/ei_classifier_porting.o 

MAP += \
demoElectronicNoseAllSensor4.1.map 

CPP_DEPS += \
./src/edge-impulse-sdk/porting/ambiq/debug_log.d \
./src/edge-impulse-sdk/porting/ambiq/ei_classifier_porting.d 


# Each subdirectory must supply rules for building sources it contributes
src/edge-impulse-sdk/porting/ambiq/%.o: ../src/edge-impulse-sdk/porting/ambiq/%.cpp
	@echo 'Building file: $<'
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mlittle-endian -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Os -ffunction-sections -fdata-sections -fno-strict-aliasing -fmessage-length=0 -funsigned-char -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Waggregate-return -Wno-parentheses-equality -Wfloat-equal -g3 -std=c++11 -fshort-enums -fno-exceptions -fno-rtti -fno-use-cxa-atexit -fno-unroll-loops -I"E:\\MCU_project\\demoElectronicNoseAllSensor4.1\\ra_gen" -I"." -I"E:\\MCU_project\\demoElectronicNoseAllSensor4.1\\ra_cfg\\fsp_cfg\\bsp" -I"E:\\MCU_project\\demoElectronicNoseAllSensor4.1\\ra_cfg\\fsp_cfg" -I"E:\\MCU_project\\demoElectronicNoseAllSensor4.1\\src" -I"E:\\MCU_project\\demoElectronicNoseAllSensor4.1\\ra\\fsp\\inc" -I"E:\\MCU_project\\demoElectronicNoseAllSensor4.1\\ra\\fsp\\inc\\api" -I"E:\\MCU_project\\demoElectronicNoseAllSensor4.1\\ra\\fsp\\inc\\instances" -I"E:\\MCU_project\\demoElectronicNoseAllSensor4.1\\ra\\arm\\CMSIS_6\\CMSIS\\Core\\Include" -D_RA_CORE=CM33 -D_RA_ORDINAL=1 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -x c++ "$<" -c -o "$@")
	@clang++ --target=arm-none-eabi @"$@.in"

