#include "at.h"
#include "user_interface.h"
#include "osapi.h"
#include "driver/uart.h"

/** @defgroup AT_PORT_Defines
  * @{
  */
#define at_cmdLenMax 128
#define at_dataLenMax 2048
/**
  * @}
  */

/** @defgroup AT_PORT_Extern_Variables
  * @{
  */
extern uint16_t at_sendLen;
extern uint16_t at_tranLen;
//extern UartDevice UartDev;
extern bool IPMODE;
extern os_timer_t at_delayChack;
/**
  * @}
  */

/** @defgroup AT_PORT_Extern_Functions
  * @{
  */
extern void at_ipDataSending(uint8_t *pAtRcvData);
/**
  * @}
  */

os_event_t    at_recvTaskQueue[at_recvTaskQueueLen];
os_event_t    at_procTaskQueue[at_procTaskQueueLen];

BOOL specialAtState = TRUE;
at_stateType  at_state;
uint8_t *pDataLine;

static uint8_t at_cmdLine[at_cmdLenMax];
uint8_t at_dataLine[at_dataLenMax];/////
//uint8_t *at_dataLine;

/** @defgroup AT_PORT_Functions
  * @{
  */

static void at_procTask(os_event_t *events);

/**
  * @brief  Uart receive task.
  * @param  events: contain the uart receive data
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_recvTask(void)
{
  static uint8_t atHead[2];
  static uint8_t *pCmdLine;
  uint8_t temp;

//  temp = events->par;
  temp = READ_PERI_REG(UART_FIFO(UART0)) & 0xFF;
//  temp = 'X';
  //add transparent determine
  if(at_state != at_statIpTraning)
  {
    if(temp != '\n')
    {
      uart_tx_one_char(temp); //display back
    }
  }

  switch (at_state)
  {
  case at_statIdle: //serch "AT" head
    atHead[0]=atHead[1];
    atHead[1]=temp;
    if((os_memcmp(atHead, "AT", 2) == 0) || (os_memcmp(atHead, "at", 2) == 0))
    {
      at_state = at_statRecving;
      pCmdLine = at_cmdLine;
      atHead[1] = 0x00;
    }
    else if(temp == '\r') //only get enter
    {
      uart0_sendStr("\r\nError\r\n");
    }
    break;

  case at_statRecving: //push receive data to cmd line
    *pCmdLine = temp;
    if(temp == '\r')
    {
      system_os_post(at_procTaskPrio, 0, 0);
      at_state = at_statProcess;
      uart0_sendStr("\r\n"); ///////////
    }
    else if (pCmdLine == &at_cmdLine[at_cmdLenMax-1])
    {
      at_state = at_statIdle;
    }
    pCmdLine++;
    break;

  case at_statProcess: //process data
    if(temp == '\r')
    {
      uart0_sendStr("\r\nbusy now ...\r\n");
    }
    break;

  case at_statIpSending:
  	*pDataLine = temp;
		if ((pDataLine == &at_dataLine[at_sendLen-1]) ||
		    (pDataLine == &at_dataLine[at_dataLenMax-1]))
    {
      system_os_post(at_procTaskPrio, 0, 0);
      at_state = at_statIpSended;
    }
    pDataLine++;
//    *pDataLine = temp;
//    if (pDataLine == &UartDev.rcv_buff.pRcvMsgBuff[at_sendLen-1])
//    {
//      system_os_post(at_procTaskPrio, 0, 0);
//      at_state = at_statIpSended;
//    }
//    pDataLine++;
    break;

  case at_statIpSended: //send data
    if(temp == '\r')
    {
      uart0_sendStr("busy\r\n");
    }
    break;

  case at_statIpTraning:
    os_timer_disarm(&at_delayChack);
    *pDataLine = temp;
    if (pDataLine == &at_dataLine[at_dataLenMax-1])
    {
      ETS_UART_INTR_DISABLE();
//      pDataLine++;
      at_tranLen++;
      os_timer_arm(&at_delayChack, 1, 0);
      break;
    }
    pDataLine++;
    at_tranLen++;
    os_timer_arm(&at_delayChack, 20, 0);
    break;

  default:
    if(temp == '\r')
    {
    }
    break;
  }
}

/**
  * @brief  Task of process command or txdata.
  * @param  events: no used
  * @retval None
  */
static void ICACHE_FLASH_ATTR
at_procTask(os_event_t *events)
{
  if(at_state == at_statProcess)
  {
    at_cmdProcess(at_cmdLine);
    if(specialAtState)
    {
      at_state = at_statIdle;
    }
  }
  else if(at_state == at_statIpSended)
  {
    at_ipDataSending(at_dataLine);//UartDev.rcv_buff.pRcvMsgBuff);
    if(specialAtState)
    {
      at_state = at_statIdle;
    }
  }
}

/**
  * @brief  Initializes build two tasks.
  * @param  None
  * @retval None
  */
void ICACHE_FLASH_ATTR
at_init(void)
{
//  system_os_task(at_recvTask, at_recvTaskPrio, at_recvTaskQueue, at_recvTaskQueueLen);
  system_os_task(at_procTask, at_procTaskPrio, at_procTaskQueue, at_procTaskQueueLen);
}

/**
  * @}
  */
