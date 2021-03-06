#include <asf.h>
#include "conf_board.h"
#include "mcu6050.h"
#include <string.h>

/************************************************************************/
/* defines                                                              */
/************************************************************************/

// TRIGGER
#define LED_PIO      PIOC
#define LED_PIO_ID   ID_PIOC
#define LED_IDX      8
#define LED_IDX_MASK (1 << LED_IDX)

// Green
#define Green_PIO      PIOA
#define Green_PIO_ID   ID_PIOA
#define Green_IDX  19
#define Green_IDX_MASK (1 << Green_IDX)

// Red
#define Red_PIO      PIOC
#define Red_PIO_ID   ID_PIOC
#define Red_IDX  31
#define Red_IDX_MASK (1 << Red_IDX)

// Yellow
#define Yellow_PIO      PIOC
#define Yellow_PIO_ID   ID_PIOC
#define Yellow_IDX  30
#define Yellow_IDX_MASK (1 << Yellow_IDX)

// Blue
#define Blue_PIO      PIOA
#define Blue_PIO_ID   ID_PIOA
#define Blue_IDX  0
#define Blue_IDX_MASK (1 << Blue_IDX)

// Orange
#define Orange_PIO      PIOD
#define Orange_PIO_ID   ID_PIOD
#define Orange_IDX  28
#define Orange_IDX_MASK (1 << Orange_IDX)

// Palheta up
#define Palhetaup_PIO      PIOD
#define Palhetaup_PIO_ID   ID_PIOD
#define Palhetaup_IDX  20
#define Palhetaup_IDX_MASK (1 << Palhetaup_IDX)

// Palheta down
#define Palhetadown_PIO      PIOC
#define Palhetadown_PIO_ID   ID_PIOC
#define Palhetadown_IDX  17
#define Palhetadown_IDX_MASK (1 << Palhetadown_IDX)

#define ID_Green 1
#define ID_Red 2
#define ID_Yellow 3
#define ID_Blue 4
#define ID_Orange 5
#define ID_Palhetaup 6
#define ID_Palhetadown 7
#define ID_Afec 8
#define ID_Imu 9

#define AFEC_CHANNEL_TEMP_SENSOR 5

// usart (bluetooth)
#define USART_COM_ID ID_USART0
#define USART_COM    USART0

/** RTOS  */
#define TASK_PROCESS_STACK_SIZE            (6*1024/sizeof(portSTACK_TYPE))
#define TASK_PROCESS_STACK_PRIORITY        (tskIDLE_PRIORITY+1)

#define TASK_AFEC_STACK_SIZE            (4*1024/sizeof(portSTACK_TYPE))
#define TASK_AFEC_STACK_PRIORITY        (tskIDLE_PRIORITY)

#define TASK_IMU_STACK_SIZE            (4*1024/sizeof(portSTACK_TYPE))
#define TASK_IMU_STACK_PRIORITY        (tskIDLE_PRIORITY)

//IMU
#define TWIHS_MCU6050_ID    ID_TWIHS0
#define TWIHS_MCU6050       TWIHS0

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName);
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);
extern void vApplicationMallocFailedHook(void);
extern void xPortSysTickHandler(void);

/** prototypes */
void but_callback(void);
static void ECHO_init(void);
static void USART1_init(void);
uint32_t usart_puts(uint8_t *pstring);

QueueHandle_t xQueueBt;

/************************************************************************/
/* RTOS application funcs                                               */
/************************************************************************/

/**
 * \brief Called if stack overflow during execution
 */
extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,
		signed char *pcTaskName)
{
	printf("stack overflow %x %s\r\n", pxTask, (portCHAR *)pcTaskName);
	/* If the parameters have been corrupted then inspect pxCurrentTCB to
	 * identify which task has overflowed its stack.
	 */
	for (;;) {
	}
}

/**
 * \brief This function is called by FreeRTOS idle task
 */
extern void vApplicationIdleHook(void)
{
	pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
}

/**
 * \brief This function is called by FreeRTOS each tick
 */
extern void vApplicationTickHook(void)
{
}

extern void vApplicationMallocFailedHook(void)
{
	/* Called if a call to pvPortMalloc() fails because there is insufficient
	free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	internally by FreeRTOS API functions that create tasks, queues, software
	timers, and semaphores.  The size of the FreeRTOS heap is set by the
	configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */

	/* Force an assert. */
	configASSERT( ( volatile void * ) NULL );
}

