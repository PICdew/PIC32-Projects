/*******************************************************************
 * Author: xfrings
 *  
 * Created: 29-Jan-2015
 * 
 * HW: PIC18F4520 on PICDEM2 Plus board 
 * SW: UART RX-TX with 16x2 LCD interface
 *  
 *******************************************************************/

#include <p18f4520.h>

/* LCD Control pins */
#define E  PORTDbits.RD6
#define RW PORTDbits.RD5
#define RS PORTDbits.RD4

/* Prototypes */
void delay(int);
void init_serial(void);
void init_lcd(void);
void send_to_lcd(int, int);
void wait_busy_lcd(void);
void refresh_lcd(void);
void set_position_lcd(int);

/* Keep DDRAM write count */
int position = 0;

/* Main */
void main()
{
   /* Allow peripheral init */
   delay(1000);

   /* Initialize PIC USART to operate at 19200 baud */
   init_serial();

   /* Turn ON and setup up LCD */
   init_lcd();
   
   /* Loop reception and echo it back */
   receive_next:
  
   while(!PIR1bits.RCIF);
   /* Echo received character */
   TXREG = RCREG;
   refresh_lcd();
   send_to_lcd(RCREG, 0);
   wait_busy_lcd();
   goto receive_next;
   
   /* Shouldn't get here! */
   while(1);
}

/* Arbitrary delay routine */
void delay(int del)
{
   while(del >= 0) del--;
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
