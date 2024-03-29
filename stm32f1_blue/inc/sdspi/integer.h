/*-------------------------------------------*/
/* Integer type definitions for FatFs module */
/*-------------------------------------------*/

#ifndef _INTEGER

#if 0
#include <windows.h>
#else

#include "inc/stm32f10x.h"

/* These types must be 16-bit, 32-bit or larger integer */
typedef int						INT;
typedef unsigned int	UINT;

/* These types must be 8-bit integer */
typedef signed char		CHAR;
typedef unsigned char	UCHAR;
typedef unsigned char	BYTE;

/* These types must be 16-bit integer */
typedef short						SHORT;
typedef unsigned short	USHORT;
typedef unsigned short	WORD;
typedef unsigned short	WCHAR;

/* These types must be 32-bit integer */
typedef long					LONG;
typedef unsigned long	ULONG;
typedef unsigned long	DWORD;

//typedef u08 BOOL;
//typedef enum {__FALSE=0, __TRUE=!__FALSE} BOOL;

typedef u32 U32;
typedef u16 U16;
typedef u08 U8;
/* Boolean type */
// typedef enum { FALSE = 0, TRUE } BOOL;
#include <stdbool.h>
typedef bool BOOL;
#ifndef FALSE
#define FALSE false
#define TRUE true
#endif

#ifndef __FALSE
#define __FALSE false
#define __TRUE  true
#endif

#endif

#define _INTEGER
#endif