/************************************************************************/
/* handlers / callbacks                                                 */
/************************************************************************/
void Green_callback(void)
{
	int8_t dado = 0;
	if(!pio_get(Green_PIO, PIO_INPUT, PIO_PA19)){
		dado = ID_Green<<4 | 1<<0;
	}
	else{
		dado = ID_Green<<4 | 0<<0;
	}
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xQueueSendFromISR(xQueueBt, &dado, &xHigherPriorityTaskWoken);
}

void Red_callback(void)
{
	int8_t dado = 0;
	if(!pio_get(Red_PIO, PIO_INPUT, PIO_PC31)){
		dado = ID_Red<<4 | 1<<0;
	}
	else{
		dado = ID_Red<<4 | 0<<0;
	}
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xQueueSendFromISR(xQueueBt, &dado, &xHigherPriorityTaskWoken);
}

void Yellow_callback(void)
{
	int8_t dado = 0;
	if(!pio_get(Yellow_PIO, PIO_INPUT, PIO_PC30)){
		dado = ID_Yellow<<4 | 1<<0;
	}
	else{
		dado = ID_Yellow<<4 | 0<<0;
	}
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xQueueSendFromISR(xQueueBt, &dado, &xHigherPriorityTaskWoken);
}

void Blue_callback(void)
{
	int8_t dado = 0;
	if(!pio_get(Blue_PIO, PIO_INPUT, PIO_PA0)){
		dado = ID_Blue<<4 | 1<<0;
	}
	else{
		dado = ID_Blue<<4 | 0<<0;
	}
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;	
	xQueueSendFromISR(xQueueBt, &dado, &xHigherPriorityTaskWoken);
}

void Orange_callback(void)
{
	int8_t dado = 0;
	if(!pio_get(Orange_PIO, PIO_INPUT, PIO_PD28)){
		dado = ID_Orange<<4 | 1<<0;
	}
	else{
		dado = ID_Orange<<4 | 0<<0;
	}
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;	
	xQueueSendFromISR(xQueueBt, &dado, &xHigherPriorityTaskWoken);
}

void Palhetaup_callback(void)
{
	int8_t dado = 0;
	if(!pio_get(Palhetaup_PIO, PIO_INPUT, PIO_PD20)){
		dado = ID_Palhetaup<<4 | 1<<0;
	}
	else{
		dado = ID_Palhetaup<<4 | 0<<0;
	}
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xQueueSendFromISR(xQueueBt, &dado, &xHigherPriorityTaskWoken);
}

void Palhetadown_callback(void)
{
	int8_t dado = 0;
	if(!pio_get(Palhetadown_PIO, PIO_INPUT, PIO_PC17)){
		dado = ID_Palhetadown<<4 | 1<<0;
	}
	else{
		dado = ID_Palhetadown<<4 | 0<<0;
	}
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xQueueSendFromISR(xQueueBt, &dado, &xHigherPriorityTaskWoken);
}

static void AFEC_Callback(void)
{
	uint8_t g_ul_value = afec_channel_get_value(AFEC0, AFEC_CHANNEL_TEMP_SENSOR)/100;
	uint8_t dado = ID_Afec << 4 | (g_ul_value & 0b1111) << 0;
	
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xQueueSendFromISR(xQueueBt, &dado, &xHigherPriorityTaskWoken);
}

/************************************************************************/
/* funcoes                                                              */
/************************************************************************/

/**
 * \brief Configure the console UART.
 */

static void configure_console(void){
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
#if (defined CONF_UART_CHAR_LENGTH)
		.charlength = CONF_UART_CHAR_LENGTH,
#endif
		.paritytype = CONF_UART_PARITY,
#if (defined CONF_UART_STOP_BITS)
		.stopbits = CONF_UART_STOP_BITS,
#endif
	};

	/* Configure console UART. */
	stdio_serial_init(CONF_UART, &uart_serial_options);

	/* Specify that stdout should not be buffered. */
#if defined(__GNUC__)
	setbuf(stdout, NULL);
#else
	/* Already the case in IAR's Normal DLIB default configuration: printf()
	 * emits one character at a time.
	 */
