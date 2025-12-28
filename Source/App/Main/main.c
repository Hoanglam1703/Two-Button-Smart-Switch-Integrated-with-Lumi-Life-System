#include "app/framework/include/af.h"
#include "Source/Mid/LED/led.h"
#include "Source/Mid/Button/button.h"
#include "Source/Mid/Kalman_filter/kalman_filter.h"
#include "Source/Mid/LDR/ldr.h"
#include "protocol/zigbee/stack/include/binding-table.h"
#include "Source/App/Network/network.h"
#include "Source/App/Send/send.h"
#include "Source/App/Receive/receive.h"
#include "main.h"
#include "math.h"

#define PERIOD_SCAN_SENSORLIGHT									60000 	//	ms

bool networkReady = false;					// flag danh dau mang da san sang
systemState system_State = POWER_ON_STATE;  // trang thai hien tai cua he thong
uint32_t lux1 = 0;	// bien luu gia tri do sang lan doc hien tai
uint32_t lux2 = 0;  // bien luu gia tri do sang lan doc truoc de so sanh


// Khai bao prototype callback xu ly su kien nut bam
void Main_ButtonPressCallbackHandler(uint8_t button, BUTTON_Event_t pressHandler);
void Main_ButtonHoldCallbackHandler(uint8_t button, BUTTON_Event_t holdingHandler);

// Khai bao callback xu ly su kien mang Zigbee
void Main_networkEventHandler(uint8_t networkResult);

/* Event **************************************************************/
// Event dieu khien viec doc cam bien anh sang
EmberEventControl LightSensor_Read_Control;

// Event dieu khien may trang thai chinh cua he thong
EmberEventControl mainStateEventControl;

// Event tim va tham gia mang Zigbee
EmberEventControl FindNetworkControl;

/** @brief Main Init
 *
 *
 * This function is called from the application's main function. It gives the
 * application a chance to do any initialization required at system startup.
 * Any code that you would normally put into the top of the application's
 * main() routine should be put into this function.
        Note: No callback
 * in the Application Framework is associated with resource cleanup. If you
 * are implementing your application on a Unix host where resource cleanup is
 * a consideration, we expect that you will use the standard Posix system
 * calls, including the use of atexit() and handlers for signals such as
 * SIGTERM, SIGINT, SIGCHLD, SIGPIPE and so on. If you use the signal()
 * function to register your signal handler, please mind the returned value
 * which may be an Application Framework function. If the return value is
 * non-null, please make sure that you call the returned function from your
 * handler to avoid negating the resource cleanup of the Application Framework
 * itself.
 *
 */
void emberAfMainInitCallback(void)
{
	emberAfCorePrintln("Main Init");

	// Khoi tao module dieu khien LED
	ledInit();

	// Khoi tao nut bam va dang ky callback xu ly bam va giu
	buttonInit(Main_ButtonHoldCallbackHandler, Main_ButtonPressCallbackHandler);

	// Khoi tao chuc nang mang Zigbee
	Network_Init(Main_networkEventHandler);

	// Khoi tao cam bien anh sang
	LDRInit();

	// Khoi tao bo loc Kalman voi tham so mac dinh
	KalmanFilterInit(2, 2, 0.001);

	// Kich hoat event xu ly trang thai he thong
	emberEventControlSetActive(mainStateEventControl);

	// Dat thoi gian doc cam bien anh sang lan dau
	emberEventControlSetDelayMS(LightSensor_Read_Control, 1000);
}


/*
 * @func	Main_ButtonPressCallbackHandler
 * @brief	Event Button Handler
 * @param	button, pressHandler
 * @retval	None
 */
void Main_ButtonPressCallbackHandler(uint8_t button, BUTTON_Event_t pressHandler)
{
	switch(pressHandler)
	{
		case press_1:
			// Xu ly su kien bam 1 lan
			if(button == SW_1)
			{
				emberAfCorePrintln("SW1: 1 time");
				turnOnLed(LED1,ledBlue);
				SEND_OnOffStateReport(1, LED_ON);

				// Gui lenh ON toi cac thiet bi da binding
				SEND_BindingInitToTarget(REMOTE_ENDPOINT,
										 LOCAL_ENDPOINT,
										 true,
										 HC_NETWORK_ADDRESS);
			}
			else
			{
				emberAfCorePrintln("SW2: 1 time");
				turnOnLed(LED2,ledBlue);
				SEND_OnOffStateReport(2, LED_ON);
			}
			break;

		case press_2:
			// Xu ly su kien bam 2 lan
			if(button == SW_1)
			{
				emberAfCorePrintln("SW1: 2 times");
				SEND_OnOffStateReport(1, LED_OFF);
				turnOffRBGLed(LED1);

				// Gui lenh OFF toi cac thiet bi da binding
				SEND_BindingInitToTarget(REMOTE_ENDPOINT,
										 LOCAL_ENDPOINT,
										 false,
										 HC_NETWORK_ADDRESS);
			}
			else
			{
				emberAfCorePrintln("SW2: 2 time");
				turnOffRBGLed(LED2);
				SEND_OnOffStateReport(2, LED_OFF);
			}
			break;

		case press_3:
			// Xu ly su kien bam 3 lan
			if(button == SW_1)
			{
				emberAfCorePrintln("SW1: 3 time");

				// Khoi dong che do Find and Bind Target
				emberAfPluginFindAndBindTargetStart(1);
				toggleLed(LED2,ledyellow,3,200,200);
			}
			break;

		case press_4:
			// Xu ly su kien bam 4 lan
			if(button == SW_1)
			{
				emberAfCorePrintln("SW1: 4 time");

				// Khoi dong che do Find and Bind Initiator
				emberAfPluginFindAndBindInitiatorStart(1);
				toggleLed(LED2,ledRGB,3,200,200);
			}
			break;

		case press_5:
			// Xu ly su kien bam 5 lan
			if(button == SW_1)
			{
				emberAfCorePrintln("SW1: 5 time");
			}
			else
			{
				emberAfCorePrintln("SW2: 5 time");
				toggleLed(LED1,ledRed, 2, 200, 200);

				// Chuyen he thong sang trang thai reboot
				system_State = REBOOT_STATE;
				emberEventControlSetDelayMS(mainStateEventControl,3000);
			}
			break;

		default:
			break;
	}
}

