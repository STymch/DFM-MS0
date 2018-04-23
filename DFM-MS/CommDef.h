// CommDef.h
// Macro & Types

#ifndef _COMMDEF_h
#define _COMMDEF_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

// Enable printing out nice debug messages, comment macros for disable debug messages
//#define _DEBUG_TRACE
#define _SETUP_TRACE

// Setup debug printing macros
#ifdef _DEBUG_TRACE
// Define where debug output will be printed
#define DEBUG_PRN_DEVICE	Serial

#define DBG_PRN(...)		{ DEBUG_PRN_DEVICE.print(__VA_ARGS__);		}
#define DBG_PRNL(...)		{ DEBUG_PRN_DEVICE.println(__VA_ARGS__);	}
#define DBG_PRN_LOGO(a,b)	{ DBG_PRNL(); DBG_PRN(a); DBG_PRN(b);		}
#else
#define DBG_PRN(...)		{}
#define DBG_PRNL(...)		{}
#define DBG_PRN_LOGO(a,b)	{}
#endif

#ifdef _SETUP_TRACE
// Define where debug output will be printed
#define PRN_DEVICE	Serial

#define STP_PRN(...)		{ PRN_DEVICE.print(__VA_ARGS__);		}
#define STP_PRNL(...)		{ PRN_DEVICE.println(__VA_ARGS__);	}
#define STP_PRN_LOGO(a,b)	{ STP_PRNL(); STP_PRN(a); STP_PRN(b);		}
#else
#define STP_PRN(...)		{}
#define STP_PRNL(...)		{}
#define STP_PRN_LOGO(a,b)	{}
#endif

// --- TRUE, FALSE
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE  1
#endif

// --- Data Types
typedef							bool	BOOL;	// 1 byte
typedef							char	CHAR;	// 1 byte
typedef		unsigned			char	BYTE;	// 1 byte
typedef		unsigned			short	WORD;	// 2 byte
typedef		unsigned			int		UINT;	// 2 byte
typedef							int		INT;	// 2 byte
typedef							long	LONG;	// 4 byte
typedef		unsigned			long	DWORD;	// 4 byte
typedef							float	FLOAT;	// 4 byte
typedef		unsigned	long	long	QWORD;	// 8 byte

// --- Macro
// - copy data from (source) into (copy) with disable interrupts: 
#define COPY_NOINT(copy, source) {noInterrupts(); copy = source; interrupts();}

// - numerical compare operations
#define Greater(a, b)      ( ( (a) - (b) ) >  0 ) 
#define GreaterEqual(a, b) ( ( (a) - (b) ) >= 0 )
#define Less(a, b)         ( ( (a) - (b) ) <  0 )
#define LessEqual(a, b)    ( ( (a) - (b) ) <= 0 )

// - bytes, words, dwords
#ifndef LOBYTE
#define LOBYTE(w )     ( (BYTE)(w) )
#endif
#ifndef HIBYTE
#define HIBYTE(w )     ( (BYTE)( (UINT)(w) >> 8 ) )
#endif
#ifndef LOWORD
#define LOWORD(dw)     ( (WORD)(dw) )
#endif
#ifndef HIWORD
#define HIWORD(dw)     ( (WORD)( (DWORD)(dw) >> 16 ) )
#endif

#define MKWORD(bh,bl)  ( (WORD) (((BYTE)(bl)) | (((WORD) ((BYTE)(bh)))<<8 )) )
#define MKDWORD(wh,wl) ( (DWORD)(((WORD)(wl)) | (((DWORD)((WORD)(wh)))<<16)) )

// - bits in BYTE
#define READBIT(B,n)	( ( ((BYTE)(B)) & (1<<(n)) ) >> (n) )				/* read bit number n from byte */
#define WRITEBIT(B,n,b) ( (b) ? ( (B)|=(1<<(n)) ):( (B)&=(~(1<<(n))) ) )	/* write bit number n into byte */
#define SETBIT(B,n)		( (B)|=(1<<(n)) ) 									/* write 1 into bit number n into byte */
#define CLEARBIT(B,n)	( (B)&=(~(1<<(n))) )								/* write 0 into bit number n into byte */


#endif	// _COMMDEF_h

