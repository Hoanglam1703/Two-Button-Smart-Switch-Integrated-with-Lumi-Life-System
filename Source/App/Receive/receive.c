#include "app/framework/include/af.h"
#include "Source/Mid/LED/led.h"
#include "Source/App/Send/send.h"
#include "receive.h"

/**
 * @func    emberAfPreCommandReceivedCallback
 * @brief   Process Command Received
 * @param   EmberAfClusterCommand
 * @retval  boolean
 */
boolean emberAfPreCommandReceivedCallback(EmberAfClusterCommand* cmd)
{
	// Chi xu ly cac lenh cluster specific (khong xu ly global command)
	if(cmd->clusterSpecific)
	{
		// Phan loai cluster dua theo Cluster ID
		switch(cmd->apsFrame->clusterId)
		{
			case ZCL_ON_OFF_CLUSTER_ID:
				// Xu ly cac lenh thuoc cluster On/Off
				RECEIVE_HandleOnOffCluster(cmd);
				break;

			case ZCL_LEVEL_CONTROL_CLUSTER_ID:
				// Xu ly cac lenh dieu khien muc do sang (Level Control)
//				RECEIVE_HandleLevelControlCluster(cmd);
				break;

			default:
				// Khong xu ly cac cluster khac
				break;
		}
	}
	// Tra ve false de cho phep framework tiep tuc xu ly neu can
	return false;
}


/**
 * @func    emberAfPreMessageReceivedCallback
 * @brief   Process Pre message received
 * @param   EmberAfIncomingMessage
 * @retval  None
 */
boolean emberAfPreMessageReceivedCallback(EmberAfIncomingMessage* incommingMessage)
{
	// Kiem tra neu message la phan hoi Active Endpoints (ZDO)
	if(incommingMessage->apsFrame->clusterId == ACTIVE_ENDPOINTS_RESPONSE)
	{
		// Chan khong cho framework xu ly tiep ban tin nay
		return true;
	}
	// Cac message khac cho phep framework xu ly binh thuong
	return false;
}


/**
 * @func    RECEIVE_HandleLevelControlCluster
 * @brief   Handler Level led
 * @param   EmberAfClusterCommand
 * @retval  None
 */
//bool RECEIVE_HandleLevelControlCluster(EmberAfClusterCommand* cmd)
//{
//	uint8_t commandID = cmd->commandId;
//	uint8_t endPoint  = cmd->apsFrame -> destinationEndpoint;
//
//	// Vi tri bat dau payload trong buffer nhan duoc
//	uint8_t payloadOffset = cmd->payloadStartIndex;
//
//	uint8_t level;
//	uint16_t transitionTime;
//
//	// In ra Cluster ID de debug
//	emberAfCorePrintln("ClusterID: 0x%2X", cmd->apsFrame->clusterId);
//
///******************************************LEVEL CONTROL LED***************************************************************************/
//	switch(commandID)
//	{
//		case ZCL_MOVE_TO_LEVEL_COMMAND_ID:
//
//			// Kiem tra do dai payload co hop le hay khong
//			if(cmd->bufLen < payloadOffset + 1u)
//			{
//				// Payload loi dinh dang
//				return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
//			}
//
//			// Doc gia tri level (1 byte) tu payload
//			level = emberAfGetInt8u(cmd->buffer, payloadOffset, cmd->bufLen);
//
//			// Doc thoi gian chuyen doi (2 byte)
//			transitionTime = emberAfGetInt16u(cmd->buffer, payloadOffset+1, cmd->bufLen);
//
//			emberAfCorePrintln(" RECEIVE_HandleLevelControlCluster LEVEL = %d, time: 0x%2X\n", level, transitionTime);
//
//			// Chi xu ly cho endpoint 1
//			if(endPoint == 1)
//			{
//				// Dieu khien mau LED theo muc level nhan duoc
//				if(level >=80)
//				{
//					emberAfCorePrintln("LED GREEN");
//					turnOnLed(LED1, ledGreen);
//				}
//				else if(level>=40)
//				{
//					emberAfCorePrintln("LED RED");
//					turnOnLed(LED1, ledRed);
//				}
//				else if(level>0)
//				{
//					emberAfCorePrintln("LED BLUE");
//					turnOnLed(LED1, ledBlue);
//				}
//				else
//				{
//					emberAfCorePrintln("turn off");
//					turnOffRBGLed(LED1);
//				}
//			}
//			break;
//
//		default:
//			// Khong xu ly cac lenh level control khac
//			break;
//	}
//
//	return false;
//}


/**
 * @func    RECEIVE_HandleOnOffCluster
 * @brief   Handler On/Off command
 * @param   EmberAfClusterCommand
 * @retval  bool
 */
