/*The project "Respondent". Real device.
Association "EVT".*/

void Reg_Start_up ()
{
	// Call from main
	// Turning on the "Work" and "Alarm" indicators
	DDRD = 0;
	PORTD = 0x02;
	for (a = 0; a < 100; a ++) {};
	
	
	GLINTD = 1;		// Disable All Interrupts
	PORTE = 0x00;	// Getting button codes and modes
	DDRE  = 0x00;
	PORTC = 0x00;	// Numbers on the scoreboard cell
	DDRC  = 0x00;
	PORTD = 0x00;	// Power for indicator and button polling
	DDRD  = 0x00;
		
	PIR1    = 0x00;	// Reset Interrupt Request Flags
	PIE1    = 0x00;	//01 RCIE setting: USART receiver interrupt enable bit 
					// (there is data in the receiver buffer)
	T0STA   = 0x28;	// Switching on TMR0 (internal clock frequency, 1:16 pre-selector)
	// T0STA does not matter since interruptions not allowed
					// add if PA0/INT = 0  to interrupt
	INTSTA  = 0x00;	//08 PEIE setting: interrupt off
	
	TXSTA = 0x42;	// 0b01000010 9bit, asynchronous,
	RCSTA = 0xD0;	// 0b11010000 on port, 9bit, continuous reception
	SPBRG = 0x9B;	// 155
	USB_CTRL = 0x01;	// USB off
	
	GLINTD  = 0; // Reset All Interrupt Disable Bit
	
	//DDRE = 0xFC; 	// 0b11111100 Buttons * 5 and MANUAL/AUTO
	
	// Flags
	flag_manual_auto = 0;
	flag_mode_ampl = 0;	
    flag_msg_received = 0;	// Flag of received message
    flag_rw = 0;
    flag_send_mode = 0;		// Turn on to receive data (?)
    
    // Variables
	LED[0] = LED[1] = LED[2] = LED[3] = LED[4] = 0;
    a = b = c = d = 0;
    count_receive_data = 0;
    error_code = 0;
    error_code_interrupt = 0;
	led_active = 4;	// The number of the selected indicator. 
					// 4 is the far left
    led_count = 3;
    mode = 255;
}

void Btns_action (uc btn)
{
	uc temp = btn, count = 0;
	while(temp)
	{
		if (temp & 1)
			count ++;
		temp = temp >> 1;
	}
	// Will not work if none is pressed, or pressed more than 2 buttons
	if (count != 1)
		return;
		
	if (btn & 0x10)		// RE7: Left 0x80
	{
		if (led_active == 4 - led_count)
			led_active = 5;
		led_active --;
	}
	else if (btn & 0x08)// RE6: Rihgt 0x40
	{
		led_active ++;
		if (led_active > 4)
			led_active = 4 - led_count;
	}
	else if (btn & 0x04)// RE5: Up 0x20
	{
		LED[led_active] = LED[led_active] + 1;
		if (LED[led_active] > 9)
			LED[led_active] = 0;
	}
	else if (btn & 0x02)// RE4: Down 0x10
	{
		if (LED[led_active] == 0)
			LED[led_active] = 10;
		LED[led_active] = LED[led_active] - 1;
	}
	else if (btn & 0x01)// RE5: Send 0x08
	{
		if(flag_send_mode == 0)
		{
			/*flag_send_mode = 1;
			flag_rw = 1; //Write
			led_on_E = 0x01;
			*/
		}
		else // STOP sending
			flag_send_mode = flag_rw = 0;
	}
	return;
}
