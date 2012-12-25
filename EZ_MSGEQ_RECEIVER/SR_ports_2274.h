#ifndef SR_PORTS_H_
#define SR_PORTS_H_

#define SR_PORT(x) P4##x

// port P1 masks for shift register
#define SR_SER 0x08
#define SR_OE  0x10	
#define SR_CLK 0x20	

#endif /*SR_PORTS_H_*/
