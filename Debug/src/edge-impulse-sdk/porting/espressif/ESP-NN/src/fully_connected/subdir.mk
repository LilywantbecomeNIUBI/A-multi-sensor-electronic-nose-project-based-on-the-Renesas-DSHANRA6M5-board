################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_UPPER_SRCS += \
../src/edge-impulse-sdk/porting/espressif/ESP-NN/src/fully_connected/esp_nn_fully_connected_s8_esp32s3.S 

C_SRCS += \
../src/edge-impulse-sdk/porting/espressif/ESP-NN/src/fully_connected/esp_nn_fully_connected_ansi.c 

CREF += \
demoElectronicNoseAllSensor5.3.cref 

C_DEPS += \
./src/edge-impulse-sdk/porting/espressif/ESP-NN/src/fully_connected/esp_nn_fully_connected_ansi.d 

OBJS += \
./src/edge-impulse-sdk/porting/espressif/ESP-NN/src/fully_connected/esp_nn_fully_connected_ansi.o \
./src/edge-impulse-sdk/porting/espressif/ESP-NN/src/fully_connected/esp_nn_fully_connected_s8_esp32s3.o 

MAP += \
demoElectronicNoseAllSensor5.3.map 

S_UPPER_DEPS += \
./src/edge-impulse-sdk/porting/espressif/ESP-NN/src/fully_connected/esp_nn_fully_connected_s8_esp32s3.d 


# Each subdirectory must supply rules for building sources it contributes
src/edge-impulse-sdk/porting/espressif/ESP-NN/src/fully_connected/%.o: ../src/edge-impulse-sdk/porting/espressif/ESP-NN/src/fully_connected/%.c
	@echo 'Building file: $<'
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mlittle-endian -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Os -ffunction-sections -fdata-sections -fno-strict-aliasing -fmessage-length=0 -funsigned-char -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Waggregate-return -Wno-parentheses-equality -Wfloat-equal -g3 -std=c99 -fshort-enums -fno-unroll-loops -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra_gen" -I"." -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra_cfg\\fsp_cfg\\bsp" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra_cfg\\fsp_cfg" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\src" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra\\fsp\\inc" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra\\fsp\\inc\\api" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra\\fsp\\inc\\instances" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra\\arm\\CMSIS_6\\CMSIS\\Core\\Include" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\src\\edge-impulse-sdk" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\src\\tflite-model" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\src\\model-parameters" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\lvgl" -D_RENESAS_RA_ -DARM_MATH_CM33 -DEI_CLASSIFIER_TFLITE_ENABLE_CMSIS_NN=1 -DDISABLEFLOAT16 -D_RA_CORE=CM33 -D_RA_ORDINAL=1 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -x c "$<" -c -o "$@")
	@clang --target=arm-none-eabi @"$@.in"
src/edge-impulse-sdk/porting/espressif/ESP-NN/src/fully_connected/%.o: ../src/edge-impulse-sdk/porting/espressif/ESP-NN/src/fully_connected/%.S
	@echo 'Building file: $<'
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mlittle-endian -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Os -ffunction-sections -fdata-sections -fno-strict-aliasing -fmessage-length=0 -funsigned-char -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Waggregate-return -Wno-parentheses-equality -Wfloat-equal -g3 -x assembler-with-cpp -D_RENESAS_RA_ -D_RA_CORE=CM33 -D_RA_ORDINAL=1 -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra_gen" -I"." -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra_cfg\\fsp_cfg\\bsp" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra_cfg\\fsp_cfg" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\src" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra\\fsp\\inc" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra\\fsp\\inc\\api" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra\\fsp\\inc\\instances" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra\\arm\\CMSIS_6\\CMSIS\\Core\\Include" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c "$<" -o "$@")
	@clang  --target=arm-none-eabi @"$@.in"

