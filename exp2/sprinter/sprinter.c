
#include <msp430.h>
#include <stdlib.h>
#include "driverlib.h"
#include "SDCardLib/sdcard.h"
#include "sprinter.h"

void sInit(sprintSpec* spec){
    initClock(spec->clk_active, spec->clk_range);
    initGPIO();
    //rf_init_lora(1000);
    waiting = 0;
    seconds = 0;
    //Enable/Start sampling and conversion
    timerCounter = 0;

    //__bis_SR_register(LPM0_bits | GIE);     // Enter LPM0 w/ interrupt
    __enable_interrupt();
    return;
}

void sStart(sprintSpec* spec){

    outputs = (taskOutput**)malloc(spec->batchSize * sizeof(taskOutput*));
    for(int i = 0; i < spec->iter; i++){
        sCollect(spec);

        //delay for freq seconds before moving on to processing
        GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
        wait(spec->batchFreq);

        sExecute(spec);
        GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
    }
    __bis_SR_register(LPM4_bits | GIE);     // Enter LPM0 w/ interrupt
    return;
}

void sCollect(sprintSpec* spec){
    //tune clock here
    
    for(int i = 0; i < spec->batchSize; i++){

        output = (taskOutput*)malloc(sizeof(taskOutput));
        spec -> collect(output);
        outputs[i] = output;

        //delay batchFreq seconds
        wait(spec->batchFreq);
    }
}

void sExecute(sprintSpec* spec){
    //tune clock here
    spec->execute(outputs, spec->batchSize);
}

//timer that triggers an interrupt every second
void initTimer(int clk){
    timerMultiplier = clk;
    TA0CCTL0 = CCIE;                        // TACCR0 interrupt enabled
    TA0CCR0 = 10000;
    TA0CTL = TASSEL__SMCLK | MC__UP; // SMCLK, up mode

}

void initClock(CLK clock, int range){
    
    uint16_t clk_base;
    uint16_t clk_range;

    switch (clock)
    {
    case M_0:
        clk_base = CS_DCOFSEL_0;
        timerMultiplier = 100;
        break;
    case M_1:
        clk_base = CS_DCOFSEL_1;
        timerMultiplier = 267;
        break;
    case M_2:
        clk_base = CS_DCOFSEL_2;
        timerMultiplier = 350;
        break;
    case M_3:
        clk_base = CS_DCOFSEL_3;
        timerMultiplier = 400;
        break;
    case M_4:
        clk_base = CS_DCOFSEL_4;
        timerMultiplier = 533;
        break;
    case M_5:
        clk_base = CS_DCOFSEL_5;
        timerMultiplier = 700;
        break;
    case M_6:
        clk_base = CS_DCOFSEL_6;
        timerMultiplier = 800;
        break;
    default:
        break;
    }
    
    switch(range){
        case 0:
            clk_range = CS_DCORSEL_0;
            break;
        case 1:
            clk_range = CS_DCORSEL_1;
            break;
        default:
            clk_range = CS_DCORSEL_0;
            break;
    }
    
    // Set DCO frequency to 8 MHz
    CS_setDCOFreq(CS_DCORSEL_0, clk_base);
    //FRCTL0 = FRCTLPW | NWAITS_1; // above 16Mhz needs FRAM wait states
    //Set external clock frequency to 32.768 KHz
    CS_setExternalClockSource(32768, 0);
    //Set ACLK=LFXT
    CS_initClockSignal(CS_ACLK, CS_LFXTCLK_SELECT, CS_CLOCK_DIVIDER_1);
    // Set SMCLK = DCO with frequency divider of 1
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    // Set MCLK = DCO with frequency divider of 1
    CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    
        /*
        CSCTL0_H = CSKEY_H;                     // Unlock CS registers
        CSCTL1 = DCOFSEL_0;                     // Set DCO to 1 MHz
        CSCTL2 = SELM__DCOCLK | SELS__DCOCLK | SELA__VLOCLK;
        CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;   // Set all dividers to 1
        CSCTL4 = LFXTOFF | HFXTOFF;
        CSCTL0_H = 0;*/
    initTimer(timerMultiplier); //pass in clock speed in kHz
}

/*
 * GPIO Initialization
 */
 void initGPIO(){
    // Set all GPIO pins to output low for low power
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P7, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_PJ, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7|GPIO_PIN8|GPIO_PIN9|GPIO_PIN10|GPIO_PIN11|GPIO_PIN12|GPIO_PIN13|GPIO_PIN14|GPIO_PIN15);
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN0);

    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P6, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P7, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P8, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_PJ, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7|GPIO_PIN8|GPIO_PIN9|GPIO_PIN10|GPIO_PIN11|GPIO_PIN12|GPIO_PIN13|GPIO_PIN14|GPIO_PIN15);

	// Configure P2.0 - UCA0TXD and P2.1 - UCA0RXD
	GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
	GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0);
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P2, GPIO_PIN0 + GPIO_PIN1, GPIO_SECONDARY_MODULE_FUNCTION);
    //GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P2, GPIO_PIN1, GPIO_SECONDARY_MODULE_FUNCTION);

    // Set PJ.4 and PJ.5 as Primary Module Function Input, LFXT.
    GPIO_setAsPeripheralModuleFunctionInputPin(
           GPIO_PORT_PJ,
           GPIO_PIN4 + GPIO_PIN5,
           GPIO_PRIMARY_MODULE_FUNCTION
           );

    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    PMM_unlockLPM5();
}

void wait(int sec){
    waiting = sec;
    __bis_SR_register(LPM4_bits | GIE);     // Enter LPM0 w/ interrupt
    return;
}

// Timer0_A0 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer0_A0_ISR (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) Timer0_A0_ISR (void)
#else
#error Compiler not supported!
#endif
{
     //clear counter after 1 second
    if(waiting > 0){

        if(timerCounter >= timerMultiplier) {
            timerCounter = 0;
            seconds++;

            if(seconds == waiting){
                seconds = 0;
                waiting = 0;
                
                __low_power_mode_off_on_exit();
                __enable_interrupt();
                //GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
            }
        }
        timerCounter ++;
    }

    //TA0CCR0 += 10000;                       // Add Offset to TA0CCR0
}
