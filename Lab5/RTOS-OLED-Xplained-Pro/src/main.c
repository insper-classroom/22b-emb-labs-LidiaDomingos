#include <asf.h>
#include "conf_board.h"

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

/* Botao da placa */
#define BUT_PIO     PIOD
#define BUT_PIO_ID  ID_PIOD
#define BUT_PIO_PIN 28
#define BUT_PIO_PIN_MASK (1 << BUT_PIO_PIN)

#define ECHO_PIO     PIOB
#define ECHO_PIO_ID  ID_PIOB
#define ECHO_PIO_PIN 4
#define ECHO_PIO_PIN_MASK (1 << ECHO_PIO_PIN)

#define TRIG_PIO       PIOA
#define TRIG_PIO_ID    ID_PIOA
#define TRIG_IDX       4
#define TRIG_IDX_MASK  (1 << TRIG_IDX)

#define tmin 58*1e-6

/** RTOS  */
#define TASK_OLED_STACK_SIZE                (1024*6/sizeof(portSTACK_TYPE))
#define TASK_OLED_STACK_PRIORITY            (tskIDLE_PRIORITY)

QueueHandle_t xQueueFila1;
QueueHandle_t xQueueFila2;

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,  signed char *pcTaskName);
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);
extern void vApplicationMallocFailedHook(void);
extern void xPortSysTickHandler(void);

/** prototypes */
void but_callback(void);
void echo_callback(void);
void limpa_barra(void);
void io_init(void);
static void RTT_init(float freqPrescale, uint32_t IrqNPulses, uint32_t rttIRQSource);
/************************/
/* RTOS application funcs                                               */
/************************/

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName) {
	printf("stack overflow %x %s\r\n", pxTask, (portCHAR *)pcTaskName);
	for (;;) {	}
}

extern void vApplicationIdleHook(void) { }

extern void vApplicationTickHook(void) { }

extern void vApplicationMallocFailedHook(void) {
	configASSERT( ( volatile void * ) NULL );
}

/************************/
/* handlers / callbacks                                                 */
/************************/
void RTT_Handler(void) {
	uint32_t ul_status;
	ul_status = rtt_get_status(RTT);

	/* IRQ due to Alarm */
	if ((ul_status & RTT_SR_ALMS) == RTT_SR_ALMS) {
		printf("RTT estourou o tempo limite");
	}
}

void but_callback(void) {
	int x = 1;
	BaseType_t xHigherPriorityTaskWoken = pdTRUE;
	xQueueSendFromISR(xQueueFila1, &x, &xHigherPriorityTaskWoken);
}

void echo_callback(void) {
	if (pio_get(ECHO_PIO, PIO_INPUT, ECHO_PIO_PIN_MASK)) {
		RTT_init(1/(tmin*2), 0, 0);
		} else {
		uint32_t cont = rtt_read_timer_value(RTT);
		BaseType_t xHigherPriorityTaskWoken = pdTRUE;
		xQueueSendFromISR(xQueueFila2, &cont, &xHigherPriorityTaskWoken);
	}
}

void limpa_barra(){
	gfx_mono_draw_filled_rect(0, 0, 128,32, GFX_PIXEL_CLR);
}
/************************/
/* TASKS                                                                */
/************************/

static void task_oled(void *pvParameters) {
	gfx_mono_ssd1306_init();
	
	io_init();
	printf("task oled\n");
	
	int x;
	int i;
	for (;;)  {
		if (xQueueReceive(xQueueFila1, &x, 0)) {
			pio_set(TRIG_PIO, TRIG_IDX_MASK);
			delay_us(10);
			pio_clear(TRIG_PIO, TRIG_IDX_MASK);
		}
		
		if (xQueueReceive(xQueueFila2, &i, 0)) {
			float tempo = (tmin * 2) * i;
			float dist = (tempo * 340) / 2;
			char string[50];
			sprintf(string, "%.2f m", dist);
			limpa_barra();
			gfx_mono_draw_string(string, 0, 0, &sysfont);
		}

	}
}

/************************/
/* funcoes                                                              */
/************************/

