// CommDef.h
// Contents : общеупотребительные макросы и типы данных

#ifndef _COMMDEF_h
#define _COMMDEF_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

// Выдача отладочных сообщений в аппаратный COM-порт Arduino
#ifndef _DEBUG_TRACE
#define _DEBUG_TRACE
#endif

// --- ТИПЫ ДАННЫХ
typedef		unsigned			char	BYTE;	// 1 byte
typedef		unsigned			short	WORD;	// 2 byte
typedef		unsigned			int		UINT;	// 2 byte
typedef		signed				int		INT;	// 2 byte
typedef		unsigned			long	DWORD;	// 4 byte
typedef							float	FLOAT;	// 4 byte
typedef		unsigned	long	long	QWORD;	// 8 byte

// --- МАКРОСЫ
// - сравнение числовых величин
#define Greater(a, b)      ( ( (a) - (b) ) >  0 )
#define GreaterEqual(a, b) ( ( (a) - (b) ) >= 0 )
#define Less(a, b)         ( ( (a) - (b) ) <  0 )
#define LessEqual(a, b)    ( ( (a) - (b) ) <= 0 )

// - манипуляции байтами, словами, двойными словами
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

// - специальная манипуляция битами в BYTE
#define READBIT(B,n)	( ( ((BYTE)(B)) & (1<<(n)) ) >> (n) )				/* read bit number n from byte */
#define WRITEBIT(B,n,b) ( (b) ? ( (B)|=(1<<(n)) ):( (B)&=(~(1<<(n))) ) )	/* write bit number n into byte */
#define SETBIT(B,n)		( (B)|=(1<<(n)) ) 									/* write 1 into bit number n into byte */
#define CLEARBIT(B,n)	( (B)&=(~(1<<(n))) )								/* write 0 into bit number n into byte */


#endif	// _COMMDEF_h

