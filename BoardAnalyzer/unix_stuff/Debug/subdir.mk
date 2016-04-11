################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../BoardAnalyzer.cpp \
../CBoard.cpp \
../CBoardAnalyzer.cpp \
../CCard.cpp \
../GTO_demo_1.cpp \
../GTO_demo_2.cpp \
../UnitTest.cpp 

CPP_DEPS += \
./BoardAnalyzer.d \
./CBoard.d \
./CBoardAnalyzer.d \
./CCard.d \
./GTO_demo_1.d \
./GTO_demo_2.d \
./UnitTest.d 

OBJS += \
./BoardAnalyzer.o \
./CBoard.o \
./CBoardAnalyzer.o \
./CCard.o \
./GTO_demo_1.o \
./GTO_demo_2.o \
./UnitTest.o 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I../pokerenum/include -Ipokerenum/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