void io_init(void) {
	
	pmc_enable_periph_clk(TRIG_PIO);
	pmc_enable_periph_clk(ECHO_PIO);
	pmc_enable_periph_clk(BUT_PIO);

	pio_configure(TRIG_PIO, PIO_OUTPUT_0, TRIG_IDX_MASK, PIO_PULLUP| PIO_DEBOUNCE);
	pio_configure(ECHO_PIO, PIO_INPUT, ECHO_PIO_PIN_MASK, PIO_DEFAULT);
	pio_configure(BUT_PIO, PIO_INPUT, BUT_PIO_PIN_MASK, PIO_PULLUP| PIO_DEBOUNCE);

	pio_handler_set(BUT_PIO, 
					BUT_PIO_ID, 
					BUT_PIO_PIN_MASK, 
					PIO_IT_FALL_EDGE,
					but_callback);
	
	pio_handler_set(ECHO_PIO, 
					ECHO_PIO_ID, 
					ECHO_PIO_PIN_MASK, 
					PIO_IT_EDGE,
					echo_callback);

	pio_enable_interrupt(TRIG_PIO, TRIG_IDX_MASK);
	pio_get_interrupt_status(TRIG_PIO);
	
	pio_enable_interrupt(ECHO_PIO, ECHO_PIO_PIN_MASK);
	pio_get_interrupt_status(ECHO_PIO);
	
	pio_enable_interrupt(BUT_PIO, BUT_PIO_PIN_MASK);
	pio_get_interrupt_status(BUT_PIO);

	NVIC_EnableIRQ(TRIG_PIO_ID);
	NVIC_EnableIRQ(ECHO_PIO_ID);
	NVIC_EnableIRQ(BUT_PIO_ID);
	
	NVIC_SetPriority(TRIG_PIO_ID, 4);
	NVIC_SetPriority(ECHO_PIO_ID, 4);
	NVIC_SetPriority(BUT_PIO_ID, 4);
}

static void configure_console(void) {
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
		.charlength = CONF_UART_CHAR_LENGTH,
		.paritytype = CONF_UART_PARITY,
		.stopbits = CONF_UART_STOP_BITS,
	};

	/* Configure console UART. */
	stdio_serial_init(CONF_UART, &uart_serial_options);

	/* Specify that stdout should not be buffered. */
	setbuf(stdout, NULL);
}

static void RTT_init(float freqPrescale, uint32_t IrqNPulses, uint32_t rttIRQSource) {

	uint16_t pllPreScale = (int) (((float) 32768) / freqPrescale);
	
	rtt_sel_source(RTT, false);
	rtt_init(RTT, pllPreScale);
	
	if (rttIRQSource & RTT_MR_ALMIEN) {
		uint32_t ul_previous_time;
		ul_previous_time = rtt_read_timer_value(RTT);
		while (ul_previous_time == rtt_read_timer_value(RTT));
		rtt_write_alarm_time(RTT, IrqNPulses+ul_previous_time);
	}

	/* config NVIC */
	NVIC_DisableIRQ(RTT_IRQn);
	NVIC_ClearPendingIRQ(RTT_IRQn);
	NVIC_SetPriority(RTT_IRQn, 4);
	NVIC_EnableIRQ(RTT_IRQn);

	/* Enable RTT interrupt */
	if (rttIRQSource & (RTT_MR_RTTINCIEN | RTT_MR_ALMIEN))
	rtt_enable_interrupt(RTT, rttIRQSource);
	else
	rtt_disable_interrupt(RTT, RTT_MR_RTTINCIEN | RTT_MR_ALMIEN);
	
}

/************************/
/* main                                                                 */
/************************/


int main(void) {
	/* Initialize the SAM system */
	sysclk_init();
	board_init();

	/* Initialize the console uart */
	configure_console();

	/* Create task to control oled */
	if (xTaskCreate(task_oled, "oled", TASK_OLED_STACK_SIZE, NULL, TASK_OLED_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create oled task\r\n");
	}
	
	xQueueFila1 = xQueueCreate(32, sizeof(uint32_t));
	xQueueFila2 = xQueueCreate(32, sizeof(uint32_t));
	
	if (xQueueFila1 == NULL)
	printf("falha em criar a queue Fila1\n");

	if (xQueueFila2 == NULL)
	printf("falha em criar a queue Fila2\n");

	/* Start the scheduler. */
	vTaskStartScheduler();

	/* RTOS não deve chegar aqui !! */
	while(1){}

	/* Will only get here if there was insufficient memory to create the idle task. */
	return 0;
}