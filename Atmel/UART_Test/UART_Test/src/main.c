/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# "Insert system clock initialization code here" comment
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */
#include <asf.h>
#include <string.h>

// LED Vermelho
#define LEDR_PIO      PIOA
#define LEDR_PIO_ID   ID_PIOA
#define LEDR_IDX      24
#define LEDR_IDX_MASK (1 << LEDR_IDX)

// LED Verde
#define LEDG_PIO      PIOA
#define LEDG_PIO_ID   ID_PIOA
#define LEDG_IDX      4
#define LEDG_IDX_MASK (1 << LEDG_IDX)

// LED Azul
#define LEDB_PIO      PIOD
#define LEDB_PIO_ID   ID_PIOD
#define LEDB_IDX      26
#define LEDB_IDX_MASK (1 << LEDB_IDX)

#define AFEC_CHANNEL_TEMP_SENSOR 5

#define VOLT_REF        (3300)
#define MAX_DIGITAL     (4095)

// Descomente o define abaixo, para desabilitar o Bluetooth e utilizar modo Serial via Cabo
//#define DEBUG_SERIAL

#ifdef DEBUG_SERIAL
#define UART_COMM USART1
#else
#define UART_COMM USART0
#endif

volatile long g_systimer = 0;
volatile int g_ul_value = 0;
volatile bool g_is_conversion_done = false;
volatile char analogico = '0';


void SysTick_Handler() {
	g_systimer++;
}

// Analogico

static void AFEC_Temp_callback(void)
{
	g_ul_value = afec_channel_get_value(AFEC0, AFEC_CHANNEL_TEMP_SENSOR)/100;
	g_is_conversion_done = true;
	if(g_ul_value==9){
		analogico = '9';
	}
	if(g_ul_value==8){
		analogico = '8';
	}
	if(g_ul_value==7){
		analogico = '7';
	}
	if(g_ul_value==6){
		analogico = '6';
	}
	if(g_ul_value==5){
		analogico = '5';
	}
	if(g_ul_value==4){
		analogico = '4';
	}
	if(g_ul_value==3){
		analogico = '3';
	}
	if(g_ul_value==2){
		analogico = '2';
	}
	if(g_ul_value==1){
		analogico = '1';
	}
	if(g_ul_value==0){
		analogico = '0';
	}
	
}


static void config_ADC_TEMP(void){
/*************************************
   * Ativa e configura AFEC
   *************************************/
  /* Ativa AFEC - 0 */
	afec_enable(AFEC0);

	/* struct de configuracao do AFEC */
	struct afec_config afec_cfg;

	/* Carrega parametros padrao */
	afec_get_config_defaults(&afec_cfg);

	/* Configura AFEC */
	afec_init(AFEC0, &afec_cfg);

	/* Configura trigger por software */
	//afec_set_trigger(AFEC0, AFEC_TRIG_SW);

	/* configura call back */
	afec_set_callback(AFEC0, AFEC_INTERRUPT_EOC_5,	AFEC_Temp_callback, 1);

	/*** Configuracao específica do canal AFEC ***/
	struct afec_ch_config afec_ch_cfg;
	afec_ch_get_config_defaults(&afec_ch_cfg);
	afec_ch_cfg.gain = AFEC_GAINVALUE_0;
	afec_ch_set_config(AFEC0, AFEC_CHANNEL_TEMP_SENSOR, &afec_ch_cfg);

	/*
	* Calibracao:
	* Because the internal ADC offset is 0x200, it should cancel it and shift
	 down to 0.
	 */
	afec_channel_set_analog_offset(AFEC0, AFEC_CHANNEL_TEMP_SENSOR, 0x200);

	/***  Configura sensor de temperatura ***/
	struct afec_temp_sensor_config afec_temp_sensor_cfg;

	afec_temp_sensor_get_config_defaults(&afec_temp_sensor_cfg);
	afec_temp_sensor_set_config(AFEC0, &afec_temp_sensor_cfg);

	/* Selecina canal e inicializa conversão */
	afec_channel_enable(AFEC0, AFEC_CHANNEL_TEMP_SENSOR);
}

void config_console(void) {
	usart_serial_options_t config;
	config.baudrate = 9600;
	config.charlength = US_MR_CHRL_8_BIT;
	config.paritytype = US_MR_PAR_NO;
	config.stopbits = false;
	usart_serial_init(USART1, &config);
	usart_enable_tx(USART1);
	usart_enable_rx(USART1);
}

void usart_put_string(Usart *usart, char str[]) {
	usart_serial_write_packet(usart, str, strlen(str));
}

int usart_get_string(Usart *usart, char buffer[], int bufferlen, int timeout_ms) {
	long timestart = g_systimer;
	uint32_t rx;
	uint32_t counter = 0;
	
	while(g_systimer - timestart < timeout_ms && counter < bufferlen - 1) {
		if(usart_read(usart, &rx) == 0) {
			//timestart = g_systimer; // reset timeout
			buffer[counter++] = rx;
		}
	}
	buffer[counter] = 0x00;
	return counter;
}

void usart_send_command(Usart *usart, char buffer_rx[], int bufferlen, char buffer_tx[], int timeout) {
	usart_put_string(usart, buffer_tx);
	usart_get_string(usart, buffer_rx, bufferlen, timeout);
}

void usart_log(char* name, char* log) {
	usart_put_string(USART1, "[");
	usart_put_string(USART1, name);
	usart_put_string(USART1, "] ");
	usart_put_string(USART1, log);
	usart_put_string(USART1, "\r\n");
}

