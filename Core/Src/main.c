#include "main.h"
#include "w5500_spi.h"
#include "wizchip_conf.h"
#include "socket.h"
#include <stdio.h>
#include "stm32f0xx_hal.h"
#include <string.h>

#define LISTEN_PORT	1502
#define RECEIVE_BUFF_SIZE	128


SPI_HandleTypeDef hspi1;
ADC_HandleTypeDef hadc;
UART_HandleTypeDef huart1;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
static void UWriteData( const char data);
static void MX_ADC_Init(void);

static void PHYStatusCheck(void);
static void PrintPHYConf(void);

wiz_NetInfo gWIZNETINFO = {
		.mac = {0x00, 0x08, 0xdc, 0xab, 0xcd, 0xef}, // MSB - LSB
		.ip  = { 192, 168, 1, 112},
		.sn  = { 255, 255, 255, 0},
		.gw  = { 192, 168, 1, 1},
		.dns = { 8,8,8,8}, // google domain server ?
		.dhcp = NETINFO_STATIC
};
uint8_t destination_ip[] = {192,168,1,10};
uint16_t destination_port = 1502;
uint8_t recieve_buff[RECEIVE_BUFF_SIZE];
//uint8_t recieve_buff[RECEIVE_BUFF_SIZE] = "Hello ";


uint32_t reading1;
uint32_t reading2;
uint32_t reading3;
char msg1[32];
char msg2[32];
char msg3[32];
ADC_ChannelConfTypeDef sConfig = {0};
uint32_t millivolts = 0;
uint32_t temp =0;
uint32_t tempC = 0;

