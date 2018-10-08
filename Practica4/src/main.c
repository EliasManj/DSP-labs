/**
 ******************************************************************************
 * @file    DAC_SignalsGeneration/main.c
 * @author  MCD Application Team
 * @version V1.0.0
 * @date    19-September-2011
 * @brief   Main program body.
 ******************************************************************************
 * @attention
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4_discovery.h"
/** @addtogroup STM32F4_Discovery_Peripheral_Examples
 * @{
 */

/** @addtogroup DAC_SignalsGeneration
 * @{
 */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define DAC_DHR12R2_ADDRESS    (uint32_t)&DAC->DHR12R2
#define ADC3_DR_ADDRESS    (uint32_t)(&(ADC3->DR))//
//#define ADC3_DR_ADDRESS     (uint32_t)(&(ADC3->DR))//((uint32_t)0x4001224C)
#define BUFFER_SIZE			1024
#define BAUDRATE			230400
#define delay_10ms 			480
#define delay_4ms 			192
#define offset_2mv			248
#define N_COEFS				10
#define BIT12_OFFSET		2048
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
DAC_InitTypeDef DAC_InitStructure;
volatile uint16_t buffADC_0[BUFFER_SIZE]; //Array for ADC samples
volatile uint16_t buffADC_1[BUFFER_SIZE]; //Array for ADC samples
volatile uint16_t buffDAC_0[BUFFER_SIZE]; //Array for DAC samples
volatile uint16_t buffDAC_1[BUFFER_SIZE]; //Array for DAC samples
volatile int16_t temp0[N_COEFS];
volatile int16_t temp_outputDAC;

uint16_t Sine12bit[32] = { 2047, 2447, 2831, 3185, 3498, 3750, 3939, 4056, 4095,
		4056, 3939, 3750, 3495, 3185, 2831, 2447, 2047, 1647, 1263, 909, 599,
		344, 155, 38, 0, 38, 155, 344, 599, 909, 1263, 1647 };
int16_t coefficients[N_COEFS] = {4,4,4,4};

volatile int32_t memory;
volatile int32_t mem_delay;
volatile int32_t index_t;
volatile int32_t index_t_delay;
volatile int32_t interrupt;
int i;
uint32_t flag;
/* Private function prototypes -----------------------------------------------*/
void TIM2_Config(void);
void DAC_Ch2_SineWaveConfig(void);
void ADC3_CH12_DMA_Config(void);

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Main program
 * @param  None
 * @retval None
 */

int32_t FIR(int16_t adc){
	return (((int32_t)adc) - BIT12_OFFSET) + temp0[0] + temp0[1] + temp0[2] + temp0[3];
}

int main(void) {
	/*!< At this stage the microcontroller clock setting is already configured,
	 this is done through SystemInit() function which is called from startup
	 file (startup_stm32f4xx.s) before to branch to application main.
	 To reconfigure the default setting of SystemInit() function, refer to
	 system_stm32f4xx.c file
	 */

	/* Preconfiguration before using DAC----------------------------------------*/
	GPIO_InitTypeDef GPIO_InitStructure;

	/* DMA1 clock and GPIOA clock enable (to be used with DAC) */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1 | RCC_AHB1Periph_GPIOA, ENABLE);

	/* DAC Periph clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

	/* DAC channel 1 & 2 (DAC_OUT1 = PA.4)(DAC_OUT2 = PA.5) configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* TIM6 Configuration ------------------------------------------------------*/
	TIM2_Config();

	/* Sine Wave generator -----------------------------------------------*/
	DAC_Ch2_SineWaveConfig();

	/*ADC config*/
	//x[n] + x[n-1] + x[n-2] + x[n-3] + x[n-4]
	ADC3_CH12_DMA_Config(); //Configure ADC and DMA
	ADC_Cmd(ADC3, ENABLE); //Enable ADC
	memory = 0;
	while (1) {
		if (interrupt == 1) {
			interrupt = 0;
			if (memory) {
				for (index_t = 0; index_t < BUFFER_SIZE; index_t++) {
					for (i = 0; i < N_COEFS; i++) {
						index_t_delay = index_t - i;
						if (index_t_delay > 0) {
							temp0[i] = (int32_t)buffADC_0[index_t_delay];
						} else {
							index_t_delay = BUFFER_SIZE + index_t_delay;
							temp0[i] = (int32_t)buffADC_1[index_t_delay];
						}
					}
					//Substract offset 
					for(i = 0; i < BUFFER_SIZE; i++){
						temp0[i] = temp0[i] - BIT12_OFFSET;
					}
					//FIR Filter
					temp_outputDAC = FIR(buffADC_0[index_t]);
					 //Add offset
					buffDAC_0[index_t] = (uint32_t)(temp_outputDAC + BIT12_OFFSET);
				}
			} else {
				for (index_t = 0; index_t < BUFFER_SIZE; index_t++) {
					for (i = 0; i < N_COEFS; i++) {
						index_t_delay = index_t - i;
						if (index_t_delay > 0) {
							temp0[i] = (int16_t)buffADC_1[index_t_delay];
						} else {
							index_t_delay = BUFFER_SIZE + index_t_delay;
							temp0[i] = (int16_t)buffADC_0[index_t_delay];
						}
					}
					//Substract offset 
					for(i = 0; i < BUFFER_SIZE; i++){
						temp0[i] = temp0[i] - BIT12_OFFSET;
					}
					//FIR Filter
					temp_outputDAC = FIR(buffADC_1[index_t]);
					 //Add offset
					buffDAC_1[index_t] = (uint32_t)(temp_outputDAC + BIT12_OFFSET);
				}
			}

		}
	};
}

