//----------------------------------------------------------------------------
//  Demo Application for MSP430/CC1100-2500 Interface Code Library v1.0
//  For EZ-RF2500 -- Roscoe Giles
// Basic operation is to push button 1 and send a message to the other board.
// When a message is sent, the green LED is toggled (LED2)
// When a message is received, the red LED is toggled. (LED 1)
//-----------------------------------------------------------------------------
//  Originally by
//  K. Quiring
//  Texas Instruments, Inc.
//  July 2006
//----------------------------------------------------------------------------



#include "TI_CC/include.h"

// bit masks for P1 on the RF2500 target board
#define LED1_MASK              0x01	
#define LED2_MASK              0x02
#define SW1_MASK               0x04 

//KK
/*==============Changes======================*/

#include "msp430f2274.h"
#include "amplitude.h" 
#include "band.h" 
#include "SR_ports_2274.h"
#include "bit_macros.h"


/* global variable for result (for debugging) */
int spectrumValue[7] = {0,0,0,0,0,0,0}; // Holds the ADC values obtained from MSGEQ7
const unsigned char row_code[]= 
	{AMP_1,AMP_1,AMP_2,AMP_3,AMP_4,AMP_5,AMP_6,AMP_7};
const unsigned char col_code[]=
	{COL_0,COL_1,COL_2,COL_3,COL_4,COL_5,COL_6,COL_7};


/* Function Definitions */
void delay(unsigned int);
extern void SR_put_2byte(unsigned char, unsigned char);

/* Function */
void delay(unsigned int ms)
{
 while (ms--)
    {
        __delay_cycles(1000); // set for 16Mhz change it to 1000 for 1 Mhz
    }
}

/*==============End Changes===================*/
//KKend

extern char paTable[];		// power table for C2500
extern char paTableLen;


char rxBuffer[30];
unsigned int i;

void main (void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

  P2SEL &= 0x3F;							//clear select bits for XIN,XOUT, which are set by default

  TI_CC_SPISetup();                         // Initialize SPI port
  TI_CC_PowerupResetCCxxxx();               // Reset CCxxxx
  writeRFSettings();                        // Write RF settings to config reg
  TI_CC_SPIWriteBurstReg(TI_CCxxx0_PATABLE, paTable, paTableLen);//Write PATABLE

  // Configure ports -- switch inputs, LEDs, GDO0 to RX packet info from CCxxxx
 
  // Input switch (on target board).
  // Pushing switch pulls down the input bit SW1 and triggers an input interrupt
  //   on port 1.  The handler sends a message to the second board.
  
  P1REN |= SW1_MASK; //  enable pullups on SW1
  P1OUT |= SW1_MASK;
  P1IES = SW1_MASK; //Int on falling edge	
  P1IFG &= ~(SW1_MASK);//Clr flag for interrupt
  P1IE = SW1_MASK;//enable input interrupt
  
  // Configure LED outputs on port 1
  P1DIR = LED1_MASK + LED2_MASK ; //Outputs
  P1OUT |= (LED1_MASK+LED2_MASK); // both lights on
   
  // setup for interrupts related to receipt of a message from the CC2500
  
  TI_CC_GDO0_PxIES |= TI_CC_GDO0_PIN;       // Int on falling edge of GDO0 (end of pkt)
  TI_CC_GDO0_PxIFG &= ~TI_CC_GDO0_PIN;      // Clear Interrupt flag for GDO0 pin
  TI_CC_GDO0_PxIE |= TI_CC_GDO0_PIN;        // Enable interrupt on end of packet

  // turn on the CC2500 in receive mode
  TI_CC_SPIStrobe(TI_CCxxx0_SRX);           // Initialize CCxxxx in RX mode.
                                            // When a pkt is received, it will
                                            // signal on GDO0 and wake CPU
  _BIS_SR(GIE);                 // Enter LPM3, enable interrupts

/*==================LED Array Code===================*/
	// setup output ports for output to the SR
	BIC(SR_PORT(OUT),SR_CLK);	// clocks low
	BIS(SR_PORT(OUT),SR_OE);	// output disabled
	BIS(SR_PORT(DIR),SR_SER|SR_OE|SR_CLK); // set all three as output	
	while(1){
		P4OUT |= 0x01; //Reset is high
		delay(20);
		P4OUT &= 0x02; //Reset is low
		
		unsigned char row,col;
		
		//Loop through the 7 different strobes and determine the number
		//of lights to display on each column of the LED matrix based on
		//the intensity that is associated with each.
		//The intensity is found by the MSGEQ IC
		for (i=0;i<7;i++){
			P4OUT ^= 0x02; //Strobe goes low
 	 		delay(1);
			BIS(P4OUT,SR_OE); // turn off output to the display
			
			if (spectrumValue[i] < 150){
				row=AMP_1;
				col=col_code[7-i];
			}
			else if (spectrumValue[i] < 300) {
				row=AMP_2;
				col=col_code[7-i];
			}
			else if (spectrumValue[i] < 450) {
				row=AMP_3;
				col=col_code[7-i];
			}			
			else if (spectrumValue[i] < 600) {
				row=AMP_4;
				col=col_code[7-i];
			}
			else if (spectrumValue[i] < 750) {
				row=AMP_5;
				col=col_code[7-i];
			}
			else if (spectrumValue[i] < 900) {
				row=AMP_6;
				col=col_code[7-i];
			}
			else if (spectrumValue[i] < 1000) {
				row=AMP_7;
				col=col_code[7-i];
			}												
			else {
				row=AMP_8;
				col=col_code[7-i];
			}	
			SR_put_2byte(row,col);
			BIC(SR_PORT(OUT),SR_OE); // turn on the display output
			P4OUT ^= 0x02; //Strobe goes high
		}
	}
/*================End LED Array Code=================*/
}


// ISR for received packet
// The ISR assumes the int came from the pin attached to GDO0 and therefore
// does not check the other seven inputs.  Interprets this as a signal from
// CCxxxx indicating packet received.
#pragma vector=PORT2_VECTOR
__interrupt void port2_ISR (void)
{
  char len=29;                               // Len of pkt to be RXed (only addr
                                             // plus data; size byte not incl b/c
                                             // stripped away within RX function)
  if (RFReceivePacket(rxBuffer,&len)){       // Fetch packet from CCxxxx
    //P1OUT ^= rxBuffer[1];                  // Toggle LED according to pkt data (LED1)
  	int j,c = 1;
  	for(j = 0; j < 7 ; j++){
  		spectrumValue[j] = (rxBuffer[c]-'0') + (rxBuffer[c+1]-'0')*10 + (rxBuffer[c+2]-'0')*100 + (rxBuffer[c+3]-'0')*1000;
  		c+=4;
  	}
  }
  P2IFG &= ~TI_CC_GDO0_PIN;                  // Clear flag
}


