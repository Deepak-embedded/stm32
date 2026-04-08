################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/ADC.c \
../Core/Src/COM_PORT_RS232_1.c \
../Core/Src/Configuration.c \
../Core/Src/DIDO.c \
../Core/Src/DeltaOTA.c \
../Core/Src/EC200U.c \
../Core/Src/Lcd_16x2.c \
../Core/Src/MX25L_flash.c \
../Core/Src/Modem_MQTT.c \
../Core/Src/OTA.c \
../Core/Src/RAK_Lora.c \
../Core/Src/RTC_Time.c \
../Core/Src/RxRingProcess.c \
../Core/Src/cJSON.c \
../Core/Src/common.c \
../Core/Src/freertos.c \
../Core/Src/json_parser.c \
../Core/Src/lwrb.c \
../Core/Src/main.c \
../Core/Src/modbus.c \
../Core/Src/modem_GPS.c \
../Core/Src/stm32h7xx_hal_msp.c \
../Core/Src/stm32h7xx_hal_timebase_tim.c \
../Core/Src/stm32h7xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32h7xx.c \
../Core/Src/w25q_mem.c 

OBJS += \
./Core/Src/ADC.o \
./Core/Src/COM_PORT_RS232_1.o \
./Core/Src/Configuration.o \
./Core/Src/DIDO.o \
./Core/Src/DeltaOTA.o \
./Core/Src/EC200U.o \
./Core/Src/Lcd_16x2.o \
./Core/Src/MX25L_flash.o \
./Core/Src/Modem_MQTT.o \
./Core/Src/OTA.o \
./Core/Src/RAK_Lora.o \
./Core/Src/RTC_Time.o \
./Core/Src/RxRingProcess.o \
./Core/Src/cJSON.o \
./Core/Src/common.o \
./Core/Src/freertos.o \
./Core/Src/json_parser.o \
./Core/Src/lwrb.o \
./Core/Src/main.o \
./Core/Src/modbus.o \
./Core/Src/modem_GPS.o \
./Core/Src/stm32h7xx_hal_msp.o \
./Core/Src/stm32h7xx_hal_timebase_tim.o \
./Core/Src/stm32h7xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32h7xx.o \
./Core/Src/w25q_mem.o 

