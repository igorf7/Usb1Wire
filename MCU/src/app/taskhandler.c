/*!
 \file   devicetask.c
 \date   June-2025 (created)
 \brief  device task handler
*/
#include "taskhandler.h"

static uint8_t txBuffer[64];
static AppLayerPacket_t *response = (AppLayerPacket_t*)txBuffer;

/*!
 \brief Sends an app-layer packet to the host
 \param Command code
 \param PDU size
*/
static void SendToHost(uint8_t opcode, uint16_t size)
{
	response->rep_id = eRepId_4;
	size += sizeof(response->rep_id);
	response->opcode = opcode;
	size += sizeof(response->opcode);
    response->data_size = size;
	size += sizeof(response->data_size);
	
	USB_SendToHost(txBuffer, size);
}

/*!
 \brief Search ROM on a 1-Wire bus
 \param optional parameter
*/
void TaskSearchRom(void *prm)
{
	bool result = OW_SearchRom(response->data);
	
	if (!result) {
		memset(response->data, 0, OW_ROM_SIZE);
		OW_ClearSearchResult();
	}
	
	SendToHost(eOwSearchRom, OW_ROM_SIZE);
}

/*!
 \brief Reset/Presence procedure on a 1-Wire bus
 \param optional parameter
*/
void TaskBusReset(void *prm)
{
	response->data[0] = OW_Reset();
	SendToHost(eOwBusReset, 1);
}

/*!
 \brief Writes data on the 1-Wire bus
 \param optional parameter
*/
void TaskBusWrite(void *prm)
{
	AppLayerPacket_t *writedata = (AppLayerPacket_t*)prm;
	
	for (int i = 0; i < writedata->data_size; i++) {
		OW_WriteByte(writedata->data[i]);
	}
	SendToHost(eOwBusWrite, 0);
}

/*!
 \brief Reads data from the 1-Wire bus
 \param optional parameter
*/
void TaskBusRead(void *prm)
{
	uint16_t size = *((uint16_t*)prm);
	
	for (uint16_t i = 0; i < size; i++) {
		response->data[i] = OW_ReadByte();
	}
	SendToHost(eOwBusRead, size);
}
