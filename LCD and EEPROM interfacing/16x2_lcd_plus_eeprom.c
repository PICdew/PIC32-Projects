/*******************************************************************
 * Author: xfrings
 *  
 * Created: 13-Feb-2015
 * 
 * HW: PIC18F4520 on PICDEM2 Plus board 
 * SW: I2C interface with EEPROM 24LC256
 *  
 *******************************************************************/
#include <p18f4520.h>
#include <i2c.h>

/* LCD Control pins */
#define E  PORTDbits.RD6
#define RW PORTDbits.RD5
#define RS PORTDbits.RD4

/* Prototypes */
void delay(int);
void init_serial(void);
void init_lcd(void);
void dump_to_lcd(void);
void send_to_lcd(int, int);
void wait_busy_lcd(void);
void refresh_lcd(void);
void set_position_lcd(int);

/* EEPROM Interface */
void init_i2c(void);
void write_to_eeprom(char, short int);
unsigned char HDByteWriteI2C( unsigned char ControlByte, unsigned char HighAdd, unsigned char LowAdd, unsigned char data );
unsigned char HDByteReadI2C( unsigned char ControlByte, unsigned char HighAdd, unsigned char LowAdd, unsigned char *data, unsigned char length );

/* Keep DDRAM write count */
int position = 0;

/* Main */
void main()
{
   unsigned short int count = 0;
   unsigned char data;

   /* Allow peripheral init */
   delay(1000);

   /* Initialize PIC USART to operate at 19200 baud */
   init_serial();

   /* Initialize I2C */
   init_i2c();

   /* Turn ON and setup up LCD */
   init_lcd();
   
   /* Loop reception and echo it back */
   receive_next:
  
   while(!PIR1bits.RCIF);
   /* Echo received character */
   data = RCREG;
   TXREG = data;
   
   /* Send the received byte to EEPROM */
   write_to_eeprom(data, count);

   count++;

   if(count == 16)
   {
      dump_to_lcd();
      count = 0;
   }
  
   goto receive_next;
   
   /* Shouldn't get here! */
   while(1);
}

/* Arbitrary delay routine */
void delay(int del)
{
   while(del >= 0) del--;
}


/* Write 16 chars to LCD */
void dump_to_lcd()
{
   unsigned short int i;
   unsigned char chr;

   for(i=0; i<16; i++)
   {
     /* Read from EEPROM */
     HDByteReadI2C(0xA0, 0x00, i, &chr, 0x01);
     
     /* Write to LCD */
     refresh_lcd();
     send_to_lcd(chr, 0);
     wait_busy_lcd();
   }
}


/* Initiialze USART */
void init_serial()
{
   /* Baud = 19200 => SPBRG = 12 @ 4MHz core when BRGH=1 (lower error rate)*/
   SPBRGH = 0x00;
   SPBRG = 0x0C;
   /* Manually configured 8-bit baud */
   BAUDCON = 0x0;

   /* RX and TX pins should be digital */ 
   ADCON1 = 0x0F;
   TRISC = 0xC0;

   /* Enable serial RX-TX - Async/8-bit/0-Parity/BRGH=1 */
   /* Turn on serial RX-TX */
   TXSTA = 0x26;
   RCSTA = 0x90;
}

/* Initialize I2C */
void init_i2c()
{
    SSPCON1 = 0x28; 
    SSPCON2 = 0x00;
    SSPSTATbits.SMP = 1;
    SSPADD = 0x09;

    TRISCbits.RC3 = 1;
    TRISCbits.RC4 = 1;
}


void write_to_eeprom(char chr, short int addr)
{
   HDByteWriteI2C(0xA0, 0x00, (char)addr, chr);
}

/* Initialize LCM HD44780 */
void init_lcd()
{
   int command;
   /* Configure PORTD to output */
   TRISD = 0x00;
  
   /* Enable LCD */
   PORTDbits.RD7 = 1;

   /* Wait for HD44780 to boot */
   delay(10000);

   /* Write 1 nibble - 4-bit interface */
   TRISD &= 0xF0; RW = 0; RS = 0;
   E = 1;
   command = PORTD & 0xF0; 
   command |= 0x02;
   PORTD = command; 
   Nop(); 
   E = 0; 
   delay(10000);

   /* Function set = 0x28 - 4-bit interface */
   command = 0x28;
   send_to_lcd(command, 1);
   wait_busy_lcd();

   /* Display set */
   command = 0x0D;
   send_to_lcd(command, 1);
   wait_busy_lcd();
   
   /* Entry mode set */
   command = 0x06;
   send_to_lcd(command, 1);
   wait_busy_lcd();

   /* Clear display */
   command = 0x01;
   send_to_lcd(command, 1);
   wait_busy_lcd();
}

