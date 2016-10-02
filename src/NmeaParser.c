/*
 ============================================================================
 Name        : NmeaParser.c
 Author      : Paulo A. Silva
 Version     :
 Copyright   : Your copyright notice
 Description : NMEA parser programming test
 Manual      : The application is able to read the input data from a text file
  	  	  	   or from the console, to insert data from a text file insert the
  	  	  	   path of the text file as the first argument.
			   Example.: ./NmeaParser inputData.txt (Reads from text file)
			   Example.: ./NmeaParser 				(Reads from the console)
			   The inputData.txt can be found on the Debug folder.
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NmeaParser.h"

/**
 * @brief Function to convert checksum that is in hex string format to unsigned integer.
 * @param addr Specifies the address of the first digit of the checksum.
 * @return The checksum converted value.
 */
U8 ConvertCheckSum (const char* addr)
{
	char checksum[2]; //2 bytes checksum
	strncpy(checksum,addr,sizeof(checksum));
	return (U8)(strtoul(checksum, NULL, 16));
}

/**
 * @brief Function that calculates the checksum and check against specified value.
 * @param payload Start address of the nmea packet, address of the '$' char.
 * @param checkSum Checksum value to be compared with.
 * @return TRUE If checksum does match and FALSE if checksum does not.
 */
U8 CheckNmeaChecksum (const char* payload,U8 checkSum)
{
	U8 calculatedChecksum = 0;

	for (payload++; *payload!='*' ; payload++)
		calculatedChecksum ^= *payload;

	if(calculatedChecksum == checkSum)
		return true;

	return false;
}

/**
 * @brief Function that prints the GPRMC Nmea packet type
 * @param gprmPacket Address of the structure to be printed.
 */
void PrintGprmPacket (const nmeaGprmcPacketType* gprmPacket)
{
	printf("UTC TIME : %.2f\r\n",gprmPacket->utcTime);
	printf("STATUS : %c\r\n",gprmPacket->status);
	printf("LATITUDE : %.5f\r\n",gprmPacket->latitude);
	printf("N/S INDICATOR : %c\r\n",gprmPacket->nsIndicator);
	printf("LONGITUDE : %.5f\r\n",gprmPacket->longitude);
	printf("E/W INDICATOR : %c\r\n",gprmPacket->ewIndicator);
	printf("SPEED OVER GROUND : %.3f\r\n",gprmPacket->speedOverGround);
	printf("COURSE OVER GROUND : %.2f\r\n",gprmPacket->courseOverGround);
	printf("DATE : %02u%02u%02u\r\n",gprmPacket->date.day,gprmPacket->date.month,gprmPacket->date.year);
	printf("MAGNETIC VARIATION : %.1f\r\n",gprmPacket->magneticVariation);
	printf("MAGNETIC VARIATION E/W : %c\r\n",gprmPacket->magneticVariationEw);
	printf("MODE INDICATOR : %c\r\n",gprmPacket->modeIndicator);
	printf("CHECKSUM: %X\r\n",gprmPacket->checkSum);
}


/**
 * @brief Function that tokenize the Nmea packet captured from the stream and convert tokens to proper data format.
 * @param payload Start address of the Nmea packet.
 * @param type Type of the Nmea packet to be parsed.
 * @return 0 indicating a normal operation and -1 for error.
 */
int NmeaParse (char* payload, nmeaParseType type)
{
	const char* NMEA_DELIMITER = ",";
    int         argc = 0;
    char        *argv[NMEA_MAX_ARGS];

    /* Tokenization */
    for (argc = 0; argc < NMEA_MAX_ARGS; argc++)
    {
        argv[argc]=strsep(&payload, NMEA_DELIMITER);
        if (argv[argc] == 0) break;
    }

    if (argc <= 1)
    {
    	return(-1);
    }

    switch ( type )
    {
		case NMEA_GPRMC:
		{
			/* Converting the tokens to a proper kind of data that can be logged. */
			nmeaGprmcPacketType gprmcPacket;
			gprmcPacket.utcTime=(F32)(atof(argv[1]));
			gprmcPacket.status=*argv[2];
			gprmcPacket.latitude=(F32)(atof(argv[3]));
			gprmcPacket.nsIndicator=*argv[4];
			gprmcPacket.longitude=(F32)(atof(argv[5]));
			gprmcPacket.ewIndicator=*argv[6];
			gprmcPacket.speedOverGround=(F32)(atof(argv[7]));
			gprmcPacket.courseOverGround=(F32)(atof(argv[8]));

			char day[2];
			char month[2];
			char year[2];
			strncpy(day,argv[9],sizeof(day));
			strncpy(month,argv[9]+2,sizeof(month));
			strncpy(year,argv[9]+4,sizeof(year));
			gprmcPacket.date.day=(U8)(strtoul(day, NULL, 10));
			gprmcPacket.date.month=(U8)(strtoul(month, NULL, 10));
			gprmcPacket.date.year=(U8)(strtoul(year, NULL, 10));

			if (strcmp(argv[10], "") == 0)
				gprmcPacket.magneticVariation=0;
			else
				gprmcPacket.magneticVariation=(F32)(atof(argv[10]));

			if (strcmp(argv[11], "") == 0)
				gprmcPacket.magneticVariationEw='0';
			else
				gprmcPacket.magneticVariationEw=*argv[11];

			char mode;
			strncpy(&mode,argv[12],sizeof(mode));
			gprmcPacket.modeIndicator=mode;
			char checksum[2];
			strncpy(checksum,argv[12]+2,sizeof(checksum));
			gprmcPacket.checkSum=(U8)(strtoul(checksum, NULL, 16));

			PrintGprmPacket(&gprmcPacket);
			break;
		}
		case NMEA_GPGGA:
		{
			break;
		}
		case NMEA_UNKNOWN:
		{
			break;
		}
    }
    return(0);
}

