// CommDef.h
// Contents : ������������������� ������� � ���� ������

#ifndef _COMMDEF_h
#define _COMMDEF_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

/*class CommDefClass
{
 protected:


 public:
	void init();
};

extern CommDefClass CommDef;*/


// --- ���� ������
typedef unsigned char	BYTE;
typedef unsigned short	WORD;
typedef unsigned int	UINT;
typedef   signed int	INT;
typedef unsigned long	DWORD;
typedef float			FLOAT;

// --- �������
// - ��������� �������� �������
#define Greater(a, b)      ( ( (a) - (b) ) >  0 )
#define GreaterEqual(a, b) ( ( (a) - (b) ) >= 0 )
#define Less(a, b)         ( ( (a) - (b) ) <  0 )
#define LessEqual(a, b)    ( ( (a) - (b) ) <= 0 )

// - ����������� �������, �������, �������� �������
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

// - ����������� ����������� ������ � BYTE
#define GETBIT(B, n)  ( ( ((BYTE)(B)) & (1<<(n)) ) >> (n) ) /* get bit number n from byte */
#define PUTBIT(B,b,n) ( (b) ? ( (B)|=(1<<(n)) ):( (B)&=(~(1<<(n))) ) ) /* put bit number n into byte */


// --- ������� ��������� ����������( ������������ , �� , ��� )
// --- ������ ( 4 ����� ) : [ code ][ op1 ][ op2 ][ op3 ]
struct CCommand
{
	BYTE m_bCode; // ��� �������
	BYTE m_bOp1; // ������ �������
	BYTE m_bOp2; // ������ �������
	BYTE m_bOp3; // ������ �������
};
// --- �������� ������� �� �� �����������
inline void CMND(CCommand* cmnd, BYTE code = 0, BYTE op1 = 0, BYTE op2 = 0, BYTE op3 = 0)
{
	cmnd->m_bCode = code; // ���
	cmnd->m_bOp1 = op1; // ������ �������
	cmnd->m_bOp2 = op2; // ������ �������
	cmnd->m_bOp3 = op3; // ������ �������
}

#endif	// _COMMDEF_h

