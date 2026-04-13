################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/edge-impulse-sdk/dsp/kissfft/kiss_fft.cpp \
../src/edge-impulse-sdk/dsp/kissfft/kiss_fftr.cpp 

CREF += \
demoElectronicNoseAllSensor5.3.cref 

OBJS += \
./src/edge-impulse-sdk/dsp/kissfft/kiss_fft.o \
./src/edge-impulse-sdk/dsp/kissfft/kiss_fftr.o 

MAP += \
demoElectronicNoseAllSensor5.3.map 

CPP_DEPS += \
./src/edge-impulse-sdk/dsp/kissfft/kiss_fft.d \
./src/edge-impulse-sdk/dsp/kissfft/kiss_fftr.d 


# Each subdirectory must supply rules for building sources it contributes
src/edge-impulse-sdk/dsp/kissfft/%.o: ../src/edge-impulse-sdk/dsp/kissfft/%.cpp
	@echo 'Building file: $<'
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mlittle-endian -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Os -ffunction-sections -fdata-sections -fno-strict-aliasing -fmessage-length=0 -funsigned-char -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Waggregate-return -Wno-parentheses-equality -Wfloat-equal -g3 -std=c++11 -fshort-enums -fno-unroll-loops -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\lvgl" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra_gen" -I"." -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra_cfg\\fsp_cfg\\bsp" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra_cfg\\fsp_cfg" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\src" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra\\fsp\\inc" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra\\fsp\\inc\\api" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra\\fsp\\inc\\instances" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra\\arm\\CMSIS_6\\CMSIS\\Core\\Include" -D_RA_CORE=CM33 -DEI_CLASSIFIER_TFLITE_ENABLE_CMSIS_NN=1 -DARM_MATH_CM33 -DDISABLEFLOAT16 -D_RA_ORDINAL=1 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -x c++ "$<" -c -o "$@")
	@clang++ --target=arm-none-eabi @"$@.in"

