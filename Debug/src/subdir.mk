################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/array.c \
../src/helpers.c \
../src/list.c \
../src/restart.c \
../src/sockets.c \
../src/webserver.c 

OBJS += \
./src/array.o \
./src/helpers.o \
./src/list.o \
./src/restart.o \
./src/sockets.o \
./src/webserver.o 

C_DEPS += \
./src/array.d \
./src/helpers.d \
./src/list.d \
./src/restart.d \
./src/sockets.d \
./src/webserver.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


