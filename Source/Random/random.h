/************************************************************************************
** File: - E:\ARM\lm3s8962projects\ScreenSaver\Source\Random\random.h
**  
** Copyright (C), Long.Luo, All Rights Reserved!
** 
** Description: 
**      random.h - Protoytpes for the random number generator.
** 
** Version: 1.0
** Date created: 00:04:50,23/04/2013
** Author: Long.Luo
** 
** --------------------------- Revision History: --------------------------------
** 	<author>	<data>			<desc>
** 
************************************************************************************/

#ifndef __RANDOM_H__
#define __RANDOM_H__

//*****************************************************************************
//
// Prototypes for the random number generator functions.
//
//*****************************************************************************
extern void RandomAddEntropy(unsigned long ulEntropy);
extern void RandomSeed(void);
extern unsigned long RandomNumber(void);

#endif /* __RANDOM_H__ */

