/*The project "Respondent". Real device.
Association "EVT".*/

void Reg_Start_up ();

// Buttons and switch operation
void Btns_action (uc btn);
void Change_led_count (uc num);
void Input_and_Correction (uc up);
uc Get_port_e_in_ten(uc part, uc data);


// Messaging
void Send();
void Send_part(bit flag_first_launch);
uc Show_ERROR();