int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  setbuf(stdout,NULL); // disable buffering feature

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  W5500Init();
  MX_ADC_Init();
  ctlnetwork(CN_SET_NETINFO,(void*)&gWIZNETINFO);


  //PHYStatusCheck(); // code will block here unless cable is plugged
 // PrintPHYConf();
  /* USER CODE BEGIN 2 */

  while(1)
     {
     	printf("\r\n Insitalizing server socket \r\n");

     	//params in order socket id, tcp or upd, port number, flags = 0;
     	//return value is socket id on sucess
     	if(socket(1,Sn_MR_TCP, LISTEN_PORT,0) !=1)
     	{
     		//error
     		printf("Cannot create socket ! \r\n");
     		while(1);// wait here
     	}
     	 //success
     	printf("Socket created successfully \r\n" );

     	uint8_t socket_io_mode = SOCK_IO_BLOCK;

     	ctlsocket(1, CS_SET_IOMODE, &socket_io_mode);//set blcoking io mode

     	printf("Start listening on port %d \r\n", LISTEN_PORT);
     	printf("Waiting for a client connection \r\n");

     	//make it a passive socket i.e. listen for connection
     	if(listen(1) != SOCK_OK) // our socekt id is 1 (w5500 has 8 sockets from 0-7
     	{
     		//error
     		printf("Cannot listen on port %d", LISTEN_PORT);
     		while(1);
     	}

     	uint8_t sr = 0x00;// socket status register

     	do{
     		sr = getSn_SR(1);// read status reg SR of socekt 1

     	}while(sr != 0x17 && sr != 0x00);

     	if(sr ==0x17)
     	{
     		//we come here when client has connected
     		//now we can read data form the socket
     		printf("A client connected ! \r\n");
     		printf("Waiting for client data... \r\n");



     		while(1)
     		{
     			/*int len=recv(1, recieve_buff, RECEIVE_BUFF_SIZE);

     			if(len == SOCKERR_SOCKSTATUS)
     			{
     				//client has disconnected
     				printf("client has disconnected \r\n");
     				printf("*** SESSIONOVER *** \r\n");
     				break;
     			}
     			*/
     			//recieve_buff[len] = '\0';

     			//printf("Recieved %d bytes from client \r\n", len);
     			//printf("Data recieved : %s", recieve_buff);

     			//echo the data back to seerver in brackets

     			//send(1,(uint8_t*)"[",1);// staring bracket
     			//send(1,recieve_buff,len);// the data
     			//send(1,"[1,3,5,7,7]\r",12);// the data
     			//printf("fuck");
     			send(1,(uint8_t *)msg1,strlen((uint8_t *)msg1));// the data

     			HAL_Delay(1000);
     			//send(1,"[0,0,0,0,0]\r",12);// the data
     			send(1,(uint8_t *)msg2,strlen((uint8_t *)msg2));// the data
     			HAL_Delay(1000);

     			//printf("\r\n ECHO sent back to client \r\n");

     			//lock for quit message and quit if recieved
     		/*	if(strcmp((char*)recieve_buff, "QUIT") ==0)
     			{
     				printf("Recieved QUIT command from Client \r\n");
     				printf("Disconnection ... \r\n");
     				printf(" **** Session over *** \r\n\r\n");
     				disconnect(1); //disconnect from the client
     				break; // come out of loop
     			}
		*/
     			// sConfig.Channel &=~ (sConfig.Channel);

     			sConfig.Channel =  ADC_CHANNEL_8;
     				  ADC1->CHSELR &=~ (1<< 0);
     				  ADC1->CHSELR &=~ (1<< 15);
     				  HAL_ADC_ConfigChannel(&hadc, &sConfig);

     				 HAL_ADC_Start(&hadc);
     				 if( HAL_ADC_PollForConversion(&hadc, 5) == HAL_OK)
     				 {
     				  reading1 = HAL_ADC_GetValue(&hadc);
     				 }
     				  //sprintf(msg1, "Read 1: %lu \t", reading1 );
     				 sprintf(msg1, "%lu", reading1 );
     				strcat(msg1, ",1");
     				  HAL_UART_Transmit(&huart1, (uint8_t *)msg1, strlen(msg1), HAL_MAX_DELAY);
     				  HAL_Delay(50);

     				 // HAL_ADC_Stop(&hadc);
     				 // sConfig.Channel &=~ (sConfig.Channel);
     				  ADC1->CHSELR &=~ (1<< 8);
     				  sConfig.Channel =  ADC_CHANNEL_9;
     				  HAL_ADC_ConfigChannel(&hadc, &sConfig);

     				  HAL_ADC_Start(&hadc);
     				  if(HAL_ADC_PollForConversion(&hadc, 5) == HAL_OK)
     				  {
     				  reading2 = HAL_ADC_GetValue(&hadc);
     				  }
     				  //sprintf(msg2, "Read 2: %lu \n\r", reading2);
     				 sprintf(msg2, "%lu", reading2);
     				 strcat(msg2, ",2");
     				  HAL_UART_Transmit(&huart1, (uint8_t *)msg2, strlen(msg2), HAL_MAX_DELAY);
     				  HAL_Delay(50);

     				  //temperature sensor
     				  ADC1->CHSELR &=~ (1<< 9);
         				  sConfig.Channel =  ADC_CHANNEL_15;
         				  HAL_ADC_ConfigChannel(&hadc, &sConfig);

         				  HAL_ADC_Start(&hadc);
         				  if(HAL_ADC_PollForConversion(&hadc, 5) == HAL_OK)
         				  {
         				  millivolts = HAL_ADC_GetValue(&hadc);
         				  }
         				  //sprintf(msg2, "Read 2: %lu \n\r", reading2);
         				  temp = millivolts * (3300 / 4096);
         				  tempC = (temp - 500) / 10;

         				 sprintf(msg3, "%lu", tempC);
         				 strcat(msg3, ",2");
         				  HAL_UART_Transmit(&huart1, (uint8_t *)msg3, strlen(msg3), HAL_MAX_DELAY);
         				  HAL_Delay(50);




     		} // while loop (as long as client is connected

     	}// if block, client connect success



     }// whiel loop for next client wait



  /*

  //allocate a socket
    // socket number 'id' 0-7 note 0 is bugg, socket type ie: tcp, local port number 0 when client to listen to random port, socket flag 0 by default
    if(socket(1, Sn_MR_TCP, 0, 0) == 1)
    {
  	  printf("\r\n Socket Created Successfully");
    }
    else
    {

  	  printf("\r\n Cannot create socket");
  	  while(1); // hang here during error
    }
    printf("\r\n Connecting to server: %d.%d.%d.%d @ TCP Port: %d", destination_ip[0], destination_ip[1], destination_ip[2], destination_ip[3], destination_port);
    	  // passing 1 for using socket 1 id as configured above
    if(connect(1, destination_ip, destination_port) == SOCK_OK)
    {
  	  printf("\n\r Connected with server");
    }
    else
    {
  	  //failure
  	  printf("\r\n Cannot connect with server !");
  	  while(1); // hang here
    }

    // sending data
   // char *myString = "Here is data!\r\n";

    while(1)
    {
  	  //return value of the send() function is the amoutn of data data
  	  if(send(1, "Here is data\r\n", 18) <= SOCK_ERROR) // per DS error codes are < 0
  	  {
  		  printf("\r\n Sending Failed");
  		  while(1);
  	  }
  	  else
  	  {
  		  printf("\r\nSending Success !");
  	  }
  	  HAL_Delay(1000);
    }


 */



  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
 /* while (1)
  {
	  for( int i = 0; i< 100; i++)
	  {
		  printf("Hello world ! %d \r\n", i);
		  //HAL_UART_Transmit(&huart1, "Using HAL USART ", 17, 1000);
		  HAL_Delay(1000);
	  }
  }
  */
  /* USER CODE END 3 */
}

