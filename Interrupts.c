/*The project "Respondent". Association "EVT".*/


void Check_mail (uc mail, bit nine, bit ferr)
{
	// Called in Handler_receiver
	// Parity calculation
	while (mail)
	{
		if (mail & 0x01)
			nine = !nine;
		mail = mail >> 1;
	}
	
	// If the parity does not converge, or there are errors of buffer overflow 
	// or framing, reset the reception and notify about it
	if ((nine == 1) || (OERR || ferr))
	{
		if (OERR || ferr)
			mark = 1;
		error_code_interrupt = 1; // Parity error
		CREN = 0;	// Receiver is off
	}
}

void Handler_receiver ()
{
	// Called in main - Interruption part
	/*Reception of data in variables a, b, c, d*/
		
	if(count_receive_data == 0)
		error_code_interrupt = 0;
	
	// RCIF == USART receiver interrupt request flag
	while (RCIF)	
	{
		// RCIF = 0; // This is read only
		
		// The framing error bit and parity bet must be read before reading data
		bit mail_parity = RX9D;
		bit ferr = FERR;
		uc mail = RCREG;
				
		Check_mail (mail, mail_parity, ferr);
		if ((error_code_interrupt == 0) && (count_receive_data < 4))
		{
			if (count_receive_data == 0)
				a = mail;
			else if (count_receive_data == 1)
				b = mail;
			else if (count_receive_data == 2)
				c = mail;
			else
				d = mail;
			count_receive_data++;
		}
	}
	
	if ((error_code_interrupt > 0) || (count_receive_data > 3))
	{
		flag_msg_received = 1;
		CREN = 0;	//Receiver off
	}
	
	PEIF = 0;	// Флаг запроса периферийного прерывания
	//PIR1 = 0; // Not work. Only hardware
}