void send_to_lcd(int data, int command)
{
   int temp = 0;
   /* Configure PORTD to output */
   TRISD &= 0xF0;

   /* Write */
   RW = 0;
    
   /* Command or data register */
   if(command == 1) 
     RS = 0;
   else
     RS = 1;

   /* Upper nibble */
   E = 1;
   temp = PORTD & 0xF0;
   temp |= (0x0F & (data >> 4));
   PORTD = temp;
   Nop(); 
   E = 0; 
   
   delay(3);
   
   /* Lower nibble */
   E = 1;
   temp = PORTD & 0xF0;
   temp |= (0x0F & data);
   PORTD = temp;
   Nop(); E = 0;

   delay(3);
}

void wait_busy_lcd()
{
   int busy = 0;
   /* Make PORTD input */
   TRISD |= 0x0F;

   /* Read */
   RW = 1;
   RS = 0;
   
   wait:
   /* Address nibble */
   E = 1; 
   Nop();
   E = 0;
   
   /* BF nibble */
   E = 1; 
   busy = PORTD; 
   E = 0;

   if(busy & 0x08) goto wait;
}

void refresh_lcd()
{
   position++;

   /* Second line */
   if(position == 17)
   {
     set_position_lcd(2);
   }
   /* First line */
   else if(position == 33)
   {
      position = 1;
      set_position_lcd(1); 
   }
}


void set_position_lcd(int line)
{
   int ddram;
   /* Set DDRAM address for 16x2 LCD */
   if(line == 1) ddram = 0x80;
   else ddram = 0xC0;

   send_to_lcd(ddram, 1);
   wait_busy_lcd();
}


/************************************************************************
*     Function Name:    HDByteWriteI2C                                  *   
*     Parameters:       EE memory ControlByte, address and data         *
*     Description:      Writes data one byte at a time to I2C EE        *
*                       device. This routine can be used for any I2C    *
*                       EE memory device, which only uses 1 byte of     *
*                       address data as in the 24LC01B/02B/04B/08B/16B. *
*                                                                       *     
************************************************************************/

unsigned char HDByteWriteI2C( unsigned char ControlByte, unsigned char HighAdd, unsigned char LowAdd, unsigned char data )
{
  IdleI2C();                      // ensure module is idle
  StartI2C();                     // initiate START condition
  while ( SSPCON2bits.SEN );      // wait until start condition is over 
  WriteI2C( ControlByte );        // write 1 byte - R/W bit should be 0
  IdleI2C();                      // ensure module is idle
  WriteI2C( HighAdd );            // write address byte to EEPROM
  IdleI2C();                      // ensure module is idle
  WriteI2C( LowAdd );             // write address byte to EEPROM
  IdleI2C();                      // ensure module is idle
  WriteI2C ( data );              // Write data byte to EEPROM
  IdleI2C();                      // ensure module is idle
  StopI2C();                      // send STOP condition
  while ( SSPCON2bits.PEN );      // wait until stop condition is over 
  while (EEAckPolling(ControlByte));  //Wait for write cycle to complete
  return ( 0 );                   // return with no error
}

/********************************************************************
*     Function Name:    HDByteReadI2C                               *
*     Parameters:       EE memory ControlByte, address, pointer and *
*                       length bytes.                               *
*     Description:      Reads data string from I2C EE memory        *
*                       device. This routine can be used for any I2C*
*                       EE memory device, which only uses 1 byte of *
*                       address data as in the 24LC01B/02B/04B/08B. *
*                                                                   *  
********************************************************************/

unsigned char HDByteReadI2C( unsigned char ControlByte, unsigned char HighAdd, unsigned char LowAdd, unsigned char *data, unsigned char length )
{
  IdleI2C();                      // ensure module is idle
  StartI2C();                     // initiate START condition
  while ( SSPCON2bits.SEN );      // wait until start condition is over 
  WriteI2C( ControlByte );        // write 1 byte 
  IdleI2C();                      // ensure module is idle
  WriteI2C( HighAdd );            // WRITE word address to EEPROM
  IdleI2C();                      // ensure module is idle
  while ( SSPCON2bits.RSEN );     // wait until re-start condition is over 
  WriteI2C( LowAdd );             // WRITE word address to EEPROM
  IdleI2C();                      // ensure module is idle
  RestartI2C();                   // generate I2C bus restart condition
  while ( SSPCON2bits.RSEN );     // wait until re-start condition is over 
  WriteI2C( ControlByte | 0x01 ); // WRITE 1 byte - R/W bit should be 1 for read
  IdleI2C();                      // ensure module is idle
  getsI2C( data, length );       // read in multiple bytes
  NotAckI2C();                    // send not ACK condition
  while ( SSPCON2bits.ACKEN );    // wait until ACK sequence is over 
  StopI2C();                      // send STOP condition
  while ( SSPCON2bits.PEN );      // wait until stop condition is over 
  return ( 0 );                   // return with no error
}
