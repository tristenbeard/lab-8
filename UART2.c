/* UART2.c
 * Tristen Beard & 
 * Data: Nov 10, 2026
 * PA22 UART2 Rx from other microcontroller PA8 UART1 Tx<br>
 */

#include <ti/devices/msp/msp.h>
#include "UART2.h"
#include "../inc/Clock.h"
#include "../inc/LaunchPad.h"
#include "../inc/FIFO1.h"
#define PA8INDEX  18 // UART1_TX  SPI0_CS0  UART0_RTS TIMA0_C0  TIMA1_C0N
#define PA9INDEX  19 // UART1_RX  SPI0_PICO UART0_CTS TIMA0_C1  RTC_OUT   TIMA0_C0N TIMA1_C1N CLK_OUT
#define PA22INDEX 46 // UART2_RX  TIMG8_C1  UART1_RTS TIMA0_C1  CLK_OUT   TIMA0_C0N TIMG6_C1

uint32_t LostData;

// power Domain PD0
// for 80MHz bus clock, UART clock is ULPCLK 40MHz
// initialize UART2 for 2000 baud rate
// no transmit, interrupt on receive timeout
void UART2_Init(void){
    // do not reset or activate PortA, already done in LaunchPad_Init
    // RSTCLR to UART2 peripherals
    //   bits 31-24 unlock key 0xB1
    //   bit 1 is Clear reset sticky bit
    //   bit 0 is reset gpio port
  UART0->GPRCM.RSTCTL = 0xB1000003;
  UART0->GPRCM.PWREN = 0x26000001;
  Clock_Delay(24); // time for uart to power up
  IOMUX->SECCFG.PINCM[PA10INDEX]  = 0x00000082;
  IOMUX->SECCFG.PINCM[PA11INDEX]  = 0x00040082;
  TxFifo_Init();
  RxFifo_Init();
  UART0->CLKSEL = 0x08; // bus clock
  UART0->CLKDIV = 0x00; // no divide
  UART0->CTL0 &= ~0x01; // disable UART0
  UART0->CTL0 = 0x00020018;
 // assumes an 80 MHz bus clock
  UART0->IBRD = 21;//   divider = 21+45/64 = 21.703125
  UART0->FBRD = 45; // baud =2,500,000/21.703125 = 115,191
  UART0->LCRH = 0x00000030;
  UART0->CPU_INT.IMASK = 0x0C01;
  // bit 11 TXINT
  // bit 10 RXINT
  // bit 0  Receive timeout
  UART0->IFLS = 0x0422;
  // bits 11-8 RXTOSEL receiver timeout select 4 (0xF highest)
  // bits 6-4  RXIFLSEL 2 is greater than or equal to half
  // bits 2-0  TXIFLSEL 2 is less than or equal to half
  NVIC->ICPR[0] = 1<<15; // UART0 is IRQ 15
  NVIC->ISER[0] = 1<<15;
  NVIC->IP[3] = (NVIC->IP[3]&(~0xFF000000))|(2<<30);	// priority (bits 31,30) 
  UART0->CTL0 |= 0x01; // enable UART0

    // Enable power to UART1 peripherals
     // write this
}
//For each register in the above initialization, look up the definition in the book or in the data sheet. You will make the following changes
//Change UART0 to UART2
//Change PA11 (U0_Rx) to PA22 (U2_Rx)
//Remove/delete PA10 (U0_Tx), it is not needed
//Remove TxFifo_Init
//Change RxFifo_Init to your FIFO1_Init
//Change the baud rate from 115,200 to 2000 bps
//Interrupt only on receiver timeout RTOUT (no TXINT, no RXINT)
//Do NOT change the IFLS register, the 4 means RTOUT set on 4 bits of idle
//Change from interrupt 15 (UART0) to interrupt 14 (UART2)
//The priority bits for interrupt 14 are bits 23-22 in the NVIC-IP[3] register

//------------UART2_InChar------------
// Get new serial port receive data from FIFO1
// Input: none
// Output: Return 0 if the FIFO1 is empty
//         Return nonzero data from the FIFO1 if available
char UART2_InChar(void){
   while((UART0->STAT&0x04) == 0x04){}; // wait while RxFifo empty
   return((char)(UART0->RXDATA));
   }


void UART2_IRQHandler(void){ 
  uint32_t status;
  status = UART0->CPU_INT.IIDX; // reading clears bit in RIS
  if(status == 0x01){       // 0x01 receive timeout RTOUT
    copyHardwareToSoftware();
  }else if(status == 0x0B){ // 0x0B receive RXINT
    copyHardwareToSoftware();
  }else if(status == 0x0C){ // 0x0C transmit TXINT
    copySoftwareToHardware();
    if(TxFifo_Size() == 0){         	// software TX FIFO is empty
      UART0->CPU_INT.IMASK &= ~0x0800;	// disable TX FIFO interrupt
    }
  }

// write this
// acknowledge RTOUT
  GPIOB->DOUTTGL31_0 = BLUE; // toggle PB22 (minimally intrusive debugging)
  GPIOB->DOUTTGL31_0 = BLUE; // toggle PB22 (minimally intrusive debugging)
  // read all data and put in FIFO1
  GPIOB->DOUTTGL31_0 = BLUE; // toggle PB22 (minimally intrusive debugging)
}
//For each register in the above function, look up the definition in the book or in the data sheet. You will make the following changes
//Change UART0 to UART2
//Change RxFifo_Put to your FIFO1_Put
//Remove handling of RXINT and TXINT cases and just handle receive timeout RTOUT
//Remove handling of the FIFO full error (this will never fill)
//Add a triple toggle to the ISR so we can measure how long the ISR runs and how often
