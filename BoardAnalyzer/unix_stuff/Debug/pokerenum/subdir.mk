################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../pokerenum/combinations.c \
../pokerenum/deck.c \
../pokerenum/deck_std.c \
../pokerenum/enumerate.c \
../pokerenum/enumord.c \
../pokerenum/evx.c \
../pokerenum/rules_std.c \
../pokerenum/t_botcard.c \
../pokerenum/t_botfivecards.c \
../pokerenum/t_botfivecardsj.c \
../pokerenum/t_cardmasks.c \
../pokerenum/t_maskrank.c \
../pokerenum/t_nbits.c \
../pokerenum/t_straight.c \
../pokerenum/t_topbit.c \
../pokerenum/t_topcard.c \
../pokerenum/t_topfivebits.c \
../pokerenum/t_topfivecards.c \
../pokerenum/t_toptwobits.c 

C_DEPS += \
./pokerenum/combinations.d \
./pokerenum/deck.d \
./pokerenum/deck_std.d \
./pokerenum/enumerate.d \
./pokerenum/enumord.d \
./pokerenum/evx.d \
./pokerenum/rules_std.d \
./pokerenum/t_botcard.d \
./pokerenum/t_botfivecards.d \
./pokerenum/t_botfivecardsj.d \
./pokerenum/t_cardmasks.d \
./pokerenum/t_maskrank.d \
./pokerenum/t_nbits.d \
./pokerenum/t_straight.d \
./pokerenum/t_topbit.d \
./pokerenum/t_topcard.d \
./pokerenum/t_topfivebits.d \
./pokerenum/t_topfivecards.d \
./pokerenum/t_toptwobits.d 

OBJS += \
./pokerenum/combinations.o \
./pokerenum/deck.o \
./pokerenum/deck_std.o \
./pokerenum/enumerate.o \
./pokerenum/enumord.o \
./pokerenum/evx.o \
./pokerenum/rules_std.o \
./pokerenum/t_botcard.o \
./pokerenum/t_botfivecards.o \
./pokerenum/t_botfivecardsj.o \
./pokerenum/t_cardmasks.o \
./pokerenum/t_maskrank.o \
./pokerenum/t_nbits.o \
./pokerenum/t_straight.o \
./pokerenum/t_topbit.o \
./pokerenum/t_topcard.o \
./pokerenum/t_topfivebits.o \
./pokerenum/t_topfivecards.o \
./pokerenum/t_toptwobits.o 


# Each subdirectory must supply rules for building sources it contributes
pokerenum/%.o: ../pokerenum/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -Ipokerenum/include -I../pokerenum/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


