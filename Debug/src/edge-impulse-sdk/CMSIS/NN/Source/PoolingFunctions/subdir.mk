################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/edge-impulse-sdk/CMSIS/NN/Source/PoolingFunctions/arm_avgpool_s16.c \
../src/edge-impulse-sdk/CMSIS/NN/Source/PoolingFunctions/arm_avgpool_s8.c \
../src/edge-impulse-sdk/CMSIS/NN/Source/PoolingFunctions/arm_max_pool_s16.c \
../src/edge-impulse-sdk/CMSIS/NN/Source/PoolingFunctions/arm_max_pool_s8.c \
../src/edge-impulse-sdk/CMSIS/NN/Source/PoolingFunctions/arm_pool_q7_HWC.c 

CREF += \
demoElectronicNoseAllSensor5.3.cref 

C_DEPS += \
./src/edge-impulse-sdk/CMSIS/NN/Source/PoolingFunctions/arm_avgpool_s16.d \
./src/edge-impulse-sdk/CMSIS/NN/Source/PoolingFunctions/arm_avgpool_s8.d \
./src/edge-impulse-sdk/CMSIS/NN/Source/PoolingFunctions/arm_max_pool_s16.d \
./src/edge-impulse-sdk/CMSIS/NN/Source/PoolingFunctions/arm_max_pool_s8.d \
./src/edge-impulse-sdk/CMSIS/NN/Source/PoolingFunctions/arm_pool_q7_HWC.d 

OBJS += \
./src/edge-impulse-sdk/CMSIS/NN/Source/PoolingFunctions/arm_avgpool_s16.o \
./src/edge-impulse-sdk/CMSIS/NN/Source/PoolingFunctions/arm_avgpool_s8.o \
./src/edge-impulse-sdk/CMSIS/NN/Source/PoolingFunctions/arm_max_pool_s16.o \
./src/edge-impulse-sdk/CMSIS/NN/Source/PoolingFunctions/arm_max_pool_s8.o \
./src/edge-impulse-sdk/CMSIS/NN/Source/PoolingFunctions/arm_pool_q7_HWC.o 

MAP += \
demoElectronicNoseAllSensor5.3.map 


# Each subdirectory must supply rules for building sources it contributes
src/edge-impulse-sdk/CMSIS/NN/Source/PoolingFunctions/%.o: ../src/edge-impulse-sdk/CMSIS/NN/Source/PoolingFunctions/%.c
	@echo 'Building file: $<'
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mlittle-endian -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Os -ffunction-sections -fdata-sections -fno-strict-aliasing -fmessage-length=0 -funsigned-char -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Waggregate-return -Wno-parentheses-equality -Wfloat-equal -g3 -std=c99 -fshort-enums -fno-unroll-loops -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra_gen" -I"." -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra_cfg\\fsp_cfg\\bsp" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra_cfg\\fsp_cfg" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\src" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra\\fsp\\inc" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra\\fsp\\inc\\api" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra\\fsp\\inc\\instances" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra\\arm\\CMSIS_6\\CMSIS\\Core\\Include" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\src\\edge-impulse-sdk" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\src\\tflite-model" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\src\\model-parameters" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\lvgl" -D_RENESAS_RA_ -DARM_MATH_CM33 -DEI_CLASSIFIER_TFLITE_ENABLE_CMSIS_NN=1 -DDISABLEFLOAT16 -D_RA_CORE=CM33 -D_RA_ORDINAL=1 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -x c "$<" -c -o "$@")
	@clang --target=arm-none-eabi @"$@.in"

