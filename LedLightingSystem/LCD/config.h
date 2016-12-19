/******************************************************************************
 *
 * Copyright:
 *    (C) 2000 - 2005 Embedded Artists AB
 *
 * Description:
 *    Framework for ARM7 processor
 *
 *****************************************************************************/
#ifndef _config_h_
#define _config_h_


#define PLL_MValue			12
#define PLL_NValue			1
#define CCLKDivValue		6
#define USBCLKDivValue		6

#define Fosc	12000000
#define Fcco	((2 * PLL_MValue * Fosc) / PLL_NValue)
#define Fcclk	(Fcco / CCLKDivValue)
#define Fpclk	(Fcclk / 4)

#endif  /* _config_h_ */
