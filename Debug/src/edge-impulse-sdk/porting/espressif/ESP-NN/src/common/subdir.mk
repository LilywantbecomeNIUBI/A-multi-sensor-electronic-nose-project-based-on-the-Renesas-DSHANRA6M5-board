################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_UPPER_SRCS += \
../src/edge-impulse-sdk/porting/espressif/ESP-NN/src/common/esp_nn_common_functions_esp32s3.S \
../src/edge-impulse-sdk/porting/espressif/ESP-NN/src/common/esp_nn_multiply_by_quantized_mult_esp32s3.S \
../src/edge-impulse-sdk/porting/espressif/ESP-NN/src/common/esp_nn_multiply_by_quantized_mult_ver1_esp32s3.S 

CREF += \
demoElectronicNoseAllSensor5.3.cref 

OBJS += \
./src/edge-impulse-sdk/porting/espressif/ESP-NN/src/common/esp_nn_common_functions_esp32s3.o \
./src/edge-impulse-sdk/porting/espressif/ESP-NN/src/common/esp_nn_multiply_by_quantized_mult_esp32s3.o \
./src/edge-impulse-sdk/porting/espressif/ESP-NN/src/common/esp_nn_multiply_by_quantized_mult_ver1_esp32s3.o 

MAP += \
demoElectronicNoseAllSensor5.3.map 

S_UPPER_DEPS += \
./src/edge-impulse-sdk/porting/espressif/ESP-NN/src/common/esp_nn_common_functions_esp32s3.d \
./src/edge-impulse-sdk/porting/espressif/ESP-NN/src/common/esp_nn_multiply_by_quantized_mult_esp32s3.d \
./src/edge-impulse-sdk/porting/espressif/ESP-NN/src/common/esp_nn_multiply_by_quantized_mult_ver1_esp32s3.d 


# Each subdirectory must supply rules for building sources it contributes
src/edge-impulse-sdk/porting/espressif/ESP-NN/src/common/%.o: ../src/edge-impulse-sdk/porting/espressif/ESP-NN/src/common/%.S
	@echo 'Building file: $<'
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mlittle-endian -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Os -ffunction-sections -fdata-sections -fno-strict-aliasing -fmessage-length=0 -funsigned-char -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Waggregate-return -Wno-parentheses-equality -Wfloat-equal -g3 -x assembler-with-cpp -D_RENESAS_RA_ -D_RA_CORE=CM33 -D_RA_ORDINAL=1 -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra_gen" -I"." -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra_cfg\\fsp_cfg\\bsp" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra_cfg\\fsp_cfg" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\src" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra\\fsp\\inc" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra\\fsp\\inc\\api" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra\\fsp\\inc\\instances" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra\\arm\\CMSIS_6\\CMSIS\\Core\\Include" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c "$<" -o "$@")
	@clang  --target=arm-none-eabi @"$@.in"

