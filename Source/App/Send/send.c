#include PLATFORM_HEADER
#include "stack/include/ember.h"
#include "app/framework/include/af.h"
#include "send.h"
#include "Source/App/Receive/receive.h"
#include "zigbee-framework/zigbee-device-common.h"


/**
 * @func    SEND_SendCommandUnicast
 * @brief   Send uinicast command
 * @param   source, destination, address
 * @retval  None
 */
static void SEND_SendCommandUnicast(uint8_t source,
							 uint8_t destination,
							 uint8_t address)
{
	// Thiet lap endpoint nguon va endpoint dich cho goi tin
	emberAfSetCommandEndpoints(source, destination);

	// Gui lenh unicast truc tiep toi thiet bi trung tam
	(void) emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, address);
}


/**
 * @func    SEND_FillBufferGlobalCommand
 * @brief   Send fill buffer global command
 * @param   clusterID, attributeID, globalCommand, value, length, dataType
 * @retval  None
 */
static void SEND_FillBufferGlobalCommand(EmberAfClusterId clusterID,
								  EmberAfAttributeId attributeID,
								  uint8_t globalCommand,
								  uint8_t* value,
								  uint8_t length,
								  uint8_t dataType)
{
	uint8_t data[MAX_DATA_COMMAND_SIZE];

	// Byte thap cua Attribute ID
	data[0] = (uint8_t)(attributeID & 0x00FF);

	// Byte cao cua Attribute ID
	data[1] = (uint8_t)((attributeID & 0xFF00)>>8);

	// Trang thai tra ve thanh cong
	data[2] = EMBER_SUCCESS;

	// Kieu du lieu cua attribute
	data[3] = (uint8_t)dataType;

	// Copy gia tri attribute vao payload
	memcpy(&data[4], value, length);

	// Dong goi frame ZCL global command vao buffer gui
	(void) emberAfFillExternalBuffer(
		(ZCL_GLOBAL_COMMAND |
		 ZCL_FRAME_CONTROL_CLIENT_TO_SERVER |
		 ZCL_DISABLE_DEFAULT_RESPONSE_MASK),
		clusterID,
		globalCommand,
		"b",
		data,
		length + 4);
}


/**
 * @func    SEND_ReportInfoHc
 * @brief   Send Report to HC
 * @param   None
 * @retval  None
 */
