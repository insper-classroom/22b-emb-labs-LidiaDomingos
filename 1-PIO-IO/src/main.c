/**
 * 5 semestre - Eng. da Computação - Insper
 * Rafael Corsi - rafael.corsi@insper.edu.br
 *
 * Projeto 0 para a placa SAME70-XPLD
 *
 * Objetivo :
 *  - Introduzir ASF e HAL
 *  - Configuracao de clock
 *  - Configuracao pino In/Out
 *
 * Material :
 *  - Kit: ATMEL SAME70-XPLD - ARM CORTEX M7
 */

/************************************************************************/
/* includes                                                             */
/************************************************************************/

#include "asf.h"

/************************************************************************/
/* defines                                                              */
/************************************************************************/

#define LED_PIO1           PIOA                 // periferico que controla o LED
#define LED_PIO_ID1        ID_PIOA                  // ID do periférico PIOC (controla LED)
#define LED_PIO_IDX1       0                    // ID do LED no PIO
#define LED_PIO_IDX_MASK1  (1 << LED_PIO_IDX1) 

#define LED_PIO2           PIOC                 // periferico que controla o LED
#define LED_PIO_ID2        ID_PIOC                  // ID do periférico PIOC (controla LED)
#define LED_PIO_IDX2       30                    // ID do LED no PIO
#define LED_PIO_IDX_MASK2  (1 << LED_PIO_IDX2)

#define LED_PIO3           PIOB                 // periferico que controla o LED
#define LED_PIO_ID3        ID_PIOB                  // ID do periférico PIOC (controla LED)
#define LED_PIO_IDX3       2                    // ID do LED no PIO
#define LED_PIO_IDX_MASK3  (1 << LED_PIO_IDX3)

  // Mascara para CONTROLARMOS o LED

// Configuracoes do botao
#define BUT_PIO1           PIOD
#define BUT_PIO_ID1        ID_PIOD
#define BUT_PIO_IDX1       28
#define BUT_PIO_IDX_MASK1 (1u << BUT_PIO_IDX1) // esse já está pronto.

#define BUT_PIO2           PIOC
#define BUT_PIO_ID2        ID_PIOC
#define BUT_PIO_IDX2       31
#define BUT_PIO_IDX_MASK2 (1u << BUT_PIO_IDX2) // esse já está pronto.

#define BUT_PIO3           PIOA
#define BUT_PIO_ID3        ID_PIOA
#define BUT_PIO_IDX3       19
#define BUT_PIO_IDX_MASK3 (1u << BUT_PIO_IDX3) // esse já está pronto.




/************************************************************************/
/* constants                                                            */
/************************************************************************/

/************************************************************************/
/* variaveis globais                                                    */
/************************************************************************/

/************************************************************************/
/* prototypes                                                           */
/************************************************************************/

void init(void);

/************************************************************************/
/* interrupcoes                                                         */
/************************************************************************/

/************************************************************************/
/* funcoes                                                              */
/************************************************************************/

// Função de inicialização do uC
void init(void)
{ 
	// Initialize the board clock
	sysclk_init();

	// Desativa WatchDog Timer
	WDT->WDT_MR = WDT_MR_WDDIS;
	
	// Ativa o PIO na qual o LED foi conectado
	// para que possamos controlar o LED.
	pmc_enable_periph_clk(LED_PIO_ID1);
	pmc_enable_periph_clk(LED_PIO_ID2);
	pmc_enable_periph_clk(LED_PIO_ID3);
	
	//Inicializa PC8 como saída
	pio_set_output(LED_PIO1, LED_PIO_IDX_MASK1, 0, 0, 0);
	pio_set_output(LED_PIO2, LED_PIO_IDX_MASK2, 0, 1, 1);
	pio_set_output(LED_PIO3, LED_PIO_IDX_MASK3, 0, 2, 2);


	// Ativa o PIO na qual o BOTÃO foi conectado
	// para que possamos controlar o BOTÃO.
	pmc_enable_periph_clk(BUT_PIO_ID1);
	pmc_enable_periph_clk(BUT_PIO_ID2);
	pmc_enable_periph_clk(BUT_PIO_ID3);

	pio_set_input(BUT_PIO1, BUT_PIO_IDX_MASK1, PIO_DEFAULT);
	pio_set_input(BUT_PIO2, BUT_PIO_IDX_MASK2, PIO_DEFAULT);
	pio_set_input(BUT_PIO3, BUT_PIO_IDX_MASK3, PIO_DEFAULT);
	
	pio_pull_up(BUT_PIO1,BUT_PIO_IDX_MASK1,0);
	pio_pull_up(BUT_PIO2,BUT_PIO_IDX_MASK2,1);
	pio_pull_up(BUT_PIO3,BUT_PIO_IDX_MASK3,2);

}

/************************************************************************/
/* Main                                                                 */
/************************************************************************/

// Funcao principal chamada na inicalizacao do uC.
int main(void)
{
  init();

  // super loop
  // aplicacoes embarcadas não devem sair do while(1).
  while (1)
  {
	if (pio_get(BUT_PIO1, PIO_INPUT, BUT_PIO_IDX_MASK1) == 0){
		for (int i = 0;i<25;i++){                      
			pio_clear(LED_PIO1, LED_PIO_IDX_MASK1);    // Coloca 0 no pino do LED
			delay_ms(100);
			pio_set(LED_PIO1, LED_PIO_IDX_MASK1);      // Coloca 1 no pino LED
			delay_ms(100);
		}
	}
	else if (pio_get(BUT_PIO2, PIO_INPUT, BUT_PIO_IDX_MASK2) == 0 ){
		for (int i = 0;i<25;i++){
			pio_clear(LED_PIO2, LED_PIO_IDX_MASK2);    // Coloca 0 no pino do LED
			delay_ms(100);
			pio_set(LED_PIO2, LED_PIO_IDX_MASK2);      // Coloca 1 no pino LED
			delay_ms(100);
		}
	}
	else if (pio_get(BUT_PIO3, PIO_INPUT, BUT_PIO_IDX_MASK3) == 0 ){
		for (int i = 0;i<25;i++){
			pio_clear(LED_PIO3, LED_PIO_IDX_MASK3);    // Coloca 0 no pino do LED
			delay_ms(100);
			pio_set(LED_PIO3, LED_PIO_IDX_MASK3);      // Coloca 1 no pino LED
			delay_ms(100);
		}
	}
	else  {
		// Ativa o pino LED_IDX (par apagar)
		pio_set(LED_PIO1, LED_PIO_IDX_MASK1);
		pio_set(LED_PIO2, LED_PIO_IDX_MASK2);
		pio_set(LED_PIO3, LED_PIO_IDX_MASK3);
	}
	
  }
  return 0;
}
