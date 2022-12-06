#include <asf.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

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

#define LED_PIO1           PIOC                 // periferico que controla o LED
#define LED_PIO_ID1        ID_PIOC                  // ID do periférico PIOC (controla LED)
#define LED_PIO_IDX1       8                    // ID do LED no PIO
#define LED_PIO_IDX_MASK1  (1 << LED_PIO_IDX1) 

/*  Default pin configuration (no attribute). */
#define PIO_DEFAULT             (0u << 0)
/*  The internal pin pull-up is active. */
#define PIO_PULLUP              (1u << 0)
/*  The internal glitch filter is active. */
#define PIO_DEGLITCH            (1u << 1)
/*  The internal debouncing filter is active. */
#define PIO_DEBOUNCE            (1u << 3)

volatile char but_flag;
volatile char but_interrupt;
volatile char but_flag_diminui;
int delay = 100;
char str[128];

void init(void);
void pisca_led(int n, int freq);

void pisca_led(int n, int freq){
	for (int i = 0; i < n; i++){
		pio_clear(LED_PIO1, LED_PIO_IDX_MASK1);
		delay_ms(freq);
		pio_set(LED_PIO1, LED_PIO_IDX_MASK1);
		delay_ms(freq);
	}
}

void but_callback(void){
	if (pio_get(BUT_PIO1, PIO_INPUT, BUT_PIO_IDX_MASK1)) {
		but_flag = 0;
	}
	else{
		but_flag = 1;
	}
}

void but_callback_para(void){ 
	if (but_interrupt == 0){
		but_interrupt = 1;
	}
	else{
		but_interrupt = 0;
	}

}
void but_callback_dim(void){
	but_flag_diminui = 1;
}

void init(void)
{

	// Configura led
	pmc_enable_periph_clk(LED_PIO_ID1);
	pio_set_output(LED_PIO1, LED_PIO_IDX_MASK1, 0, 0, 0);

	// Inicializa clock do periférico PIO responsavel pelo botao
	pmc_enable_periph_clk(BUT_PIO_ID1);
	pmc_enable_periph_clk(BUT_PIO_ID2);
	pmc_enable_periph_clk(BUT_PIO_ID3);

	// Configura PIO para lidar com o pino do botão como entrada
	// com pull-up
	pio_set_input(BUT_PIO1, BUT_PIO_IDX_MASK1, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_input(BUT_PIO2, BUT_PIO_IDX_MASK2, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_input(BUT_PIO3, BUT_PIO_IDX_MASK3, PIO_PULLUP | PIO_DEBOUNCE);

	pio_set_debounce_filter(BUT_PIO1, BUT_PIO_IDX_MASK1, 60);
	pio_set_debounce_filter(BUT_PIO2, BUT_PIO_IDX_MASK2, 60);
	pio_set_debounce_filter(BUT_PIO3, BUT_PIO_IDX_MASK3, 60);

	pio_handler_set(BUT_PIO1, BUT_PIO_ID1, BUT_PIO_IDX_MASK1, PIO_IT_EDGE, but_callback);
	pio_handler_set(BUT_PIO2, BUT_PIO_ID2, BUT_PIO_IDX_MASK2, PIO_IT_FALL_EDGE, but_callback_para);
	pio_handler_set(BUT_PIO3, BUT_PIO_ID3, BUT_PIO_IDX_MASK3, PIO_IT_FALL_EDGE, but_callback_dim);

	pio_enable_interrupt(BUT_PIO1, BUT_PIO_IDX_MASK1);
	pio_get_interrupt_status(BUT_PIO1);
	pio_enable_interrupt(BUT_PIO2, BUT_PIO_IDX_MASK2);
	pio_get_interrupt_status(BUT_PIO2);
	pio_enable_interrupt(BUT_PIO3, BUT_PIO_IDX_MASK3);
	pio_get_interrupt_status(BUT_PIO3);


	NVIC_EnableIRQ(BUT_PIO_ID1);
	NVIC_SetPriority(BUT_PIO_ID1, 4);
	NVIC_EnableIRQ(BUT_PIO_ID2);
	NVIC_SetPriority(BUT_PIO_ID2, 0);
	NVIC_EnableIRQ(BUT_PIO_ID3);
	NVIC_SetPriority(BUT_PIO_ID3, 3);

}
int main (void)
{
	board_init();
	sysclk_init();
	delay_init();
	WDT->WDT_MR = WDT_MR_WDDIS;
	init();
	
  // Init OLED
	gfx_mono_ssd1306_init();
	sprintf(str, "%d", delay);
	gfx_mono_draw_string("freq = ", 0, 16, &sysfont);

  /* Insert application code here, after the board has been initialized. */
	while(1) {
		if (but_flag){
			delay_ms(300);
			if (but_flag){
				sprintf(str, "%d", delay);
				gfx_mono_draw_string(str, 64, 16, &sysfont);
				delay = delay + 100;
				sprintf(str, "%d", delay);
				gfx_mono_draw_string(str, 64, 16, &sysfont);
				but_flag = 0;
			}
			but_flag = 0;
			sprintf(str, "%d", delay);
			gfx_mono_draw_string(str, 64, 16, &sysfont);
			
		}
		if (but_flag_diminui){
			delay = delay - 100;
			but_flag_diminui = 0;
		}
		
		sprintf(str, "%d", delay);
		gfx_mono_draw_string(str, 64, 16, &sysfont);
		if (!but_flag && !but_interrupt){
			pisca_led(1, delay);
		}
	}

}