/*
 * //(uint32_t)248 son .2 v porque 3.3 es la amplitud dek DAC y toma 2^12 muesstras por ciclo, entonces .2 entre esto da
 //248 unidades = .2 V
 +0.2
 buffDAC_0[index_t] = buffADC_0[index_t]+(uint32_t)248;
 buffDAC_1[index_t] = buffADC_1[index_t]+(uint32_t)248;
 Delay

 * */

/**
 * @brief  TIM6 Configuration
 * @note   TIM6 configuration is based on CPU @168MHz and APB1 @42MHz
 * @note   TIM6 Update event occurs each 37.5MHz/256 = 16.406 KHz
 * @param  None
 * @retval None
 */
void TIM2_Config(void) {
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	/* TIM6 Periph clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	/* Time base configuration */
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Period = 0xF0;
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	/* TIM6 TRGO selection */
	TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);

	/* TIM6 enable counter */
	TIM_Cmd(TIM2, ENABLE);
}

/**
 * @brief  DAC  Channel2 SineWave Configuration
 * @param  None
 * @retval None
 */
void DAC_Ch2_SineWaveConfig(void) {
	DMA_InitTypeDef DMA_InitStructure;

	/* DAC channel2 Configuration */
	DAC_InitStructure.DAC_Trigger = DAC_Trigger_T2_TRGO;
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
	DAC_Init(DAC_Channel_2, &DAC_InitStructure);

	/* DMA1_Stream5 channel7 configuration **************************************/
	DMA_DeInit(DMA1_Stream6);
	DMA_InitStructure.DMA_Channel = DMA_Channel_7;
	DMA_InitStructure.DMA_PeripheralBaseAddr = DAC_DHR12R2_ADDRESS;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t) (&buffDAC_0[0]);
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_BufferSize = BUFFER_SIZE;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA1_Stream6, &DMA_InitStructure);

	/* Enable DMA1_Stream5 */
	DMA_DoubleBufferModeConfig(DMA1_Stream6, (uint32_t) (&buffDAC_1[0]), 0x00);
	DMA_DoubleBufferModeCmd(DMA1_Stream6, ENABLE);
	DMA_Cmd(DMA1_Stream6, ENABLE);
	/* Enable DAC Channel2 */
	DAC_Cmd(DAC_Channel_2, ENABLE);

	/* Enable DMA for DAC Channel2 */
	DAC_DMACmd(DAC_Channel_2, ENABLE);
}

/* @brief  ADC3 channel12 with DMA configuration*/

void ADC3_CH12_DMA_Config(void) {
	/* Initialization structures *******************************************************/
	ADC_InitTypeDef ADC_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Enable ADC3, DMA2 and GPIO clocks ****************************************/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);

	/* DMA2 Stream0 channel2 configuration **************************************/
	DMA_InitStructure.DMA_Channel = DMA_Channel_2;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) ADC3_DR_ADDRESS;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t) (&buffADC_0[0]);
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = BUFFER_SIZE;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA2_Stream0, &DMA_InitStructure);
	DMA_DoubleBufferModeConfig(DMA2_Stream0, (uint32_t) (&buffADC_1[0]), 0x00);
	DMA_DoubleBufferModeCmd(DMA2_Stream0, ENABLE);
	//Interrupt on transfer complete enable
	DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE);
	//Enable stream
	DMA_Cmd(DMA2_Stream0, ENABLE);

	/* Configure ADC3 Channel12 pin as analog input ******************************/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* ADC Common Init **********************************************************/
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay =
	ADC_TwoSamplingDelay_15Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);

	/* ADC3 Init ****************************************************************/
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge =
	ADC_ExternalTrigConvEdge_Rising;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_TRGO;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = 1;
	ADC_Init(ADC3, &ADC_InitStructure);

	/* ADC3 regular channel12 configuration *************************************/
	ADC_RegularChannelConfig(ADC3, ADC_Channel_12, 1, ADC_SampleTime_15Cycles);

	/* Enable DMA request after last transfer (Single-ADC mode) */
	ADC_DMARequestAfterLastTransferCmd(ADC3, ENABLE);

	/* Configure Interrupt */
	/* Enable and set DMA Interrupt to the lowest priority */
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable ADC3 DMA */
	ADC_DMACmd(ADC3, ENABLE);
}

void DMA2_Stream0_IRQHandler(void) {
	memory = DMA_GetCurrentMemoryTarget(DMA2_Stream0);
	interrupt = 1;
	DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TCIF0);
}

#ifdef  USE_FULL_ASSERT

/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while (1)
	{
	}
}
#endif

/**
 * @}
 */

/**
 * @}
 */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
