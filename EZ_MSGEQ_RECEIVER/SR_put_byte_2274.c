/* SR_put_byte 
 * outputs a byte, MSB first, to a shift register.
 * It uses a clock and data lines
 * which are defined in the SR_ports include file
 * 2231 Launchpad Board version
 */
 
#include "msp430f2274.h"
#include "amplitude.h" 
#include "band.h" 
#include "SR_ports_2274.h"
#include "bit_macros.h"

void SR_put_2byte(unsigned char row, unsigned char col){
	unsigned char mask=0x80; // variable bit mask
	unsigned char thisbit;   // current bit
	int i;	
	// loop to shift in the data bits
	// note: ninth time is to clk last bit into the output register.
	//       for ninth time, mask=0 and we write an extra 0 bit
	// MSB first.
	for (i=0;i<8;++i){ 
		BIC(SR_PORT(OUT),SR_CLK); 		// drop the clock line so that data can change
		thisbit= (row & mask) ? SR_SER:0;	// compute current bit to send
		SR_PORT(OUT) = (SR_PORT(OUT) & ~SR_SER) | thisbit; // write the bit
		BIS(P4OUT,SR_CLK); 				// raise the clock line to trigger read
		mask /= 2;						// shift the mask to the right.
	}
	mask=0x80;
	for (i=0;i<9;++i){ 
		BIC(SR_PORT(OUT),SR_CLK); 		// drop the clock line so that data can change
		thisbit= (col & mask) ? SR_SER:0;	// compute current bit to send
		SR_PORT(OUT) = (SR_PORT(OUT) & ~SR_SER) | thisbit; // write the bit
		BIS(P4OUT,SR_CLK); 				// raise the clock line to trigger read
		mask /= 2;						// shift the mask to the right.
	}
	
}

