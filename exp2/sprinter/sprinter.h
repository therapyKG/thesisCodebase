//***************************************************************************************
//  ACLK = n/a, MCLK = SMCLK = default DCO
//
//***************************************************************************************

#include <msp430.h>
#include <stdlib.h>
#include "driverlib.h"
#include "SDCardLib/sdcard.h"
#include "radio/radio.h"

uint16_t seconds;
//Enable/Start sampling and conversion
int timerCounter;
int timerMultiplier;
int waiting;
/*
typedef enum CLK {
    VLOW,
    LOW,
    M_0 = CS_DCOFSEL_0,
    M_1 = CS_DCOFSEL_1,
    M_2 = CS_DCOFSEL_2,
    M_3 = CS_DCOFSEL_3,
    M_4 = CS_DCOFSEL_4,
    M_5 = CS_DCOFSEL_5,
    M_6 = CS_DCOFSEL_6
} CLK;
*/
typedef enum CLK {
    VLOW,
    LOW,
    M_0,
    M_1,
    M_2,
    M_3,
    M_4,
    M_5,
    M_6
} CLK;

typedef struct taskOutput {
    unsigned int temp; //output from temp sensing app
    int img[28][28]; //output from camera/SD Card
} taskOutput;

typedef struct sprintSpec {
    CLK clk_active; //active clock frequency
    CLK clk_wait; //low power mode clock frequency
    int clk_range; //clock range bit for active frequency, in lpm defaults to 0
    int batchFreq; //time between each sCollect call
    int batchSize; //number of sCollect calls before calling sExecute
    int iter; //controll variable, controlles total number of loops before next task
    void (*collect)(taskOutput* output);
    void (*execute)(taskOutput** outputs, int batchSize);
} sprintSpec;

taskOutput* output;
taskOutput** outputs;

void initTimer(int clk);

void initClock(CLK clock, int range);

void initGPIO();

void wait(int sec);

void sInit(sprintSpec* spec);

void sStart(sprintSpec* spec);

void sCollect(sprintSpec* spec);

void sExecute(sprintSpec* spec);
