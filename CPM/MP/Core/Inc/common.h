#ifndef _COMMON_DEFS
#define _COMMON_DEFS

#ifndef NULL
#define NULL		0
#endif

#define FALSE		0
#define TRUE		1

#define OFF			0
#define ON			1

#define LOW			0
#define HIGH		1

#define FOREVER		1


#define bit0		0x1
#define bit1		0x2
#define bit2		0x4
#define bit3		0x8
#define bit4		0x10
#define bit5		0x20
#define bit6		0x40
#define bit7		0x80
#define bit8		0x100
#define bit9		0x200
#define bit10		0x400
#define bit11		0x800
#define bit12		0x1000
#define bit13		0x2000
#define bit14		0x4000
#define bit15		0x8000
#define bit16		0x10000
#define bit17		0x20000
#define bit18		0x40000
#define bit19		0x80000
#define bit20		0x100000
#define bit21		0x200000
#define bit22		0x400000
#define bit23		0x800000
#define bit24		0x1000000
#define bit25		0x2000000
#define bit26		0x4000000
#define bit27		0x8000000
#define bit28		0x10000000
#define bit29		0x20000000
#define bit30		0x40000000
#define bit31		0x80000000

#define GetBitValue(data, bitindex)				((data & (1 << bitindex)) >> bitindex)
#define SetBitValue(data, bitindex)				(data = (data | (1 << bitindex)))
#define ClearBitValue(data, bitindex)			(data = ~((~data) | (1 << bitindex)))

#define mHighNibble(b)				((unsigned char)((b & 0xF0) >> 4))
#define mLowNibble(b)				((unsigned char)(b & 0x0F))
#define mMsb(s)						((unsigned char)((s & 0xFF00) >> 8))
#define mLsb(s)						((unsigned char)(s & 0x00FF))
#define mMsw(l)						((unsigned short)((l & 0xFFFF0000) >> 16))
#define mLsw(l)						((unsigned short)(l & 0x0000FFFF))

//compat
#define lsb							mLsb
#define msb							mMsb
#define lsw							mLsw
#define msw							mMsw

#define mHex2BCDWord(s)				((unsigned short)(((s / 1000) << 12) | (((s % 1000) / 100) << 8) | (((s % 100) / 10) << 4) | (s % 10)))
#define mBCD2HexWord(sBCD)			((unsigned short)((mHighNibble(mMsb(sBCD)) * 1000) + (mLowNibble(mMsb(sBCD)) * 100) + (mHighNibble(mLsb(sBCD)) * 10) + mLowNibble(mLsb(sBCD))))
#define mHex2BCD(b)					((unsigned char)(((b / 10) << 4) | (b % 10)))
#define mBCD2Hex(bBCD)				((unsigned char)((mHighNibble(bBCD) * 10) + mLowNibble(bBCD)))

#define mASCII2Number(ASCII)		(ASCII - '0')
#define mNumber2ASCII(Number)		((unsigned char)Number + '0')
#define mIsNumber(ASCII)			((ASCII >= '0') && (ASCII <= '9'))
#define mIsAlpha(ASCII)				(ASCII >= 'A' ? TRUE : FALSE)
#define mAlpha2Number(Alpha)		((unsigned char)Alpha - 0x37)
#define mNumber2Alpha(Number)		((unsigned char)Number + 0x37)
#define Hex2ASCII(hexDigit)			(uint8_t)((hexDigit < 10) ? (hexDigit + '0') : ((hexDigit - 10) + 'A'))
#define ASCII2Hex(ASCIIDigit)		(uint8_t)((ASCIIDigit <= '9') ? (ASCIIDigit - '0') : ((ASCIIDigit - 'A') + 10))	//assumes digit is '0'..'9' or 'A'..'F'


//export data
extern const unsigned long laValue2Bit[32];

#endif
