#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>

#include <msp430.h>
#include "driverlib.h"
#include "lenet/lenet.h"
#include "sprinter/sprinter.h"
#include "uart/uart.h"
#include "ssim/ssim.h"

 // Temperature Sensor Calibration-30 C 2.0V ref
#define CAL_ADC_12T30_L  *(int8_t *)(0x1A1E)
#define CAL_ADC_12T30_H  *(int8_t *)(0x1A1F)
// Temperature Sensor Calibration-85 C 2.0V ref
#define CAL_ADC_12T85_L  *(int8_t *)(0x1A20)
#define CAL_ADC_12T85_H  *(int8_t *)(0x1A21)

//Enable/Start sampling and conversion
int test = 0;
int global_temp = 0;

//for UART
char gStr[30];
int testNum = 0;
//for lenet
short int i,percent,j,predict;
uint8 left;

image *test_data={0};
uint8 *test_label={0};

int testImg[ROW][COL];

/*
 * UART Communication Initialization
 * CLK set to LFXT to avoid disruption from other clock behaviors
 */
void Init_UART(){
    // Configure UART
    EUSCI_A_UART_initParam param = {0};
    param.selectClockSource = EUSCI_A_UART_CLOCKSOURCE_ACLK;
    param.clockPrescalar = 3; //52//3
    param.firstModReg = 0; //1
    param.secondModReg = 3; //0x49//0x92
    param.parity = EUSCI_A_UART_NO_PARITY;
    param.msborLsbFirst = EUSCI_A_UART_LSB_FIRST;
    param.numberofStopBits = EUSCI_A_UART_ONE_STOP_BIT;
    param.uartMode = EUSCI_A_UART_MODE;
    param.overSampling = EUSCI_A_UART_LOW_FREQUENCY_BAUDRATE_GENERATION;

    if(STATUS_FAIL == EUSCI_A_UART_init(EUSCI_A0_BASE, &param))
        return;

    EUSCI_A_UART_enable(EUSCI_A0_BASE);

    EUSCI_A_UART_clearInterrupt(EUSCI_A0_BASE,
                                EUSCI_A_UART_RECEIVE_INTERRUPT);

    // Enable USCI_A0 RX interrupt
    EUSCI_A_UART_enableInterrupt(EUSCI_A0_BASE,
                                 EUSCI_A_UART_RECEIVE_INTERRUPT); // Enable interrupt

    // Enable global interrupt
    __enable_interrupt();
}

void senseTemp(taskOutput* output){
    while(REFCTL0 & REFGENBUSY);            //wait until busy flag is cleared
    REFCTL0 |= REFVSEL_2 | REFON;           //enable internal voltage reference for temp sensor

    //ref30 = *((unsigned int*)(TLV_START + TLV_ADC12CAL + 0x09));
    //ref85 = *((unsigned int*)(TLV_START + TLV_ADC12CAL + 0x0B));

    ADC12CTL0 = ADC12SHT0_6 | ADC12ON;
    ADC12CTL1 = ADC12SHP;
    ADC12CTL2 = ADC12RES_2;
    ADC12CTL3 = ADC12TCMAP;
    ADC12MCTL0 = ADC12VRSEL_1 | ADC12INCH_30;

    ADC12CTL0 |= ADC12ENC | ADC12SC;
    while((ADC12IFGR0 & BIT0) == 0);

    unsigned int adc_raw = ADC12MEM0;
    //int tempCel = (55 * ((float)adc_raw - ref30))/(ref85 - ref30) + 30;

    output->temp = adc_raw;

    return;
}

void writeTemp(taskOutput** outputs, int batchSize){
    
    unsigned int ref30 = *((unsigned int*)(TLV_START + TLV_ADC12CAL + 0x09));
    unsigned int ref85 = *((unsigned int*)(TLV_START + TLV_ADC12CAL + 0x0B));
    uint8_t * tempBuffer = (uint8_t *)malloc(batchSize * sizeof(uint8_t *));
    for(int i = 0; i < batchSize; i++){
        tempBuffer[i] = (uint8_t)(55 * ((float)outputs[i]->temp - ref30))/(ref85 - ref30) + 30;
    }

    rf_init_lora(1000);
    sx1276_send(tempBuffer, batchSize);
}

/*
 * Transmit time measurement through UART
 */
void sendTicks(int temp ){
	//__delay_cycles(8000000);
	// Select UART TXD on P2.0
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN0, GPIO_SECONDARY_MODULE_FUNCTION);

    // Send Temp Sensor Data
    int lower = temp % 10;
    int higher = (temp - lower) / 10;

    EUSCI_A_UART_transmitData(EUSCI_A0_BASE, higher + '0');
    EUSCI_A_UART_transmitData(EUSCI_A0_BASE, lower + '0');
    EUSCI_A_UART_transmitData(EUSCI_A0_BASE, ' ');


    while(EUSCI_A_UART_queryStatusFlags(EUSCI_A0_BASE, EUSCI_A_UART_BUSY));

    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
	GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0);

}

int testFirstImage()
{

    left = 7;
    predict = Predict(test_data[0], 10);

    if(left == predict)
    {
        myUart_writeBuf( BACKCHANNEL_UART, "IS 7", NULL, CRLF );
        return 10+predict;
    } else {
        myUart_writeBuf( BACKCHANNEL_UART, "NOT 7", NULL, CRLF );
        return predict;
    }

}

void testUART(int val){
    sprintf(gStr, "PREDICTION: %d", val);
    myUart_writeBuf( BACKCHANNEL_UART, (unsigned char *)gStr, NULL, CRLF );

    myUart_writeBuf( BACKCHANNEL_UART, "--", NULL, CRLF );
    return;
}

void main(void) {
    volatile uint32_t i;

    // Stop Watchdog timer
    WDT_A_hold(WDT_A_BASE);

    sprintSpec spec = {
        M_6,
        VLOW,
        0, //clk range
        1,  //batch frequency
        5, //batch size
        32,  //iterations
        &senseTemp,
        &writeTemp
    };

    sInit(&spec);

    // Initialize UART (for backchannel communications)
    if ( myUart_init( BACKCHANNEL_UART, 9600, &myUart_Param_9600_8N1_SMCLK8MHz ) >> 0 ){
        GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN1);
    }

    putTestImg(testImg);

    while(1) {
        //wait(3);
        /*
        //GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
        int lum = getLuminenceScore(testImg);

        sprintf(gStr, "Lum: %d", lum);
        myUart_writeBuf( BACKCHANNEL_UART, (unsigned char *)gStr, NULL, CRLF );

        int contrast = getContrastScore(testImg, lum);
        sprintf(gStr, "contrast: %d", contrast);
        myUart_writeBuf( BACKCHANNEL_UART, (unsigned char *)gStr, NULL, CRLF );

        int ssim = SSIM(testImg, testImg);
        sprintf(gStr, "SSIM: %d", ssim);
        myUart_writeBuf( BACKCHANNEL_UART, (unsigned char *)gStr, NULL, CRLF );
        //sStart(&spec);
        //GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
        wait(2);
        
        */
       _delay_cycles(5000000);
       testFirstImage();
    }
}

