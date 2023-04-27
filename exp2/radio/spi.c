#include <msp430.h>
#include <stdint.h>
#include "spi.h"

volatile uint8_t spi_buf = 0;

#define CS      BIT0

//#define INT1    BIT4
//#define INT2    BIT3

void spi_init() {
  /*
  UCB0CTL1 = UCSWRST;
  UCB0CTL0 = UCSYNC + UCSPB + UCMSB + UCCKPH;   //UCMSB + UCMST + UCSYNC; // 3-pin, 8-bit SPI master
  UCB0CTL1 |= UCSSEL_2;                         // SMCLK
  UCB0BR0 = 0x02;                               // Frequency CPU / 2 (16Mhz / 2 = 8 Mhz SPI)
  UCB0BR1 = 0;

  P1SEL  |= SCLK | SDI | SDO;                   // P1.6 is MISO and P1.7 is MOSI
  P1SEL2 |= SCLK | SDI | SDO;

  P1DIR |= SCLK | SDO;
  P1DIR &= ~SDI;

  P2DIR |= CS;// | CS2;                           // P2.0 CS (chip select)
  P2OUT |= CS;// | CS2;

  //P1DIR &= ~(INT1 | INT2);                      // P1.4 and P1.3 as INT (INTERRUPT, not used yet)
*/
  UCB0CTL1 &= ~UCSWRST;                         // Initialize USCI state machine
  P6DIR |= CS;// | CS2;                           // P2.0 CS (chip select)
     /*
    * Select Port 1
    * Set Pin 0 as output
    */
    GPIO_setAsOutputPin(
        GPIO_PORT_P1,
        GPIO_PIN0
    );
        GPIO_setAsOutputPin(
        GPIO_PORT_P6,
        GPIO_PIN0
    );
    /*
    * Select Port 1
    * Set Pin 0 to output Low.
    */
    GPIO_setOutputLowOnPin(
        GPIO_PORT_P1,
        GPIO_PIN0
    );
    GPIO_setOutputLowOnPin(
        GPIO_PORT_P6,
        GPIO_PIN0
    );
    /*
    * Select Port 1
    * Set Pin 5 to input Secondary Module Function, (UCA0CLK).
    */
    GPIO_setAsPeripheralModuleFunctionOutputPin(
        GPIO_PORT_P1,
        GPIO_PIN5,
        GPIO_SECONDARY_MODULE_FUNCTION
    );

    /*
    * Select Port 2
    * Set Pin 0, 1 to input Secondary Module Function, (UCA0TXD/UCA0SIMO, UCA0RXD/UCA0SOMI).
    */
    GPIO_setAsPeripheralModuleFunctionInputPin(
        GPIO_PORT_P5,
        GPIO_PIN0 + GPIO_PIN1,
        GPIO_SECONDARY_MODULE_FUNCTION
    );

     //Initialize Master
    EUSCI_A_SPI_initMasterParam param = {0};
    param.selectClockSource = EUSCI_A_SPI_CLOCKSOURCE_SMCLK;
    param.clockSourceFrequency = CS_getSMCLK();
    param.desiredSpiClock = CS_getSMCLK()/2;
    param.msbFirst = EUSCI_A_SPI_MSB_FIRST;
    param.clockPhase = EUSCI_A_SPI_PHASE_DATA_CAPTURED_ONFIRST_CHANGED_ON_NEXT;
    param.clockPolarity = EUSCI_A_SPI_CLOCKPOLARITY_INACTIVITY_HIGH;
    param.spiMode = EUSCI_A_SPI_3PIN;
    EUSCI_A_SPI_initMaster(EUSCI_A0_BASE, &param);

    //Enable SPI module
    EUSCI_A_SPI_enable(EUSCI_A0_BASE);

    EUSCI_A_SPI_clearInterrupt(EUSCI_A0_BASE,
            EUSCI_A_SPI_RECEIVE_INTERRUPT);
    // Enable USCI_A0 RX interrupt
    EUSCI_A_SPI_enableInterrupt(EUSCI_A0_BASE,
        EUSCI_A_SPI_RECEIVE_INTERRUPT);

    EUSCI_A_SPI_clearInterrupt(EUSCI_A0_BASE,
            EUSCI_A_SPI_TRANSMIT_INTERRUPT);
    // Enable USCI_A0 RX interrupt
    EUSCI_A_SPI_enableInterrupt(EUSCI_A0_BASE,
        EUSCI_A_SPI_TRANSMIT_INTERRUPT);
  

    //Wait for slave to initialize
    __delay_cycles(10000);
}

void spi_txready() {
  //USCI_A0 TX buffer ready?
    while (!EUSCI_A_SPI_getInterruptStatus(EUSCI_A0_BASE,
        EUSCI_A_SPI_TRANSMIT_INTERRUPT)) ;
}

void spi_rxready() {
  while (!EUSCI_A_SPI_getInterruptStatus(EUSCI_A0_BASE,
        EUSCI_A_SPI_RECEIVE_INTERRUPT)) ; // RX Received?
}

void spi_send(uint8_t data) {
  spi_txready();
  //Transmit Data to slave
  EUSCI_A_SPI_transmitData(EUSCI_A0_BASE, data);
}

void spi_recv() {
  spi_rxready();
  spi_buf = UCB0RXBUF;         // Store received data
}

void spi_transfer(uint8_t data) {
  spi_send(data);
  spi_recv();
}

void spi_chipEnable() {
  GPIO_setOutputHighOnPin(
    GPIO_PORT_P6,
    GPIO_PIN0
  );
  return;
}

void spi_chipDisable() {
  GPIO_setOutputLowOnPin(
    GPIO_PORT_P6,
    GPIO_PIN0
  );
  return;
}
