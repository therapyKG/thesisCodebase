//***************************************************************************************
//  ACLK = n/a, MCLK = SMCLK = default DCO
//
//***************************************************************************************
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "driverlib/driverlib.h"
#include <msp430.h>

#define CLK_SPEED_8MHz      1
#define CLK_SPEED_16MHz     0

unsigned int adc_raw = 0;
unsigned int prev_raw = 0;
int tempCel = 0;
int prevCel = 0;

void UART_TX(char ch){
    UCA0IFG &= ~UCTXIFG;
    UCA0TXBUF = ch;

    while((UCA0IFG & UCTXIFG) == 0); //wait until register is set
}

void UART_TX_INT(int temp){
    int lower = temp % 10;
    int higher = (temp - lower) / 10;
    UART_TX(higher + '0');
    UART_TX(lower + '0');
    UART_TX(' ');
}

const int SIZE = 64;

void matMul(void){
    unsigned int i,j,k;

    int ** fst = (int **)malloc(SIZE * sizeof(int*));
    int ** snd = (int **)malloc(SIZE * sizeof(int*));
    int ** product = (int **)malloc(SIZE * sizeof(int*));

    for(i = SIZE; i > 0; i--){
        fst[i-1] = (int *)malloc(SIZE * sizeof(int));
        snd[i-1] = (int *)malloc(SIZE * sizeof(int));
        product[i-1] = (int *)malloc(SIZE * sizeof(int));
    }
    //init matrices

    for(i = SIZE; i > 0; i--){
        for (j = SIZE; j > 0; j--) {
            fst[i-1][j-1] = i+j;
            snd[i-1][j-1] = i*j;
        }
    }

    // Multiplying first and second matrices and storing it in result
    for(i = SIZE; i > 0; i--){
        for(j = SIZE; j > 0; j--){
            for(k = SIZE; k > 0; k--){
                product[i-1][j-1] += fst[i-1][k-1] * snd[k-1][j-1];
            }
        }
    }
}

void clock_1(){
    // Configure GPIO
        P1OUT = 0;
        P1DIR = 0xFF;

        P1OUT = 0;
        P1DIR = 0xFF;

        P2OUT = 0;
        P2DIR = 0xFF;

        P4OUT = 0;
        P4DIR = 0xFF;

        P5OUT = 0;
        P5DIR = 0xFF;

        P6OUT = 0;
        P6DIR = 0xFF;

        P7OUT = 0;
        P7DIR = 0xFF;

        P8DIR = 0xFF;
        P8OUT = 0;

        P9DIR = 0xFF;
        P9OUT = 0;

        PADIR = 0xFF;
        PAOUT = 0;

        PBDIR = 0xFF;
        PBOUT = 0;

        PCDIR = 0xFF;
        PCOUT = 0;

        PDDIR = 0xFF;
        PDOUT = 0;

        PEDIR = 0xFF;
        PEOUT = 0;

        PJOUT = 0;
        PJSEL0 = BIT4 | BIT5;                   // For XT1
        PJDIR = 0xFFFF;

    P3DIR |= BIT4;
    P3SEL1 |= BIT4;                         // Output SMCLK
    P3SEL0 |= BIT4;

    // Clock System Setup
    CSCTL0_H = CSKEY_H;                     // Unlock CS registers
    CSCTL1 = DCOFSEL_0;                     // Set DCO to 1MHz
    // Set SMCLK = MCLK = DCO, ACLK = VLOCLK
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;

    FRCTL0 = FRCTLPW | NWAITS_6; // above 16Mhz needs FRAM wait states

    // Per Device Errata set divider to 4 before changing frequency to
    // prevent out of spec operation from overshoot transient
    CSCTL3 = DIVA__4 | DIVS__4 | DIVM__4;   // Set all corresponding clk sources to divide by 4 for errata
    CSCTL1 = DCOFSEL_6;                     // Set DCO to 8MHz ? 24 MHz
    CSCTL1 |= DCORSEL;                     // Set DCO to 24 MHz

    // Delay by ~10us to let DCO settle. 60 cycles = 20 cycles buffer + (10us / (1/4MHz))
    __delay_cycles(180);
    //CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;   // Set all dividers to 1 for 8MHz operation
    CSCTL0_H = 0;                           // Lock CS Registers
}

