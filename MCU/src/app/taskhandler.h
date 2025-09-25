/*!
 \file   taskhandler.h
 \date   June-2025 (created)
 \brief  task handler header file
*/
#ifndef __TASKHANDLER_H
#define __TASKHANDLER_H

#include "stm32f10x.h"
#include "onewire.h"
#include "usb_pwr.h"
#include "usb_desc.h"
#include "usb_endp.h"
#include "usb_prop.h"
#include "usb_hw_config.h"
#include "usb_lib.h"
#include "string.h"

#define SCRATCHPAD_SIZE	8U

/* Device Opcodes */
typedef enum
{
    eOwSearchRom = (uint8_t)1,
    eOwBusReset,
    eOwBusWrite,
    eOwBusRead,
	eUsbHeartbeat
} AppLayerOpcode_t;

/* Application Layer Packet Structure */
typedef struct
{
    uint8_t rep_id;     // report id
    uint8_t opcode;     // comand code
    uint8_t data_size;  // data field size
    uint8_t data[];		// data field address
} AppLayerPacket_t;

/* API */
void TaskSearchRom(void *prm);
void TaskBusReset(void *prm);
void TaskBusWrite(void *prm);
void TaskBusRead(void *prm);
#endif // __TASKHANDLER_H