static void UWriteData( const char data)
{
	while(__HAL_UART_GET_FLAG(&huart1,UART_FLAG_TXE) == RESET);

	huart1.Instance->TDR = data;
}

//standard linking function for usart
int __io_putchar(int ch)
{
	UWriteData(ch);
	return ch;
}

static void PHYStatusCheck(void)
{
	uint8_t tmp;

	printf("\r\nChecking cable presense ...");
	do{
		ctlwizchip(CW_GET_PHYLINK, (void*)&tmp);

		if(tmp ==  PHY_LINK_OFF)
		{
			printf(" No cable connected!");
			HAL_Delay(1500);
		}

	}while(tmp == PHY_LINK_OFF);

	printf("\n\r Good! Cable got connected");

}
static void PrintPHYConf(void)
{
	wiz_PhyConf phyconf;

	ctlwizchip(CW_GET_PHYCONF, (void*)&phyconf);

	if( phyconf.by == PHY_CONFBY_HW)
	{
		printf("\n\r PHY Configured by Hardware pins");
	}
	else
	{
		printf("\n\r PHY Configured by Registers");
	}
	if(phyconf.mode== PHY_MODE_AUTONEGO)
	{
		printf("\n\r Autonegotiation Enabled");
	}
	else
	{
		printf("\n\r Autonegotiation not ENabled");
	}
	if(phyconf.duplex == PHY_DUPLEX_FULL)
	{
		printf("\n\r Duplex mode : full");
	}
	else
	{
		printf("\n\r Duplex mode : Half");
	}
	if( phyconf.speed == PHY_SPEED_10)
	{
		printf("\n\r Speed: 10Mbps");
	}
	else
	{
		printf("\r\n SPeed: 100Mbps");
	}
}
/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSI14;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSI14State = RCC_HSI14_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.HSI14CalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC_Init(void)
{


  hadc.Instance = ADC1;
  hadc.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc.Init.Resolution = ADC_RESOLUTION_12B;
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
  hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc.Init.LowPowerAutoWait = DISABLE;
  hadc.Init.LowPowerAutoPowerOff = DISABLE;
  hadc.Init.ContinuousConvMode = DISABLE;
  hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.DMAContinuousRequests = DISABLE;
  hadc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  if (HAL_ADC_Init(&hadc) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
 // sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
 // sConfig.Channel = ADC_CHANNEL_9;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC_Init 2 */

  /* USER CODE END ADC_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */
  __HAL_SPI_ENABLE(&hspi1); // i added this here
  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(TCPselect_GPIO_Port, TCPselect_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(resetTCP_GPIO_Port, resetTCP_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : TCPselect_Pin */
  GPIO_InitStruct.Pin = TCPselect_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(TCPselect_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : resetTCP_Pin */
  GPIO_InitStruct.Pin = resetTCP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(resetTCP_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
