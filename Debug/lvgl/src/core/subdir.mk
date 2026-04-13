################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lvgl/src/core/lv_disp.c \
../lvgl/src/core/lv_event.c \
../lvgl/src/core/lv_group.c \
../lvgl/src/core/lv_indev.c \
../lvgl/src/core/lv_indev_scroll.c \
../lvgl/src/core/lv_obj.c \
../lvgl/src/core/lv_obj_class.c \
../lvgl/src/core/lv_obj_draw.c \
../lvgl/src/core/lv_obj_pos.c \
../lvgl/src/core/lv_obj_scroll.c \
../lvgl/src/core/lv_obj_style.c \
../lvgl/src/core/lv_obj_style_gen.c \
../lvgl/src/core/lv_obj_tree.c \
../lvgl/src/core/lv_refr.c \
../lvgl/src/core/lv_theme.c 

CREF += \
demoElectronicNoseAllSensor5.3.cref 

C_DEPS += \
./lvgl/src/core/lv_disp.d \
./lvgl/src/core/lv_event.d \
./lvgl/src/core/lv_group.d \
./lvgl/src/core/lv_indev.d \
./lvgl/src/core/lv_indev_scroll.d \
./lvgl/src/core/lv_obj.d \
./lvgl/src/core/lv_obj_class.d \
./lvgl/src/core/lv_obj_draw.d \
./lvgl/src/core/lv_obj_pos.d \
./lvgl/src/core/lv_obj_scroll.d \
./lvgl/src/core/lv_obj_style.d \
./lvgl/src/core/lv_obj_style_gen.d \
./lvgl/src/core/lv_obj_tree.d \
./lvgl/src/core/lv_refr.d \
./lvgl/src/core/lv_theme.d 

OBJS += \
./lvgl/src/core/lv_disp.o \
./lvgl/src/core/lv_event.o \
./lvgl/src/core/lv_group.o \
./lvgl/src/core/lv_indev.o \
./lvgl/src/core/lv_indev_scroll.o \
./lvgl/src/core/lv_obj.o \
./lvgl/src/core/lv_obj_class.o \
./lvgl/src/core/lv_obj_draw.o \
./lvgl/src/core/lv_obj_pos.o \
./lvgl/src/core/lv_obj_scroll.o \
./lvgl/src/core/lv_obj_style.o \
./lvgl/src/core/lv_obj_style_gen.o \
./lvgl/src/core/lv_obj_tree.o \
./lvgl/src/core/lv_refr.o \
./lvgl/src/core/lv_theme.o 

MAP += \
demoElectronicNoseAllSensor5.3.map 


# Each subdirectory must supply rules for building sources it contributes
lvgl/src/core/%.o: ../lvgl/src/core/%.c
	@echo 'Building file: $<'
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mlittle-endian -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Os -ffunction-sections -fdata-sections -fno-strict-aliasing -fmessage-length=0 -funsigned-char -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Waggregate-return -Wno-parentheses-equality -Wfloat-equal -g3 -std=c99 -fshort-enums -fno-unroll-loops -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra_gen" -I"." -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra_cfg\\fsp_cfg\\bsp" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra_cfg\\fsp_cfg" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\src" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra\\fsp\\inc" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra\\fsp\\inc\\api" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra\\fsp\\inc\\instances" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra\\arm\\CMSIS_6\\CMSIS\\Core\\Include" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\src\\edge-impulse-sdk" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\src\\tflite-model" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\src\\model-parameters" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\lvgl" -D_RENESAS_RA_ -DARM_MATH_CM33 -DEI_CLASSIFIER_TFLITE_ENABLE_CMSIS_NN=1 -DDISABLEFLOAT16 -D_RA_CORE=CM33 -D_RA_ORDINAL=1 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -x c "$<" -c -o "$@")
	@clang --target=arm-none-eabi @"$@.in"

