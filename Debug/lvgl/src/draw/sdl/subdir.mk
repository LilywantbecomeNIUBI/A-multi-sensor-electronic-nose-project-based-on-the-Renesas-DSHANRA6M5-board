################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lvgl/src/draw/sdl/lv_draw_sdl.c \
../lvgl/src/draw/sdl/lv_draw_sdl_arc.c \
../lvgl/src/draw/sdl/lv_draw_sdl_bg.c \
../lvgl/src/draw/sdl/lv_draw_sdl_composite.c \
../lvgl/src/draw/sdl/lv_draw_sdl_img.c \
../lvgl/src/draw/sdl/lv_draw_sdl_label.c \
../lvgl/src/draw/sdl/lv_draw_sdl_layer.c \
../lvgl/src/draw/sdl/lv_draw_sdl_line.c \
../lvgl/src/draw/sdl/lv_draw_sdl_mask.c \
../lvgl/src/draw/sdl/lv_draw_sdl_polygon.c \
../lvgl/src/draw/sdl/lv_draw_sdl_rect.c \
../lvgl/src/draw/sdl/lv_draw_sdl_stack_blur.c \
../lvgl/src/draw/sdl/lv_draw_sdl_texture_cache.c \
../lvgl/src/draw/sdl/lv_draw_sdl_utils.c 

CREF += \
demoElectronicNoseAllSensor5.3.cref 

C_DEPS += \
./lvgl/src/draw/sdl/lv_draw_sdl.d \
./lvgl/src/draw/sdl/lv_draw_sdl_arc.d \
./lvgl/src/draw/sdl/lv_draw_sdl_bg.d \
./lvgl/src/draw/sdl/lv_draw_sdl_composite.d \
./lvgl/src/draw/sdl/lv_draw_sdl_img.d \
./lvgl/src/draw/sdl/lv_draw_sdl_label.d \
./lvgl/src/draw/sdl/lv_draw_sdl_layer.d \
./lvgl/src/draw/sdl/lv_draw_sdl_line.d \
./lvgl/src/draw/sdl/lv_draw_sdl_mask.d \
./lvgl/src/draw/sdl/lv_draw_sdl_polygon.d \
./lvgl/src/draw/sdl/lv_draw_sdl_rect.d \
./lvgl/src/draw/sdl/lv_draw_sdl_stack_blur.d \
./lvgl/src/draw/sdl/lv_draw_sdl_texture_cache.d \
./lvgl/src/draw/sdl/lv_draw_sdl_utils.d 

OBJS += \
./lvgl/src/draw/sdl/lv_draw_sdl.o \
./lvgl/src/draw/sdl/lv_draw_sdl_arc.o \
./lvgl/src/draw/sdl/lv_draw_sdl_bg.o \
./lvgl/src/draw/sdl/lv_draw_sdl_composite.o \
./lvgl/src/draw/sdl/lv_draw_sdl_img.o \
./lvgl/src/draw/sdl/lv_draw_sdl_label.o \
./lvgl/src/draw/sdl/lv_draw_sdl_layer.o \
./lvgl/src/draw/sdl/lv_draw_sdl_line.o \
./lvgl/src/draw/sdl/lv_draw_sdl_mask.o \
./lvgl/src/draw/sdl/lv_draw_sdl_polygon.o \
./lvgl/src/draw/sdl/lv_draw_sdl_rect.o \
./lvgl/src/draw/sdl/lv_draw_sdl_stack_blur.o \
./lvgl/src/draw/sdl/lv_draw_sdl_texture_cache.o \
./lvgl/src/draw/sdl/lv_draw_sdl_utils.o 

MAP += \
demoElectronicNoseAllSensor5.3.map 


# Each subdirectory must supply rules for building sources it contributes
lvgl/src/draw/sdl/%.o: ../lvgl/src/draw/sdl/%.c
	@echo 'Building file: $<'
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mlittle-endian -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Os -ffunction-sections -fdata-sections -fno-strict-aliasing -fmessage-length=0 -funsigned-char -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Waggregate-return -Wno-parentheses-equality -Wfloat-equal -g3 -std=c99 -fshort-enums -fno-unroll-loops -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra_gen" -I"." -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra_cfg\\fsp_cfg\\bsp" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra_cfg\\fsp_cfg" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\src" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra\\fsp\\inc" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra\\fsp\\inc\\api" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra\\fsp\\inc\\instances" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\ra\\arm\\CMSIS_6\\CMSIS\\Core\\Include" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\src\\edge-impulse-sdk" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\src\\tflite-model" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\src\\model-parameters" -I"E:\\MCU_project\\demoElectronicNoseAllSensor5.3\\lvgl" -D_RENESAS_RA_ -DARM_MATH_CM33 -DEI_CLASSIFIER_TFLITE_ENABLE_CMSIS_NN=1 -DDISABLEFLOAT16 -D_RA_CORE=CM33 -D_RA_ORDINAL=1 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -x c "$<" -c -o "$@")
	@clang --target=arm-none-eabi @"$@.in"

