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
	
	PIR1    = 0x00;	// Reset Peripheral Interrupt Requests
	PIE1    = 0x01;	//00 RCIE setting: USART receiver interrupt enable bit 
					// (there is data in the receiver buffer)
	T0STA   = 0x28;	// Switching on TMR0 (internal clock frequency, 1:16 pre-selector)
	// T0STA does not matter since interruptions not allowed
					// add if PA0/INT = 0  to interrupt
	//INTSTA  = 0x00;	//08 PEIE setting: interrupt off
	INTSTA  = 0x08;	// PEIE setting: peripheral interruptions is on
	
	TXSTA = 0x42;	// 0b01000010 9bit, asynchronous,
	RCSTA = 0xD0;	// 0b11010000 on port, 9bit, continuous reception, 
					// Receiver is off
	//SPBRG = 0x9B;	// 155
	SPBRG = 0x41;	// 64
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
	led_active = 0;	// The number of the selected indicator. 
					// 4 is the far left
    led_count = 3;
    mode = 255;
}


// Buttons and switch operation ------------------------------------------------
void Btns_action (uc btn)
{
	// Called in main - while - Button part
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
		led_active ++;
		if (led_active > led_count)
			led_active = 0;
	}
	else if (btn & 0x08)// RE6: Rihgt 0x40
	{
		if (led_active == 0)
			led_active = led_count + 1;
		led_active --;
	}
	else if (!flag_send_mode && (btn & 0x06))// Changing indicators is blocked when sending
	{
		if (btn & 0x04)// RE5: Up 0x20
			Input_and_Correction(1);
		else if (btn & 0x02)// RE4: Down 0x10
			Input_and_Correction(0);
	}
	else if (btn & 0x01)// RE5: Send 0x08
	{
		if(flag_send_mode == 0)
		{
			flag_send_mode = 1;
			flag_rw = 1; //Write
			
			mark = 1;
		}
		else // STOP sending
		{
			flag_send_mode = flag_rw = 0;
			CREN = 0;	// Receiver is off
			
			mark = 0;
		}
	}
	return;
}


void Input_and_Correction(uc up)
{
	//uc L[5] = {9, 9, 9, 9, 9}; // L - Limit
	uc L[5];
	L[0]=9;L[1]=9;L[2]=9;L[3]=9;L[4]=9;
	uc bottom = 0;
	uc step = 1;
	
	
	// Preparation
	if (mode == 0) // D == 19
	{
		L[1] = 1;
		L[0] = 9;
	}
	else if (mode == 1)// BN == 99999
	{ }
	else if (mode == 2) // V = A1999
	{
		L[4] = 1;	// it's mean letter "A"
		L[3] = 1;
	}
	else if (mode == 3)// OT == 99
	{
		if (led_active == 0)	 //0X
		{
			L[0] = 1;	// 0, 5, or 9
			if (L[1] > 4 && L[1] < 9)
				L[0] = 0;	// 50, 60, 70, 80
		}
		else if (led_active == 1)//X0
		{
			if (L[0] == 9)
				return;	// 99
				//bottom = 9;
			else if (L[0] == 5)
				L[1] = 4;	// 05, 15, 25, 35, 45
		}
	}
	else if (mode > 7 && mode < 10) // 8, 9
	{
		L[0] = 1;
	}
	else // PO, PK, PG
	{
		// 0x000
		L[3] = 4;
		if (LED[2] > 0 || (LED[1] == 9 && LED[0] > 5))
			L[3] = 3; 
		else if(mode == 6) // PG
			L[3] = 5;
		// 00x0x
		if (LED[3] > 3)	
		{
			L[2] = 0;
			L[0] = 5;
		}
	}
	
	
	// Addition or subtraction
	if (up)// RE5: Up 0x20
	{
		LED[led_active] = LED[led_active] + step;
		uc t = L[led_active];
		if (LED[led_active] > t)
			LED[led_active] = 0;
	}
	else // RE4: Down 0x10
	{
		if (LED[led_active] == 0)
		{
			uc t = L[led_active];
			LED[led_active] = t + 1;
		}
		LED[led_active] = LED[led_active] - step;
	}
	
	
	// Correction
	if (mode == 3 && led_active == 0)// OT == 99
	{
		if (LED[0] == 1) // jump up
		{
			if (LED[1] < 5)
				LED[0] = 5;
			else if (LED[1] == 9)
				LED[0] = 9;
		}
		else			// jump down
			LED[0] = 0;
	}
	else if (mode == 6 && led_active == 3) // PG
		if (LED[3] == 5)		// jump up
			LED[3] = 9;
		else if (LED[3] == 8)	// jump down
			LED[3] = 4;
}

void Change_led_count (uc num)
{
	// Called in main - while - Mode part
	if (num == 0 || num == 3)// D, OT
		led_count = 1;
	else if (num == 1 || num == 2)	// BN, V
		led_count = 4;
	else if (num > 7 && num < 10)	// 8, 9
		led_count = 0;
	else				// PX, AX
		led_count = 3;
}

