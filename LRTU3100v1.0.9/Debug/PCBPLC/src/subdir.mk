################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../PCBPLC/src/pcbplc.c \
../PCBPLC/src/pcbplcConfig.c \
../PCBPLC/src/pcbplcInterface.c \
../PCBPLC/src/pcbplcService.c \
../PCBPLC/src/pcbplcTask.c \
../PCBPLC/src/pcbplcTimerTask.c \
../PCBPLC/src/pcbplccomm.c 

OBJS += \
./PCBPLC/src/pcbplc.o \
./PCBPLC/src/pcbplcConfig.o \
./PCBPLC/src/pcbplcInterface.o \
./PCBPLC/src/pcbplcService.o \
./PCBPLC/src/pcbplcTask.o \
./PCBPLC/src/pcbplcTimerTask.o \
./PCBPLC/src/pcbplccomm.o 

C_DEPS += \
./PCBPLC/src/pcbplc.d \
./PCBPLC/src/pcbplcConfig.d \
./PCBPLC/src/pcbplcInterface.d \
./PCBPLC/src/pcbplcService.d \
./PCBPLC/src/pcbplcTask.d \
./PCBPLC/src/pcbplcTimerTask.d \
./PCBPLC/src/pcbplccomm.d 


# Each subdirectory must supply rules for building sources it contributes
PCBPLC/src/%.o PCBPLC/src/%.su PCBPLC/src/%.cyclo: ../PCBPLC/src/%.c PCBPLC/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H753xx -c -I../Core/Inc -I"D:/VishalK/PoC/LRTU3100v1.0.9/PCBPLC/src" -I"D:/VishalK/PoC/LRTU3100v1.0.9/PCBPLC/include" -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../LWIP/App -I../LWIP/Target -I../Middlewares/Third_Party/LwIP/src/include -I../Middlewares/Third_Party/LwIP/src/include/compat/posix -I../Middlewares/Third_Party/LwIP/src/include/compat/stdc -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/arpa -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/net -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/sys -I../Middlewares/Third_Party/LwIP/src/include/lwip -I../Middlewares/Third_Party/LwIP/src/include/lwip/apps -I../Middlewares/Third_Party/LwIP/src/include/lwip/priv -I../Middlewares/Third_Party/LwIP/src/include/netif -I../Middlewares/Third_Party/LwIP/src/include/netif/ppp -I../Middlewares/Third_Party/LwIP/system -I../Middlewares/Third_Party/LwIP/system/arch -I../Drivers/BSP/Components/lan8742 -I../Middlewares/Third_Party/LwIP/src/include/lwip/prot -I../PCBPLC/include -Og -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-PCBPLC-2f-src

clean-PCBPLC-2f-src:
	-$(RM) ./PCBPLC/src/pcbplc.cyclo ./PCBPLC/src/pcbplc.d ./PCBPLC/src/pcbplc.o ./PCBPLC/src/pcbplc.su ./PCBPLC/src/pcbplcConfig.cyclo ./PCBPLC/src/pcbplcConfig.d ./PCBPLC/src/pcbplcConfig.o ./PCBPLC/src/pcbplcConfig.su ./PCBPLC/src/pcbplcInterface.cyclo ./PCBPLC/src/pcbplcInterface.d ./PCBPLC/src/pcbplcInterface.o ./PCBPLC/src/pcbplcInterface.su ./PCBPLC/src/pcbplcService.cyclo ./PCBPLC/src/pcbplcService.d ./PCBPLC/src/pcbplcService.o ./PCBPLC/src/pcbplcService.su ./PCBPLC/src/pcbplcTask.cyclo ./PCBPLC/src/pcbplcTask.d ./PCBPLC/src/pcbplcTask.o ./PCBPLC/src/pcbplcTask.su ./PCBPLC/src/pcbplcTimerTask.cyclo ./PCBPLC/src/pcbplcTimerTask.d ./PCBPLC/src/pcbplcTimerTask.o ./PCBPLC/src/pcbplcTimerTask.su ./PCBPLC/src/pcbplccomm.cyclo ./PCBPLC/src/pcbplccomm.d ./PCBPLC/src/pcbplccomm.o ./PCBPLC/src/pcbplccomm.su

.PHONY: clean-PCBPLC-2f-src

