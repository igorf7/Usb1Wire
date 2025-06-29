/*!
 \file   main.c
 \date   November-2024 (created)
 \brief  main application module
*/
#include "main.h"
#include "scheduler.h"
#include "systick.h"
#include "taskhandler.h"

static HidProp_t hidProp;
static HidEndp_t hidEndp;
static uint32_t usbHeartbeat = 0x01020304;
static bool isHeartbeatSend = false;
static uint8_t usbRxBuffer[wMaxPacketSize];

/*!
 \brief Program entry point
*/
int main(void)
{
    /* Initialize the USB hardware */
    Wait_ticks(3000000);
    hidEndp.rx_buffer = usbRxBuffer;
    hidEndp.rx_handler = USB_HandleRxData;
    USB_SetHidEndp(&hidEndp);
    hidProp.rx_buffer = usbRxBuffer;
    hidProp.rx_handler = USB_HandleRxData;
    USB_SetHidProp(&hidProp);
    USB_Interrupts_Config();
    Set_USBClock();
    USB_Init();
    
    /* Program startup indicator */
    LED_Init(LED1_PORT, LED1_PIN);
    LED_Blink(LED1_PORT, LED1_PIN, 1000000);
    
    /* Initialize the task scheduler */
    InitTaskSheduler(&BackgroundTask);
    
    /* Initialize the 1-Wire Bus */
    OW_InitBus();
    
    /* Initializing the SysTick Timer */
    InitSystickTimer(SysTick_Callback);
    StartSystickTimer(300);
	
    /* Enable Watchdog */
    #ifndef DEBUG
    InitWatchdog();
    #endif
	
    __enable_irq();
    
    /* Mailoop */
    while (1)
	{
        RunTaskSheduler();
	}
}

/*!
 \brief Background task
*/
static void BackgroundTask(void)
{
    /* Reload watchdog */
    #ifndef DEBUG
    WatchdogReload(KR_KEY_Reload);
    #endif
    
    /* Send USB Heartbeat */
    if (isHeartbeatSend) {
        isHeartbeatSend = false;
        USB_SendToHost(eUsbHeartbeat, (uint8_t*)&usbHeartbeat, sizeof(usbHeartbeat));
    }
}

/*!
 \brief USB receive callback function
*/
void USB_HandleRxData(void)
{
	/* Check report ID */
    if (usbRxBuffer[0] == eRepId_3) {
		/* Check app layer opcode */
        switch (usbRxBuffer[1])
        {
            case eOwEnumerate:
                PutTask(TaskEnumerate, NULL);
        		break;
			case eOwBusReset:
				PutTask(TaskBusReset, NULL);
				break;
			case eOwBusWrite:
				PutTask(TaskBusWrite, usbRxBuffer);
				break;
			case eOwBusRead:
				PutTask(TaskBusRead, usbRxBuffer);
				break;
        	default:
        		break;
        }
    }
}

/*!
 \brief SysTick event callback function
*/
void SysTick_Callback(void)
{
    isHeartbeatSend = true;
}