void SEND_ReportInfoHc(void)
{
	// Model ID cua thiet bi
	uint8_t modelID[13] = {7, 'S', 'W', '2','_','L','M','1'};

	// Manufacturer name cua thiet bi
	uint8_t manufactureID[5] = {4, 'L', 'u', 'm', 'i'};

	// Phien ban firmware
	uint8_t version = 1;

	// Neu chua join mang Zigbee thi khong gui ban tin
	if(emberAfNetworkState() != EMBER_JOINED_NETWORK){
		return;
	}

	// Gui Model Identifier ve Home Controller
	SEND_FillBufferGlobalCommand(ZCL_BASIC_CLUSTER_ID,
								 ZCL_MODEL_IDENTIFIER_ATTRIBUTE_ID,
								 ZCL_READ_ATTRIBUTES_RESPONSE_COMMAND_ID,
								 modelID,
								 13,
								 ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
	SEND_SendCommandUnicast(LOCAL_ENDPOINT,
							REMOTE_ENDPOINT,
							HC_NETWORK_ADDRESS);

	// Gui Manufacturer Name ve Home Controller
	SEND_FillBufferGlobalCommand(ZCL_BASIC_CLUSTER_ID,
								 ZCL_MANUFACTURER_NAME_ATTRIBUTE_ID,
								 ZCL_READ_ATTRIBUTES_RESPONSE_COMMAND_ID,
								 manufactureID,
								 5,
								 ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
	SEND_SendCommandUnicast(LOCAL_ENDPOINT,
							REMOTE_ENDPOINT,
							HC_NETWORK_ADDRESS);

	// Gui Application Version ve Home Controller
	SEND_FillBufferGlobalCommand(ZCL_BASIC_CLUSTER_ID,
								 ZCL_APPLICATION_VERSION_ATTRIBUTE_ID,
								 ZCL_READ_ATTRIBUTES_RESPONSE_COMMAND_ID,
								 &version,
								 1,
								 ZCL_INT8U_ATTRIBUTE_TYPE);
	SEND_SendCommandUnicast(LOCAL_ENDPOINT,
							REMOTE_ENDPOINT,
							HC_NETWORK_ADDRESS);
}


/**
 * @func    SEND_OnOffStateReport
 * @brief   Send On/Off State
 * @param   Endpoint, value
 * @retval  None
 */
void SEND_OnOffStateReport(uint8_t Endpoint, uint8_t value){
	// Dong goi gia tri On/Off vao frame ZCL
	SEND_FillBufferGlobalCommand(ZCL_ON_OFF_CLUSTER_ID,
						   ZCL_ON_OFF_ATTRIBUTE_ID,
						   ZCL_READ_ATTRIBUTES_RESPONSE_COMMAND_ID,
						   &value,
						   1,
						   ZCL_BOOLEAN_ATTRIBUTE_TYPE);

	// Gui trang thai On/Off ve Home Controller
	SEND_SendCommandUnicast(Endpoint,
							REMOTE_ENDPOINT,
							HC_NETWORK_ADDRESS);

	// Cap nhat gia tri attribute vao attribute table
	emberAfWriteServerAttribute(Endpoint,
								ZCL_ON_OFF_CLUSTER_ID,
								ZCL_ON_OFF_ATTRIBUTE_ID,
								&value,
								ZCL_BOOLEAN_ATTRIBUTE_TYPE);
}


/**
 * @func    SEND_PIRStateReport
 * @brief   Send PIR value to App
 * @param   Endpoint, value
 * @retval  None
 */
void SEND_PIRStateReport(uint8_t Endpoint, uint8_t value){
	// Dong goi trang thai PIR vao frame ZCL
	SEND_FillBufferGlobalCommand(ZCL_IAS_ZONE_CLUSTER_ID,
								 ZCL_ZONE_STATUS_ATTRIBUTE_ID,
								 ZCL_READ_ATTRIBUTES_RESPONSE_COMMAND_ID,
								 &value,
								 1,
						   	   	 ZCL_BOOLEAN_ATTRIBUTE_TYPE);

	// Gui trang thai PIR ve Home Controller
	SEND_SendCommandUnicast(Endpoint,
							REMOTE_ENDPOINT,
							HC_NETWORK_ADDRESS);

	// Ghi gia tri PIR vao attribute table
	emberAfWriteServerAttribute(Endpoint,
								ZCL_IAS_ZONE_CLUSTER_ID,
								ZCL_READ_ATTRIBUTES_RESPONSE_COMMAND_ID,
								&value,
								ZCL_BOOLEAN_ATTRIBUTE_TYPE);
}

/**
 * @func    SEND_LDRStateReport
 * @brief   Send lux value to app
 * @param   source, destination, address
 * @retval  None
 */
void SEND_LDRStateReport(uint8_t Endpoint, uint32_t value){
	// Dong goi gia tri do sang (lux) vao frame ZCL
	SEND_FillBufferGlobalCommand(ZCL_ILLUM_MEASUREMENT_CLUSTER_ID,
								 ZCL_ILLUM_MEASURED_VALUE_ATTRIBUTE_ID,
								 ZCL_READ_ATTRIBUTES_RESPONSE_COMMAND_ID,
								 (uint8_t*) &value,
								 sizeof(value),
								 ZCL_INT32U_ATTRIBUTE_TYPE);

	// Gui gia tri lux ve Home Controller
	SEND_SendCommandUnicast(Endpoint,
							REMOTE_ENDPOINT,
							HC_NETWORK_ADDRESS);

	// Cap nhat gia tri lux vao attribute table
	emberAfWriteServerAttribute(Endpoint,
								ZCL_ILLUM_MEASUREMENT_CLUSTER_ID,
								ZCL_ILLUM_MEASURED_VALUE_ATTRIBUTE_ID,
								(uint8_t*) &value,
								ZCL_INT32U_ATTRIBUTE_TYPE);
}


/**
 * @func    SendZigDevRequest
 * @brief   Send ZigDevRequest
 * @param   None
 * @retval  None
 */
void SendZigDevRequest(void)
{
	uint8_t contents[ZDO_MESSAGE_OVERHEAD + 1];

	// Trang thai phan hoi leave
	contents[1] = 0x00;  // leave thanh cong

	// Gui Zigbee Device Object request ve Home Controller
	(void) emberSendZigDevRequest(HC_NETWORK_ADDRESS,
								 LEAVE_RESPONSE,     // ZDO cmd id
								 EMBER_AF_DEFAULT_APS_OPTIONS,
								 contents,
								 sizeof(contents));
}

/**
 * @func    SEND_BindingInitToTarget
 * @brief   Send Binding command
 * @param   remoteEndpoint, localEndpoint, value, nodeID
 * @retval  None
 */
void SEND_BindingInitToTarget(uint8_t remoteEndpoint,
							 uint8_t localEndpoint,
							 bool value,
							 uint16_t nodeID)
{
	EmberStatus status = EMBER_INVALID_BINDING_INDEX;

	// Duyet toan bo bang binding
	for(uint8_t i = 0; i < EMBER_BINDING_TABLE_SIZE ; i++)
	{
		EmberBindingTableEntry binding;

		// Lay thong tin binding tai vi tri i
		status = emberGetBinding(i, &binding);

		// Lay Node ID cua thiet bi dich trong binding
		uint16_t bindingNodeID = emberGetBindingRemoteNodeId(i);

		// Neu doc binding that bai thi thoat ham
		if(status != EMBER_SUCCESS)
		{
			return;
		}
		// Neu binding da ton tai dung endpoint va node ID thi bo qua
		else if((binding.local == localEndpoint) &&
				(binding.remote == remoteEndpoint) &&
				(bindingNodeID == nodeID))
		{
			continue;
		}
		// Bo qua cac dia chi broadcast
		else if((bindingNodeID != EMBER_SLEEPY_BROADCAST_ADDRESS) &&
				(bindingNodeID != EMBER_RX_ON_WHEN_IDLE_BROADCAST_ADDRESS) &&
				(bindingNodeID != EMBER_BROADCAST_ADDRESS))
		{
			// Chi xu ly binding cho cluster On/Off
			if(binding.local == localEndpoint &&
			   binding.clusterId == ZCL_ON_OFF_CLUSTER_ID)
			{
				switch (value) {
					case true:
						// Gui lenh ON toi thiet bi dich
						emberAfCorePrintln("SEND ON INIT TO TARGET");
						emberAfFillCommandOnOffClusterOn();
						emberAfSetCommandEndpoints(binding.local, binding.remote);
						emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, bindingNodeID);

						// Report trang thai LED ON
						SEND_OnOffStateReport(binding.local, LED_ON);
						break;

					case false:
						// Gui lenh OFF toi thiet bi dich
						emberAfCorePrintln("SEND OFF INIT TO TARGET");
						emberAfFillCommandOnOffClusterOff();
						emberAfSetCommandEndpoints(binding.local, binding.remote);
						emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, bindingNodeID);

						// Report trang thai LED OFF
						SEND_OnOffStateReport(binding.local, LED_OFF);
						break;
				}
			}
		}
	}
}
