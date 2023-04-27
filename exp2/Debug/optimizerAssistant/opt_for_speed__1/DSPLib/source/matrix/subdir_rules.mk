################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
DSPLib/source/matrix/%.obj: ../DSPLib/source/matrix/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccs1220/ccs/tools/compiler/ti-cgt-msp430_21.6.1.LTS/bin/cl430" -vmspx --data_model=restricted --opt_for_speed=1 --use_hw_mpy=F5 --include_path="C:/ti/ccs1220/ccs/ccs_base/msp430/include" --include_path="C:/Users/kurtg/workspace2/exp2/driverlib/MSP430FR5xx_6xx" --include_path="C:/Users/kurtg/workspace2/exp2/DSPLib/include" --include_path="C:/Users/kurtg/workspace2/exp2" --include_path="C:/ti/ccs1220/ccs/tools/compiler/ti-cgt-msp430_21.6.1.LTS/include" --advice:power="all" --advice:hw_config=all --define=__MSP430FR5994__ --define=_MPU_ENABLE -g --c99 --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="DSPLib/source/matrix/$(basename $(<F)).d_raw" --obj_directory="DSPLib/source/matrix" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


