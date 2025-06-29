/*!
 \file   devicetask.c
 \date   June-2025 (created)
 \brief  device task handler
*/
#include "taskhandler.h"

static uint8_t usbTxBuffer[wMaxPacketSize];

/*!
 \brief Search and enumerate devices on a 1-Wire bus
 \param optional parameter
*/
void TaskEnumerate(void *prm)
{
	bool result = false;
	uint8_t address[OW_ROM_SIZE];
		
	do {
		result = OW_SearchRom(address);
		if (result) {
			USB_SendToHost(eOwEnumerate, address, sizeof(address));
		}
	}
	while (result);
	
	OW_ClearSearchResult();
	USB_SendToHost(eOwEnumerateDone, NULL, 0);
}

/*!
 \brief Reset/Presence procedure on a 1-Wire bus
 \param optional parameter
*/
void TaskBusReset(void *prm)
{
	uint8_t result = OW_Reset();
	USB_SendToHost(eOwBusReset, (uint8_t*)&result, sizeof(result));
}

/*!
 \brief Writes data on the 1-Wire bus
 \param optional parameter
*/
void TaskBusWrite(void *prm)
{
	AppLayerPacket_t *packet = (AppLayerPacket_t*)prm;
	
	for (int i = 0; i < packet->data_size; i++) {
		OW_WriteByte(packet->data[i]);
	}
	USB_SendToHost(eOwBusWrite, NULL, 0);
}

/*!
 \brief Reads data from the 1-Wire bus
 \param optional parameter
*/
void TaskBusRead(void *prm)
{
	uint16_t size = *((uint16_t*)prm);
	
	for (uint16_t i = 0; i < size; i++) {
		usbTxBuffer[i] = OW_ReadByte();
	}
	USB_SendToHost(eOwBusRead, usbTxBuffer, size);
}
