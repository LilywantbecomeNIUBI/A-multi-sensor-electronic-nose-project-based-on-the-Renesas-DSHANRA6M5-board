################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/ei_infer.cpp \
../src/hal_entry.cpp 

C_SRCS += \
../src/ads1115.c \
../src/app_button.c \
../src/app_debug.c \
../src/app_infer.c \
../src/app_systick.c \
../src/hal_warmstart.c \
../src/i2c_master.c \
../src/lcd_port.c \
../src/mq_acquire.c \
../src/mq_judge.c \
../src/mq_metrics.c \
../src/mq_state_machine.c \
../src/sensirion_common.c \
../src/sensirion_i2c_ra6m5.c \
../src/sensirion_voc_algorithm.c \
../src/sensor_hub.c \
../src/sensor_uart.c \
../src/sgp40.c \
../src/sgp40_voc_simple.c \
../src/sgp_git_version.c \
../src/sht40.c \
../src/sht40_sgp40_service.c \
../src/spray_ctrl.c \
../src/st7789.c 

CREF += \
demoElectronicNoseAllSensor4.1.cref 

C_DEPS += \
./src/ads1115.d \
./src/app_button.d \
./src/app_debug.d \
./src/app_infer.d \
./src/app_systick.d \
./src/hal_warmstart.d \
./src/i2c_master.d \
./src/lcd_port.d \
./src/mq_acquire.d \
./src/mq_judge.d \
./src/mq_metrics.d \
./src/mq_state_machine.d \
./src/sensirion_common.d \
./src/sensirion_i2c_ra6m5.d \
./src/sensirion_voc_algorithm.d \
./src/sensor_hub.d \
./src/sensor_uart.d \
./src/sgp40.d \
./src/sgp40_voc_simple.d \
./src/sgp_git_version.d \
./src/sht40.d \
./src/sht40_sgp40_service.d \
./src/spray_ctrl.d \
./src/st7789.d 

OBJS += \
./src/ads1115.o \
./src/app_button.o \
./src/app_debug.o \
./src/app_infer.o \
./src/app_systick.o \
./src/ei_infer.o \
./src/hal_entry.o \
./src/hal_warmstart.o \
./src/i2c_master.o \
./src/lcd_port.o \
./src/mq_acquire.o \
./src/mq_judge.o \
./src/mq_metrics.o \
./src/mq_state_machine.o \
./src/sensirion_common.o \
./src/sensirion_i2c_ra6m5.o \
./src/sensirion_voc_algorithm.o \
./src/sensor_hub.o \
./src/sensor_uart.o \
./src/sgp40.o \
./src/sgp40_voc_simple.o \
./src/sgp_git_version.o \
./src/sht40.o \
./src/sht40_sgp40_service.o \
./src/spray_ctrl.o \
./src/st7789.o 

MAP += \
demoElectronicNoseAllSensor4.1.map 

CPP_DEPS += \
./src/ei_infer.d \
./src/hal_entry.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mlittle-endian -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Os -ffunction-sections -fdata-sections -fno-strict-aliasing -fmessage-length=0 -funsigned-char -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Waggregate-return -Wno-parentheses-equality -Wfloat-equal -g3 -std=c99 -fshort-enums -fno-unroll-loops -I"E:\\MCU_project\\demoElectronicNoseAllSensor4.1\\ra_gen" -I"." -I"E:\\MCU_project\\demoElectronicNoseAllSensor4.1\\ra_cfg\\fsp_cfg\\bsp" -I"E:\\MCU_project\\demoElectronicNoseAllSensor4.1\\ra_cfg\\fsp_cfg" -I"E:\\MCU_project\\demoElectronicNoseAllSensor4.1\\src" -I"E:\\MCU_project\\demoElectronicNoseAllSensor4.1\\ra\\fsp\\inc" -I"E:\\MCU_project\\demoElectronicNoseAllSensor4.1\\ra\\fsp\\inc\\api" -I"E:\\MCU_project\\demoElectronicNoseAllSensor4.1\\ra\\fsp\\inc\\instances" -I"E:\\MCU_project\\demoElectronicNoseAllSensor4.1\\ra\\arm\\CMSIS_6\\CMSIS\\Core\\Include" -D_RENESAS_RA_ -D_RA_CORE=CM33 -D_RA_ORDINAL=1 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -x c "$<" -c -o "$@")
	@clang --target=arm-none-eabi @"$@.in"
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mlittle-endian -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Os -ffunction-sections -fdata-sections -fno-strict-aliasing -fmessage-length=0 -funsigned-char -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Waggregate-return -Wno-parentheses-equality -Wfloat-equal -g3 -std=c++11 -fshort-enums -fno-exceptions -fno-rtti -fno-use-cxa-atexit -fno-unroll-loops -I"E:\\MCU_project\\demoElectronicNoseAllSensor4.1\\ra_gen" -I"." -I"E:\\MCU_project\\demoElectronicNoseAllSensor4.1\\ra_cfg\\fsp_cfg\\bsp" -I"E:\\MCU_project\\demoElectronicNoseAllSensor4.1\\ra_cfg\\fsp_cfg" -I"E:\\MCU_project\\demoElectronicNoseAllSensor4.1\\src" -I"E:\\MCU_project\\demoElectronicNoseAllSensor4.1\\ra\\fsp\\inc" -I"E:\\MCU_project\\demoElectronicNoseAllSensor4.1\\ra\\fsp\\inc\\api" -I"E:\\MCU_project\\demoElectronicNoseAllSensor4.1\\ra\\fsp\\inc\\instances" -I"E:\\MCU_project\\demoElectronicNoseAllSensor4.1\\ra\\arm\\CMSIS_6\\CMSIS\\Core\\Include" -D_RA_CORE=CM33 -D_RA_ORDINAL=1 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -x c++ "$<" -c -o "$@")
	@clang++ --target=arm-none-eabi @"$@.in"

