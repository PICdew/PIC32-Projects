/*******************************************************************
 * Author: xfrings
 *
 * Created: 04-Mar-2015
 *
 * HW: PIC32MX795F512L USB Starter Kit II, PICtail SD Card
 * SW: SPI Interface to SD Card, SDHC Kingston 8GB
 *
 *******************************************************************/
#ifndef LAB3_H
#define	LAB3_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <p32xxxx.h>

/*
 * WD - RF1/RG1
 * CD - RF0/RG0
 * CS - RB1/RB9
 * SCK - RF6/RG6
 * SDI - RF7/RG7
 * SDO - RF8/RG8
 */

/* Prototypes */
int SDCard_Init(void);
void SPI_Init(void);
void delay(int count);
void delay_seconds(int count);
unsigned char SPI_Write(unsigned char c);
int SDCard_SendCommand(unsigned char command, unsigned int addr, int num_response, unsigned char crc);
int SDCard_WriteSector(unsigned int addr, char* buffer);
int SDCard_ReadSector(unsigned int addr, char* buffer);

/* Pin connections */
#define SD_WRITE_PROTECT _RG1
#define SD_CARD_DETECT   _RG0
#define SD_CARD_SELECT   _RB9

/* Easy macros */
#define SDCard_Disable() SD_CARD_SELECT = 1; SPI_Clock()
#define SDCard_Enable()  SD_CARD_SELECT = 0

/* Read from SPI device by shifting out dummy 0xFF */
#define SPI_Read()  SPI_Write(0xFF)

/* Clock SPI device by shifting out dummy 0xFF */
#define SPI_Clock() SPI_Write(0xFF)

/* SD Card commands */
/* RESET = CMD0, INIT = CMD1 */
#define CMD0   0
#define CMD1   1
#define CMD8   8
#define CMD17  17
#define CMD24  24
#define CMD55  55
#define ACMD41 41

/* Initialize response time - SDCard size dependent */
#define INIT_TIMER  10000
#define READ_TIMER  10000
#define WRITE_TIMER 10000

/* Error codes */
#define ERROR_RESET 101
#define ERROR_INIT  102
#define ERROR_VLTG  103
#define ERROR_READ  104
#define ERROR_WRITE 105

/* More defines */
#define START_TOKEN  0xFE
#define ACCEPT_TOKEN 0x05
#define SECTOR_SIZE  512

/* Response types */
#define RESP_RA1 1
#define RESP_RA7 5

/* Delay */
#define ONE_SECOND 320000

#ifdef	__cplusplus
}
#endif

#endif	/* LAB3_H */