// Translation of port E code from binary bit to more convenient, 10th
uc Get_port_e_in_ten(uc part, uc data)
{
	// Called in main - while - Mode part
	uc count = 0;
	uc i;
	
	if (part == 1)
		part = 5;
	else	// part == 3
		part = 0;
	
	for(i = 0; i < 5; i ++)
	{
		if (data & (0x10))
		{
			part += i;	
			count ++;
		}
		data = data << 1;
	}
	// Bug fix if 2 buttons are pressed adjacent to the mode
	if (count > 1)
		return mode;
	
	// Correction for parcel codes
	if( part > 5)		// 6, 7, 8 ==> 4, 5, 6
		part -= 2;
	else if (part > 3)	// 4, 5 ==> 8, 9
		part += 4;
		
	return part;
}


// Messaging -------------------------------------------------------------------
void Check_and_correct(uc num)
{
	// Called in Send
	clrwdt();
	// 10 = A, 11 = B, 15 = F
	if ((num == 7) || (num == 10) || (num == 11) || (num == 15))
	{
		error_code = 5; // Wrong mode
		return;
	}	
	else if (flag_rw == 0) // When reading, only the mode number is important
		return;
		
	int i = 0;
	int24 led_max = 2047;
	
	if (num == 0)
		led_max = 199;
	else if (num == 1)
		return;
		//led_max = 99999; // This is the maximum scoreboard
	else if (num == 2)
		led_max = 1999;
	else if (num == 3)
		led_max = 99;
	else if (num > 7 && num < 10)	// 8, 9
		led_max = 1;	
	// For 4-6 and 12-14 modes, the limit will remain 2047
	
	int24 led_real = 0;
	int24 factor = 1;
	int24 temp = 0;
	
	for (i = 0; i < 5; i ++)
	{
		int j = 0, j_max = (int)LED[i];
		temp = 0;
		//led_real += factor * temp;	compilator fail
		for (j = 0; j < j_max; j ++)
			temp += factor;
			
		led_real += temp;
		factor = factor * 10;
	}
	
	//If the limit is exceeded - the display will reset to the maximum value
	if (led_real > led_max)
	{
		error_code = 6;
		temp = 10000;
		for (i = 4; i >= 0; i --)
		{
			// I doubt it
			// LED[i] = (led max / (10000 / (10^i))) % 10`
			factor = led_max / temp;
			factor = factor % 10;
			LED[i] = factor;
			temp /= 10;
		}
	}
	return;
}

void Read_Msg()
{
	// Called in Send_part()
	clrwdt();
	
	error_code = error_code_interrupt;
	
	uc temp = a >> 4;
	
	// Because the amplitude mode is modified "thresholds"
	if (flag_mode_ampl == 1)
		temp -= 8;
	
	if (temp != mode)
		error_code = 1; // Parity error
	
	
	if (error_code == 0)
	{
		// Transfer of a parcel from Hex to Dec --------------------------------
		uc Rcv_numbers [5];
		
		Rcv_numbers[0] = Rcv_numbers[1] = 0;
		Rcv_numbers[2] = Rcv_numbers[3] = Rcv_numbers[4] = 0;
		
		// Package[1] - Package[3]
		if (temp == 8 || temp == 9)
			Rcv_numbers[0] = b & 0x01;
		else 
		{
			Rcv_numbers[0] = d & 0x0F;
			Rcv_numbers[1] = d >> 4;
			if (temp != 3)	// !Fuel remaining == 00
			{
				Rcv_numbers[2] = c & 0x0F;
				if (temp != 0) 	// !Distance == 000
				{
					Rcv_numbers[3] = c >> 4;
					if (temp == 1)	// Board number == 00000
						Rcv_numbers[4] = b & 0x0F;
				}
			}
		}
		
		// Compare or set new values -------------------------------------------
		for (temp = 0; temp < 5; temp ++)
		{
			if (error_code == 0)
			{
				uc temp2 = Rcv_numbers[temp];	// Bugs and features of 
												// the compiler
				if (flag_rw == 0)
					LED[temp] = temp2;
				else if (LED[temp] != temp2)
					error_code = 1; // Parity error
			}
		}
		// (error_code == 0) - Otherwise, the alarm signal will be 
		// replaced by a parity error

		if (b & 0x40)	// Alarm signal, 12 mode
			error_code = 3;
	}
	return ;//1;
}

