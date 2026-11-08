/* UART1.c
 * Tristen Beard & 
 * Date: Nov 10, 2026
 * PA8 UART1 Tx to other microcontroller PA22 UART2 Rx
 */


#include <ti/devices/msp/msp.h>
#include "UART1.h"
#include "../inc/Clock.h"
#define PA8INDEX  18 // UART1_TX  SPI0_CS0  UART0_RTS TIMA0_C0  TIMA1_C0N


// power Domain PD0
// for 80MHz bus clock, UART clock is ULPCLK 40MHz
// initialize UART1 for 2000 baud rate
// blind, no synchronization on transmit
void UART1_Init(void){
    // do not reset or activate PortA, already done in LaunchPad_Init
   UART1->GPRCM.RSTCTL = 0xB1000003; // reset UART0
   UART1->GPRCM.PWREN = 0x26000001; // activate UART0
   Clock_Delay(24); // time for uart to activate
// configure PA11 PA10 as alternate UART0 function
   IOMUX->SECCFG.PINCM[PA8INDEX] = 0x00000082;
//bit 7 PC connected
//bits 5-0=2 for UART0_Tx
//bit 18 INENA input enable
//bit 7 PC connected
//bits 5-0=2 for UART0_Rx
   UART1->CLKSEL = 0x08; // bus clock
   UART1->CLKDIV = 0x00; // no divide
   UART1->CTL0 &= ~0x01; // disable UART0
   UART1->CTL0 = 0x00020018; // enable fifos, tx and rx
// 40000000/16 = 2,500,000, 2,500,000/115200 = 21.70139
   UART1->IBRD = 1250; // divider = 21+45/64 = 21.703125
   UART1->FBRD = 0;//lab doc asks for 2k bps which 2500000/2000 = 1250 no remainder
   UART1->LCRH = 0x00000030; // 8bit, 1 stop, no parity
   UART1->CTL0 |= 0x01; // enable UART0
     // write this
}


//------------UART1_OutChar------------
// Output 8-bit to serial port
// blind synchronization
// 10 bit frame, 2000 baud, 5ms per frame
// Input: letter is an 8-bit ASCII character to be transferred
// Output: none
void UART1_OutChar(char data){
// simply output data to transmitter without waiting or checking status
  // write this
   UART1->TXDATA = data;
}