bool RECEIVE_HandleOnOffCluster(EmberAfClusterCommand* cmd)
{
	uint8_t commandID = cmd->commandId;

	// Endpoint dich nhan lenh
	uint8_t remoteEndpoint = cmd ->apsFrame -> destinationEndpoint;

	// Endpoint nguon gui lenh
	uint8_t localEndpoint = cmd->apsFrame -> sourceEndpoint;

	// Node ID cua thiet bi gui lenh
	uint16_t localNodeID = cmd->source;

/************************************ON-OFF LED******************************************************************************************/
	emberAfCorePrintln(
		"RECEIVE_HandleOnOffCluster SourceEndpoint = %d, RemoteEndpoint = %d, commandID = %d, nodeID %2X\n",
		localEndpoint,
		remoteEndpoint,
		commandID,
		localNodeID);

	switch(commandID)
	{
	case ZCL_OFF_COMMAND_ID:

		emberAfCorePrintln("Turn off LED");

		// Phan loai theo kieu ban tin nhan (unicast hay multicast)
		switch (cmd->type) {

			case EMBER_INCOMING_UNICAST:

				// Xu ly tat LED tai endpoint 1
				if(localEndpoint == 1)
				{
					turnOffRBGLed(LED1);
					SEND_OnOffStateReport(localEndpoint, LED_OFF);

					// Kiem tra xem co binding nao hay khong
					emberAfCorePrintln("check: %d",checkBindingTable(localEndpoint));
					if(checkBindingTable(localEndpoint) >= 1)
					{
						// Gui lenh OFF den thiet bi duoc binding
						SEND_BindingInitToTarget(remoteEndpoint, localEndpoint, false, localNodeID);
					}
				}

				// Xu ly tat LED tai endpoint 2
				if(localEndpoint == 2)
				{
					turnOffRBGLed(LED2);
					SEND_OnOffStateReport(localEndpoint, LED_OFF);
					if(checkBindingTable(localEndpoint) >= 1)
					{
						SEND_BindingInitToTarget(remoteEndpoint, localEndpoint, true, localNodeID);
					}
				}
				break;

//			case EMBER_INCOMING_MULTICAST:
//				// Neu nhan lenh multicast thi tat tat ca LED
//				emberAfCorePrintln("Multicast");
//				turnOnLed(LED1, ledOff);
//				turnOnLed(LED2, ledOff);
//				break;

			default:
				break;
		}
		break;

	case ZCL_ON_COMMAND_ID:

		emberAfCorePrintln("Turn on LED");

		switch (cmd->type) {

			case EMBER_INCOMING_UNICAST:

				// Xu ly bat LED tai endpoint 1
				if(localEndpoint == 1)
				{
					turnOnLed(LED1, ledBlue);
					SEND_OnOffStateReport(localEndpoint, LED_ON);

					// Neu co binding thi gui lenh ON sang thiet bi dich
					if(checkBindingTable(localEndpoint) >= 1)
					{
						SEND_BindingInitToTarget(remoteEndpoint, localEndpoint, true, localNodeID);
					}
				}

				// Xu ly bat LED tai endpoint 2
				if(localEndpoint == 2)
				{
					turnOnLed(LED2, ledBlue);
					SEND_OnOffStateReport(localEndpoint, LED_ON);
					if(checkBindingTable(localEndpoint) >= 1)
					{
						SEND_BindingInitToTarget(remoteEndpoint, localEndpoint, true, localNodeID);
					}
				}
				break;

//			case EMBER_INCOMING_MULTICAST:
//				// Lenh multicast ON
//				emberAfCorePrintln("Multicast");
//				turnOnLed(LED2, ledGreen);
//				break;

			default:
				break;
		}
		break;

	default:
		break;
	}

	return false;
}

/*
 * @function 			: checkBindingTable
 *
 * @brief				: API support to check information on binding table.
 *
 * @parameter			: localEndpoint
 *
 * @return value		: True or false
 */

uint8_t checkBindingTable(uint8_t localEndpoint)
{
	uint8_t index = 0;

	// Duyet toan bo bang binding
	for(uint8_t i=0; i< EMBER_BINDING_TABLE_SIZE; i++)
	{
		EmberBindingTableEntry binding;

		// Bo qua cac dia chi broadcast
		if(emberGetBindingRemoteNodeId(i) != EMBER_SLEEPY_BROADCAST_ADDRESS)
		{
			emberGetBinding(i, &binding);

			// Kiem tra binding unicast trung endpoint
			if(binding.local == localEndpoint &&
			   (binding.type == EMBER_UNICAST_BINDING))
			{
				index++;
			}
		}
	}

	// Tra ve so luong binding tim duoc
	return index;
}
