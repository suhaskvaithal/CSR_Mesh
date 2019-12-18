/******************************************************************************
 *  Copyright 2015 - 2016 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.1
 *  Application version 2.1.0
 *
 *  FILE
 *      main_app.c
 *
 *  DESCRIPTION
 *      This file implements the CSR Mesh application.
 *
 *****************************************************************************/
#include "user_config.h"
#include "byte_queue.h"     /* Interface to this source file */

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/
#ifdef CSR101x_A05
#include <ls_app_if.h>
#include <config_store.h>
#include <csr_ota.h>
#include <uart.h>
#include <random.h>
#else
#include <uart.h>           /* Functions to interface with the UART */
#include <uart_sdk.h>       /* Enums to interface with the UART */
#include <configstore_api.h>
#include <configstore_id.h> 
#include <mem.h>
#endif /* !CSR101x_A05 */
#include <timer.h>
#include <uart.h>
#include <pio.h>
#include <nvm.h>
#include <debug.h>
/*============================================================================*
 *  Local Header Files
 *============================================================================*/

#include "app_debug.h"
#include "battery_hw.h"
#include "app_hw.h"
#include "nvm_access.h"
#include "main_app.h"
#include "gap_service.h"
#include "gatt_service.h"
#include "mesh_control_service.h"
#include "advertisement_handler.h"
#include "connection_handler.h"
#include "mesh_control_service.h"
#include "conn_param_update.h"
#if !defined(CSR101x_A05)
#include "gaia_service.h"
#ifdef GAIA_OTAU_SUPPORT
#include "gaia_otau_api.h"
#include "app_otau_handler.h"
#endif /* GAIA_OTAU_SUPPORT  */
#ifdef GAIA_OTAU_RELAY_SUPPORT
#include "gaia_client_service.h"
#include "gaia_otau_client_api.h"
#endif
#else
#include "csr_ota_service.h"
#endif /* (!CSR101x_A05) */
#include <data_server.h>
#include <data_client.h>
#include "app_mesh_handler.h"
#include "iot_hw.h"     
#define BAUD_RATE (0x01d9) /*1152*/

/*============================================================================*
 *  Private Definitions
 *============================================================================*/
#ifndef CSR101x_A05
/* Standard setup of CSR102x boards */
#define UART_PIO_TX                    (8)
#define UART_PIO_RX                    (9)
#define UART_PIO_RTS                   (PIO_NONE)
#define UART_PIO_CTS                   (PIO_NONE)
#define USER_KEY1                      (69)
#else
/* The User key index where the application config flags are stored */
#define CSKEY_INDEX_USER_FLAGS         (0)
#endif /* !CSR101x_A05 */

/* NVM Store ID */
#define NVM_ID                          (2)

/*============================================================================*
 *  Private Data
 *===========================================================================*/
/* Declare space for application timers. */
static uint16 app_timers[SIZEOF_APP_TIMER * MAX_APP_TIMERS];

/*============================================================================*
 *  Private Function Prototypes
 *============================================================================*/
/* UART Receive callback */

static uint16 UartDataRxCallback ( void* p_rx_data, uint16 data_count,
                                   uint16* p_num_additional_words );
static void processRxCmd(void);
uint16 atoh( const uint8 *string);
#ifndef CSR101x_A05

/* Cached Value of UUID. */
uint16 cached_uuid[UUID_LENGTH_WORDS];

/* Cached Value of Authorization Code. */
#ifdef USE_AUTHORISATION_CODE
uint16 cached_auth_code[AUTH_CODE_LENGTH_WORDS];
#endif /* USE_AUTHORISATION_CODE */

#endif /* !CSR101x_A05 */

/*============================================================================*
 *  Private Function Implementations
 *============================================================================*/

/*-----------------------------------------------------------------------------*
 *  NAME
 *      UartDataRxCallback
 *
 *  DESCRIPTION
 *      This callback is issued when data is received over UART. Application
 *      may ignore the data, if not required. For more information refer to
 *      the API documentation for the type "uart_data_out_fn"
 *
 *  RETURNS
 *      The number of words processed, return data_count if all of the received
 *      data had been processed (or if application don't care about the data)
 *
 *----------------------------------------------------------------------------*/
UART_DECLARE_BUFFER(rx_data,UART_BUF_SIZE_BYTES_32);
UART_DECLARE_BUFFER(tx_data,UART_BUF_SIZE_BYTES_32);