void clock_setup(void){

#if CLK_SPEED_8MHz
    //Set DCO Frequency to 8MHz
    CS_setDCOFreq(CS_DCORSEL_0, CS_DCOFSEL_6);

    //configure MCLK, SMCLK to be source by DCOCLK
    CS_initClockSignal(CS_MCLK,  CS_DCOCLK_SELECT,  CS_CLOCK_DIVIDER_1); //16mhz
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT,  CS_CLOCK_DIVIDER_1); // 16mhz

#endif

#if CLK_SPEED_16MHz

    FRCTL0 = FRCTLPW | NWAITS_1; // above 16Mhz needs FRAM wait states
    CS_setDCOFreq(CS_DCORSEL_1, CS_DCOFSEL_4);
    CS_initClockSignal (CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    CS_initClockSignal (CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);

#endif

    __bis_SR_register(GIE);

    //Verify if the Clock settings are as expected
    volatile uint32_t clockValue;
    clockValue = CS_getMCLK();
    clockValue = CS_getACLK();
    clockValue = CS_getSMCLK();
    if(clockValue);

    _delay_cycles(1000);

}

void senseTemp(){
    while(REFCTL0 & REFGENBUSY);            //wait until busy flag is cleared
    REFCTL0 |= REFVSEL_2 | REFON;           //enable internal voltage reference for temp sensor

    int ref30 = *((unsigned int*)(TLV_START + TLV_ADC12CAL + 0x09));
    int ref85 = *((unsigned int*)(TLV_START + TLV_ADC12CAL + 0x0B));

    ADC12CTL0 = ADC12SHT0_6 | ADC12ON;
    ADC12CTL1 = ADC12SHP;
    ADC12CTL2 = ADC12RES_2;
    ADC12CTL3 = ADC12TCMAP;
    ADC12MCTL0 = ADC12VRSEL_1 | ADC12INCH_30;

    ADC12CTL0 |= ADC12ENC | ADC12SC;
    while((ADC12IFGR0 & BIT0) == 0);

    adc_raw = ADC12MEM0;
    prevCel = tempCel;

    tempCel = (55 * ((float)adc_raw - ref30))/(ref85 - ref30) + 30;
    //UART_TX_INT(tempCel);

}

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;                   // Disable the GPIO power-on default high-impedance mode
                                            // to activate previously configured port settings
    P3DIR |= 0x01;                          // Set P3.0 to output direction
    P3OUT = 0;
    //P1DIR |= 0x01;                          // Set P1.0 to output direction

    TA0CCTL0 = CCIE;                        // TACCR0 interrupt enabled
    TA0CCR0 = 50000;
    TA0CTL = TASSEL__SMCLK | MC__CONTINOUS; // SMCLK, continuous mode

    __bis_SR_register(LPM0_bits | GIE);     // Enter LPM0 w/ interrupt

    //clock_1();
    clock_setup();
    /*
    UCA0CTLW0 = UCSWRST;                    //reset states
    UCA0CTLW0 |= UCSSEL_2;                  //eUSCI clock source: SMCLK
    UCA0BRW = 6;
    UCA0MCTLW = UCOS16 | (8 << 4) | (32 << 8); //set baud rate to 9600 8N1
    P2SEL1 |= BIT0 | BIT1;
    P2SEL0 &= ~(BIT0 | BIT1);
    UCA0CTLW0 &= ~UCSWRST;*/
    __delay_cycles(2000);

}

int timerCounter = 0;
int txTimer = 0;
int txOn = 0;

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
    if(timerCounter == 100) {
        //senseTemp();
        P3OUT ^= BIT0;
        timerCounter = 0;
        txTimer = 0;
        txOn = 1;
    }
    //tune transmission time
    if(txOn && txTimer == 20){
        P3OUT ^= BIT0;
        txOn = 0;
        txTimer = 0;
    }

    TA0CCR0 += 50000;                       // Add Offset to TA0CCR0
    timerCounter ++;
    txTimer ++;
}
