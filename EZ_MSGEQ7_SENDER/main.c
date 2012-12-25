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
#define WDTCONFIG (WDTCNTCL|WDTSSEL)


//KK
/*================================ADC CODE================================================*/
#include "msp430f2274.h"

#define ADC_INPUT_BIT_MASK 0x40 //PIN 10
#define ADC_INCH INCH_14

/* a simple function to read the single value from the ADC.  
 * There are actually a pair of functions:
 * (1) start_conversion(), which initiates a single conversion
 * (2) get_result() which returns a number from 0 t0 1023
 *  	get_result WAITS for a value to be ready.
 */
 void init_adc(void);
 void start_conversion(void);
 int get_result(void);
 
/* global variable for result (for debugging) */
volatile int latest_result;

 /* basic adc operations */

unsigned delay_hits=0;	/* counter to estimate the busy wait time */

void delay(unsigned int ms)
{
 while (ms--)
    {
        __delay_cycles(1000); // set for 16Mhz change it to 1000 for 1 Mhz
    }
}

void start_conversion(){
 	ADC10CTL0 |= ADC10SC;
 }
 
 int get_result(){
 	delay_hits=0;
 	while (ADC10CTL1 & ADC10BUSY) {++delay_hits;}// busy wait for busy flag off
 	return ADC10MEM; 	
 }
 
 void init_adc(){
  	ADC10CTL1= ADC_INCH	//input channel bit field
 			  +SHS_0 //use ADC10SC bit to trigger sampling
 			  +ADC10DIV_4 // ADC10 clock/5
 			  +ADC10SSEL_0 // Clock Source=ADC10OSC
 			  +CONSEQ_0; // single channel, single conversion
 			  ;
 	ADC10AE1=ADC_INPUT_BIT_MASK; // enable A4 analog input	  
 	ADC10CTL0= SREF_0	//reference voltages are Vss and Vcc
 	          +ADC10SHT_3 //64 ADC10 Clocks for sample and hold time (slowest)
 	          +ADC10ON	//turn on ADC10
 	          +ENC		//enable (but not yet start) conversions
 	          ;		  
 }
/*================================END ADC CODE============================================*/
//KKend



extern char paTable[];		// power table for C2500
extern char paTableLen;

char txBuffer[30];
unsigned int i;

void main (void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
 // BCSCTL1 = 0xF0;			// 8Mhz calibration for clock
  //DCOCTL  = 0x00;

  BCSCTL1 = CALBC1_1MHZ;	// 1 Mhz calibration for clock
  DCOCTL  = CALDCO_1MHZ;
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

//KK
/*==============================ADC SETUP================================*/
  P4DIR |= 0x18;		//Set P4.3 and P4.4 to output (set reset and strobe as outputs) 0x18 Reset is PIN8 Strobe is PIN9
  P4OUT |= 0x10;		//Set (reset is low and strobe is high) 0x10
  init_adc();
/*==============================ADC SETUP END============================*/
//KKend






while(1){

	//KK
	/*==============================MSGEQ7 SETUP================================*/
	P4OUT |= 0x08;//Reset is high 0x18
	delay(10);
	P4OUT &= 0x10;//Reset is low 0x10
	/*==============================MSGEQ7 SETUP END============================*/
	//KKend
		
		
	// Build packet to send to the other board
	txBuffer[0] = 29;                           // Packet length
	txBuffer[1] = 0x01;                        // Packet address

	/*=========================Put ADC Values into txBuffer======================*/
	for (i = 0; i < 7; i++){
  	 	P4OUT ^= 0x10; //Strobe goes low
  	 	delay(5);
  	 	start_conversion();
  		latest_result=get_result();		
  		
  		txBuffer[(i*4)+2] = (char)(((int)'0')+latest_result%10);  //_ _ _ x

		txBuffer[(i*4)+3] = (char)(((int)'0')+(latest_result/10)%10); // _ _ x _

		txBuffer[(i*4)+4] = (char)(((int)'0')+(latest_result/100)%10); // _ x _ _

		txBuffer[(i*4)+5] = (char)(((int)'0')+(latest_result/1000)%10); // x _ _ _ 
		 	
  		P4OUT ^= 0x10; //Strobe goes high
  	 	
  	 }
	
	
	/*=========================Put ADC Values into txBuffer======================*/

	WDTCTL  =  WDTPW|WDTCONFIG;
	RFSendPacket(txBuffer, 30);                 // Send the packet value over RF
	WDTCTL = WDTPW | WDTCNTCL; 	//kick the dog if we are stuck in an 
	WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
	
	P1OUT ^= LED2_MASK;			 			 // toggle LED2 on THIS board
  
	P1IFG &= ~SW1_MASK;                        //Clr flag that caused int
	P2IFG &= ~TI_CC_GDO0_PIN;                  // After pkt TX, this flag is set.
		
	}
	
	
	
}





