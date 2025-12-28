#include "app/framework/include/af.h"
#include <math.h>
#include <Source/Mid/Kalman_filter/kalman_filter.h>
#include <Source/Mid/LDR/ldr.h>


volatile uint32_t lux = 0; // bien luu gia tri cam bien anh sang sau khi loc
uint32_t valueADC=0;	   // bien luu gia tri cam bien anh sang truoc khi loc

/**
 * @func    LDRInit
 * @brief   LDR initialize
 * @param   None
 * @retval  None
 */
void LDRInit(void)
{
	  // Cau hinh tong quat
	  IADC_Init_t init = IADC_INIT_DEFAULT;

	  // Cau hinh cac tham so co ban cho ADC
	  IADC_AllConfigs_t initAllConfigs = IADC_ALLCONFIGS_DEFAULT;

	  // Khoi tao cau hinh o single mode
	  IADC_InitSingle_t initSingle = IADC_INITSINGLE_DEFAULT;

	  // Khoi tao cau hinh chon dau vao cho ADC
	  IADC_SingleInput_t initSingleInput = IADC_SINGLEINPUT_DEFAULT;

	  // Enable IADC clock
	  CMU_ClockEnable(cmuClock_IADC0, true);
	  CMU_ClockEnable(cmuClock_GPIO, true);

	  // Reset ADC
	  IADC_reset(IADC0);

	  // Cau hinh nguon clock cho IADC
	  CMU_ClockSelectSet(cmuClock_IADCCLK, cmuSelect_FSRCO);

	  // Giu trang thai san sang sau khi do xong
	  init.warmup = iadcWarmupKeepWarm;

	  // Chia he so tu nguon clock
	  init.srcClkPrescale = IADC_calcSrcClkPrescale(IADC0, CLK_SRC_ADC_FREQ, 0);

	  // Configuration 0 is used by both scan and single conversions by default
	  // Cai hinh dien ap tham chieu
	  initAllConfigs.configs[0].reference = iadcCfgReferenceVddx;

	  // Chia CLK_SRC_ADC de set tan so lay mau cho CLK_ADC
	  initAllConfigs.configs[0].adcClkPrescale = IADC_calcAdcClkPrescale(IADC0,CLK_ADC_FREQ,0,iadcCfgModeNormal,init.srcClkPrescale);

	  // Set oversampling rate to 32x
	  // do phan giai luc nay la 16 bit
	  initAllConfigs.configs[0].osrHighSpeed = iadcCfgOsrHighSpeed32x;

	  // Set so mau hop le de doc
	  initSingle.dataValidLevel = _IADC_SINGLEFIFOCFG_DVL_VALID1;

	  // Cau hinh dau vao cho che do single ended
	  initSingleInput.posInput = iadcPosInputPortCPin5;
	  initSingleInput.negInput = iadcNegInputGnd;

	  // Initialize IADC
	  IADC_init(IADC0, &init, &initAllConfigs);

	  // Initialize Scan
	  IADC_initSingle(IADC0, &initSingle, &initSingleInput);

	  // Allocate the analog bus for ADC0 inputs
	  GPIO->IADC_INPUT_0_BUS |= GPIO_CDBUSALLOC_CDODD0_ADC0;  //IADC_INPUT_BUSALLOC
}

/**
 * @func   LightSensor_AdcPollingReadHandler
 * @brief  Read value from ADC
 * @param  None
 * @retval None
 */
uint32_t LightSensor_AdcPollingRead(void)
{
	IADC_Result_t iadcResult;

	// Bat dau qua trinh chuyen doi
	IADC_command(IADC0, iadcCmdStartSingle);

	// Doi qua trinh chuyen doi hoan tat
	// Khi bit 8 va 6 chua co ket qua la 1 va 0 tuc la da chuyen doi xong va co du lieu thi tiep tuc doi
	while((IADC0->STATUS & (_IADC_STATUS_CONVERTING_MASK | _IADC_STATUS_SINGLEFIFODV_MASK)) != IADC_STATUS_SINGLEFIFODV);

	// Lay gia tri ADC
	iadcResult = IADC_pullSingleFifoResult(IADC0);
	valueADC = iadcResult.data;

	// Su dung kalman de loc nhieu
	lux = Kalman_sensor(valueADC);

	return lux;
}
