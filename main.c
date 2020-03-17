/*The project "Respondent". Real device.
Association "EVT".
Tasks in this version: Light indicators*/
#include "1886ve4d.h"
#include "int17xxx.h"
#pragma origin 0x8

typedef unsigned char uc;

const uc Translate_num_to_LED[10] = {
//  0,	  1,	2,	  3,	4,	  5,	6,	  7,	8,	  9.
	0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90};

bit flag_manual_auto;
bit flag_mode_ampl;
bit flag_msg_received;
bit flag_rw; // 0 read, 1 write
bit flag_send_mode;

uc LED [5];
uc a, b, c, d;	// Cells for receiving messages
uc count_receive_data;
uc error_code;
uc error_code_interrupt;
int led_active;
uc led_count;
uc mode;

interrupt iServer(void)
{
    multi_interrupt_entry_and_save

    PERIPHERAL_service:
        PEIF = 0;
        interrupt_exit_and_restore
    TMR0_service:
    	// save on demand: PRODL,PRODH,TBLPTRH,TBLPTRL,FSR0,FSR1
    	/* process Timer 0 interrupt */
    	// T0IF is automatically cleared when the CPU vectors to 0x10
    	// restore on demand: PRODL,PRODH,TBLPTRH,TBLPTRL,FSR0,FSR1
        T0IF = 0;
        interrupt_exit_and_restore
    T0CKI_service:
        T0CKIF = 0;
        interrupt_exit_and_restore
    INT_service:
        INTF = 0;
        interrupt_exit_and_restore
}
/******************/
#include "math24.h"
#include "Functions.h"

void main(void)
{
	Reg_Start_up();
	
	int d_line = 0;	
	bit flag_first_launch = 1;
    uc led_blink = 0;
    uc led_blink_temp = 0;
    uc led_blink_frq = 0x08;
    uc temp = 0;
    
    uc mode_temp = 255, mode_time = 0;
    uc buttons = 0, buttons_time = 0; 
    
    uc E_time = 0;
    bit E_part = 0;
    
    for (temp = 0; temp < 5; temp ++)
    	LED[temp] = temp;
    	
    
	while (1)
	{
		clrwdt();
		// PORT D --------------------------------------------------------------
		temp = 0x80;
		temp = temp >> d_line; 
		//temp |= Show_ERROR ();
		temp |= 0x01;
		PORTC = 0xFF;
		PORTD = temp;
		
		// -------------------------
		for (temp = 0; temp < 5; temp ++) {};
		// -------------------------
		
		// PORT C --------------------------------------------------------------
		// Work with blinking, turning off, and setting values on the indicator.
		if (d_line == led_active)
		{
			led_blink_temp ++;
			if (led_blink_temp > 254)
			{
				led_blink_temp = 0;
				led_blink ++;
				if (led_blink > 254)		// Delay for blinking
					led_blink = 0;
			}
		}
		
		// Blinking. The frequency depends on led_blink_frq (= 0x08).
		if ((d_line == led_active) && (led_blink & led_blink_frq))
			temp = 0xFF; 
		// Turn off non-working indicators. Depends on the mode.
		else if (d_line > led_count)
			temp = 0xFF; 
		else
		{
			temp = LED[(int)d_line];// The order of indicators is determined 
									// here.
			temp = Translate_num_to_LED[(int)temp];
		}
		
		
		PORTC = temp;
		
		for (temp = 0; temp < 20; temp ++) {};
		
		// PORT E --------------------------------------------------------------
		
		DDRE = 0xF8;	// 7-3 Input
		PORTE = 0xF8;
		PORTC = 0xFF;	// That there would be no excess backlight
		for (temp = 0; temp < 10; temp ++) {};
		temp = PORTE ^ 0xF8;
		DDRE = 0;
		PORTE = 0;
		
		
		temp = (temp >> 3) & 0x1F;
		
		if((d_line & 0x01) && (temp > 0))	// Mode
		{
			temp = Get_port_e_in_ten(d_line, temp);
			
			if (mode != temp)
			{
				if(mode_temp == temp)
				{
					mode_time ++;
					if (mode_time > 20)
					{
						mode = temp;
						// flag_send_mode = 1;
						// flag_rw = 0; //Read
						Change_led_count (mode);
					}
				}
				else
				{
					mode = 255;		// Fuse
					flag_send_mode = 0;
					mode_temp = temp;
					mode_time = 0;
					led_active = 0;
					led_count = 0; 
					LED[0] = LED[1] = LED[2] = LED[3] = LED[4] = 0;
				}
			}	
		}
		else if ((d_line & 0x01) == 0)	//Buttons
		{
			if (temp == buttons)
			{
				if (buttons_time <= 50)	// A pressed key will work
					buttons_time ++;	// only once
				if ((buttons_time == 50) && buttons > 0)
					Btns_action (buttons);
			}
			else 
			{
				buttons_time = 0;
				buttons = temp;
			}
		}
		
		d_line ++;
		if (d_line > 4)
			d_line = 0;
	}
}
#include "Functions.c"
