/*
 * NmeaParser.h
 *
 *  Created on: 01/10/2016
 *      Author: Paulo A. Silva
 */

#ifndef NMEAPARSER_H_
#define NMEAPARSER_H_

#define NMEA_MAX_ARGS 			30
#define NMEA_BUFFERSIZE 		120
#define STREAM_INPUT_MAX_SIZE	2048

#define DISABLE   0
#define ENABLE    1
#define DISABLED  0
#define ENABLED   1
#define OFF       0
#define ON        1
#define FALSE     0
#define TRUE      1
#define false     FALSE
#define true      TRUE

typedef signed char             S8 ;  //!< 8-bit signed integer.
typedef unsigned char           U8 ;  //!< 8-bit unsigned integer.
typedef signed short int        S16;  //!< 16-bit signed integer.
typedef unsigned short int      U16;  //!< 16-bit unsigned integer.
typedef signed long int         S32;  //!< 32-bit signed integer.
typedef unsigned long int       U32;  //!< 32-bit unsigned integer.
typedef signed long long int    S64;  //!< 64-bit signed integer.
typedef unsigned long long int  U64;  //!< 64-bit unsigned integer.
typedef float                   F32;  //!< 32-bit floating-point number.
typedef double                  F64;  //!< 64-bit floating-point number.

typedef enum
{
	NMEA_GPGGA,
	NMEA_GPRMC,
	NMEA_UNKNOWN,
}nmeaParseType;

typedef struct
{
	char* addr;
	char* checkSumAddr;
	int index;
	U16	length;
}nmeaPacketInfo;

typedef struct
{
	U8 year;
	U8 month;
	U8 day;
}nmeaDateType;

typedef struct
{
	F32 utcTime;
	U8 status;
	F32 latitude;
	U8 nsIndicator;
	F32 longitude;
	U8 ewIndicator;
	F32 speedOverGround;
	F32 courseOverGround;
	nmeaDateType date;
	F32 magneticVariation;
	U8 magneticVariationEw;
	U8 modeIndicator;
	U8 checkSum;
}nmeaGprmcPacketType;


#endif /* NMEAPARSER_H_ */