void Send()
{
	
	// Call from Send_part()
	uc Package [4], temp = 0;
	
	mark = 2;
	
	clrwdt();
	a = b = c = d = 0;
	CREN = 1;				// Receiver ON
	count_receive_data = 0; // In case of loss of parcels, or line break
	flag_msg_received = 0;
	
	//Package [0]
	Package[0] = mode;
	
	
	// Amplitude mode ----------------------------------------------------------
	// Turn on amplitude mode
	if ((Package[0] > 3) && (Package[0] < 7))	// PO, PK, PG
	{
		if ((flag_mode_ampl == 0) && (LED[3] == 9))
			flag_mode_ampl = 1;
	}
	
	
	if (flag_mode_ampl == 1)
	{
		// Turn off amplitude mode
		if ((Package[0] < 4) || (Package[0] > 6))
			flag_mode_ampl = 0;
		// or modification of the parcel code for "amplitude mode"
		else
		{
			Package[0] += 8;// PO(0100) ==> AO(1100)
			flag_rw = 0;
		}
	}
	
	// The mode is greater than 13, or does 
	// not fit into the limits for the mode
	Check_and_correct(Package[0]);
	// Indicator values fixed
	if (error_code == 6) 
	{
		flag_msg_received = 1;
		return;
	}
	
	
	// Filling parcels ---------------------------------------------------------
	if (flag_rw == 0) // Read
		Package[1] = Package[2] = Package[3] = 0;
	else //Write
	{
		if (Package[0] == 8 || Package[0] == 9)
		{	
			Package[1] = LED[0];
			Package[2] = Package[3] = 0;
		}
		else
		{
			Package[1] = LED[4];
			Package[2] = (LED[3] << 4) | LED[2];
			Package[3] = (LED[1] << 4) | LED[0];
		}
		Package[1] |= 0x80; // Record message label
	}
	
	if (flag_manual_auto)
		Package[1] |= 0x20;
	
	Package[0] = (Package[0] << 4) | 0x0F;
	
	
	// Sending -----------------------------------------------------------------
	int i = 0, max = 4;
	temp = 0;
	uc temp2 = 0;
	//TXEN = 1;
	//mark = 0;
	
	while ((i < max) && (temp < 250))
	{
		if (TXIF == 1)	// TXREG is empty
		{
			bit parity = 0;
			uc t = Package[i];
			
			while (t)
			{
				if (t & 0x01)
					parity = !parity;
				t = t >> 1;
			}
			
			TX9D = parity;
			TXREG = Package[i];
			TXEN = 1; // Transmitter Turn On
			i++;
			temp = 0;
		}
		else
			temp2 ++;	// fuze
			if (temp2 == 255)
			{
				temp2 = 0;
				temp ++;
			}
	}
	
	// Check register TRMT to turn off the transmitter -------------------------
	temp = 0;
	while (temp < 250)
	{
		if (TRMT == 1)	// TSR is empty
		{
			TXEN = 0;	// Transmitter Turn Off
			temp = 251;
		}
		temp ++;
	}
	
	//mark = i + 3; // смещение 
	if (i != max) // Sent more or less
	{
		error_code = 4;
		//mark = 3;
	}
	//	error_code = 4; // Send Error
	
	return ; 
}

void Send_part(bit flag_first_launch)
{
	// Called in main - while - in the end
	static uc i;
	static uc j;
	
	clrwdt();
	
	j ++;
	if (j > 100)
	{
		j = 0;
		i ++;
	}
	
	if (((i == 0) && (j == 1)) || (flag_first_launch == 1))
	{
		Send();
		if (error_code == 6)  // Too large numbers on LED
		{
			error_code = 0;
			// Reset of dispatch without consequences
			i = j = 0;
			flag_send_mode = 0;
		}
	}
	else if ((i == 3) || ((flag_msg_received == 1) && (flag_mode_ampl == 0)))
	{
		i = j = 0;
		if (flag_msg_received == 1)
		{
			Read_Msg();
			flag_msg_received = 0;
			if ((flag_mode_ampl == 0) && (error_code == 0))
				flag_send_mode = 0;
		}
		else if (error_code != 4) // Send Error
			error_code = 2; // Line is broken
	}
}

uc Show_ERROR()
{
	// Called in main - while - at the begin
	static uc i;
	static uc j;
	uc work_led = 0x01;	// 0x01 work; 0x02 error
	clrwdt();
	
	j++;
	if (j == 255)
	{
		j = 0;
		i ++;
		if (i == 255)
			i = 0;		
	}
		
	if(error_code == 0)			// Work
		i = j = 0;
	else if (error_code == 1)	// Parity
	{
		if (i < 8)				// 0.5c
			work_led = 0x02; 
	}
	else if(error_code == 2)	// Line is broken
	{
		if (i & 0x10)//(i < 128)	//16 - 1c
			work_led = 0x02;
	}
	else if(error_code == 3)	// Alarm signal, 12 mode
		work_led = 0x01;
	else if(error_code == 4)	// Send Error - Sent more or less
	{		
		if (((i & 0x70) == 0x50) || ((i & 0x70) == 0x70))
			work_led = 0x02;
	}
	else if(error_code == 5)	// Wrong mode
	{
		uc tempi = (i >> 6);
		uc tempi2 = 0x10 >> tempi; // unsigned int
		if (i & tempi2)
			work_led = 0x02;
	}
	// (error_code == 6)  // Too large numbers on LED
	
	return work_led;
}