/*
 * @func	Main_ButtonHoldCallbackHandler
 * @brief	Event Button Handler
 * @param	button, holdingHandler
 * @retval	None
 */
void Main_ButtonHoldCallbackHandler(uint8_t button, BUTTON_Event_t holdingHandler)
{
	// Chua xu ly hanh dong giu nut
}


/*
 * @func	mainStateEventHandler
 * @brief	Handle Event State Network
 * @param	None
 * @retval	None
 */
void mainStateEventHandler(void)
{
	emberEventControlSetInactive(mainStateEventControl);
	EmberNetworkStatus networkStatusCurrent;

	switch (system_State) {
		case POWER_ON_STATE:
			// Kiem tra trang thai mang ngay sau khi khoi dong
			networkStatusCurrent = emberAfNetworkState();
			if(networkStatusCurrent == EMBER_NO_NETWORK)
			{
				toggleLed(LED1,ledRed,3,200,200);

				// Kich hoat lai event tim mang
				emberEventControlSetInactive(FindNetworkControl);
				emberEventControlSetActive(FindNetworkControl);
			}
			system_State = IDLE_STATE;
			break;

		case REPORT_STATE:
			// Gui thong tin thiet bi ve Home Controller
			system_State = IDLE_STATE;
			SEND_ReportInfoHc();
			break;

		case IDLE_STATE:
			emberAfCorePrintln("IDLE_STATE");
			break;

		case REBOOT_STATE:
			// Xu ly reset va roi khoi mang
			system_State = IDLE_STATE;
			EmberNetworkStatus networkStatus = emberAfNetworkState();
			if (networkStatus != EMBER_NO_NETWORK) {
				SendZigDevRequest();     // Gui ZDO Leave Response cho HC
				emberClearBindingTable();
				emberLeaveNetwork();
			} else {
				halReboot();  // Reboot thiet bi
			}
			break;

		default:
			break;
	}
}

/*
 * @func	Main_networkEventHandler
 * @brief	Handler Event Result Network
 * @param	networkResult
 * @retval	None
 */
void Main_networkEventHandler(uint8_t networkResult)
{
	emberAfCorePrintln("Network Event Handle");

	switch (networkResult) {
		case NETWORK_HAS_PARENT:
			// Thiet bi da ket noi den parent
			emberAfCorePrintln("Network has parent");
			toggleLed(LED1,ledPink,3,300,300);
			networkReady = true;
			system_State = REPORT_STATE;
			emberEventControlSetDelayMS(mainStateEventControl, 1000);
			break;

		case NETWORK_JOIN_FAIL:
			// Gia nhap mang that bai
			system_State = IDLE_STATE;
			toggleLed(LED1,ledBlue,3,300,300);
			emberAfCorePrintln("Network Join Fail");
			emberEventControlSetDelayMS(mainStateEventControl, 1000);
			break;

		case NETWORK_JOIN_SUCCESS:
			// Gia nhap mang thanh cong
			emberEventControlSetInactive(FindNetworkControl);
			emberAfCorePrintln("Network Join Success");
			toggleLed(LED1,ledPink,3,300,300);
			networkReady =true;
			system_State = REPORT_STATE;
			emberEventControlSetDelayMS(mainStateEventControl, 1000);
			break;

		case NETWORK_LOST_PARENT:
			// Mat ket noi voi parent
			emberAfCorePrintln("Network lost parent");
			toggleLed(LED1,ledPink,3,300,300);
			system_State = IDLE_STATE;
			emberEventControlSetDelayMS(mainStateEventControl, 1000);
			break;

		case NETWORK_OUT_NETWORK:
			// Bi day ra khoi mang Zigbee
			if(networkReady)
			{
				toggleLed(LED1,ledRed,3,300,300);
				system_State = REBOOT_STATE;
				emberEventControlSetDelayMS(mainStateEventControl, 3000);
			}
			break;

		default:
			break;
	}
}

void FindNetworkHandler(void)
{
	// Xu ly event tim va tham gia mang Zigbee
	emberEventControlSetInactive(FindNetworkControl);
	NETWORK_FindAndJoin();
	emberEventControlSetDelayMS(FindNetworkControl, 10000);
}

/**
 * @func   LightSensor_Read_1timeHandler
 * @brief  Read value from ADC
 * @param  None
 * @retval None
 */
void LightSensor_Read_Handler(void)
{
	emberEventControlSetInactive(LightSensor_Read_Control);

	// Doc gia tri anh sang tu ADC
	lux1 = LightSensor_AdcPollingRead();

	// Chi gui report khi su thay doi lon hon nguong 30
	if(abs(lux2 - lux1) > 30)
	{
		lux2 = lux1;
		SEND_LDRStateReport(3,lux2);
		emberAfCorePrintln("Light: %d lux",lux2);

		// Dieu khien LED dua tren muc do sang
		if(lux2 > 500)
		{
			turnOnLed(LED2,ledGreen);
		}
		else
		{
			turnOffRBGLed(LED2);
		}
	}

	// Dat lai chu ky doc cam bien
	emberEventControlSetDelayMS(LightSensor_Read_Control,PERIOD_SCAN_SENSORLIGHT);
}