void hc05_config_server(void) {
	usart_serial_options_t config;
	config.baudrate = 9600;
	config.charlength = US_MR_CHRL_8_BIT;
	config.paritytype = US_MR_PAR_NO;
	config.stopbits = false;
	usart_serial_init(USART0, &config);
	usart_enable_tx(USART0);
	usart_enable_rx(USART0);
	
	 // RX - PB0  TX - PB1 
	 pio_configure(PIOB, PIO_PERIPH_C, (1 << 0), PIO_DEFAULT);
	 pio_configure(PIOB, PIO_PERIPH_C, (1 << 1), PIO_DEFAULT);
}

int hc05_server_init(void) {
	char buffer_rx[128];
	usart_send_command(USART0, buffer_rx, 1000, "AT", 1000);
	usart_send_command(USART0, buffer_rx, 1000, "AT", 1000);	
	usart_send_command(USART0, buffer_rx, 1000, "AT+NAMEJorg", 1000);
	usart_log("hc05_server_init", buffer_rx);
	usart_send_command(USART0, buffer_rx, 1000, "AT", 1000);
	usart_send_command(USART0, buffer_rx, 1000, "AT+PIN0000", 1000);
	usart_log("hc05_server_init", buffer_rx);
}

int led_init(void){
	// Configura led vermelho
	pmc_enable_periph_clk(LEDR_PIO_ID);
	pio_configure(LEDR_PIO, PIO_OUTPUT_0, LEDR_IDX_MASK, PIO_DEFAULT);
	
	// Configura led verde
	pmc_enable_periph_clk(LEDG_PIO_ID);
	pio_configure(LEDG_PIO, PIO_OUTPUT_0, LEDG_IDX_MASK, PIO_DEFAULT);
	
	// Configura led azul
	pmc_enable_periph_clk(LEDB_PIO_ID);
	pio_configure(LEDB_PIO, PIO_OUTPUT_0, LEDB_IDX_MASK, PIO_DEFAULT);	
}



int main (void)
{
	board_init();
	sysclk_init();
	delay_init();
	SysTick_Config(sysclk_get_cpu_hz() / 1000); // 1 ms
	config_console();
	led_init();
	config_ADC_TEMP();
	afec_start_software_conversion(AFEC0);

	#ifndef DEBUG_SERIAL
	usart_put_string(USART1, "Inicializando...\r\n");
	usart_put_string(USART1, "Config HC05 Server...\r\n");
	hc05_config_server();
	hc05_server_init();
	#endif
	
	char verde = '0';
	char vermelho = '0';
	char amarelo = '0';
	char azul = '0';
	char laranja = '0';
	char palheta_down = '0';
	char palheta_up = '0';
	char eof = 'X';
	char str[8];
	
	while(1) {	
		verde = '0';
		vermelho = '0';
		amarelo = '0';
		azul = '0';
		laranja = '0';
		palheta_down = '0';
		palheta_up = '0';
		pio_set(LEDG_PIO, LEDG_IDX_MASK);
		pio_set(LEDR_PIO, LEDR_IDX_MASK);
		pio_set(LEDB_PIO, LEDB_IDX_MASK);
		
		if(pio_get(PIOA, PIO_INPUT, PIO_PA19) == 0) {
			verde = '1';
			pio_clear(LEDG_PIO, LEDG_IDX_MASK);
		}
		if(pio_get(PIOC, PIO_INPUT, PIO_PC31) == 0) {
			vermelho = '1';
			pio_clear(LEDR_PIO, LEDR_IDX_MASK);
		}
		if(pio_get(PIOC, PIO_INPUT, PIO_PC30) == 0) {
			amarelo = '1';
			pio_clear(LEDG_PIO, LEDG_IDX_MASK);
			pio_clear(LEDR_PIO, LEDR_IDX_MASK);
		}
		if(pio_get(PIOA, PIO_INPUT, PIO_PA0) == 0) {
			azul = '1';
			pio_clear(LEDB_PIO, LEDB_IDX_MASK);
		}
		if(pio_get(PIOD, PIO_INPUT, PIO_PD28) == 0) {
			laranja = '1';
			pio_clear(LEDG_PIO, LEDG_IDX_MASK);
			pio_clear(LEDR_PIO, LEDR_IDX_MASK);
			pio_clear(LEDB_PIO, LEDB_IDX_MASK);
		}
		if(pio_get(PIOC, PIO_INPUT, PIO_PC17) == 0) {
			palheta_down = '1';
		}
		if(pio_get(PIOA, PIO_INPUT, PIO_PA3) == 0) {
			palheta_up = '1';
		}
		
		while(!usart_is_tx_ready(UART_COMM));
		usart_write(UART_COMM, verde);
		while(!usart_is_tx_ready(UART_COMM));
		usart_write(UART_COMM, vermelho);
		while(!usart_is_tx_ready(UART_COMM));
		usart_write(UART_COMM, amarelo);
		while(!usart_is_tx_ready(UART_COMM));
		usart_write(UART_COMM, azul);
		while(!usart_is_tx_ready(UART_COMM));
		usart_write(UART_COMM, laranja);
		while(!usart_is_tx_ready(UART_COMM));
		usart_write(UART_COMM, palheta_down);
		while(!usart_is_tx_ready(UART_COMM));
		usart_write(UART_COMM, palheta_up);
		/*sprintf(str, "%d", g_ul_value);*/
		while(!usart_is_tx_ready(UART_COMM));
		usart_write(UART_COMM, analogico);
		afec_start_software_conversion(AFEC0);
		while(!usart_is_tx_ready(UART_COMM));
		usart_write(UART_COMM, eof);
	}
}
