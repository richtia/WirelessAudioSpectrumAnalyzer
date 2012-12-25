#ifndef AMPLITUDE_H_
#define AMPLITUDE_H_

#define R1 0x01
#define R2 0x02
#define R3 0x04
#define R4 0x08
#define R5 0x10
#define R6 0x20
#define R7 0x40
#define R8 0x80

// values for digits
#define AMP_0 (R1+R2+R3+R4+R5+R6+R7+R8)			//All Ground
#define AMP_1 (R2+R3+R4+R5+R6+R7+R8)			//Pin 1 VCC
#define AMP_2 (R3+R4+R5+R6+R7+R8)				//Pin 1,2 VCC
#define AMP_3 (R4+R5+R6+R7+R8)					//Pin 1,2,3 VCC
#define AMP_4 (R5+R6+R7+R8)						//Pin 1,2,3,4 VCC
#define AMP_5 (R6+R7+R8)						//Pin 1,2,3,4,5 VCC
#define AMP_6 (R7+R8)							//Pin 1,2,3,4,5,6 VCC
#define AMP_7 (R8)								//Pin 1,2,3,4,5,6,7 VCC
#define AMP_8 (0x00)							//All VCC
#endif /*AMPLITUDE_H_*/
