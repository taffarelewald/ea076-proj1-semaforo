/* ===================================================================
**	Projeto 1: Semaforo
**  
**  Taffarel Cunha Ewald 147957
**  Daniel Rodrigues Silveira Freitas 145782
**        
**		Funcionamento do Programa:
**	1) Estado inicial: LED verde dos carros ON e LED vermelho dos pedestres ON.
**	Todos os outros LEDs OFF.
**	2) Acionamento do botão do pedestre: Espera 2 segundos, LED amarelo dos carros ON, 
**	LED vermelho dos pedestres ON. Todos os outros LEDs OFF. Estado permanece por 2 segundos.
**	3) LED vermelho dos carros ON, LED verde dos pedestres ON. Todos os outros LEDs OFF. 
**	Estado permanece por 2 segundos.
**	4) LED vermelho dos pedestres pisca por 2 segundos.
**	5) Volta para estado inicial.
**	6) A qualquer momento durante a execucao do programa, caso o sensor de luminosidade (LDR) indique 
**	um valor arbitrario, e escuro o suficiente, e assim permaneca por mais de 3 segundos, o LED amarelo
**	dos carros começa a piscar ate que a luminosidade seja reestabelecida. LED vermelho dos pedestres ON.
**	Todos os outros LEDs OFF.
** ===================================================================*/

/* MODULE Events */

#include "Cpu.h"	
#include "Events.h"