C_DEPS += \
./Core/Src/ADC.d \
./Core/Src/COM_PORT_RS232_1.d \
./Core/Src/Configuration.d \
./Core/Src/DIDO.d \
./Core/Src/DeltaOTA.d \
./Core/Src/EC200U.d \
./Core/Src/Lcd_16x2.d \
./Core/Src/MX25L_flash.d \
./Core/Src/Modem_MQTT.d \
./Core/Src/OTA.d \
./Core/Src/RAK_Lora.d \
./Core/Src/RTC_Time.d \
./Core/Src/RxRingProcess.d \
./Core/Src/cJSON.d \
./Core/Src/common.d \
./Core/Src/freertos.d \
./Core/Src/json_parser.d \
./Core/Src/lwrb.d \
./Core/Src/main.d \
./Core/Src/modbus.d \
./Core/Src/modem_GPS.d \
./Core/Src/stm32h7xx_hal_msp.d \
./Core/Src/stm32h7xx_hal_timebase_tim.d \
./Core/Src/stm32h7xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32h7xx.d \
./Core/Src/w25q_mem.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H753xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../PCBPLC/include -I"D:/VishalK/PoC/LRTU3100v1.0.9/Middlewares" -I"D:/VishalK/PoC/LRTU3100v1.0.9/Middlewares/Third_Party/FreeRTOS/Source/include" -I"D:/VishalK/PoC/LRTU3100v1.0.9/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F" -I"D:/VishalK/PoC/LRTU3100v1.0.9/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang" -Og -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/ADC.cyclo ./Core/Src/ADC.d ./Core/Src/ADC.o ./Core/Src/ADC.su ./Core/Src/COM_PORT_RS232_1.cyclo ./Core/Src/COM_PORT_RS232_1.d ./Core/Src/COM_PORT_RS232_1.o ./Core/Src/COM_PORT_RS232_1.su ./Core/Src/Configuration.cyclo ./Core/Src/Configuration.d ./Core/Src/Configuration.o ./Core/Src/Configuration.su ./Core/Src/DIDO.cyclo ./Core/Src/DIDO.d ./Core/Src/DIDO.o ./Core/Src/DIDO.su ./Core/Src/DeltaOTA.cyclo ./Core/Src/DeltaOTA.d ./Core/Src/DeltaOTA.o ./Core/Src/DeltaOTA.su ./Core/Src/EC200U.cyclo ./Core/Src/EC200U.d ./Core/Src/EC200U.o ./Core/Src/EC200U.su ./Core/Src/Lcd_16x2.cyclo ./Core/Src/Lcd_16x2.d ./Core/Src/Lcd_16x2.o ./Core/Src/Lcd_16x2.su ./Core/Src/MX25L_flash.cyclo ./Core/Src/MX25L_flash.d ./Core/Src/MX25L_flash.o ./Core/Src/MX25L_flash.su ./Core/Src/Modem_MQTT.cyclo ./Core/Src/Modem_MQTT.d ./Core/Src/Modem_MQTT.o ./Core/Src/Modem_MQTT.su ./Core/Src/OTA.cyclo ./Core/Src/OTA.d ./Core/Src/OTA.o ./Core/Src/OTA.su ./Core/Src/RAK_Lora.cyclo ./Core/Src/RAK_Lora.d ./Core/Src/RAK_Lora.o ./Core/Src/RAK_Lora.su ./Core/Src/RTC_Time.cyclo ./Core/Src/RTC_Time.d ./Core/Src/RTC_Time.o ./Core/Src/RTC_Time.su ./Core/Src/RxRingProcess.cyclo ./Core/Src/RxRingProcess.d ./Core/Src/RxRingProcess.o ./Core/Src/RxRingProcess.su ./Core/Src/cJSON.cyclo ./Core/Src/cJSON.d ./Core/Src/cJSON.o ./Core/Src/cJSON.su ./Core/Src/common.cyclo ./Core/Src/common.d ./Core/Src/common.o ./Core/Src/common.su ./Core/Src/freertos.cyclo ./Core/Src/freertos.d ./Core/Src/freertos.o ./Core/Src/freertos.su ./Core/Src/json_parser.cyclo ./Core/Src/json_parser.d ./Core/Src/json_parser.o ./Core/Src/json_parser.su ./Core/Src/lwrb.cyclo ./Core/Src/lwrb.d ./Core/Src/lwrb.o ./Core/Src/lwrb.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/modbus.cyclo ./Core/Src/modbus.d ./Core/Src/modbus.o ./Core/Src/modbus.su ./Core/Src/modem_GPS.cyclo ./Core/Src/modem_GPS.d ./Core/Src/modem_GPS.o ./Core/Src/modem_GPS.su ./Core/Src/stm32h7xx_hal_msp.cyclo ./Core/Src/stm32h7xx_hal_msp.d ./Core/Src/stm32h7xx_hal_msp.o ./Core/Src/stm32h7xx_hal_msp.su ./Core/Src/stm32h7xx_hal_timebase_tim.cyclo ./Core/Src/stm32h7xx_hal_timebase_tim.d ./Core/Src/stm32h7xx_hal_timebase_tim.o ./Core/Src/stm32h7xx_hal_timebase_tim.su ./Core/Src/stm32h7xx_it.cyclo ./Core/Src/stm32h7xx_it.d ./Core/Src/stm32h7xx_it.o ./Core/Src/stm32h7xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32h7xx.cyclo ./Core/Src/system_stm32h7xx.d ./Core/Src/system_stm32h7xx.o ./Core/Src/system_stm32h7xx.su ./Core/Src/w25q_mem.cyclo ./Core/Src/w25q_mem.d ./Core/Src/w25q_mem.o ./Core/Src/w25q_mem.su

.PHONY: clean-Core-2f-Src

