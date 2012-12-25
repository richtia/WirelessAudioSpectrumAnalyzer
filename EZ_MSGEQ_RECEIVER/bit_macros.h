#ifndef BIT_MACROS_H_
#define BIT_MACROS_H_

// helpful macros
#define BIC(location,mask) ((location) &= ~(mask))
#define BIS(location,mask) ((location) |= (mask))

#endif /*BIT_MACROS_H_*/