#ifdef __cplusplus
extern "C" {
#endif 

/*
 * Declaracao das variaveis globais 
 */
static int push_button = 1; /*push_button inicializa em 1 pois sua rotina de interrupcao 
                            eh executada no inicio do programa*/
int timer = 0;              //variavel para contagem do tempo
int night_timer = 0;		    //variavel para contagem do tempo de baixa luminosidade
int night_flag = 0;			    //flag indicadora de estado noturno
uint8_t LDR_value;			    //variavel de 8 bits para medicao ADC do sinal luminoso

/* User includes (#include below this line is not maintained by Processor Expert) */

/*
** ===================================================================
**     Event       :  Cpu_OnNMIINT (module Events)
**
**     Component   :  Cpu [MKL25Z128LK4]
*/
/*!
**     @brief
**         This event is called when the Non maskable interrupt had
**         occurred. This event is automatically enabled when the [NMI
**         interrupt] property is set to 'Enabled'.
*/
/* ===================================================================*/
void Cpu_OnNMIINT(void)
{
	
}

/*
 * Funcao de inicializacao do sistema
 */
void Status_init()
{
	LED_red_traffic_ClrVal(); 			//apaga LED vermelho dos carros
	LED_yellow_traffic_ClrVal();		//apaga LED amarelo dos carros
	LED_green_traffic_SetVal();			//acende LED verde dos carros
	LED_green_pedestrian_ClrVal();		//apaga LED verde dos pedestres
	LED_red_pedestrian_SetVal();		//acende LED vermelho dos pedestres
	push_button = 0;					//botao eh inicializado como nao-pressionado
	night_flag = 0;						//indica estado diurno
}

/*
** ===================================================================
**     Event       :  TI1_OnInterrupt (module Events)
**
**     Component   :  TI1 [TimerInt]
**     Description :
**         When a timer interrupt occurs this event is called (only
**         when the component is enabled - <Enable> and the events are
**         enabled - <EnableEvent>). This event is enabled only if a
**         <interrupt service/event> is enabled.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/

/*
 * Funcao que reinicia contagem de tempo
 */
void Clr_timer()
{
	timer = 0;
}

/*
 * Funcao de interrupcao temporal executada a cada 500ms
 */
void TI1_OnInterrupt(void)
{
	
	AD1_Measure(FALSE);		//chamada da rotina para conversao ADC do sinal luminoso
	
	/*
	 * São necessarias as seguintes condicoes para a evolucao dos estados:
	 * 1) O LED verde do pedestre deve estar apagado, o que demonstra a necessidade de atravessar a rua
	 * 2) O botao deve ter sido apertado pelo pedestre (push_button = 1)
	 * 3) Condicao de periodo diurno (LDR_value <= 200) 
	 * 
	 */
	if(push_button && !LED_green_pedestrian_GetVal() && LDR_value <= 200)
	{
		timer++;
		
		if(timer == 4)	//2 segundos de espera para apagar o LED verde e acender o amarelo dos carros
		{
			LED_green_traffic_ClrVal();
			LED_yellow_traffic_SetVal();			
		}
		if(timer == 8)  /* 
						         * 2 segundos apos o estado anterior, acende o LED vermelho dos carros e
			           	   * o LED verde dos pedestres, e apaga todos os outros
			               */
		{
			LED_yellow_traffic_ClrVal();
			LED_red_traffic_SetVal();
			LED_red_pedestrian_ClrVal();
			LED_green_pedestrian_SetVal();
			push_button = 0;
		}
	}
	if(LED_red_traffic_GetVal())    /*
	 	 	 	 	 	 	 	                 	 * Caso para comecar a piscar o LED vermelho do pedestre, indicando
	 	 	 	 	 	 	               	  	 * que seu semaforo vai fechar
	  						                   */
	{
		timer++;

		if(timer >= 12 && timer <= 16)	//periodo em que o LED vermelho do pedestre pisca
		{
			LED_green_pedestrian_ClrVal();
			LED_red_pedestrian_NegVal();
		}
		if(timer > 16)	//volta para o estado inicial
		{
			Clr_timer();
			Status_init();
		}
	}
}

/*
** ===================================================================
**     Event       :  AD1_OnEnd (module Events)
**
**     Component   :  AD1 [ADC]
**     Description :
**         This event is called after the measurement (which consists
**         of <1 or more conversions>) is/are finished.
**         The event is available only when the <Interrupt
**         service/event> property is enabled.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/

/*
 * Funcao de interrupcao executada apos conversao ADC
 */
void AD1_OnEnd(void)
{
	AD1_GetValue8(&LDR_value);	  //atribui a variavel de 8 bits o valor do sinal digital
	if(LDR_value > 200)			      //condicao de baixa luminosidade
	{
		night_timer++;
		/*
		 * Para que se estabeleca a condicao de periodo noturno, eh necessario que a baixa luminosidade
		 * permaneca por no minimo 3 segundos, impedindo que essa condicao seja provocada por perturbacoes.
		 * A partir dos 3 segundos, LED amarelo dos carros comeca a piscar
		 */
		if(night_timer > 6)
		{
			Clr_timer();
			night_flag = 1;
			LED_yellow_traffic_NegVal();
			LED_red_traffic_ClrVal();
			LED_green_traffic_ClrVal();
			LED_green_pedestrian_ClrVal();		
		} 
	}
	/*
	 * Quando o periodo diurno eh reestabelecido, a variavel night_timer eh reinicializada e
	 * a variavel night_flag estara setada, indicando que a noite acabou e portanto, deve-se
	 * retornar ao estado inicial
	 */
	else
	{
		night_timer = 0;
		if(night_flag == 1)
		{
			Status_init();
		}
	}
}

/*
** ===================================================================
**     Event       :  AD1_OnCalibrationEnd (module Events)
**
**     Component   :  AD1 [ADC]
**     Description :
**         This event is called when the calibration has been finished.
**         User should check if the calibration pass or fail by
**         Calibration status method./nThis event is enabled only if
**         the <Interrupt service/event> property is enabled.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void AD1_OnCalibrationEnd(void)
{
  /* Write your code here ... */
}

/*
** ===================================================================
**     Event       :  EInt1_OnInterrupt (module Events)
**
**     Component   :  EInt1 [ExtInt]
**     Description :
**         This event is called when an active signal edge/level has
**         occurred.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/

/*
 * Funcao de interrupcao executada pelo acionamento do push_button
 */
void EInt1_OnInterrupt(void)
{
	/*
	 * São necessarias as seguintes condicoes para negacao do valor do push_button:
	 * 1) A variavel de contagem de tempo deve estar reiniciada, indicando que a 
	 * rotina dos LEDs nao esta sendo executada
	 * 2) Condicao de periodo diurno (LDR_value <= 200) 
	 */
	if(timer == 0 && night_flag == 0)
	{
		if(push_button)
		{
			push_button = 0;
		} 
		else
		{
			push_button = 1;
		}
	}
}

/* END Events */

#ifdef __cplusplus
}  /* extern "C" */
#endif 

/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.3 [05.09]
**     for the Freescale Kinetis series of microcontrollers.
**
** ###################################################################
*/