#endif
}

uint32_t usart_puts(uint8_t *pstring){
	uint32_t i ;

	while(*(pstring + i))
		if(uart_is_tx_empty(USART_COM))
			usart_serial_putchar(USART_COM, *(pstring+i++));
}

void io_init(void){
	pmc_enable_periph_clk(Green_PIO_ID);
	pmc_enable_periph_clk(Red_PIO_ID);
	pmc_enable_periph_clk(Yellow_PIO_ID);
	pmc_enable_periph_clk(Blue_PIO_ID);
	pmc_enable_periph_clk(Orange_PIO_ID);
	pmc_enable_periph_clk(Palhetaup_PIO_ID);
	pmc_enable_periph_clk(Palhetadown_PIO_ID);

	pio_configure(Green_PIO, PIO_INPUT, Green_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(Green_PIO, Green_IDX_MASK, 20);
	pio_configure(Red_PIO, PIO_INPUT, Red_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(Red_PIO, Red_IDX_MASK, 20);
	pio_configure(Yellow_PIO, PIO_INPUT, Yellow_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(Yellow_PIO, Yellow_IDX_MASK, 20);
	pio_configure(Blue_PIO, PIO_INPUT, Blue_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(Blue_PIO, Blue_IDX_MASK, 20);
	pio_configure(Orange_PIO, PIO_INPUT, Orange_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(Orange_PIO, Orange_IDX_MASK, 20);
	pio_configure(Palhetadown_PIO, PIO_INPUT, Palhetadown_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(Palhetadown_PIO, Palhetadown_IDX_MASK, 20);
	pio_configure(Palhetaup_PIO, PIO_INPUT, Palhetaup_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(Palhetaup_PIO, Palhetaup_IDX_MASK, 20);
	
	pio_handler_set(Green_PIO,Green_PIO_ID,Green_IDX_MASK,PIO_IT_EDGE,Green_callback);
	pio_handler_set(Red_PIO,Red_PIO_ID,Red_IDX_MASK,PIO_IT_EDGE,Red_callback);
	pio_handler_set(Yellow_PIO,Yellow_PIO_ID,Yellow_IDX_MASK,PIO_IT_EDGE,Yellow_callback);
	pio_handler_set(Blue_PIO,Blue_PIO_ID,Blue_IDX_MASK,PIO_IT_EDGE,Blue_callback);
	pio_handler_set(Orange_PIO,Orange_PIO_ID,Orange_IDX_MASK,PIO_IT_EDGE,Orange_callback);
	pio_handler_set(Palhetaup_PIO,Palhetaup_PIO_ID,Palhetaup_IDX_MASK,PIO_IT_EDGE,Palhetaup_callback);
	pio_handler_set(Palhetadown_PIO,Palhetadown_PIO_ID,Palhetadown_IDX_MASK,PIO_IT_EDGE,Palhetadown_callback);

	NVIC_EnableIRQ(Green_PIO_ID);
	NVIC_SetPriority(Green_PIO_ID, 4); // Prioridade 4
	
	NVIC_EnableIRQ(Red_PIO_ID);
	NVIC_SetPriority(Red_PIO_ID, 4); // Prioridade 4
	
	NVIC_EnableIRQ(Yellow_PIO_ID);
	NVIC_SetPriority(Yellow_PIO_ID, 4); // Prioridade 4
	
	NVIC_EnableIRQ(Blue_PIO_ID);
	NVIC_SetPriority(Blue_PIO_ID, 4); // Prioridade 4
	
	NVIC_EnableIRQ(Orange_PIO_ID);
	NVIC_SetPriority(Orange_PIO_ID, 4); // Prioridade 4
	
	NVIC_EnableIRQ(Palhetaup_PIO_ID);
	NVIC_SetPriority(Palhetaup_PIO_ID, 4); // Prioridade 4
	
	NVIC_EnableIRQ(Palhetadown_PIO_ID);
	NVIC_SetPriority(Palhetadown_PIO_ID, 4); // Prioridade 4
	
	pio_enable_interrupt(Green_PIO, Green_IDX_MASK);
	pio_enable_interrupt(Red_PIO, Red_IDX_MASK);
	pio_enable_interrupt(Yellow_PIO, Yellow_IDX_MASK);
	pio_enable_interrupt(Blue_PIO, Blue_IDX_MASK);
	pio_enable_interrupt(Orange_PIO, Orange_IDX_MASK);
	pio_enable_interrupt(Palhetaup_PIO, Palhetaup_IDX_MASK);
	pio_enable_interrupt(Palhetadown_PIO, Palhetadown_IDX_MASK);
}

void usart_put_string(Usart *usart, char str[]) {
  usart_serial_write_packet(usart, str, strlen(str));
}

int usart_get_string(Usart *usart, char buffer[], int bufferlen, uint timeout_ms) {
  uint timecounter = timeout_ms;
  uint32_t rx;
  uint32_t counter = 0;
  
  while( (timecounter > 0) && (counter < bufferlen - 1)) {
    if(usart_read(usart, &rx) == 0) {
      buffer[counter++] = rx;
    }
    else{
      timecounter--;
      vTaskDelay(1);
    }    
  }
  buffer[counter] = 0x00;
  return counter;
}

void usart_send_command(Usart *usart, char buffer_rx[], int bufferlen, char buffer_tx[], int timeout) {
  usart_put_string(usart, buffer_tx);
  usart_get_string(usart, buffer_rx, bufferlen, timeout);
}

void hc05_config_server(void) {
  sysclk_enable_peripheral_clock(USART_COM_ID);
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
  usart_send_command(USART1, buffer_rx, 1000, "AT", 1000);
  usart_send_command(USART1, buffer_rx, 1000, "AT", 1000);
  usart_send_command(USART1, buffer_rx, 1000, "AT+NAMEJorg", 1000);
  usart_send_command(USART1, buffer_rx, 1000, "AT", 1000);
  usart_send_command(USART1, buffer_rx, 1000, "AT+PIN0000", 1000);
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
	afec_set_trigger(AFEC0, AFEC_TRIG_SW);

	/* configura call back */
	afec_set_callback(AFEC0, AFEC_INTERRUPT_EOC_5,	AFEC_Callback, 5);

	/*** Configuracao espec?fica do canal AFEC ***/
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

	/* Selecina canal e inicializa convers?o */
	afec_channel_enable(AFEC0, AFEC_CHANNEL_TEMP_SENSOR);
}

void mcu6050_i2c_bus_init(void)
{
	twihs_options_t bno055_option;
	pmc_enable_periph_clk(19);

	/* Configure the options of TWI driver */
	bno055_option.master_clk = sysclk_get_cpu_hz();
	bno055_option.speed      = 40000;
	twihs_master_init(TWIHS_MCU6050, &bno055_option);
}

int8_t mcu6050_i2c_bus_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt)
{
	int32_t ierror = 0x00;

	twihs_packet_t p_packet;
	p_packet.chip         = dev_addr;
	p_packet.addr[0]      = reg_addr;
	p_packet.addr_length  = 1;
	p_packet.buffer       = reg_data;
	p_packet.length       = cnt;
	
	ierror = twihs_master_write(TWIHS_MCU6050, &p_packet);

	return (int8_t)ierror;
}

int8_t mcu6050_i2c_bus_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt)
{
	int32_t ierror = 0x00;
	
	twihs_packet_t p_packet;
	p_packet.chip         = dev_addr;
	p_packet.addr[0]      = reg_addr;
	p_packet.addr_length  = 1;
	p_packet.buffer       = reg_data;
	p_packet.length       = cnt;
	
	// TODO: Algum problema no SPI faz com que devemos ler duas vezes o registrador para
	//       conseguirmos pegar o valor correto.
	ierror = twihs_master_read(TWIHS_MCU6050, &p_packet);
	ierror = twihs_master_read(TWIHS_MCU6050, &p_packet);

	return (int8_t)ierror;
}

/************************************************************************/
/* TASKS                                                                */
/************************************************************************/

void task_bluetooth(void){
  xQueueBt = xQueueCreate(20, sizeof(int8_t));
  
  hc05_config_server();
  hc05_server_init();
  io_init();
  
  while(1){
	int8_t send_char = 0;  
	if (xQueueReceive(xQueueBt, &(send_char), (TickType_t) 100 / portTICK_PERIOD_MS )) {
		while(!usart_is_tx_ready(USART_COM));
		usart_write(USART_COM, send_char);	
		while(!usart_is_tx_ready(USART_COM));
		usart_write(USART_COM, 'X');	
	}
  }
}

void task_afec(void){
	config_ADC_TEMP();
	while(true){
		afec_start_software_conversion(AFEC0);
		vTaskDelay(50/portTICK_PERIOD_MS);
	}
}

void task_imu(void){
	/* buffer para recebimento de dados */
	int16_t  accX, accY, accZ;
	uint8_t  accXHigh, accYHigh, accZHigh;
	uint8_t  accXLow,  accYLow,  accZLow;
	uint8_t bufferRX[10];
	uint8_t bufferTX[10];
	uint8_t rtn;
	mcu6050_i2c_bus_init();
	
	rtn = mcu6050_i2c_bus_read(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_WHO_AM_I, bufferRX, 1);
	if(rtn != TWIHS_SUCCESS){
		printf("[ERRO] [i2c] [read] \n");
	}
	
	// Por algum motivo a primeira leitura � errada.
	if(bufferRX[0] != 0x68){
		printf("[ERRO] [mcu] [Wrong device] [0x%2X] \n", bufferRX[0]);
	}
	
	// Set Clock source
	bufferTX[0] = MPU6050_CLOCK_PLL_XGYRO;
	rtn = mcu6050_i2c_bus_write(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_PWR_MGMT_1, bufferTX, 1);
	if(rtn != TWIHS_SUCCESS)
	printf("[ERRO] [i2c] [write] \n");

	// Configura range acelerometro para operar com 2G
	bufferTX[0] = 0x00; // 2G
	rtn = mcu6050_i2c_bus_write(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_ACCEL_CONFIG, bufferTX, 1);
	float rangePerDigit = 0.000061f ; // 2G
	
	while (1) {
		// Le valor do acc X High e Low
		rtn = mcu6050_i2c_bus_read(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_ACCEL_XOUT_H, &accXHigh, 1);
		rtn = mcu6050_i2c_bus_read(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_ACCEL_XOUT_L, &accXLow,  1);
		
		// Le valor do acc y High e  Low
		rtn = mcu6050_i2c_bus_read(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_ACCEL_YOUT_H, &accYHigh, 1);
		rtn = mcu6050_i2c_bus_read(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_ACCEL_ZOUT_L, &accYLow,  1);
		
		// Le valor do acc z HIGH e Low
		rtn = mcu6050_i2c_bus_read(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_ACCEL_ZOUT_H, &accZHigh, 1);
		rtn = mcu6050_i2c_bus_read(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_ACCEL_ZOUT_L, &accZLow,  1);
		
		// Dados s�o do tipo complemento de dois
		accX = (accXHigh << 8) | (accXLow << 0);
		accY = (accYHigh << 8) | (accYLow << 0);
		accZ = (accZHigh << 8) | (accZLow << 0);
		
		printf("x/y : %d / %d \n", accX/100, accY/100);
		
		int8_t dado = 0;
		if((accX/100) > 115){
			dado = ID_Imu<<4 | 1<<0;
		}
		else{
			dado = ID_Imu<<4 | 0<<0;
		}
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		xQueueSendFromISR(xQueueBt, &dado, &xHigherPriorityTaskWoken);
		
		vTaskDelay(50/portTICK_PERIOD_MS);
	}
}

/************************************************************************/
/* main                                                                 */
/************************************************************************/

int main(void){
	/* Initialize the SAM system */
	sysclk_init();
	board_init();

	/* Initialize the console uart */
	configure_console();
	
	printf("Start");

	/* Create task to make led blink */
	xTaskCreate(task_bluetooth, "BLT", TASK_PROCESS_STACK_SIZE, NULL, TASK_PROCESS_STACK_PRIORITY, NULL);
    xTaskCreate(task_afec, "AFEC", TASK_AFEC_STACK_SIZE, NULL, TASK_AFEC_STACK_PRIORITY, NULL);
	xTaskCreate(task_imu, "IMU", TASK_IMU_STACK_SIZE, NULL, TASK_IMU_STACK_PRIORITY, NULL);
	
	/* Start the scheduler. */
	vTaskStartScheduler();

	while(1){}

	/* Will only get here if there was insufficient memory to create the idle task. */
	return 0;
}
