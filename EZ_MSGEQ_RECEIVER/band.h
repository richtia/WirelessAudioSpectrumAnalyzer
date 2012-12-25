#ifndef BAND_H_
#define BAND_H_

#define C1 0x01
#define C2 0x02
#define C3 0x04
#define C4 0x08
#define C5 0x10
#define C6 0x20
#define C7 0x40
#define C8 0x80

// values for digits
#define COL_0 (C1)			//All Ground
#define COL_1 (C2)			//Pin 1 VCC
#define COL_2 (C3)				//Pin 1,2 VCC
#define COL_3 (C4)					//Pin 1,2,3 VCC
#define COL_4 (C5)						//Pin 1,2,3,4 VCC
#define COL_5 (C6)						//Pin 1,2,3,4,5 VCC
#define COL_6 (C7)							//Pin 1,2,3,4,5,6 VCC
#define COL_7 (C8)								//Pin 1,2,3,4,5,6,7 VCC
#define COL_8 (0x00)							//All VCC
#endif /*BAND_H_*/
