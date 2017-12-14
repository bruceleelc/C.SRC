################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../PlateDataRecvier.cpp \
../RecogizerListener.cpp \
../RecogizerSocket.cpp \
../VehicleInfoParser.cpp \
../stdafx.cpp 

OBJS += \
./PlateDataRecvier.o \
./RecogizerListener.o \
./RecogizerSocket.o \
./VehicleInfoParser.o \
./stdafx.o 

CPP_DEPS += \
./PlateDataRecvier.d \
./RecogizerListener.d \
./RecogizerSocket.d \
./VehicleInfoParser.d \
./stdafx.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DARM_LINUX -I"/media/projects/DataCollector/AssistTool" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