uint16 atoh( const uint8 *string)
{
    uint16 val=0,dig;
    uint8 c;
    while((c=*string++)!='\0')
    {
        if((c>='0')&&(c<='9'))
            dig = (uint16)(c-'0');
        else if((c>='a')&&(c<='f'))
            dig = (uint16)(c-'a')+10;
        else if((c>='A')&&(c<='F'))
            dig = (uint16)(c-'A')+10;
        else
            break;
        
        val = (val<<4)+dig;      
     
    }
    return val;
}

static void processRxCmd(void)
{
    /* Loop until the byte queue is empty */
    while (BQGetDataSize() > 0)
    {
        static uint8 byte = '\0';
        static uint8 array[5]= {'\0','\0','\0','\0','\0'};
        
         uint8 strom[11] = {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'};
        
        
        
        /* Read in the next byte */
        if (BQPopBytes(&byte, 1) > 0)
        {
            
            switch(byte)
            {
                 
                  case 'z':
                  case 'Z':
                            if (BQPopBytes(&byte, 1) > 0)
                            { 
                                if(byte==' ')
                                {
                                                if (BQPopBytes(strom, 11) > 0)
                                                {
                                                  /*TimeDelayUSec(Random16());*/
                                                  /*StreamSendDataBlock(atoh(&array[0]),&strom[0],10);*/
                                                  /*send_param.datagramoctets_len =10;*/
                                                   CSRMESH_DATA_BLOCK_SEND_T send_param;
                                                   uint8 i,pl_cnt=0;
                                                   send_param.datagramoctets_len =0;
                                                   
                                                   for(i=0;i<=10;i++)
                                                   {
                                                       if(strom[i]!='\r')
                                                       {
                                                           send_param.datagramoctets [i] = strom[i];
                                                           pl_cnt ++;
                                                            
                                                       }
                                                       else
                                                       {
                                                           
                                                           i=10;
                                                           goto L1;
                                                           
                                                       }
                                                       
                                                   }
                                                   L1:
                                                  send_param.datagramoctets_len = pl_cnt;
                                                  DataBlockSend(0,(atoh(&array[0])),12,&send_param);
                                                }
                                                
                                            }
                        
                        
                                }
                            break;
                            
                  case 'D':                  
                  case 'd':
                            {
                               if (BQPopBytes(&byte, 1) > 0)
                               { 
                                  if(byte==' ')
                                  {
                                        if (BQPopBytes(&array[0], 4) > 0)
                                      {
                                          if (BQPopBytes(&byte, 1) > 0)
                                          { 
                                              if(byte==' ')
                                              {
                                                if (BQPopBytes(strom, 11) > 0)
                                                {
                                                  /*TimeDelayUSec(Random16());*/
                                                  /*StreamSendDataBlock(atoh(&array[0]),&strom[0],10);*/
                                                  /*send_param.datagramoctets_len =10;*/
                                                   CSRMESH_DATA_BLOCK_SEND_T send_param;
                                                   uint8 i,pl_cnt=0;
                                                   send_param.datagramoctets_len =0;
                                                   
                                                   for(i=0;i<=10;i++)
                                                   {
                                                       if(strom[i]!='\r')
                                                       {
                                                           send_param.datagramoctets [i] = strom[i];
                                                           pl_cnt ++;
                                                            
                                                       }
                                                       else
                                                       {
                                                           
                                                           i=10;
                                                           goto L2;
                                                           
                                                       }
                                                       
                                                   }
                                                   L2:
                                                  send_param.datagramoctets_len = pl_cnt;
                                                  DataBlockSend(0,(atoh(&array[0])),12,&send_param);
                                                }
                                            }
                                          }
                                      }
                                            
                                  }
                               }
                           }
                           
                            break;
                  case 'P':
                  case 'p':
                            {
                                uint16 dev_id;
                                uint8 dev_array[5]= {'\0','\0','\0','\0','\0'};
                                uint16 self_dev_id = 0;
                               
                                CSR_MESH_APP_EVENT_DATA_T get_dev_id_data;
                                get_dev_id_data.appCallbackDataPtr = &self_dev_id;
                                
                                CSRmeshGetDeviceID(CSR_MESH_DEFAULT_NETID,&get_dev_id_data);
                                dev_id = self_dev_id;
                                
                                
                                
                               
                                dev_array[0] = (dev_id>>12)&0xF;
                                dev_array[0]+='0';
                                if((dev_array[0]>'9'))
                                {
                                   dev_array[0]+=0x27; 
                                }
                                
                                dev_array[1] = (dev_id >>8)&0xF;
                                dev_array[1]+='0';
                                if((dev_array[1]>'9'))
                                {
                                   dev_array[1]+=0x27; 
                                }
                                dev_array[2] = (dev_id >>4)&0xF;
                                dev_array[2]+='0';
                                if((dev_array[2]>'9'))
                                {
                                   dev_array[2]+=0x27; 
                                }
                                dev_array[3] = dev_id& 0xF;
                                dev_array[3]+='0';
                                if((dev_array[3]>'9'))
                                {
                                   dev_array[3]+=0x27; 
                                }
                                
                                
                                UartWrite(&dev_array[0],4);
                                
                                
                            }
                          
                                                    
                           break;
              
                 
                 default: 
                         break;
            }
            
            /* Convert byte into a command */
           
         }
        
     }
}
static uint16 UartDataRxCallback ( void* p_rx_data, uint16 data_count,
        uint16* p_num_additional_words )
{
    /* Application needs 1 additional data to be received */
    
    
    
     if (data_count > 0)
    {
        /* First copy all the bytes received into the byte queue */
        BQForceQueueBytes((const uint8 *)p_rx_data, data_count);

        /* Check whether the Enter key has been pressed, in which case parse
         * the command that has been stored in the byte queue.
         */
        uint16 i = 0;
        for (i = 0; i < data_count; ++i)
        /* We should be receiving this callback for every byte received which
         * means the length should always be 1. To accommodate changes in the
         * threshold without having to rewrite this section, no assumptions
         * on the received data length are made.
         */
        {
            const char ch = ((const char *)p_rx_data)[i];
            
            /* Echo back the data */
            /*DebugWriteChar(ch);*/

            /* Check if Enter key was pressed */
            if ((ch == '\r') || (ch == '\n'))
            {
                /* Echo back newline */
                /*DebugWriteChar('\n');*/

                /* Trigger command processing */
               
                if(AppGetAssociatedState() == app_state_associated)
                    processRxCmd();
            }
        }
    }

    *p_num_additional_words = (uint16)1;

    return data_count;
}


#ifndef CSR101x_A05
/*----------------------------------------------------------------------------*
 *  NAME
 *      configStoreProcessEvent
 *
 *  DESCRIPTION
 *      This function handles the config store messages
 *
 *  RETURNS/MODIFIES
 *      Nothing
 *
 *----------------------------------------------------------------------------*/
static void configStoreProcessEvent(msg_t *msg)
{
    configstore_msg_t *cs_msg = (configstore_msg_t*)msg;
    switch(msg->header.id)
    {
        case CONFIG_STORE_READ_KEY_CFM:
        {
            configstore_read_key_cfm_t *read_key_cfm =
                    (configstore_read_key_cfm_t*)&cs_msg->body;

            if (STATUS_SUCCESS == read_key_cfm->status)
            {
                if ((USER_KEY1 == read_key_cfm->id) && (0 != read_key_cfm->value_len))
                {
                    /* Copy flags. */
                    g_cskey_flags = read_key_cfm->value[0];

                    /* Copy UUID if Random UUID is not used. */
                    if (0 == (g_cskey_flags & CSKEY_RANDOM_UUID_ENABLE_BIT))
                    {
                        if (read_key_cfm->value_len >= (UUID_LENGTH_WORDS + 1))
                        {
                            MemCopy(cached_uuid, &read_key_cfm->value[1],
                                    UUID_LENGTH_WORDS);
                        }

                        /* Copy Authorization Code if enabled. */
#ifdef USE_AUTHORISATION_CODE
                        if (read_key_cfm->value_len >= (UUID_LENGTH_WORDS + AUTH_CODE_LENGTH_WORDS + 1))
                        {
                            MemCopy(cached_auth_code, &read_key_cfm->value[1 + UUID_LENGTH_WORDS],
                                    AUTH_CODE_LENGTH_WORDS);
                        }
#endif /* USE_AUTHORISATION_CODE */
                    }
                    /* Initialize the nvm offset to max words taken by  
                     * The CM is stores its data after this.
                     */        
                    g_app_nvm_offset = NVM_OFFSET_SANITY_WORD;

                   /* Initialise the NVM. AppNvmReady is called when the NVM 
                    * initialisation complete.
                    */
                    Nvm_Init(NVM_ID,NVM_SANITY_MAGIC, &g_app_nvm_offset);
                }
                else if(CS_ID_APP_STORE_ID == read_key_cfm->id)
                {
#ifdef GAIA_OTAU_SUPPORT
                    GaiaOtauConfigStoreMsg(msg);
#endif /* GAIA_OTAU_SUPPORT */
#ifdef GAIA_OTAU_RELAY_SUPPORT
                    GaiaOtauClientConfigStoreMsg(msg);
#endif /* GAIA_OTAU_RELAY_SUPPORT */
                }
            }
        }
        break;
        default:
        break;
    }
}
#endif

/*----------------------------------------------------------------------------*
 *  NAME
 *      readUserKeys
 *
 *  DESCRIPTION
 *      This function reads the user keys from config file
 *
 *  RETURNS
 *      Nothing
 *
 *---------------------------------------------------------------------------*/
static void readUserKeys(void)
{
#ifndef CSR101x_A05
    /* Read payload filler from User key index1 */
    ConfigStoreReadKey(USER_KEY1 ,STORE_ID_UNUSED);
#else
    g_cskey_flags = CSReadUserKey(CSKEY_INDEX_USER_FLAGS);

    /* Initialize the nvm offset to max words taken by application. The CM is
     * stores its data after this.
     */
    g_app_nvm_offset = NVM_OFFSET_SANITY_WORD;

    /* Initialise the NVM. AppNvmReady is called when the NVM initialisation
     * complete.
     */
    Nvm_Init(NVM_SANITY_MAGIC, &g_app_nvm_offset);
#endif
}

/*============================================================================*
 *  Public Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      InitialiseAppSuppServices
 *
 *  DESCRIPTION
 *      This function initialises the application supported services.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void InitAppSupportedServices(void)
{
#ifndef CSR101x_A05
    g_app_nvm_offset = NVM_OFFSET_GAIA_OTA_SERVICE;
    /* Initialize the Gaia OTA service. */
    GAIAInitServerService(g_app_nvm_fresh, &g_app_nvm_offset);
#ifdef GAIA_OTAU_SUPPORT
    AppGaiaOtauInit();
#endif /* GAIA_OTAU_SUPPORT */
#ifdef GAIA_OTAU_RELAY_SUPPORT
    GaiaInitClientService(g_app_nvm_fresh, &g_app_nvm_offset);
#endif  
    g_app_nvm_offset = NVM_OFFSET_MESH_APP_SERVICES;
#else
#ifdef OTAU_BOOTLOADER
    /* Initialize the OTA service. */
    OtaInitServerService(g_app_nvm_fresh, &g_app_nvm_offset);
#endif
#endif   
    
    /* Initialize the GATT service. */
    GattExtInitServerService(TRUE, &g_app_nvm_offset, TRUE);

    /* Initialize the GAP service. */
    GapInitServerService(TRUE, &g_app_nvm_offset);

    /* Initialize the Mesh control service */
    MeshControlInitServerService(TRUE, &g_app_nvm_offset);
    
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppDataInit
 *
 *  DESCRIPTION
 *      This function is called to initialise CSRmesh application
 *      data structure.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void AppDataInit(void)
{
    /* Stop the connection parameter update if its in progress */
    StopConnParamsUpdate();

    /* Initialises the GATT Data */
    InitialiseGattData();

    /* Initialize the Mesh Control Service Data Structure */
    MeshControlServiceDataInit();
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      AppInit
 *
 *  DESCRIPTION
 *      This user application function is called after a power-on reset
 *      (including after a firmware panic), after a wakeup from Hibernate or
 *      Dormant sleep states, or after an HCI Reset has been requested.
 *
 *      The last sleep state is provided to the application in the parameter.
 *
 *      NOTE: In the case of a power-on reset, this function is called
 *      after app_power_on_reset().
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
void AppInit(sleep_state last_sleep_state)
{
    /* Initialise the application timers */
    TimerInit(MAX_APP_TIMERS, (void*)app_timers);
    
    UartInit(UartDataRxCallback, NULL,
             rx_data, UART_BUF_SIZE_BYTES_32,
             tx_data, UART_BUF_SIZE_BYTES_32,
             uart_data_unpacked);
    
    UartConfig(BAUD_RATE,0);
    UartEnable(TRUE);
    UartRead(1, 0);
    
#ifdef DEBUG_ENABLE
#ifdef CSR101x_A05
    /* Initialize UART and configure with
     * default baud rate and port configuration.
     */
    DebugInit(UART_BUF_SIZE_BYTES_256, UartDataRxCallback, NULL);
   

    /* UART Rx threshold is set to 1,
     * so that every byte received will trigger the rx callback.
     */
    UartRead(1, 0);
#else
    /* Configuration structure for the UART */
    uart_pio_pins_t uart;

    /* Standard setup of CSR102x boards */
    uart.rx  = UART_PIO_RX;
    uart.tx  = UART_PIO_TX;
    uart.rts = UART_PIO_RTS;
    uart.cts = UART_PIO_CTS;
    
    /* Initialise Default UART communications */
    DebugInit(1, UART_RATE_921K6, 0, &uart);
#endif /* CSR101x_A05 */
#endif /* DEBUG_ENABLE */
    
    
    /* Initialize Light Hardware */
    LightHardwareInit();

    /* Read the user keys */
    readUserKeys();
    if(AppGetAssociatedState()==app_state_not_associated)
    {
           PioSet(PIO_4,FALSE);
        
    }
    else if(AppGetAssociatedState()==app_state_associated)
    {
            PioSet(PIO_4,TRUE);
        
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppNvmReady
 *
 *  DESCRIPTION
 *      This function is called when the NVM is initialised
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void AppNvmReady(bool nvm_fresh, uint16 nvm_offset)
{
    /* Save the NVM data */
    g_app_nvm_fresh = nvm_fresh;
#ifdef CSR101x_A05
    g_app_nvm_offset = NVM_MAX_APP_MEMORY_WORDS;
#else
    g_app_nvm_offset = NVM_OFFSET_CM_INITIALISATION;
#endif

    /* Initilize CM */
    AppCMInit(g_app_nvm_fresh, &g_app_nvm_offset);
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      AppProcesSystemEvent
 *
 *  DESCRIPTION
 *      This user application function is called whenever a system event, such
 *      as a battery low notification, is received by the system.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
extern void AppProcessSystemEvent(sys_event_id id, void *data)
{
    
    
    switch (id)
    {
        case sys_event_pio_changed:
        {
#ifdef USE_ASSOCIATION_REMOVAL_KEY
            
             HandlePIOChangedEvent((pio_changed_data*)data);
#endif
        }
        break;

        case sys_event_battery_low:
        {
            /* Battery low event received - Broadcast the battery state to the
             * network.
             */
           /*  ();*/
        }
        break;

        default:
        break;
    }
}

#ifndef CSR101x_A05
/*----------------------------------------------------------------------------*
 *  NAME
 *      AppProcessEvent
 *
 *  DESCRIPTION
 *      Handles the system events
 *
 *  RETURNS
 *     status_t: STATUS_SUCCESS if successful
 *
 *----------------------------------------------------------------------------*/

status_t AppProcessEvent(msg_t *msg)
{
    switch(GET_GROUP_ID(msg->header.id))
    {
        case SM_GROUP_ID:
        case LS_GROUP_ID:
        case GATT_GROUP_ID:
        {
            /* CM Handles the SM, LS and GATT Messages */
            CMProcessMsg(msg);
        }
        break;

        case USER_STORE_GROUP_ID:
        {
            NvmProcessEvent(msg);
        }
        break;

        case CONFIG_STORE_GROUP_ID:
        {
            configStoreProcessEvent(msg);
        }
        break;
        
#ifdef GAIA_OTAU_SUPPORT
        case STORE_UPDATE_GROUP_ID:
        GaiaOtauHandleStoreUpdateMsg(GetConnectedDeviceId(), (store_update_msg_t*)msg );
#ifdef GAIA_OTAU_RELAY_SUPPORT
        GaiaOtauClientHandleStoreUpdateMsg(GetConnectedDeviceId(), (store_update_msg_t*)msg );
#endif
        break;
#endif

        default:
        break;
    }    
    return STATUS_SUCCESS;
}

#else
/*----------------------------------------------------------------------------*
 *  NAME
 *      AppProcessLmEvent
 *
 *  DESCRIPTION
 *      This user application function is called whenever a LM-specific event is
 *      received by the system.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/

extern bool AppProcessLmEvent(lm_event_code event_code,
                              LM_EVENT_T *p_event_data)
{
    /* CM Handles the SM, LS and GATT Messages */
    CMProcessMsg(event_code, p_event_data);
    return TRUE;
}

/*----------------------------------------------------------------------------*
 * NAME
 *   AppPowerOnReset
 *
 * DESCRIPTION
 *   This user application function is called just after a power-on reset
 *   (including after a firmware panic), or after a wakeup from Hibernate or
 *   Dormant sleep states.
 *
 *   At the time this function is called, the last sleep state is not yet
 *   known.
 *
 *   NOTE: this function should only contain code to be executed after a
 *   power-on reset or panic. Code that should also be executed after an
 *   HCI_RESET should instead be placed in the AppInit() function.
 *
 * RETURNS
 *   Nothing
 *
 *----------------------------------------------------------------------------*/
void AppPowerOnReset(void)
{
     /* Code that is only executed after a power-on reset or firmware panic 
      * should be implemented here.
      */
}
#endif
