################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/database.c \
../src/endpoints_handler.c \
../src/main.c \
../src/server.c \
../src/utils.c 

OBJS += \
./src/database.o \
./src/endpoints_handler.o \
./src/main.o \
./src/server.o \
./src/utils.o 

C_DEPS += \
./src/database.d \
./src/endpoints_handler.d \
./src/main.d \
./src/server.d \
./src/utils.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/dfranca/eclipse-workspace/task/inc" -O0 -g3 -Wall -Wextra -Werror -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


