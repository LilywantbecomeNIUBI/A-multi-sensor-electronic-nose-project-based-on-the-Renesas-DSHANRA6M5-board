################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/edge-impulse-sdk/tensorflow/lite/core/api/common.cc \
../src/edge-impulse-sdk/tensorflow/lite/core/api/error_reporter.cc \
../src/edge-impulse-sdk/tensorflow/lite/core/api/flatbuffer_conversions.cc \
../src/edge-impulse-sdk/tensorflow/lite/core/api/op_resolver.cc \
../src/edge-impulse-sdk/tensorflow/lite/core/api/tensor_utils.cc 

CREF += \
demoElectronicNoseAllSensor5.3.cref 

CC_DEPS += \
./src/edge-impulse-sdk/tensorflow/lite/core/api/common.d \
./src/edge-impulse-sdk/tensorflow/lite/core/api/error_reporter.d \
./src/edge-impulse-sdk/tensorflow/lite/core/api/flatbuffer_conversions.d \
./src/edge-impulse-sdk/tensorflow/lite/core/api/op_resolver.d \
./src/edge-impulse-sdk/tensorflow/lite/core/api/tensor_utils.d 

OBJS += \
./src/edge-impulse-sdk/tensorflow/lite/core/api/common.o \
./src/edge-impulse-sdk/tensorflow/lite/core/api/error_reporter.o \
./src/edge-impulse-sdk/tensorflow/lite/core/api/flatbuffer_conversions.o \
./src/edge-impulse-sdk/tensorflow/lite/core/api/op_resolver.o \
./src/edge-impulse-sdk/tensorflow/lite/core/api/tensor_utils.o 

MAP += \
demoElectronicNoseAllSensor5.3.map 


# Each subdirectory must supply rules for building sources it contributes
src/edge-impulse-sdk/tensorflow/lite/core/api/%.o: ../src/edge-impulse-sdk/tensorflow/lite/core/api/%.cc
	@echo 'Building file: $<'
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mlittle-endian -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Os -ffunction-sections -fdata-sections -fno-strict-aliasing -fmessage-length=0 -funsigned-char -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Waggregate-return -Wno-parentheses-equality -Wfloat-equal -g3 -std=c++11 -fshort-enums -fno-unroll-loops -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\lvgl" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra_gen" -I"." -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra_cfg\\fsp_cfg\\bsp" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra_cfg\\fsp_cfg" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\src" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra\\fsp\\inc" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra\\fsp\\inc\\api" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra\\fsp\\inc\\instances" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra\\arm\\CMSIS_6\\CMSIS\\Core\\Include" -D_RA_CORE=CM33 -DEI_CLASSIFIER_TFLITE_ENABLE_CMSIS_NN=1 -DARM_MATH_CM33 -DDISABLEFLOAT16 -D_RA_ORDINAL=1 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -x c++ "$<" -c -o "$@")
	@clang++ --target=arm-none-eabi @"$@.in"

