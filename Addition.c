 // ����������� � ����� ��� �������� ������ �������� ��������� �����
 static uint16_t  Program_flags;
 #define INTERUPT1_ON 0x0001
 #define INTERUPT2_ON 0x0002
 #define ADC1_ON 0x0004
 #define ADC2_ON 0x0008
 #define ADCS_check ADC1_ON | ADC2_ON
 #define Sending_ON 0x0100

 static uint16_t State_of_PortC
 // �������� ����� ������ ���� ��������� � ������������ � ��������� ��������, ����������� �� ���� !!!
 #define Si1_input 0x0001
 #define Si2_input 0x0002
 #define BIT_OF_INTERUPT1 0x0100
 #define BIT_OF_INTERUPT2 0x0200
 #define
 #define
 #define
 #define
 #define
 #define
 #define
 #define
 #define  Sending_Delay ______ // �������� ���������� ����������� ����� ������������ ������
 #define  Buffer_Size 16


 uint32_t Si_result, Si_Buffer[Buffer_Size];
 uint16_t Put_index, Get_index;
 uint8_t  Hello_text[20];
 uint32_t Digital_test[Buffer_Size]
 uint8_t *P_current,*P_Last;
// ===================================================================
 inline void Put_to_circular_Buffer (uint32_t Si_result) {
   Si_Buffer[Put_index]= Si_result; Put_index++; Put_index &= 0x000f;
}
// ===================================================================
// ������ �������� �������� ������� ������
void Srart_Uart_sending(uint8_t * pData,NData){
   P_current=pData;
   P_Last=P_current+ NData;
   _____________ // ����������� ����� �� �������� !!!
   Program_flags |= Sending_ON;
   UART_SendData (MDR_UART1,*P_current);
   P_current++;
}
// ===================================================================
// �������� ���������� ����� ��� ���������� �������� ��������
void Next_Uart_sending(void){
		/* Check TXFE flag*/
   if(UART_GetFlagStatus (MDR_UART1, UART_FLAG_TXFE)!= SET) return;
   if(P_current<P_Last){
        UART_SendData (MDR_UART1,*P_current);
        P_current++;
        Delay_timer=0;
   } else {
        if( Delay_timer< Sending_Delay ) return;
        _____________ // ����������� ����� �� ����� !!!
        Program_flags &= ~Sending_ON;
   }
}
// ===================================================================
 void Command_Handler(uint8_t DataByte){
  switch (DataByte) {
        case 't':   Srart_Uart_sending(Hello_text,20);
        	    break;
        case 'k':   Srart_Uart_sending((uint8_t *)Si_Buffer,Buffer_Size*4);
        	    break;
        case 'z':   Srart_Uart_sending((uint8_t *)Digital_test,Buffer_Size);
        	    break;
        }
 }

// ===================================================================
 int main (void)
{
/*

����� ��������� ������������� ���� ��������� ��������� � �.�.

*/
   uint32_t tmp ;
   Put_index=0; Get_index=0;
   for(int i=0; i<Buffer_Size; i++) {Si_Buffer[i]=0; Digital_test[i]=i;}
   Hello_text[0]=' ';
   Hello_text[1]='H';
   Hello_text[2]='e';
   Hello_text[3]='l';
   Hello_text[4]='l';
   Hello_text[5]='o';
   Hello_text[6]=' ';
   Hello_text[7]='I';
   Hello_text[8]='t';
   Hello_text[8]=' ';
   Hello_text[10]='i';
   Hello_text[11]='s';
   Hello_text[12]=' ';
   Hello_text[13]='t';
   Hello_text[14]='e';
   Hello_text[15]='s';
   Hello_text[16]='t';
   Hello_text[17]=0;

     while (1)
     {
        // ========== ��������� ������ ����������������� ���������� ==========
        if(Program_flags & ADC1_ON){
          if(ADC_GetFlagStatus(ADC1_FLAG_END_OF_CONVERSION)){
            tmp = ADC1_GetResult();
            Si_result |=  tmp & 0x00000fff;
            Program_flags &= ~ADC1_ON; //  ������� �������
          }
          if(!(Program_flags & ADCS_check)){ // ��� �� ����������� ������ ���
            Put_to_circular_Buffer (Si_result);
            Program_flags &= ~INTERUPT1_ON;  //  ������� �������
          }
        }
        if(Program_flags & ADC2_ON){
          if(ADC_GetFlagStatus(ADC2_FLAG_END_OF_CONVERSION)){
            tmp = ADC2_GetResult();
            Si_result |=  (tmp << 16) & 0x0fff0000;
            Program_flags &= ~ADC2_ON; //  ������� �������
          }
          if(!(Program_flags & ADCS_check)){ // ��� �� ����������� ������ ���
            Put_to_circular_Buffer (Si_result);
            Program_flags &= ~INTERUPT1_ON;  //  ������� �������
          }
        }
        if((!(Program_flags & INTERUPT1_ON)) &     // ����� ��� ��������, ��������� � ����������� ���������
           (!(State_of_PortC & BIT_OF_INTERUPT1)))// � ������ ���������� ���� ����������
                NVIC_EnableIRQ(EXT_INT1_IRQn);   // B������� ���������� 1
                // �������� ��������, ��� ���������� ��� �� ��������
        // ===== ����� ��������� ������ ����������������� ���������� ======
        // ===== ��������� ��������� �� UART ����� = ===============
        if ((PORT_ReadInputDataBit(MDR_PORTE,PORT_Pin_3)) &
             UART_GetFlagStatus (MDR_UART1, UART_FLAG_RXFF)== SET )
           Command_Handler(UART_ReceiveData (MDR_UART1));
        // ====== �������� ���������� ����� �� UART  ===============
        if(Program_flags & Sending_ON)   Next_Uart_sending();

     }  // End  while (1)
}


/*******************************************************************************
* Function Name  : EXT_INT1_IRQHandler
* Description    : This function handles EXT_INT1 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXT_INT1_IRQHandler(void)
{
	NVIC_DisableIRQ(EXT_INT1_IRQn);  // ��������� ���������� 1
        State_of_PortC =                // Read Port C
        if(State_of_PortC & Si1_input){
          ADC1_Start(); // ��������� ���1
          Program_flags |= ADC1_ON; //  ������������� �������
          Count_Si1++;
        }
        if(State_of_PortC & Si2_input){
          ADC2_Start(); // ��������� ���2
          Program_flags |= ADC2_ON; //  ������������� �������
          Count_Si2++;
        }
        if(Program_flags & ADCS_check){
          Si_result= (ADCS_check <<28) & 0xf0000000; // ��������� ��� ����� ���
          Program_flags |= INTERUPT1_ON; //  ������������� �������
        }
        Count_Interrupts_1 ++;
}
// ===================================================================




