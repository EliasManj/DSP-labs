

/* Includes ------------------------------------------------------------------*/
//#include "stm32f4_discovery.h"
#include "CustomDefines.h"
#include "stm32f4xx_conf.h"
#include "arm_math.h"
#include "Coefficients.h"


/** @addtogroup STM32F4_Discovery_Peripheral_Examples
  * @{
  */

/** @addtogroup ADC_ADC3_DMA
  * @{
  */
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define ADC3_DR_ADDRESS    (uint32_t)(&(ADC3->DR))//((uint32_t)0x4001224C)
#define DAC_DHR12R2_ADDRESS    (uint32_t)(&(DAC->DHR12R2))//0x40007414
//#define BufferSize			1

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO uint16_t ADCMem[BUFFER_SIZE*2];

__IO uint16_t DACMem[BUFFER_SIZE*2];

__IO uint8_t CurrMem=0;
__IO uint8_t InUseMem=0;
__IO uint8_t ADCNewBlock=0;
__IO uint8_t DACReady=0;
extern const uint32_t B[51];
extern const uint16_t BL;
//const uint16_t numTaps=5;
//const q31_t pCoeffs[5]={6000,6000,6000,6000,6000};
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void ADC3_CH12_DMA_Config(void);
void DAC_Ch2_Config(void);
void Tim_Config(void);
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */

int main(void)
{

  uint16_t i;

  q31_t TempIn[BUFFER_SIZE];
  q31_t pState[BUFFER_SIZE + BL-1];
  q31_t TempOut[BUFFER_SIZE];
  q15_t Temp[BUFFER_SIZE];
  static arm_fir_instance_q31 S;
	//Configure Tim2 ADC3 And DAC2 with DMAs, DAC1 Probe signal
  Tim_Config();
  ADC3_CH12_DMA_Config();
  DAC_Ch2_Config();
  /* Enable ADC and DMA */
  ADC_Cmd(ADC3, ENABLE);
  DAC_Cmd(DAC_Channel_2, ENABLE);
  ADC_DMACmd(ADC3, ENABLE);
  DAC_DMACmd(DAC_Channel_2, ENABLE);


  arm_fir_init_q31 (&S, BL, (q31_t *)&B, (q31_t *)&pState, (uint32_t)BUFFER_SIZE);
  while (1)
  {
	  while(!ADCNewBlock);
	  ADCNewBlock=0;
	  InUseMem=1;
	  //uint16_t (with 12 bits ADC) -> Q15
	  for(i=0;i<BUFFER_SIZE;i++){
		  Temp[i]=(q15_t)(ADCMem[i+(CurrMem<<BUFFERAddreLeng)]-2048);

	  }
	  //Q15 -> Q31
	  arm_q15_to_q31((q15_t *)&Temp, (q31_t *)&TempIn, (uint32_t)BUFFER_SIZE);

	  //Simple FIR q31
	  arm_fir_q31(&S,(q31_t *)&TempIn,(q31_t *)&TempOut,(uint32_t)BUFFER_SIZE);
	  arm_scale_q31((q31_t *)&TempOut, (q31_t)0x5FFFFFFF, (int8_t)1,(q31_t *)&TempIn,(uint32_t)BUFFER_SIZE);

	  //Q31 -> Q15
	  arm_q31_to_q15((q31_t *)&TempIn, (q15_t *)&Temp, (uint32_t)BUFFER_SIZE);
	  //Q15 -> uint16_12 with 12 bit DAC
	  for(i=0;i<BUFFER_SIZE;i++){
	  		  DACMem[i+(CurrMem<<BUFFERAddreLeng)]=((uint16_t)(Temp[i]+2048));
	  }

	 InUseMem=0;
	 CurrMem=CurrMem^1;
  }
}



  /* @brief  ADC3 channel12 with DMA configuration*/

void ADC3_CH12_DMA_Config(void)
{
 /* ADC3 configuration *******************************************************/
  ADC_InitTypeDef       ADC_InitStructure;
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
  DMA_InitTypeDef       DMA_InitStructure;
  GPIO_InitTypeDef      GPIO_InitStructure;
  NVIC_InitTypeDef      NVIC_InitStructure;

  /* Enable ADC3, DMA2 and GPIO clocks ****************************************/
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_GPIOC, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);

  /* DMA2 Stream0 channel0 configuration **************************************/
  DMA_InitStructure.DMA_Channel = DMA_Channel_2;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC3_DR_ADDRESS;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)(&ADCMem);
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
  DMA_DoubleBufferModeConfig(DMA2_Stream0, (uint32_t)(&ADCMem[BUFFER_SIZE]), DMA_Memory_0);
  DMA_DoubleBufferModeCmd(DMA2_Stream0, ENABLE);
  DMA_Init(DMA2_Stream0, &DMA_InitStructure);

  DMA_Cmd(DMA2_Stream0, ENABLE);
  //Interrupt on transfer complete enable
  DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE);

  /* Configure ADC3 Channel12 pin as analog input ******************************/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* ADC Common Init **********************************************************/
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
  ADC_CommonInit(&ADC_CommonInitStructure);

  /* ADC3 Init ****************************************************************/
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
  ADC_InitStructure.ADC_ExternalTrigConv=ADC_ExternalTrigConv_T2_TRGO;
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

}
void Tim_Config(void){
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	//uint16_t PrescalerValue = 0;
	  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	  /* Compute the prescaler value */

	  /* Time base configuration */
	  TIM_TimeBaseStructure.TIM_Period = 1903;
	  TIM_TimeBaseStructure.TIM_Prescaler = 0;
	  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	  TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);
	  /* TIM3 enable counter */
	  TIM_Cmd(TIM2, ENABLE);
}
void DAC_Ch2_Config(void)
{
  DMA_InitTypeDef DMA_InitStructure;
  DAC_InitTypeDef DAC_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;


  /* DMA1 clock and GPIOA clock enable (to be used with DAC) */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1 | RCC_AHB1Periph_GPIOA, ENABLE);

  /* DAC Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

  /* DAC channel 1 & 2 (DAC_OUT1 = PA.4)(DAC_OUT2 = PA.5) configuration */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  /* DAC channel2 Configuration */
  DAC_InitStructure.DAC_Trigger = DAC_Trigger_T2_TRGO;
  DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
  DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
  DAC_Init(DAC_Channel_2, &DAC_InitStructure);

  /* DMA1_Stream5 channel7 configuration **************************************/
  //DMA_DeInit(DMA1_Stream5);
  DMA_InitStructure.DMA_Channel = DMA_Channel_7;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(DAC_DHR12R2_ADDRESS);
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)(&DACMem);
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
  DMA_DoubleBufferModeConfig(DMA1_Stream6, (uint32_t)(&DACMem[BUFFER_SIZE]), DMA_Memory_0);
  DMA_DoubleBufferModeCmd(DMA1_Stream6, ENABLE);
  DMA_Init(DMA1_Stream6, &DMA_InitStructure);

  /* Enable DMA1_Stream5 */
  DMA_Cmd(DMA1_Stream6, ENABLE);

}

#ifdef  USE_FULL_ASSERT
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

