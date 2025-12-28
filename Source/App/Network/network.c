#include "app/framework/include/af.h"
#include "network.h"

// Event dieu khien qua trinh join mang Zigbee
EmberEventControl joinNetworkEventControl;

// Bien dem so lan thu tim va join mang
uint32_t timeFindAndJoin = 0;

// Con tro ham callback thong bao trang thai mang len lop tren
networkEventHandler networkEventHandle = NULL;


/*
 * @function 			: Network_Init
 *
 * @brief				: Handle event network.
 *
 * @parameter			: networkEventHandler
 *
 * @return value		: None
 */
void Network_Init(networkEventHandler networkResult)
{
	// Luu lai ham callback de thong bao ket qua mang
	networkEventHandle = networkResult;
}


/*
 * @function 			: NETWORK_FindAndJoin
 *
 * @brief				: Find network
 *
 * @parameter			: None
 *
 * @return value		: None
 */
void NETWORK_FindAndJoin(void)
{
	// Chi thuc hien tim mang khi thiet bi chua tham gia mang Zigbee
	if(emberAfNetworkState() == EMBER_NO_NETWORK)
	{
		// Kich hoat event join mang sau 2 giay
		emberEventControlSetDelayMS(joinNetworkEventControl, 2000);
	}
}


/*
 * @function 			: NETWORK_StopFindAndJoin
 *
 * @brief				: Stop find network
 *
 * @parameter			: None
 *
 * @return value		: None
 */
void NETWORK_StopFindAndJoin(void)
{
	// Dung plugin network steering (dung qua trinh tim va join mang)
	emberAfPluginNetworkSteeringStop();
}


/*
 * @function 			: joinNetworkEventHandler
 *
 * @brief				: Handle Event Join network
 *
 * @parameter			: None
 *
 * @return value		: None
 */
void joinNetworkEventHandler(void)
{
	// Tat event sau khi da duoc goi
	emberEventControlSetInactive(joinNetworkEventControl);

	// Neu van chua vao mang thi bat dau thuc hien network steering
	if(emberAfNetworkState() == EMBER_NO_NETWORK)
	{
		// Bat dau tim va join mang Zigbee tu dong
		emberAfPluginNetworkSteeringStart();

		// Tang bien dem so lan thu tim va join mang
		timeFindAndJoin++;
	}
}


/*
 * @function 			: emberAfStackStatusCallback
 *
 * @brief				: Stack Status
 *
 * @parameter			: EmberStatus
 *
 * @return value		: True or false
 */
boolean emberAfStackStatusCallback(EmberStatus status)
{
	emberAfCorePrintln("emberAfStackStatusCallback\n");

	// Truong hop mang Zigbee da hoat dong
	if(status == EMBER_NETWORK_UP)
	{
		// Neu da co it nhat 1 lan thu join mang
		if(timeFindAndJoin > 0) // vao mang thanh cong
		{
			// Dung viec tiep tuc tim mang
			NETWORK_StopFindAndJoin();

			// Thong bao su kien join mang thanh cong
			if(networkEventHandle != NULL)
			{
				networkEventHandle(NETWORK_JOIN_SUCCESS);
				emberAfCorePrintln("NETWORK_JOIN_SUCCESS");
			}
		}
		else
		{
			// Truong hop thiet bi co san parent (da o trong mang tu truoc)
			if(networkEventHandle != NULL)
			{
				networkEventHandle(NETWORK_HAS_PARENT);
				emberAfCorePrintln("NETWORK_HAS_PARENT");
			}
		}
	}
	else
	{
		// Kiem tra trang thai mang hien tai khi stack khong o trang thai UP
		EmberNetworkStatus networkStatusCurrent = emberAfNetworkState();

		// Truong hop thiet bi bi roi khoi mang
		if(networkStatusCurrent == EMBER_NO_NETWORK)
		{
			if(networkEventHandle != NULL)
			{
				networkEventHandle(NETWORK_OUT_NETWORK);
				emberAfCorePrintln("NETWORK_OUT_NETWORK");
			}
		}
		// Truong hop da join mang nhung mat parent
		else if(networkStatusCurrent == EMBER_JOINED_NETWORK_NO_PARENT)
		{
			emberAfCorePrintln("NETWORK_LOST_PARENT");
			networkEventHandle(NETWORK_LOST_PARENT);
		}
	}

	// Truong hop join mang that bai
	if(status == EMBER_JOIN_FAILED)
	{
		emberAfCorePrintln("NETWORK_JOIN_FAIL");
		networkEventHandle(NETWORK_JOIN_FAIL);
	}

	return false;
}