/**
 * @brief Function responsible to capture Nmea packet from the input stream.
 * @param str Start address of the input stream.
 * @param readSize Size of the stream.
 */
void NmeaPacketProcess (char* str, size_t readSize)
{
	const char* GPGGA_STR = "$GPGGA";
	const char* GPRMC_STR = "$GPRMC";
	const char	NAMEA_START_CHR = '$';
	const char	NAMEA_CHECKSUM_CHR = '*';
	const int	CHECKSUM_NUM_OF_CHARS=2;
	char* streamStartAddress=str;
	nmeaPacketInfo packetInfo;
	static char NmeaPacket[NMEA_BUFFERSIZE];
	static long int streamOffset=0;

	int i,startFound=false,length=0;
	for ( i = 0 ; i < readSize ; i++ )
	{
		if (*str==NAMEA_START_CHR)
		{
			packetInfo.addr=str;
			packetInfo.index=(int)(str-streamStartAddress)+streamOffset;
			str++;
			/* set a flag to start to count the length */
			startFound=true;
			length++;
			continue;
		}

		/* increments the length if a start was found */
		if(startFound)
			length++;

		/* Star character found, means it reached the end of the packet */
		if (startFound && (*str==NAMEA_CHECKSUM_CHR))
		{
			/* Evaluate the packet length and saves the checksum address */
			packetInfo.length=length+CHECKSUM_NUM_OF_CHARS;
			packetInfo.checkSumAddr=++str;

			/* Reset variables for new packet */
			startFound=false;
			length=0;

			/* Using the length if to copy the packet from the stream */
			if(packetInfo.length<NMEA_BUFFERSIZE)
				memcpy(NmeaPacket,packetInfo.addr,packetInfo.length);

			/* Establish the Nmea packet type, check the checksum and parse the packet information */
			if (strstr(NmeaPacket,GPGGA_STR) != NULL)
			{
				printf("\r\nGPGGA START INDEX:%d LENGTH:%u\r\n",packetInfo.index,packetInfo.length);
				if (CheckNmeaChecksum(packetInfo.addr,ConvertCheckSum(packetInfo.checkSumAddr)))
				{
					puts("$GPGGA Checksum Correct");
				}else{
					puts("$GPGGA Checksum Incorrect");
				}
			}
			else if (strstr(NmeaPacket,GPRMC_STR) != NULL)
			{
				printf("\r\nGPRMC START INDEX:%d LENGTH:%u\r\n",packetInfo.index,packetInfo.length);
				if (CheckNmeaChecksum(packetInfo.addr,ConvertCheckSum(packetInfo.checkSumAddr)))
				{
					puts("$GPRMC Checksum Correct");
					NmeaParse(packetInfo.addr,NMEA_GPRMC);
				}else{
					puts("$GPRMC Checksum Incorrect");
				}
			}
			else
			{
				printf("\r\nUNKNOWN START INDEX:%d LENGTH:%u\r\n",packetInfo.index,packetInfo.length);
				if (CheckNmeaChecksum(packetInfo.addr,ConvertCheckSum(packetInfo.checkSumAddr)))
				{
					puts("Unknown Packet Checksum Correct");
				}else{
					puts("Unknown Packet Checksum Incorrect");
				}
			}
		}

		/* It goes to the next stream char */
		str++;
	}

	/* saves the offset of the stream */
	streamOffset+=readSize;
}

int main(int argc, char* argv[])
{
	static char inputStream[STREAM_INPUT_MAX_SIZE];

	/* Reads the input data from a file */
	if(argv[1]!=NULL)
	{

		FILE* file = fopen (argv[1], "r");
		if(file!=NULL)
		{
			/* Because fgets breaks every time it finds a new line feed,
			 * the size read is sent with the address of the stream
			 * to calculate the index properly */
			while(fgets(inputStream,STREAM_INPUT_MAX_SIZE,file)!= NULL)
			{
				NmeaPacketProcess(inputStream,strlen(inputStream));
			}
		}
	}
	/* Reads the input data from the console */
	else
	{
		puts("Input:");
		while(fgets(inputStream,STREAM_INPUT_MAX_SIZE,stdin)!= NULL)
		{
			NmeaPacketProcess(inputStream,strlen(inputStream));
		}
	}
	return EXIT_SUCCESS;
}
