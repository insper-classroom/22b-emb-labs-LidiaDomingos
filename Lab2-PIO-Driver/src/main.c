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

/*  Default pin configuration (no attribute). */
#define _PIO_DEFAULT             (0u << 0)
/*  The internal pin pull-up is active. */
#define _PIO_PULLUP              (1u << 0)
/*  The internal glitch filter is active. */
#define _PIO_DEGLITCH            (1u << 1)
/*  The internal debouncing filter is active. */
#define _PIO_DEBOUNCE            (1u << 3)


/**
 * \brief Set a high output level on all the PIOs defined in ul_mask.
 * This has no immediate effects on PIOs that are not output, but the PIO
 * controller will save the value if they are changed to outputs.
 *
 * \param p_pio Pointer to a PIO instance.
 * \param ul_mask Bitmask of one or more pin(s) to configure.
 */
void _pio_set(Pio *p_pio, const uint32_t ul_mask){
	p_pio->PIO_SODR = ul_mask;
}

/**
 * \brief Set a low output level on all the PIOs defined in ul_mask.
 * This has no immediate effects on PIOs that are not output, but the PIO
 * controller will save the value if they are changed to outputs.
 *
 * \param p_pio Pointer to a PIO instance.
 * \param ul_mask Bitmask of one or more pin(s) to configure.
 */
void _pio_clear(Pio *p_pio, const uint32_t ul_mask){
	p_pio->PIO_CODR = ul_mask;
}

/**
 * \brief Configure PIO internal pull-up.
 *
 * \param p_pio Pointer to a PIO instance.
 * \param ul_mask Bitmask of one or more pin(s) to configure.
 * \param ul_pull_up_enable Indicates if the pin(s) internal pull-up shall be
 * configured.
 */

void _pio_pull_up(Pio *p_pio, const uint32_t ul_mask, const uint32_t ul_pull_up_enable){
	if (ul_pull_up_enable) {
		p_pio->PIO_PUER = ul_mask;
		} else {
		p_pio->PIO_PUDR = ul_mask;
	}	
}

/**
 * \brief Configure one or more pin(s) or a PIO controller as inputs.
 * Optionally, the corresponding internal pull-up(s) and glitch filter(s) can
 * be enabled.
 *
 * \param p_pio Pointer to a PIO instance.
 * \param ul_mask Bitmask indicating which pin(s) to configure as input(s).
 * \param ul_attribute PIO attribute(s).
 */
void _pio_set_input(Pio *p_pio, const uint32_t ul_mask, const uint32_t ul_attribute){
	_pio_pull_up(p_pio, ul_mask, ul_attribute & _PIO_PULLUP);

	if (ul_attribute & (_PIO_DEGLITCH | _PIO_DEBOUNCE)) {
		p_pio->PIO_IFER = ul_mask;
		} else {
		p_pio->PIO_IFDR = ul_mask;
	}
}
/**
 * \brief Configure one or more pin(s) of a PIO controller as outputs, with
 * the given default value. Optionally, the multi-drive feature can be enabled
 * on the pin(s).
 *
 * \param p_pio Pointer to a PIO instance.
 * \param ul_mask Bitmask indicating which pin(s) to configure.
 * \param ul_default_level Default level on the pin(s).
 * \param ul_multidrive_enable Indicates if the pin(s) shall be configured as
 * open-drain.
 * \param ul_pull_up_enable Indicates if the pin shall have its pull-up
 * activated.
 */

void _pio_set_output(Pio *p_pio, const uint32_t ul_mask, const uint32_t ul_default_level, const uint32_t ul_multidrive_enable, const uint32_t ul_pull_up_enable){
	
	p_pio->PIO_OER = ul_mask;
	p_pio->PIO_PER = ul_mask;

	if (ul_multidrive_enable) {
		p_pio->PIO_MDER = ul_mask;
		} else {
		p_pio->PIO_MDDR = ul_mask;
	}

	if (ul_default_level) {
		_pio_set(p_pio, ul_mask);
		} else {
		_pio_clear(p_pio, ul_mask);
	}
	
	_pio_pull_up(p_pio, ul_mask, ul_pull_up_enable);
}

uint32_t _pio_get(Pio *p_pio, const pio_type_t ul_type, const uint32_t ul_mask){
	uint32_t resultado;

	if ((ul_type == PIO_OUTPUT_0)) {
		resultado = p_pio->PIO_ODSR;
		}
		
	else if ((ul_type == PIO_INPUT ))	 {
		resultado = p_pio->PIO_PDSR;
	}

	if ((resultado & ul_mask) == 0) {
		return 0;
		} else {
		return 1;
	}
}

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
	_pio_set_output(LED_PIO1, LED_PIO_IDX_MASK1, 0, 0, 0);
	_pio_set_output(LED_PIO2, LED_PIO_IDX_MASK2, 0, 0, 0);
	_pio_set_output(LED_PIO3, LED_PIO_IDX_MASK3, 0, 0, 0);


	// Ativa o PIO na qual o BOTÃO foi conectado
	// para que possamos controlar o BOTÃO.
	pmc_enable_periph_clk(BUT_PIO_ID1);
	pmc_enable_periph_clk(BUT_PIO_ID2);
	pmc_enable_periph_clk(BUT_PIO_ID3);

	_pio_set_input(BUT_PIO1, BUT_PIO_IDX_MASK1, _PIO_PULLUP | _PIO_DEBOUNCE);
	_pio_set_input(BUT_PIO2, BUT_PIO_IDX_MASK2, _PIO_PULLUP | _PIO_DEBOUNCE);
	_pio_set_input(BUT_PIO3, BUT_PIO_IDX_MASK3, _PIO_PULLUP | _PIO_DEBOUNCE);

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
	if (_pio_get(BUT_PIO1, PIO_INPUT, BUT_PIO_IDX_MASK1) == 0){
		for (int i = 0;i<25;i++){                      
			_pio_clear(LED_PIO1, LED_PIO_IDX_MASK1);    // Coloca 0 no pino do LED
			delay_ms(100);
			_pio_set(LED_PIO1, LED_PIO_IDX_MASK1);      // Coloca 1 no pino LED
			delay_ms(100);
		}
	}
	else if (_pio_get(BUT_PIO2, PIO_INPUT, BUT_PIO_IDX_MASK2) == 0 ){
		for (int i = 0;i<25;i++){
			_pio_clear(LED_PIO2, LED_PIO_IDX_MASK2);    // Coloca 0 no pino do LED
			delay_ms(100);
			_pio_set(LED_PIO2, LED_PIO_IDX_MASK2);      // Coloca 1 no pino LED
			delay_ms(100);
		}
	}
	else if (_pio_get(BUT_PIO3, PIO_INPUT, BUT_PIO_IDX_MASK3) == 0 ){
		for (int i = 0;i<25;i++){
			_pio_clear(LED_PIO3, LED_PIO_IDX_MASK3);    // Coloca 0 no pino do LED
			delay_ms(100);
			_pio_set(LED_PIO3, LED_PIO_IDX_MASK3);      // Coloca 1 no pino LED
			delay_ms(100);
		}
	}
	else  {
		// Ativa o pino LED_IDX (par apagar)
		_pio_set(LED_PIO1, LED_PIO_IDX_MASK1);
		_pio_set(LED_PIO2, LED_PIO_IDX_MASK2);
		_pio_set(LED_PIO3, LED_PIO_IDX_MASK3);
	}
	
  }
  return 0;
}
