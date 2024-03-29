/******************************************************************************
 *  Copyright Cambridge Silicon Radio Limited 2015
 *  CSR Bluetooth Low Energy CSRmesh 1.3 Release
 *  Application version 1.3
 *
 *  FILE
 *      csr_mesh_light.c
 *
 *  DESCRIPTION
 *      This file implements the CSR Mesh light application.
 *
 *****************************************************************************/

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/
#include <main.h>
#include <ls_app_if.h>
#include <gatt.h>
#include <timer.h>
#include <uart.h>
#include <pio.h>
#include <nvm.h>
#include <security.h>
#include <gatt_prim.h>
#include <mem.h>
#include <panic.h>
#include <config_store.h>
#include <random.h>
#include <buf_utils.h>

/*============================================================================*
 *  Local Header Files
 *============================================================================*/
#include "user_config.h"
#include "nvm_access.h"
#include "csr_mesh_light.h"
#include "app_debug.h"
#include "app_gatt.h"
#include "csr_mesh_light_hw.h"
#include "gap_service.h"
#include "app_gatt_db.h"
#include "mesh_control_service.h"
#include "csr_mesh_light_gatt.h"
#include "appearance.h"
#include "byte_queue.h"



#if defined (ENABLE_GATT_OTA_SERVICE) || defined (ENABLE_FIRMWARE_MODEL)
#include "csr_ota.h"
#include "csr_ota_service.h"
#include "gatt_service.h"
#endif /* ENABLE_GATT_OTA_SERVICE || ENABLE_FIRMWARE_MODEL */

/*#ifdef USE_ASSOCIATION_REMOVAL_KEY*/
#include "iot_hw.h"
/*#endif *//* USE_ASSOCIATION_REMOVAL_KEY */
#include "battery_hw.h"
#include "app_data_stream.h"

/*============================================================================*
 *  CSR Mesh Header Files
 *============================================================================*/
#include <csr_mesh.h>
#include <attention_model.h>
#include <light_model.h>
#include <power_model.h>
#include <bearer_model.h>
#include <ping_model.h>
#include <debug.h>

#ifdef ENABLE_FIRMWARE_MODEL
#include <firmware_model.h>
#endif
#ifdef ENABLE_BATTERY_MODEL
#include <battery_model.h>
#endif



/*============================================================================*
 *  Private Definitions
 *============================================================================*/
/* Association Removal Button Press Duration */
#define LONG_KEYPRESS_TIME             (2 * SECOND)

/* CSRmesh Device UUID size */
#define DEVICE_UUID_SIZE_WORDS          8

/* CSRmesh Authorization Code Size in Words */
#define DEVICE_AUTHCODE_SIZE_IN_WORDS  (4)

/* Default device UUID */
#define DEFAULT_UUID                   {0x3210, 0x7654, 0xBA98, 0xFEDC,\
                                         0xCDEF, 0x89AB, 0x4567, 0x0123}

/* Default Authorisation code */
#define DEFAULT_AUTH_CODE              {0xCDEF, 0x89AB, 0x4567, 0x0123}

/* CS Key for mesh advertising interval */
#define CSKEY_INDEX_CSRMESH_ADV_INTERVAL \
                                       (0)

/* CS Key for mesh advertising time */
#define CSKEY_INDEX_CSRMESH_ADV_TIME   (1)

/* CS Key for user flags */
#define CSKEY_INDEX_USER_FLAGS         (2)

/* Used for generating Random UUID */
#define RANDOM_UUID_ENABLE_MASK        (0x0001)

/* Used for permanently Enabling/Disabling Relay */
#define RELAY_ENABLE_MASK              (0x0002)

/* Used for permanently Enabling/Disabling Bridge */
#define BRIDGE_ENABLE_MASK             (0x0004)

/* OTA Reset Defer Duration */
#define OTA_RESET_DEFER_DURATION       (500 * MILLISECOND)

/* NVM Data Write defer Duration */
#define NVM_WRITE_DEFER_DURATION       (5 * SECOND)

#define MAX_APP_TIMERS                 (15 + MAX_CSR_MESH_TIMERS)

/* Advertisement Timer for sending device identification */
#define DEVICE_ID_ADVERT_TIME          (5 * SECOND)

/* Slave device is not allowed to transmit another Connection Parameter
 * Update request till time TGAP(conn_param_timeout). Refer to section 9.3.9.2,
 * Vol 3, Part C of the Core 4.0 BT spec. The application should retry the
 * 'connection parameter update' procedure after time TGAP(conn_param_timeout)
 * which is 30 seconds.
 */
#define GAP_CONN_PARAM_TIMEOUT         (30 * SECOND)


/* TGAP(conn_pause_peripheral) defined in Core Specification Addendum 3 Revision
 * 2. A Peripheral device should not perform a Connection Parameter Update proc-
 * -edure within TGAP(conn_pause_peripheral) after establishing a connection.
 */
#define TGAP_CPP_PERIOD                (5 * SECOND)

/* TGAP(conn_pause_central) defined in Core Specification Addendum 3 Revision 2.
 * After the Peripheral device has no further pending actions to perform and the
 * Central device has not initiated any other actions within TGAP(conn_pause_ce-
 * -ntral), then the Peripheral device may perform a Connection Parameter Update
 * procedure.
 */
#define TGAP_CPC_PERIOD                (1 * SECOND)

/* NVM magic version used by the 1.1 application */
#define NVM_SANITY_MAGIC_1_1           (0xAB18)

/* APP NVM version used by the 1.1 light application */
#define APP_NVM_VERSION_1_1            (2)

/* Magic value to check the sanity of NVM region used by the application. This 
 * value should be unique for each application as the NVM layout changes for
 * every application.
 */
#define NVM_SANITY_MAGIC               (0xAB81)

/*Number of IRKs that application can store */
#define MAX_NUMBER_IRK_STORED          (1)

/* NVM offset for the application NVM version */
#define NVM_OFFSET_SANITY_WORD         (0)

/* NVM offset for NVM sanity word */
#define NVM_OFFSET_APP_NVM_VERSION     (NVM_OFFSET_SANITY_WORD + 1)

/* NVM offset for CSRmesh device UUID */
#define NVM_OFFSET_DEVICE_UUID         (NVM_OFFSET_APP_NVM_VERSION + 1)

/* NVM Offset for Authorization Code */
#define NVM_OFFSET_DEVICE_AUTHCODE     (NVM_OFFSET_DEVICE_UUID + \
                                        DEVICE_UUID_SIZE_WORDS)

#ifdef NVM_BACKWARD_COMPATIBILITY
/* NVM compatibility with earlier versions is required.
 * Adjust the NVM offsets as used by earlier versions.
 */
#define NVM_OFFSET_NETWORK_KEY         (NVM_OFFSET_DEVICE_AUTHCODE + \
                                        sizeof(CSR_MESH_AUTH_CODE_T) + 2)

#define NVM_OFFSET_DEVICE_ID           (NVM_OFFSET_NETWORK_KEY + \
                                        sizeof(CSR_MESH_NETWORK_KEY_T) + 1)
#else
#define NVM_OFFSET_NETWORK_KEY         (NVM_OFFSET_DEVICE_AUTHCODE + \
                                        sizeof(CSR_MESH_AUTH_CODE_T))

#define NVM_OFFSET_DEVICE_ID           (NVM_OFFSET_NETWORK_KEY + \
                                        sizeof(CSR_MESH_NETWORK_KEY_T))
#endif /* NVM_BACKWARD_COMPATIBILITY */

#define NVM_OFFSET_SEQUENCE_NUMBER     (NVM_OFFSET_DEVICE_ID + 1)

#define NVM_OFFSET_DEVICE_ETAG         (NVM_OFFSET_SEQUENCE_NUMBER + 2)

#define NVM_OFFSET_ASSOCIATION_STATE   (NVM_OFFSET_DEVICE_ETAG + \
                                        sizeof(CSR_MESH_ETAG_T))

#define NVM_OFFSET_LIGHT_MODEL_GROUPS  (NVM_OFFSET_ASSOCIATION_STATE + 1)

#define NVM_OFFSET_POWER_MODEL_GROUPS  (NVM_OFFSET_LIGHT_MODEL_GROUPS + \
                                        sizeof(light_model_groups))

#define NVM_OFFSET_ATTENTION_MODEL_GROUPS \
                                       (NVM_OFFSET_POWER_MODEL_GROUPS + \
                                        sizeof(power_model_groups))
#ifdef ENABLE_DATA_MODEL                                        
#define NVM_OFFSET_DATA_MODEL_GROUPS   (NVM_OFFSET_ATTENTION_MODEL_GROUPS + \
                                        sizeof(attention_model_groups))

/* NVM Offset for RGB data */
#define NVM_RGB_DATA_OFFSET            (NVM_OFFSET_DATA_MODEL_GROUPS + \
                                        sizeof(data_model_groups))
#else
                                       
/* NVM Offset for RGB data */
#define NVM_RGB_DATA_OFFSET            (NVM_OFFSET_ATTENTION_MODEL_GROUPS + \
                                        sizeof(attention_model_groups))
#endif                                       

/* Size of RGB Data in Words */
#define NVM_RGB_DATA_SIZE              (2)

/* NVM offset for Bearer Model Data */
#define NVM_BEARER_DATA_OFFSET         (NVM_RGB_DATA_OFFSET + \
                                        NVM_RGB_DATA_SIZE)

/* NVM Offset for Application data */
#define NVM_MAX_APP_MEMORY_WORDS       (NVM_BEARER_DATA_OFFSET + \
                                        sizeof(BEARER_MODEL_STATE_DATA_T))



#define BAUD_RATE (0x01d9) /*1152*/






/*============================================================================*
 *  Public Data
 *============================================================================*/
/* CSRmesh light application specific data */
CSRMESH_LIGHT_DATA_T g_lightapp_data;

/* CSRmesh network related node data */
CSR_MESH_NODE_DATA_T g_node_data ={ 
    .device_uuid.uuid = DEFAULT_UUID,
    .auth_code.auth_code = DEFAULT_AUTH_CODE,
};

/* Application VID,PID and Version. */
CSR_MESH_VID_PID_VERSION_T vid_pid_info =
{
    .vendor_id  = APP_VENDOR_ID,
    .product_id = APP_PRODUCT_ID,
    .version    = APP_VERSION,
};

/* Device Apprearance. */
CSR_MESH_APPEARANCE_T device_appearance = {APPEARANCE_ORG_BLUETOOTH_SIG,
                                           APPEARANCE_CSRMESH_LIGHT_VALUE};

/* Device Short name */
uint8 short_name[9] = "ArrayS";

/*============================================================================*
 *  Private Data
 *============================================================================*/

/* Declare space for application timers. */
static uint16 app_timers[SIZEOF_APP_TIMER * MAX_APP_TIMERS];

/* Declare space for Model Groups */
static uint16 light_model_groups[MAX_MODEL_GROUPS];
static uint16 attention_model_groups[MAX_MODEL_GROUPS];
static uint16 power_model_groups[MAX_MODEL_GROUPS];

#ifdef ENABLE_DATA_MODEL
static uint16 data_model_groups[MAX_MODEL_GROUPS];
static uint8 devid[3]={0x00,0x00,'\n'};
/*static void AIO_Read(uint16 value,uint8 *buf,uint8 n);*/
/*static void Atoh(uint16 value,uint8 *buf,uint8 n);*/
#endif /* ENABLE_DATA_MODEL */

/*#ifdef USE_ASSOCIATION_REMOVAL_KEY*/
/* Association Button Press Timer */
static timer_id long_keypress_tid;
/*#endif*/ /* USE_ASSOCIATION_REMOVAL_KEY */

/* Attention timer id */
static timer_id attn_tid = TIMER_INVALID;

#ifdef ENABLE_FIRMWARE_MODEL
/* Firmware Reset Delay Timer Id */
static timer_id ota_rst_tid = TIMER_INVALID;
#endif /* ENABLE_FIRMWARE_MODEL */

/* Time interval for sending the device id advertisements  */
static uint32 dev_id_advert_interval = DEVICE_ID_ADVERT_TIME;

/* To send the MASP associate to NW msg and Dev appearance msg alternatively */
static bool send_dev_appearance = FALSE;

/* store the bearer relay active value during connection and restore it back 
 * after the disconnection
 */
static uint16 bearer_relay_active;

/* store the promiscuous value during connection and restore it back after the 
 * disconnection
 */
static uint16 bearer_promiscuous;

/*============================================================================*
 *  Private Function Prototypes
 *============================================================================*/
/* UART Receive callback */
#ifdef DEBUG_ENABLE
static uint16 UartDataRxCallback ( void* p_rx_data, uint16 data_count,
                                   uint16* p_num_additional_words );

UART_DECLARE_BUFFER(rx_data,UART_BUF_SIZE_BYTES_32);
UART_DECLARE_BUFFER(tx_data,UART_BUF_SIZE_BYTES_32);


static void processRxCmd(void);
uint16 atoh( const uint8 *string);
#endif /* DEBUG_ENABLE */

/*#ifdef USE_ASSOCIATION_REMOVAL_KEY*/
static void handlePIOEvent(pio_changed_data *data);
/*#endif *//* USE_ASSOCIATION_REMOVAL_KEY */

/* Advert time out handler */
static void deviceIdAdvertTimeoutHandler(timer_id tid);

/* This function reads the persistent store. */
static void readPersistentStore(void);

/*============================================================================*
 *  Private Function Definitions
 *============================================================================*/
#ifdef USE_STATIC_RANDOM_ADDRESS
/*-----------------------------------------------------------------------------*
 *  NAME
 *      generateStaticRandomAddress
 *
 *  DESCRIPTION
 *      This function generates a static random address.
 *
 *  RETURNS/MODIFIES
 *      Nothing
 *
 *----------------------------------------------------------------------------*/
static void generateStaticRandomAddress(BD_ADDR_T *addr)
{
    uint16 temp[3];
    uint16 idx = 0;

    if (!addr) return;

    for (idx = 0; idx < 3;)
    {
        temp[idx] = Random16();
        if ((temp[idx] != 0) && (temp[idx] != 0xFFFF))
        {
            idx++;
        }
    }

    addr->lap = ((uint32)(temp[1]) << 16) | (temp[0]);
    addr->lap &= 0x00FFFFFFUL;
    addr->uap = (temp[1] >> 8) & 0xFF;
    addr->nap = temp[2];

    addr->nap &= ~BD_ADDR_NAP_RANDOM_TYPE_MASK;
    addr->nap |=  BD_ADDR_NAP_RANDOM_TYPE_STATIC;
}
#endif /* USE_STATIC_RANDOM_ADDRESS */

#ifdef ENABLE_FIRMWARE_MODEL
/*-----------------------------------------------------------------------------*
 *  NAME
 *      issueOTAReset
 *
 *  DESCRIPTION
 *      This function issues an OTA Reset.
 *
 *  RETURNS/MODIFIES
 *      Nothing
 *
 *----------------------------------------------------------------------------*/
static void issueOTAReset(timer_id tid)
{
    if (ota_rst_tid == tid)
    {
        ota_rst_tid = TIMER_INVALID;

        /* Issue OTA Reset. */
        OtaReset();
    }
}
#endif /* ENABLE_FIRMWARE_MODEL */

/*-----------------------------------------------------------------------------*
 *  NAME
 *      deviceIdAdvertTimeoutHandler
 *
 *  DESCRIPTION
 *      This function handles the Device ID advertise timer event.
 *
 *  RETURNS/MODIFIES
 *      Nothing
 *
 *----------------------------------------------------------------------------*/
static void deviceIdAdvertTimeoutHandler(timer_id tid)
{
    if(tid == g_lightapp_data.mesh_device_id_advert_tid)
    {
        g_lightapp_data.mesh_device_id_advert_tid = TIMER_INVALID;

        /* Start the timer only if the device is not associated */
        if(g_lightapp_data.assoc_state == app_state_not_associated)
        {
            /* Generate a random delay between 0 to 4095 ms */
            uint32 random_delay = ((uint32)(Random16() & 0x0FFF)) * (MILLISECOND);

            if(send_dev_appearance == FALSE)
            {
                /* Send the device ID advertisements */
                CsrMeshAssociateToANetwork();
                send_dev_appearance = TRUE;
            }
            else
            {
                /* Send the device appearance */
                CsrMeshAdvertiseDeviceAppearance(&device_appearance, 
                                                 short_name, 
                                                 sizeof(short_name));
                send_dev_appearance = FALSE;
            }

            g_lightapp_data.mesh_device_id_advert_tid = TimerCreate(
                                         (dev_id_advert_interval + random_delay),
                                         TRUE,
                                         deviceIdAdvertTimeoutHandler);
        }
    }
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      lightDataNVMWriteTimerHandler
 *
 *  DESCRIPTION
 *      This function handles NVM Write Timer time-out.
 *
 *  RETURNS/MODIFIES
 *      Nothing
 *
 *----------------------------------------------------------------------------*/
static void lightDataNVMWriteTimerHandler(timer_id tid)
{
    uint32 rd_data = 0;
    uint32 wr_data = 0;

    if (tid == g_lightapp_data.nvm_tid)
    {
        g_lightapp_data.nvm_tid = TIMER_INVALID;

        /* Read RGB and Power Data from NVM */
        Nvm_Read((uint16 *)&rd_data, sizeof(uint32),
                 NVM_RGB_DATA_OFFSET);

        /* Pack Data for writing to NVM */
        wr_data = ((uint32) g_lightapp_data.power.power_state << 24) |
                  ((uint32) g_lightapp_data.light_state.blue  << 16) |
                  ((uint32) g_lightapp_data.light_state.green <<  8) |
                  g_lightapp_data.light_state.red;

        /* If data on NVM is not equal to current state, write current state
         * to NVM.
         */
        if (rd_data != wr_data)
        {
            Nvm_Write((uint16 *)&wr_data, sizeof(uint32),NVM_RGB_DATA_OFFSET);
        }
    }
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      attnTimerHandler
 *
 *  DESCRIPTION
 *      This function handles Attention time-out.
 *
 *  RETURNS/MODIFIES
 *      Nothing
 *
 *----------------------------------------------------------------------------*/
static void attnTimerHandler(timer_id tid)
{
    if (attn_tid == tid)
    {
        attn_tid = TIMER_INVALID;

        if(g_lightapp_data.assoc_state == app_state_associated)
        {
            /* Restore Light State */
            /*LightHardwareSetColor(g_lightapp_data.light_state.red,
                                  g_lightapp_data.light_state.green,
                                  g_lightapp_data.light_state.blue);*/
            LightHardwarePowerControl(g_lightapp_data.power.power_state);
        }
        else
        {
            /* Restart association blink */
            /*LightHardwareSetBlink(0, 0, 127, 32, 32);*/
        }
    }
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      initiateAssociation
 *
 *  DESCRIPTION
 *      This function starts timer to send CSRmesh Association Messages
 *      and also gives visual indication that light is not associated.
 *
 *  RETURNS/MODIFIES
 *      Nothing
 *
 *----------------------------------------------------------------------------*/
static void initiateAssociation(void)
{
    /* Generate a random delay between 0 to 511 ms */
    uint32 random_delay = ((uint32)(Random16() & 0x1FF)) * (MILLISECOND);

    /* Blink light to indicate that it is not associated */
    /*LightHardwareSetBlink(0, 0, 127, 32, 32);*/

    /* Send the device ID advertisements */
    CsrMeshAssociateToANetwork();
    send_dev_appearance = TRUE;

    /* Start a timer to send Device ID messages periodically to get
     * associated to a network
     */
    g_lightapp_data.mesh_device_id_advert_tid =
                    TimerCreate((random_delay + DEVICE_ID_ADVERT_TIME),
                                TRUE,
                                deviceIdAdvertTimeoutHandler);
}


/*-----------------------------------------------------------------------------*
 *  NAME
 *      togglePowerState
 *
 *  DESCRIPTION
 *      This function toggles the power state.
 *
 *  RETURNS/MODIFIES
 *      Nothing
 *
 *----------------------------------------------------------------------------*/
static void togglePowerState(void)
{
    POWER_STATE_T curr_state = g_lightapp_data.power.power_state;

    switch (curr_state)
    {
        case POWER_STATE_ON:
            g_lightapp_data.power.power_state = POWER_STATE_OFF;
        break;

        case POWER_STATE_OFF:
            g_lightapp_data.power.power_state = POWER_STATE_ON;
        break;

        case POWER_STATE_ON_FROM_STANDBY:
            g_lightapp_data.power.power_state = POWER_STATE_STANDBY;
        break;

        case POWER_STATE_STANDBY:
            g_lightapp_data.power.power_state = POWER_STATE_ON_FROM_STANDBY;
        break;

        default:
        break;
    }
}


/*#ifdef USE_ASSOCIATION_REMOVAL_KEY*/
/*-----------------------------------------------------------------------------*
 *  NAME
 *      longKeyPressTimeoutHandler
 *
 *  DESCRIPTION
 *      This function handles the long key press timer event.
 *
 *  RETURNS/MODIFIES
 *      Nothing
 *
 *----------------------------------------------------------------------------*/
static void longKeyPressTimeoutHandler(timer_id tid)
{
    if (long_keypress_tid == tid)
    {
        long_keypress_tid = TIMER_INVALID;
    }
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      handlePIOEvent
 *
 *  DESCRIPTION
 *      This function handles the PIO Events.
 *
 *  RETURNS/MODIFIES
 *      Nothing
 *
 *----------------------------------------------------------------------------*/
void handlePIOEvent(pio_changed_data *data)
{
    /* If Switch-2 related event, then process further. Otherwise ignore */
    if (data->pio_cause & SW2_MASK)
    {
        /* Button Pressed */
        if (!(data->pio_state & SW2_MASK))
        {
            TimerDelete(long_keypress_tid);

            long_keypress_tid = TimerCreate(LONG_KEYPRESS_TIME, TRUE,
                                            longKeyPressTimeoutHandler);
        }
        else /* Button Released */
        {
            /* Button released after long press */
            if (TIMER_INVALID == long_keypress_tid)
            {
                if (app_state_not_associated != g_lightapp_data.assoc_state)
                {
                    /* Reset Association Information */
                    CsrMeshReset();

                    /* Set state to un-associated */
                    g_lightapp_data.assoc_state = app_state_not_associated;

                    /* Write association state to NVM */
                    Nvm_Write((uint16 *)&g_lightapp_data.assoc_state,
                             sizeof(g_lightapp_data.assoc_state),
                             NVM_OFFSET_ASSOCIATION_STATE);
                }
            }
            else /* Button released after a short press */
            {
                if (app_state_not_associated == g_lightapp_data.assoc_state)
                {
                    /* Delete Long Key Press Timer */
                    TimerDelete(long_keypress_tid);

                    /* Start Association to CSRmesh */
                    initiateAssociation();
                }
            }
        }
    }
}
/*#endif *//* USE_ASSOCIATION_REMOVAL_KEY */

/*----------------------------------------------------------------------------*
 *  NAME
 *      requestConnParamUpdate
 *
 *  DESCRIPTION
 *      This function is used to send L2CAP_CONNECTION_PARAMETER_UPDATE_REQUEST
 *      to the remote device when an earlier sent request had failed.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
static void requestConnParamUpdate(timer_id tid)
{
    /* Application specific preferred parameters */
    ble_con_params app_pref_conn_param;

    if(g_lightapp_data.gatt_data.con_param_update_tid == tid)
    {

        g_lightapp_data.gatt_data.con_param_update_tid = TIMER_INVALID;
        g_lightapp_data.gatt_data.cpu_timer_value = 0;

        /*Handling signal as per current state */
        switch(g_lightapp_data.state)
        {

            case app_state_connected:
            {
                /* Increment the count for Connection Parameter Update
                 * requests
                 */
                ++ g_lightapp_data.gatt_data.num_conn_update_req;

                /* If it is first or second request, preferred connection
                 * parameters should be request
                 */
                if(g_lightapp_data.gatt_data.num_conn_update_req == 1 ||
                   g_lightapp_data.gatt_data.num_conn_update_req == 2)
                {
                    app_pref_conn_param.con_max_interval =
                                                PREFERRED_MAX_CON_INTERVAL;
                    app_pref_conn_param.con_min_interval =
                                                PREFERRED_MIN_CON_INTERVAL;
                    app_pref_conn_param.con_slave_latency =
                                                PREFERRED_SLAVE_LATENCY;
                    app_pref_conn_param.con_super_timeout =
                                                PREFERRED_SUPERVISION_TIMEOUT;
                }
                /* If it is 3rd or 4th request, APPLE compliant parameters
                 * should be requested.
                 */
                else if(g_lightapp_data.gatt_data.num_conn_update_req == 3 ||
                        g_lightapp_data.gatt_data.num_conn_update_req == 4)
                {
                    app_pref_conn_param.con_max_interval =
                                                APPLE_MAX_CON_INTERVAL;
                    app_pref_conn_param.con_min_interval =
                                                APPLE_MIN_CON_INTERVAL;
                    app_pref_conn_param.con_slave_latency =
                                                APPLE_SLAVE_LATENCY;
                    app_pref_conn_param.con_super_timeout =
                                                APPLE_SUPERVISION_TIMEOUT;
                }

                /* Send Connection Parameter Update request using application
                 * specific preferred connection parameters
                 */

                if(LsConnectionParamUpdateReq(
                                 &g_lightapp_data.gatt_data.con_bd_addr,
                                 &app_pref_conn_param) != ls_err_none)
                {
                    ReportPanic(app_panic_con_param_update);
                }
            }
            break;

            default:
                /* Ignore in other states */
            break;
        }

    } /* Else ignore the timer */

}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      handleGapCppTimerExpiry
 *
 *  DESCRIPTION
 *      This function handles the expiry of TGAP(conn_pause_peripheral) timer.
 *      It starts the TGAP(conn_pause_central) timer, during which, if no activ-
 *      -ity is detected from the central device, a connection parameter update
 *      request is sent.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void handleGapCppTimerExpiry(timer_id tid)
{
    if(g_lightapp_data.gatt_data.con_param_update_tid == tid)
    {
        g_lightapp_data.gatt_data.con_param_update_tid =
                           TimerCreate(TGAP_CPC_PERIOD, TRUE,
                                       requestConnParamUpdate);
        g_lightapp_data.gatt_data.cpu_timer_value = TGAP_CPC_PERIOD;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      appDataInit
 *
 *  DESCRIPTION
 *      This function is called to initialise CSRmesh light application
 *      data structure.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
static void appDataInit(void)
{
    /* Cancel advertisement timer */
    TimerDelete(g_lightapp_data.gatt_data.app_tid);
    g_lightapp_data.gatt_data.app_tid = TIMER_INVALID;

    TimerDelete(g_lightapp_data.gatt_data.con_param_update_tid);
    g_lightapp_data.gatt_data.con_param_update_tid = TIMER_INVALID;
    g_lightapp_data.gatt_data.cpu_timer_value = 0;

    g_lightapp_data.gatt_data.st_ucid = GATT_INVALID_UCID;
    g_lightapp_data.gatt_data.advert_timer_value = 0;

    /* Reset the connection parameter variables. */
    g_lightapp_data.gatt_data.conn_interval = 0;
    g_lightapp_data.gatt_data.conn_latency = 0;
    g_lightapp_data.gatt_data.conn_timeout = 0;

    /* Initialise GAP Data structure */
    GapDataInit();

    /* Initialise the Mesh Control Service Data Structure */
    MeshControlServiceDataInit();

#ifdef ENABLE_GATT_OTA_SERVICE
    /* Initialise GATT Data structure */
    GattDataInit();

    /* Initialise the CSR OTA Service Data */
    OtaDataInit();
#endif /* ENABLE_GATT_OTA_SERVICE */
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      readPersistentStore
 *
 *  DESCRIPTION
 *      This function is used to Initialise and read NVM data
 *
 *  RETURNS/MODIFIES
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void readPersistentStore(void)
{
    /* NVM offset for supported services */
    uint16 nvm_offset = 0;
    uint16 nvm_sanity = 0xffff;
    uint16 app_nvm_version = 0;
    uint32 temp = 0;

    nvm_offset = NVM_MAX_APP_MEMORY_WORDS;

    /* Read the sanity word */
    Nvm_Read(&nvm_sanity, sizeof(nvm_sanity),
             NVM_OFFSET_SANITY_WORD);

    /* Read the Application NVM version */
    Nvm_Read(&app_nvm_version, 1, NVM_OFFSET_APP_NVM_VERSION);

    if(nvm_sanity == NVM_SANITY_MAGIC &&
       app_nvm_version == APP_NVM_VERSION )
    {

        g_lightapp_data.gatt_data.paired = FALSE;

        /* Read RGB and Power Data from NVM */
        Nvm_Read((uint16 *)&temp, sizeof(uint32), NVM_RGB_DATA_OFFSET);

        /* Read assigned Group IDs for Light model from NVM */
        Nvm_Read((uint16 *)light_model_groups, sizeof(light_model_groups),
                                                 NVM_OFFSET_LIGHT_MODEL_GROUPS);

        /* Read assigned Group IDs for Power model from NVM */
        Nvm_Read((uint16 *)power_model_groups, sizeof(power_model_groups),
                                                 NVM_OFFSET_POWER_MODEL_GROUPS);

        /* Read assigned Group IDs for Attention model from NVM */
        Nvm_Read((uint16 *)attention_model_groups, sizeof(attention_model_groups),
                                            NVM_OFFSET_ATTENTION_MODEL_GROUPS);
#ifdef ENABLE_DATA_MODEL
        /* Read assigned Group IDs for data stream model from NVM */
        Nvm_Read((uint16 *)data_model_groups, sizeof(data_model_groups),
                                            NVM_OFFSET_DATA_MODEL_GROUPS);
#endif /* ENABLE_DATA_MODEL */

        /* Unpack data in to the global variables */
        g_lightapp_data.light_state.red   = temp & 0xFF;
        temp >>= 8;
        g_lightapp_data.light_state.green = temp & 0xFF;
        temp >>= 8;
        g_lightapp_data.light_state.blue  = temp & 0xFF;
        temp >>= 8;
        g_lightapp_data.power.power_state = temp & 0xFF;
        g_lightapp_data.light_state.power = g_lightapp_data.power.power_state;

        /* Read Bearer Model Data from NVM */
        Nvm_Read((uint16 *)&g_lightapp_data.bearer_data,
                 sizeof(BEARER_MODEL_STATE_DATA_T), NVM_BEARER_DATA_OFFSET);

        /* If NVM in use, read device name and length from NVM */
        GapReadDataFromNVM(&nvm_offset);
    }
    else
    {
        /* Read Configuration flags from User CS Key */
        uint16 cskey_flags = CSReadUserKey(CSKEY_INDEX_USER_FLAGS);

        /* If the NVM_SANITY word the APP_NVM_VERSION matches the 1.1 version
         * retain the association data on the NVM and update to newer versions.
         */
        if(nvm_sanity == NVM_SANITY_MAGIC_1_1 && 
            app_nvm_version == APP_NVM_VERSION_1_1)
        {
            nvm_sanity = NVM_SANITY_MAGIC;

            /* Write NVM Sanity word to the NVM */
            Nvm_Write(&nvm_sanity, sizeof(nvm_sanity), NVM_OFFSET_SANITY_WORD);
        }
        
        if( nvm_sanity != NVM_SANITY_MAGIC)
        {
            uint8 i;

            /* NVM Sanity check failed means either the device is being brought
             * up for the first time or memory has got corrupted in which case
             * discard the data and start fresh.
             */
            nvm_sanity = NVM_SANITY_MAGIC;

            /* Write NVM Sanity word to the NVM */
            Nvm_Write(&nvm_sanity, sizeof(nvm_sanity),
                      NVM_OFFSET_SANITY_WORD);

            if (cskey_flags & RANDOM_UUID_ENABLE_MASK)
            {
                /* The flag is set so generate a random UUID and store it on NVM */
                for( i = 0 ; i < DEVICE_UUID_SIZE_WORDS ; i++)
                {
                    g_node_data.device_uuid.uuid[i] = Random16();
                }
                /* Write to NVM */
                Nvm_Write(g_node_data.device_uuid.uuid, DEVICE_UUID_SIZE_WORDS,
                          NVM_OFFSET_DEVICE_UUID);
            }
            else
            {
                /* A mass production tool may be used to program the device
                 * firmware as well as the device UUID and authorisation code
                 * in which case the NVM sanity check fails but the NVM will
                 * have a valid UUID and Authorisation code.
                 * So write the default UUID and AC only if all the words of the
                 *
                 */
                uint16 temp_word;
                for( i = 0; i < DEVICE_UUID_SIZE_WORDS ; i++)
                {
                    Nvm_Read(&temp_word, 1, NVM_OFFSET_DEVICE_UUID + i);
                    if( temp_word != NVM_DEFAULT_ERASED_WORD)
                    {
                        /* Atleast one word is not same default erased word
                         * read the UUID from NVM
                         */
                        Nvm_Read(g_node_data.device_uuid.uuid, DEVICE_UUID_SIZE_WORDS,
                                 NVM_OFFSET_DEVICE_UUID);
                        break;
                    }
                }

                if( i == DEVICE_UUID_SIZE_WORDS)
                {
                    /* Write the default UUID to NVM */
                    Nvm_Write(g_node_data.device_uuid.uuid, DEVICE_UUID_SIZE_WORDS,
                              NVM_OFFSET_DEVICE_UUID);
                }
            }
#ifdef USE_AUTHORIZATION_CODE
            uint16 temp_word;
            for( i = 0; i < DEVICE_AUTHCODE_SIZE_IN_WORDS ; i++)
            {
                Nvm_Read(&temp_word, 1, NVM_OFFSET_DEVICE_AUTHCODE + i);
                if( temp_word != NVM_DEFAULT_ERASED_WORD)
                {
                    Nvm_Read(g_node_data.auth_code.auth_code, DEVICE_AUTHCODE_SIZE_IN_WORDS,
                                                    NVM_OFFSET_DEVICE_AUTHCODE);
                    break;
                }
            }
            if( i == DEVICE_AUTHCODE_SIZE_IN_WORDS)
            {
                /* Write default authorization Code to NVM */
                Nvm_Write(g_node_data.auth_code.auth_code, DEVICE_AUTHCODE_SIZE_IN_WORDS,
                           NVM_OFFSET_DEVICE_AUTHCODE);
            }
#endif /* USE_AUTHORIZATION_CODE */

           
            /* The device will not be associated as it is coming up for the
             * first time
             */
            g_lightapp_data.assoc_state = app_state_not_associated;

            /* Write association state to NVM */
            Nvm_Write((uint16 *)&g_lightapp_data.assoc_state, 1,
                                                 NVM_OFFSET_ASSOCIATION_STATE);
        }

        /* Store new version of the NVM */
        app_nvm_version = APP_NVM_VERSION;
        Nvm_Write(&app_nvm_version, 1, NVM_OFFSET_APP_NVM_VERSION);

        /* All the persistent data below will be reset to default upon an
         * application update. If some of the data needs to be retained even
         * after an application update, it has to be moved within the sanity
         * word check
         */

        /* Update Bearer Model Data from CSKey flags for the first time. */
        g_lightapp_data.bearer_data.bearerPromiscuous = 0x0000;
        g_lightapp_data.bearer_data.bearerEnabled     = BLE_BEARER_MASK;
        g_lightapp_data.bearer_data.bearerRelayActive = 0x0000;

        if (cskey_flags & RELAY_ENABLE_MASK)
        {
            g_lightapp_data.bearer_data.bearerRelayActive |= BLE_BEARER_MASK;
            g_lightapp_data.bearer_data.bearerPromiscuous |= BLE_BEARER_MASK;
        }

        if (cskey_flags & BRIDGE_ENABLE_MASK)
        {
            g_lightapp_data.bearer_data.bearerEnabled |=     
                                                BLE_GATT_SERVER_BEARER_MASK;
            g_lightapp_data.bearer_data.bearerRelayActive |= 
                                                BLE_GATT_SERVER_BEARER_MASK;
            g_lightapp_data.bearer_data.bearerPromiscuous |= 
                                                BLE_GATT_SERVER_BEARER_MASK;
        }

        /* Update Bearer Model Data to NVM */
        Nvm_Write((uint16 *)&g_lightapp_data.bearer_data,
                  sizeof(BEARER_MODEL_STATE_DATA_T), NVM_BEARER_DATA_OFFSET);

        /* The device will not be paired as it is coming up for the first
         * time
         */
        g_lightapp_data.gatt_data.paired = FALSE;

        /* Write RGB Data and Power to NVM.
         * Data is stored in the following format.
         * HIGH WORD: MSB: POWER LSB: BLUE.
         * LOW  WORD: MSB: GREEN LSB: RED.
         */
        g_lightapp_data.light_state.red   = 0xFF;
        g_lightapp_data.light_state.green = 0xFF;
        g_lightapp_data.light_state.blue  = 0xFF;
        g_lightapp_data.light_state.power = POWER_STATE_OFF;
        g_lightapp_data.power.power_state = POWER_STATE_OFF;

        temp = ((uint32) g_lightapp_data.power.power_state << 24) |
               ((uint32) g_lightapp_data.light_state.blue  << 16) |
               ((uint32) g_lightapp_data.light_state.green <<  8) |
               g_lightapp_data.light_state.red;

        Nvm_Write((uint16 *)&temp, sizeof(uint32),
                 NVM_RGB_DATA_OFFSET);

        /* Initialise model groups */
        MemSet(light_model_groups, 0x0000, sizeof(light_model_groups));
        Nvm_Write((uint16 *)light_model_groups, sizeof(light_model_groups),
                                                 NVM_OFFSET_LIGHT_MODEL_GROUPS);

        MemSet(power_model_groups, 0x0000, sizeof(power_model_groups));
        Nvm_Write((uint16 *)power_model_groups, sizeof(power_model_groups),
                                                 NVM_OFFSET_POWER_MODEL_GROUPS);

        MemSet(attention_model_groups, 0x0000, sizeof(attention_model_groups));
        Nvm_Write((uint16 *)attention_model_groups,
                  sizeof(attention_model_groups), NVM_OFFSET_ATTENTION_MODEL_GROUPS);

        /* Data stream model */
#ifdef ENABLE_DATA_MODEL
        MemSet(data_model_groups, 0x0000, sizeof(data_model_groups));
        Nvm_Write((uint16 *)data_model_groups, sizeof(data_model_groups),
                                        NVM_OFFSET_DATA_MODEL_GROUPS);
#endif /* ENABLE_DATA_MODEL */
        /* Write device name and length to NVM for the first time */
        GapInitWriteDataToNVM(&nvm_offset);
    }

    /* Read association state from NVM */
    Nvm_Read((uint16 *)&g_lightapp_data.assoc_state,
            sizeof(g_lightapp_data.assoc_state), NVM_OFFSET_ASSOCIATION_STATE);

    /* Read the UUID from NVM */
    Nvm_Read(g_node_data.device_uuid.uuid, DEVICE_UUID_SIZE_WORDS, 
                                                        NVM_OFFSET_DEVICE_UUID);

#ifdef USE_AUTHORIZATION_CODE
    /* Read Authorization Code from NVM */
    Nvm_Read(g_node_data.auth_code.auth_code, DEVICE_AUTHCODE_SIZE_IN_WORDS,
                                                    NVM_OFFSET_DEVICE_AUTHCODE);
    g_node_data.use_authorisation = TRUE;
#endif /* USE_AUTHORIZATION_CODE */

    if(g_lightapp_data.assoc_state == app_state_associated)
    {
        g_node_data.associated = TRUE;
        /* Read node data from NVM */
        /* Network key */
        Nvm_Read(g_node_data.nw_key.key, sizeof(CSR_MESH_NETWORK_KEY_T),
                                                        NVM_OFFSET_NETWORK_KEY);
        /* Device ID */
        Nvm_Read(&g_node_data.device_id, 1, NVM_OFFSET_DEVICE_ID);
        /* Sequence Number */
        Nvm_Read((uint16 *)&g_node_data.seq_number, 2, NVM_OFFSET_SEQUENCE_NUMBER);
        /* Last ETag */
        Nvm_Read(g_node_data.device_ETag.ETag, sizeof(CSR_MESH_ETAG_T),
                                                    NVM_OFFSET_DEVICE_ETAG);
        /* As device is already associated set LE bearer to non-promiscuous.*/
        g_lightapp_data.bearer_data.bearerPromiscuous &= ~BLE_BEARER_MASK;
    }
}

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
#ifdef DEBUG_ENABLE

static void showHelp(void)
{
    
   /* UartWrite("\r\n",2);
    UartWrite("Mesh AT-CMD APP\r\n",17);
    UartWrite("<1>D Data: D ID <0/1>\r\n",22);
    UartWrite("<2>Z data: Z <Data>  \r\n",22);
    UartWrite("<3>  Device data: P \r\n",23);
    UartWrite("\r\n",2);*/
    
}

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
        uint8 byte = '\0';
        uint8 array[5]= {'\0','\0','\0','\0','\0'};
        
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
                                    if (BQPopBytes(&strom[0], 10) > 0)
                                    {
                                                  TimeDelayUSec(Random16());
                                                  StreamSendDataBlock(0x0000,&strom[0],10);
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
                                                if (BQPopBytes(&strom[0], 10) > 0)
                                                {
                                                  TimeDelayUSec(Random16());
                                                  StreamSendDataBlock(atoh(&array[0]),&strom[0],10);
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
                                uint8 dev_array[5]= {'\0','\0','\0','\0','\0'};
                                uint16 dev_id  = CsrMeshGetDeviceID();
                               
                                
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
              
                  case 'h':
                  case 'H': showHelp();
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
                if(g_lightapp_data.assoc_state == app_state_associated)
                    processRxCmd();
            }
        }
    }

    *p_num_additional_words = (uint16)1;

    return data_count;
}
#endif /* DEBUG_ENABLE */
/*----------------------------------------------------------------------------*
 *  NAME
 *      appAdvertisingExit
 *
 *  DESCRIPTION
 *      This function is called while exiting app_state_advertising and
 *      app_state_slow_advertising states.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
static void appAdvertisingExit(void)
{
        /* Stop on-going advertisements */
        GattStopAdverts();

        /* Cancel advertisement timer */
        TimerDelete(g_lightapp_data.gatt_data.app_tid);
        g_lightapp_data.gatt_data.app_tid = TIMER_INVALID;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalGattAddDBCfm
 *
 *  DESCRIPTION
 *      This function handles the signal GATT_ADD_DB_CFM
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
static void handleSignalGattAddDBCfm(GATT_ADD_DB_CFM_T *p_event_data)
{
    switch(g_lightapp_data.state)
    {
        case app_state_init:
        {
            if(p_event_data->result == sys_status_success)
            {
                /* If GATT bearer is enabled move to advertisement state 
                 * otherwise move to idle state. The advertisement would be 
                 * triggerred once the GATT bearer is enabled again.
                 */
                if(g_lightapp_data.bearer_data.bearerEnabled & 
                                                BLE_GATT_SERVER_BEARER_MASK)
                {
                    AppSetState(app_state_advertising);
                }
                else
                {
                    AppSetState(app_state_idle);
                }
            }
            else
            {
                /* Don't expect this to happen */
                ReportPanic(app_panic_db_registration);
            }
        }
        break;

        default:
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        break;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalGattCancelConnectCfm
 *
 *  DESCRIPTION
 *      This function handles the signal GATT_CANCEL_CONNECT_CFM
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
static void handleSignalGattCancelConnectCfm(void)
{
    /*Handling signal as per current state */
    switch(g_lightapp_data.state)
    {
        case app_state_advertising:
        {
            /* Do nothing here */
        }
        break;

        case app_state_connected:
            /* The CSRmesh could have been sending data on
             * advertisements so do not panic
             */
        break;

        default:
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        break;
    }
}

/*---------------------------------------------------------------------------
 *
 *  NAME
 *      handleSignalLmEvConnectionComplete
 *
 *  DESCRIPTION
 *      This function handles the signal LM_EV_CONNECTION_COMPLETE.
 *
 *  RETURNS
 *      Nothing.
 *

*----------------------------------------------------------------------------*/
static void handleSignalLmEvConnectionComplete(
                                     LM_EV_CONNECTION_COMPLETE_T *p_event_data)
{
    /* Store the connection parameters. */
    g_lightapp_data.gatt_data.conn_interval = p_event_data->data.conn_interval;
    g_lightapp_data.gatt_data.conn_latency  = p_event_data->data.conn_latency;
    g_lightapp_data.gatt_data.conn_timeout  =
                                        p_event_data->data.supervision_timeout;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalGattConnectCfm
 *
 *  DESCRIPTION
 *      This function handles the signal GATT_CONNECT_CFM
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
static void handleSignalGattConnectCfm(GATT_CONNECT_CFM_T* p_event_data)
{
    /*Handling signal as per current state */
    switch(g_lightapp_data.state)
    {
        case app_state_advertising:
        {
            if(p_event_data->result == sys_status_success)
            {
                /* Store received UCID */
                g_lightapp_data.gatt_data.st_ucid = p_event_data->cid;

                /* Store connected BD Address */
                g_lightapp_data.gatt_data.con_bd_addr = p_event_data->bd_addr;

                /* Store the bearer relay active and promiscuous onto global 
                 * as they need to be reverted after disconnection.
                 */
                bearer_relay_active = 
                    g_lightapp_data.bearer_data.bearerRelayActive;

                bearer_promiscuous = 
                    g_lightapp_data.bearer_data.bearerPromiscuous;

                g_lightapp_data.bearer_data.bearerRelayActive = 
                    BLE_BEARER_MASK | BLE_GATT_SERVER_BEARER_MASK;

                g_lightapp_data.bearer_data.bearerPromiscuous = 
                    BLE_BEARER_MASK | BLE_GATT_SERVER_BEARER_MASK;

                /* When device is connected as bridge enable the BLE and GATT 
                 * bearer relays otherwise mesh messages sent by control device 
                 * over GATT will not be forwarded on mesh.
                 */
                CsrMeshRelayEnable(
                                g_lightapp_data.bearer_data.bearerRelayActive);

                /* Enable the promiscuous mode on both the bearers which makes
                 * sure the connected control device can control any mesh n/w.
                 */
                CsrMeshEnablePromiscuousMode(
                                g_lightapp_data.bearer_data.bearerPromiscuous);

                /* Enter connected state */
                AppSetState(app_state_connected);

                /* Inform CSRmesh that we are connected now */
                CsrMeshHandleDataInConnection(
                                g_lightapp_data.gatt_data.st_ucid,
                                g_lightapp_data.gatt_data.conn_interval);

                /* Since CSRmesh application does not mandate encryption
                 * requirement on its characteristics, the remote master may
                 * or may not encrypt the link. Start a timer  here to give
                 * remote master some time to encrypt the link and on expiry
                 * of that timer, send a connection parameter update request
                 * to remote side.
                 */

                /* Don't request security as this causes connection issues
                 * with Android 4.4
                 *
                 * SMRequestSecurityLevel(&g_lightapp_data.gatt_data.con_bd_addr);
                 */

                /* If the current connection parameters being used don't
                 * comply with the application's preferred connection
                 * parameters and the timer is not running and, start timer
                 * to trigger Connection Parameter Update procedure
                 */
                if((g_lightapp_data.gatt_data.con_param_update_tid ==
                                                        TIMER_INVALID) &&
                   (g_lightapp_data.gatt_data.conn_interval <
                                             PREFERRED_MIN_CON_INTERVAL ||
                    g_lightapp_data.gatt_data.conn_interval >
                                             PREFERRED_MAX_CON_INTERVAL
#if PREFERRED_SLAVE_LATENCY
                    || g_lightapp_data.gatt_data.conn_latency <
                                             PREFERRED_SLAVE_LATENCY
#endif
                   )
                  )
                {
                    /* Set the num of conn update attempts to zero */
                    g_lightapp_data.gatt_data.num_conn_update_req = 0;

                    /* The application first starts a timer of
                     * TGAP_CPP_PERIOD. During this time, the application
                     * waits for the peer device to do the database
                     * discovery procedure. After expiry of this timer, the
                     * application starts one more timer of period
                     * TGAP_CPC_PERIOD. If the application receives any
                     * GATT_ACCESS_IND during this time, it assumes that
                     * the peer device is still doing device database
                     * discovery procedure or some other configuration and
                     * it should not update the parameters, so it restarts
                     * the TGAP_CPC_PERIOD timer. If this timer expires, the
                     * application assumes that database discovery procedure
                     * is complete and it initiates the connection parameter
                     * update procedure.
                     */
                    g_lightapp_data.gatt_data.con_param_update_tid =
                                      TimerCreate(TGAP_CPP_PERIOD, TRUE,
                                                  handleGapCppTimerExpiry);
                    g_lightapp_data.gatt_data.cpu_timer_value =
                                                        TGAP_CPP_PERIOD;
                }
                  /* Else at the expiry of timer Connection parameter
                   * update procedure will get triggered
                   */
            }
            else
            {
                /* If GATT bearer is enabled move to advertisement state 
                 * otherwise move to idle state. The advertisement would be 
                 * triggerred once the GATT bearer is enabled again.
                 */
                if(g_lightapp_data.bearer_data.bearerEnabled & 
                                                BLE_GATT_SERVER_BEARER_MASK)
                {
                    AppSetState(app_state_advertising);
                }
                else
                {
                    AppSetState(app_state_idle);
                }
            }
        }
        break;

        default:
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        break;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalSmSimplePairingCompleteInd
 *
 *  DESCRIPTION
 *      This function handles the signal SM_SIMPLE_PAIRING_COMPLETE_IND
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
static void handleSignalSmSimplePairingCompleteInd(
                                 SM_SIMPLE_PAIRING_COMPLETE_IND_T *p_event_data)
{
    /*Handling signal as per current state */
    switch(g_lightapp_data.state)
    {
        case app_state_connected:
        {
            if(p_event_data->status == sys_status_success)
            {
                /* Store temporary pairing info. */
                g_lightapp_data.gatt_data.paired = TRUE;
            }
            else
            {
                /* Pairing has failed.disconnect the link.*/
                AppSetState(app_state_disconnecting);
            }
        }
        break;

        default:
            /* Firmware may send this signal after disconnection. So don't
             * panic but ignore this signal.
             */
        break;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalLsConnParamUpdateCfm
 *
 *  DESCRIPTION
 *      This function handles the signal LS_CONNECTION_PARAM_UPDATE_CFM.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
static void handleSignalLsConnParamUpdateCfm(
                            LS_CONNECTION_PARAM_UPDATE_CFM_T *p_event_data)
{
    /*Handling signal as per current state */
    switch(g_lightapp_data.state)
    {
        case app_state_connected:
        {
            /* Received in response to the L2CAP_CONNECTION_PARAMETER_UPDATE
              * request sent from the slave after encryption is enabled. If
              * the request has failed, the device should again send the same
              * request only after Tgap(conn_param_timeout). Refer
              * Bluetooth 4.0 spec Vol 3 Part C, Section 9.3.9 and profile spec.
              */
            if ((p_event_data->status != ls_err_none) &&
                (g_lightapp_data.gatt_data.num_conn_update_req <
                                        MAX_NUM_CONN_PARAM_UPDATE_REQS))
            {
                /* Delete timer if running */
                TimerDelete(g_lightapp_data.gatt_data.con_param_update_tid);

                g_lightapp_data.gatt_data.con_param_update_tid =
                                 TimerCreate(GAP_CONN_PARAM_TIMEOUT,
                                             TRUE, requestConnParamUpdate);
                g_lightapp_data.gatt_data.cpu_timer_value =
                                             GAP_CONN_PARAM_TIMEOUT;
            }
        }
        break;

        default:
            /* Control should never come here but in one of the odd cases when 
             * the master is disconnecting during the connection param update 
             * the above msg is received after the disconnection complete from  
             * the firmware. Hence ignoring the signal is other states too.
             */
            /* ReportPanic(app_panic_invalid_state); */
        break;
    }
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalLmConnectionUpdate
 *
 *  DESCRIPTION
 *      This function handles the signal LM_EV_CONNECTION_UPDATE.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
static void handleSignalLmConnectionUpdate(
                                   LM_EV_CONNECTION_UPDATE_T* p_event_data)
{
    switch(g_lightapp_data.state)
    {
        case app_state_connected:
        case app_state_disconnecting:
        {
            /* Store the new connection parameters. */
            g_lightapp_data.gatt_data.conn_interval =
                                            p_event_data->data.conn_interval;
            g_lightapp_data.gatt_data.conn_latency =
                                            p_event_data->data.conn_latency;
            g_lightapp_data.gatt_data.conn_timeout =
                                        p_event_data->data.supervision_timeout;

            CsrMeshHandleDataInConnection(g_lightapp_data.gatt_data.st_ucid,
                                       g_lightapp_data.gatt_data.conn_interval);

            /*DEBUG_STR("Parameter Update Complete: ");
            DEBUG_U16(g_lightapp_data.gatt_data.conn_interval);
            DEBUG_STR("\r\n");*/
        }
        break;

        default:
            /* Connection parameter update indication received in unexpected
             * application state. In one of the odd cases when the master is 
             * disconnecting during the connection param update the conn update
             * is received after the disconnection complete from the 
             * firmware. Hence ignoring the signal is other states too.
             */
            /* ReportPanic(app_panic_invalid_state); */
        break;
    }
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalLsConnParamUpdateInd
 *
 *  DESCRIPTION
 *      This function handles the signal LS_CONNECTION_PARAM_UPDATE_IND.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
static void handleSignalLsConnParamUpdateInd(
                                 LS_CONNECTION_PARAM_UPDATE_IND_T *p_event_data)
{
    /*Handling signal as per current state */
    switch(g_lightapp_data.state)
    {
        case app_state_connected:
        {
            /* Delete timer if running */
            TimerDelete(g_lightapp_data.gatt_data.con_param_update_tid);
            g_lightapp_data.gatt_data.con_param_update_tid = TIMER_INVALID;
            g_lightapp_data.gatt_data.cpu_timer_value = 0;

            /* The application had already received the new connection
             * parameters while handling event LM_EV_CONNECTION_UPDATE.
             * Check if new parameters comply with application preferred
             * parameters. If not, application shall trigger Connection
             * parameter update procedure
             */

            if(g_lightapp_data.gatt_data.conn_interval <
                                                PREFERRED_MIN_CON_INTERVAL ||
               g_lightapp_data.gatt_data.conn_interval >
                                                PREFERRED_MAX_CON_INTERVAL
#if PREFERRED_SLAVE_LATENCY
               || g_lightapp_data.gatt_data.conn_latency <
                                                PREFERRED_SLAVE_LATENCY
#endif
              )
            {
                /* Set the num of conn update attempts to zero */
                g_lightapp_data.gatt_data.num_conn_update_req = 0;

                /* Start timer to trigger Connection Parameter Update
                 * procedure
                 */
                g_lightapp_data.gatt_data.con_param_update_tid =
                                TimerCreate(GAP_CONN_PARAM_TIMEOUT,
                                            TRUE, requestConnParamUpdate);
                g_lightapp_data.gatt_data.cpu_timer_value =
                                                        GAP_CONN_PARAM_TIMEOUT;
            }
        }
        break;

        default:
            /* Connection parameter update indication received in unexpected
             * application state. In one of the odd cases when the master is 
             * disconnecting during the connection param update the conn update
             * is received after the disconnection complete from the 
             * firmware. Hence ignoring the signal is other states too.
             */
            /* ReportPanic(app_panic_invalid_state); */
        break;
    }

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalGattAccessInd
 *
 *  DESCRIPTION
 *      This function handles GATT_ACCESS_IND message for attributes
 *      maintained by the application.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
static void handleSignalGattAccessInd(GATT_ACCESS_IND_T *p_event_data)
{

    /*Handling signal as per current state */
    switch(g_lightapp_data.state)
    {
        case app_state_connected:
        {
            /* GATT_ACCESS_IND indicates that the central device is still disco-
             * -vering services. So, restart the connection parameter update
             * timer
             */
             if(g_lightapp_data.gatt_data.cpu_timer_value == TGAP_CPC_PERIOD &&
                g_lightapp_data.gatt_data.con_param_update_tid != TIMER_INVALID)
             {
                TimerDelete(g_lightapp_data.gatt_data.con_param_update_tid);
                g_lightapp_data.gatt_data.con_param_update_tid =
                                    TimerCreate(TGAP_CPC_PERIOD,
                                                TRUE, requestConnParamUpdate);
             }

            /* Received GATT ACCESS IND with write access */
            if(p_event_data->flags & ATT_ACCESS_WRITE)
            {
                /* If only ATT_ACCESS_PERMISSION flag is enabled, then the
                 * firmware is asking the app for permission to go along with
                 * prepare write request from the peer. Allow it.
                 */
                if(((p_event_data->flags) &
                   (ATT_ACCESS_PERMISSION | ATT_ACCESS_WRITE_COMPLETE))
                                                    == ATT_ACCESS_PERMISSION)
                {
                    GattAccessRsp(p_event_data->cid, p_event_data->handle,
                                  sys_status_success, 0, NULL);
                }
                else
                {
                    HandleAccessWrite(p_event_data);
                }
            }
            else if(p_event_data->flags & ATT_ACCESS_WRITE_COMPLETE)
            {
                GattAccessRsp(p_event_data->cid, p_event_data->handle,
                                          sys_status_success, 0, NULL);
            }
            /* Received GATT ACCESS IND with read access */
            else if(p_event_data->flags ==
                                    (ATT_ACCESS_READ | ATT_ACCESS_PERMISSION))
            {
                HandleAccessRead(p_event_data);
            }
            else
            {
                GattAccessRsp(p_event_data->cid, p_event_data->handle,
                              gatt_status_request_not_supported,
                              0, NULL);
            }
        }
        break;

        default:
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        break;
    }
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      handleSignalLmDisconnectComplete
 *
 *  DESCRIPTION
 *      This function handles LM Disconnect Complete event which is received
 *      at the completion of disconnect procedure triggered either by the
 *      device or remote host or because of link loss
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
static void handleSignalLmDisconnectComplete(
                HCI_EV_DATA_DISCONNECT_COMPLETE_T *p_event_data)
{

    /* Reset the connection parameter variables. */
    g_lightapp_data.gatt_data.conn_interval = 0;
    g_lightapp_data.gatt_data.conn_latency = 0;
    g_lightapp_data.gatt_data.conn_timeout = 0;

    CsrMeshHandleDataInConnection(GATT_INVALID_UCID, 0);

    /* Restore the relay and the promiscuous settings to the last set values */
    g_lightapp_data.bearer_data.bearerRelayActive = bearer_relay_active;
    g_lightapp_data.bearer_data.bearerPromiscuous = bearer_promiscuous;

    CsrMeshRelayEnable(g_lightapp_data.bearer_data.bearerRelayActive);
    CsrMeshEnablePromiscuousMode(g_lightapp_data.bearer_data.bearerPromiscuous);

#ifdef ENABLE_GATT_OTA_SERVICE
    if(OtaResetRequired())
    {
        OtaReset();
    }
#endif /* ENABLE_GATT_OTA_SERVICE */

    /*Handling signal as per current state */
    switch(g_lightapp_data.state)
    {
        case app_state_connected:
        case app_state_disconnecting:
        {
            /* Connection is terminated either due to Link Loss or
             * the local host terminated connection. In either case
             * Initialise the app data and go to fast advertising.
             */
            appDataInit();
            /* If GATT bearer is enabled move to advertisement state otherwise
             * move to idle state. The advertisement would be triggerred once
             * the GATT bearer is enabled again.
             */
            if(g_lightapp_data.bearer_data.bearerEnabled & 
                                                BLE_GATT_SERVER_BEARER_MASK)
            {
                AppSetState(app_state_advertising);
            }
            else
            {
                AppSetState(app_state_idle);
            }
        }
        break;

        default:
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        break;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleCsrMeshGroupSetMsg
 *
 *  DESCRIPTION
 *      This function handles the CSRmesh Group Assignment message. Stores
 *      the group_id at the given index for the model
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
static bool handleCsrMeshGroupSetMsg(uint8 *msg, uint16 len)
{
    CSR_MESH_MODEL_TYPE_T model = msg[0];
    uint8 index = msg[1];
    uint16 group_id = msg[3] + (msg[4] << 8);
    bool update_lastetag = TRUE;

    /* In case an incorrect index is received return without updating grps. */
    if(index >= MAX_MODEL_GROUPS)
    {
        return FALSE;
    }

    if(model == CSR_MESH_LIGHT_MODEL || model == CSR_MESH_ALL_MODELS)
    {
        /* Store Group ID */
        light_model_groups[index] = group_id;

        /* Save to NVM */
        Nvm_Write(&light_model_groups[index],
                 sizeof(uint16),
                 NVM_OFFSET_LIGHT_MODEL_GROUPS + index);
    }

    if(model == CSR_MESH_POWER_MODEL || model == CSR_MESH_ALL_MODELS)
    {
        power_model_groups[index] = group_id;

        /* Save to NVM */
        Nvm_Write(&power_model_groups[index],
                 sizeof(uint16),
                 NVM_OFFSET_POWER_MODEL_GROUPS + index);
    }

    if(model == CSR_MESH_ATTENTION_MODEL || model == CSR_MESH_ALL_MODELS)
    {
        attention_model_groups[index] = group_id;

        /* Save to NVM */
        Nvm_Write(&attention_model_groups[index],
                 sizeof(uint16),
                 NVM_OFFSET_ATTENTION_MODEL_GROUPS + index);
    }

#ifdef ENABLE_DATA_MODEL
    if(model == CSR_MESH_DATA_MODEL || model == CSR_MESH_ALL_MODELS)
    {
        data_model_groups[index] = group_id;

        /* Save to NVM */
        Nvm_Write(&data_model_groups[index],
                 sizeof(uint16),
                 NVM_OFFSET_DATA_MODEL_GROUPS + index);
    }
#endif /* ENABLE_DATA_MODEL */

    return update_lastetag;
}

/*============================================================================*
 *  Public Function Definitions
 *============================================================================*/
#ifdef NVM_TYPE_FLASH
/*----------------------------------------------------------------------------*
 *  NAME
 *      WriteApplicationAndServiceDataToNVM
 *
 *  DESCRIPTION
 *      This function writes the application data to NVM. This function should
 *      be called on getting nvm_status_needs_erase
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void WriteApplicationAndServiceDataToNVM(void)
{
    uint16 nvm_sanity = 0xffff;
    nvm_sanity = NVM_SANITY_MAGIC;

    /* Write NVM sanity word to the NVM */
    Nvm_Write(&nvm_sanity, sizeof(nvm_sanity), NVM_OFFSET_SANITY_WORD);

    /* Store the Association State */
    Nvm_Write((uint16 *)&g_lightapp_data.assoc_state,
             sizeof(g_lightapp_data.assoc_state),
              NVM_OFFSET_ASSOCIATION_STATE);

    /* Write GAP service data into NVM */
    WriteGapServiceDataInNVM();
}
#endif /* NVM_TYPE_FLASH */

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppSetState
 *
 *  DESCRIPTION
 *      This function is used to set the state of the application.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void AppSetState(app_state new_state)
{
    /* Check if the new state to be set is not the same as the present state
     * of the application.
     */
    app_state old_state = g_lightapp_data.state;

    if (old_state != new_state)
    {
        /* Handle exiting old state */
        switch (old_state)
        {
            case app_state_init:
            break;

            case app_state_disconnecting:
                /* Initialise CSRmesh light data and services
                 * data structure while exiting Disconnecting state.
                 */
                appDataInit();
            break;

            case app_state_advertising:
                /* Common things to do whenever application exits advertising
                 * state.
                 */
                appAdvertisingExit();
            break;

            case app_state_connected:
                /* Do nothing here */
            break;

            default:
                /* Nothing to do */
            break;
        }

        /* Set new state */
        g_lightapp_data.state = new_state;

        /* Handle entering new state */
        switch (new_state)
        {
            case app_state_advertising:
            {
                GattTriggerFastAdverts();
            }
            break;

            case app_state_connected:
            {
                /*DEBUG_STR("Connected\r\n");*/
            }
            break;

            case app_state_disconnecting:
                GattDisconnectReq(g_lightapp_data.gatt_data.st_ucid);
            break;

            default:
            break;
        }
    }
}

/*void Atoh(uint16 value,uint8 *buf,uint8 n)
{ 
   
	buf[n] = 0;
	
    buf[0] = (value >> 24)&0xFF;
    
    
}*/


/*----------------------------------------------------------------------------*
 *  NAME
 *      ReportPanic
 *
 *  DESCRIPTION
 *      This function calls firmware panic routine and gives a single point
 *      of debugging any application level panics
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void ReportPanic(app_panic_code panic_code)
{
    /* Raise panic */
    Panic(panic_code);
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      AppPowerOnReset
 *
 *  DESCRIPTION
 *      This user application function is called just after a power-on reset
 *      (including after a firmware panic), or after a wakeup from Hibernate or
 *      Dormant sleep states.
 *
 *      At the time this function is called, the last sleep state is not yet
 *      known.
 *
 *      NOTE: this function should only contain code to be executed after a
 *      power-on reset or panic. Code that should also be executed after an
 *      HCI_RESET should instead be placed in the AppInit() function.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
void AppPowerOnReset(void)
{
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
    uint16 gatt_db_length = 0;
    uint16 *p_gatt_db_pointer = NULL;
    bool light_poweron = FALSE;
    CSR_MESH_ADVSCAN_PARAM_T adv_scan_param;

#ifdef USE_STATIC_RANDOM_ADDRESS
    /* Generate random address for the CSRmesh Device. */
    generateStaticRandomAddress(&g_lightapp_data.random_bd_addr);

    /* Set the Static Random Address of the device. */
    GapSetRandomAddress(&g_lightapp_data.random_bd_addr);
#endif /* USE_STATIC_RANDOM_ADDRESS */

    /* Initialise the application timers */
    TimerInit(MAX_APP_TIMERS, (void*)app_timers);
    g_lightapp_data.nvm_tid = TIMER_INVALID;

#ifdef DEBUG_ENABLE
    /* Initialise UART and configure with
     * default baud rate and port configuration.
     */
    
    /*DebugInit(UART_BUF_SIZE_BYTES_32, UartDataRxCallback, NULL);*/
    UartInit(UartDataRxCallback, NULL,
             rx_data, UART_BUF_SIZE_BYTES_32,
             tx_data, UART_BUF_SIZE_BYTES_32,
             uart_data_unpacked);
    
    UartConfig(BAUD_RATE,0);
    

    /* UART Rx threshold is set to 1,
     * so that every byte received will trigger the rx callback.
     */
    UartEnable(TRUE);
    UartRead(1, 0);
    
    
    
    SleepModeChange(sleep_mode_never);
    /*UartWrite("H",1);*/
#endif /* DEBUG_ENABLE */

    /*DEBUG_STR("\r\nLight Application\r\n");*/

    /* Initialise GATT entity */
    GattInit();

    /* Install GATT Server support for the optional Write procedure
     * This is mandatory only if control point characteristic is supported.
     */
    GattInstallServerWriteLongReliable();

    /* Don't wakeup on UART RX line */
    SleepWakeOnUartRX(TRUE);

#ifdef NVM_TYPE_EEPROM
    /* Configure the NVM manager to use I2C EEPROM for NVM store */
    NvmConfigureI2cEeprom();
#elif NVM_TYPE_FLASH
    /* Configure the NVM Manager to use SPI flash for NVM store. */
    NvmConfigureSpiFlash();
#endif /* NVM_TYPE_EEPROM */

    NvmDisable();
    /* Initialise the GATT and GAP data.
     * Needs to be done before readPersistentStore
     */
    appDataInit();

    /* Read persistent storage.
     * Call this before CsrMeshInit.
     */
    readPersistentStore();

    /* Initialise the CSRmesh */
    CsrMeshInit(&g_node_data);

    /* Update Relay status on Light */
    CsrMeshRelayEnable(g_lightapp_data.bearer_data.bearerRelayActive);

    /* Update promiscuous status */
    CsrMeshEnablePromiscuousMode(g_lightapp_data.bearer_data.bearerPromiscuous);

    /* Enable Notifications for raw messages */
    CsrMeshEnableRawMsgEvent(TRUE);

    /* Initialise the light model */
    LightModelInit(light_model_groups, MAX_MODEL_GROUPS);

    /* Initialise the power model */
    PowerModelInit(power_model_groups, MAX_MODEL_GROUPS);

    /* Initialise Bearer Model */
    BearerModelInit();

#ifdef ENABLE_FIRMWARE_MODEL
    /* Initialise Firmware Model */
    FirmwareModelInit();

    /* Set Firmware Version */
    g_lightapp_data.fw_version.major_version = APP_MAJOR_VERSION;
    g_lightapp_data.fw_version.minor_version = APP_MINOR_VERSION;
#endif /* ENABLE_FIRMWARE_MODEL */

    /* Initialise Attention Model */
    AttentionModelInit(attention_model_groups, MAX_MODEL_GROUPS);

#ifdef ENABLE_BATTERY_MODEL
    BatteryModelInit();
#endif /* ENABLE_BATTERY_MODEL */

#ifdef ENABLE_DATA_MODEL
    AppDataStreamInit(data_model_groups, MAX_MODEL_GROUPS);
#endif /* ENABLE_DATA_MODEL */

    /* Start CSRmesh */
    CsrMeshStart();

    /* Get the stored adv scan parameters */
    CsrMeshGetAdvScanParam(&adv_scan_param);

    /* Read the mesh advertising parameter setting from the CS User Keys */
    adv_scan_param.advertising_interval =
                                CSReadUserKey(CSKEY_INDEX_CSRMESH_ADV_INTERVAL);
    adv_scan_param.advertising_time = 
                                    CSReadUserKey(CSKEY_INDEX_CSRMESH_ADV_TIME);
    CsrMeshSetAdvScanParam(&adv_scan_param);

    /* Tell Security Manager module about the value it needs to Initialise it's
     * diversifier to.
     */
    SMInit(0);

    /* Initialise CSRmesh light application State */
    g_lightapp_data.state = app_state_init;

    /* Initialise Light Hardware */
    LightHardwareInit();

/*#ifdef USE_ASSOCIATION_REMOVAL_KEY*/
    IOTSwitchInit();
/*#endif*/ /* USE_ASSOCIATION_REMOVAL_KEY */

    /* Start a timer which does device ID adverts till the time device
     * is associated
     */
    
    PioSetMode(PIO_4, pio_mode_user);         /*000 ====================> State-0 */
    PioSetDir(PIO_4, PIO_DIRECTION_OUTPUT);
    PioSetPullModes((1UL<<PIO_4), pio_mode_strong_pull_up);
    PioSet(PIO_4,FALSE);
   
        
    
    if(app_state_not_associated == g_lightapp_data.assoc_state)
    {
         
        
        initiateAssociation();
    }
    else
    {
        /*DEBUG_STR("Light is associated\r\n");*/
        PioSetMode(PIO_4, pio_mode_user);         /*000 ====================> State-0 */
        PioSetDir(PIO_4, PIO_DIRECTION_OUTPUT);
        PioSetPullModes((1UL<<PIO_4), pio_mode_strong_pull_up);
        PioSet(PIO_4,TRUE);
        
        

        /* Light is already associated. Set the colour from NVM */
        LightHardwareSetColor(g_lightapp_data.light_state.red,
                              g_lightapp_data.light_state.green,
                              g_lightapp_data.light_state.blue);


        /* Set the light power as read from NVM */
        if ((g_lightapp_data.power.power_state == POWER_STATE_ON) ||
            (g_lightapp_data.power.power_state == POWER_STATE_ON_FROM_STANDBY))
        {
            light_poweron = TRUE;
        }
        LightHardwarePowerControl(light_poweron);
    }

    if(g_lightapp_data.assoc_state == app_state_associated)
    {
        devid[1] = (uint8)(CsrMeshGetDeviceID())>>8;
        devid[0] = (uint8)(CsrMeshGetDeviceID()&0xff);
    }
    else
    {
        devid[1]=0x00;
        devid[0]=0x00;
    }
    
    /*DebugWriteUint8 (devid[1]);
    DebugWriteUint8 (devid[0]);*/
    /* Tell GATT about our database. We will get a GATT_ADD_DB_CFM event when
     * this has completed.
     */
    SleepModeChange(sleep_mode_never);
    p_gatt_db_pointer = GattGetDatabase(&gatt_db_length);
    GattAddDatabaseReq(gatt_db_length, p_gatt_db_pointer);
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
void AppProcessSystemEvent(sys_event_id id, void *data)
{
    switch (id)
    {
        case sys_event_pio_changed:
        {
/*#ifdef USE_ASSOCIATION_REMOVAL_KEY*/
            handlePIOEvent((pio_changed_data*)data);
/*#endif*/
        }
        break;

        default:
        break;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppProcessLmEvent
 *
 *  DESCRIPTION
 *      This user application function is called whenever a LM-specific event
 *      is received by the system.
 *
 * PARAMETERS
 *      event_code [in]   LM event ID
 *      event_data [in]   LM event data
 *
 * RETURNS
 *      TRUE if the application has finished with the event data;
 *           the control layer will free the buffer.
 *----------------------------------------------------------------------------*/
extern bool AppProcessLmEvent(lm_event_code event_code,
                              LM_EVENT_T *p_event_data)
{
    switch(event_code)
    {
        /* Handle events received from Firmware */

        case GATT_ADD_DB_CFM:
            /* Attribute database registration confirmation */
            handleSignalGattAddDBCfm((GATT_ADD_DB_CFM_T*)p_event_data);
        break;

        case GATT_CANCEL_CONNECT_CFM:
            /* Confirmation for the completion of GattCancelConnectReq()
             * procedure
             */
            handleSignalGattCancelConnectCfm();
        break;

        case LM_EV_CONNECTION_COMPLETE:
            /* Handle the LM connection complete event. */
            handleSignalLmEvConnectionComplete((LM_EV_CONNECTION_COMPLETE_T*)
                                                                p_event_data);
        break;

        case GATT_CONNECT_CFM:
            /* Confirmation for the completion of GattConnectReq()
             * procedure
             */
            handleSignalGattConnectCfm((GATT_CONNECT_CFM_T*)p_event_data);
        break;

        case SM_SIMPLE_PAIRING_COMPLETE_IND:
            /* Indication for completion of Pairing procedure */
            handleSignalSmSimplePairingCompleteInd(
                (SM_SIMPLE_PAIRING_COMPLETE_IND_T*)p_event_data);
        break;

        case LM_EV_ENCRYPTION_CHANGE:
            /* Indication for encryption change event */
            /* Nothing to do */
        break;

        /* Received in response to the LsConnectionParamUpdateReq()
         * request sent from the slave after encryption is enabled. If
         * the request has failed, the device should again send the same
         * request only after Tgap(conn_param_timeout). Refer Bluetooth 4.0
         * spec Vol 3 Part C, Section 9.3.9 and HID over GATT profile spec
         * section 5.1.2.
         */
        case LS_CONNECTION_PARAM_UPDATE_CFM:
            handleSignalLsConnParamUpdateCfm(
                (LS_CONNECTION_PARAM_UPDATE_CFM_T*) p_event_data);
        break;

        case LM_EV_CONNECTION_UPDATE:
            /* This event is sent by the controller on connection parameter
             * update.
             */
            handleSignalLmConnectionUpdate(
                            (LM_EV_CONNECTION_UPDATE_T*)p_event_data);
        break;

        case LS_CONNECTION_PARAM_UPDATE_IND:
            /* Indicates completion of remotely triggered Connection
             * parameter update procedure
             */
            handleSignalLsConnParamUpdateInd(
                            (LS_CONNECTION_PARAM_UPDATE_IND_T *)p_event_data);
        break;

        case GATT_ACCESS_IND:
            /* Indicates that an attribute controlled directly by the
             * application (ATT_ATTR_IRQ attribute flag is set) is being
             * read from or written to.
             */
            handleSignalGattAccessInd((GATT_ACCESS_IND_T*)p_event_data);
        break;

        case GATT_DISCONNECT_IND:
            /* Disconnect procedure triggered by remote host or due to
             * link loss is considered complete on reception of
             * LM_EV_DISCONNECT_COMPLETE event. So, it gets handled on
             * reception of LM_EV_DISCONNECT_COMPLETE event.
             */
         break;

        case GATT_DISCONNECT_CFM:
            /* Confirmation for the completion of GattDisconnectReq()
             * procedure is ignored as the procedure is considered complete
             * on reception of LM_EV_DISCONNECT_COMPLETE event. So, it gets
             * handled on reception of LM_EV_DISCONNECT_COMPLETE event.
             */
        break;

        case LM_EV_DISCONNECT_COMPLETE:
        {
            /* Disconnect procedures either triggered by application or remote
             * host or link loss case are considered completed on reception
             * of LM_EV_DISCONNECT_COMPLETE event
             */
             handleSignalLmDisconnectComplete(
                    &((LM_EV_DISCONNECT_COMPLETE_T *)p_event_data)->data);
        }
        break;

        case LM_EV_ADVERTISING_REPORT:
        {
            CsrMeshProcessMessage((LM_EV_ADVERTISING_REPORT_T *)p_event_data);
        }
        break;

        case LS_RADIO_EVENT_IND:
        {
            CsrMeshHandleRadioEvent();
        }
        break;

        default:
            /* Ignore any other event */
        break;

    }

    return TRUE;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppProcessCsrMeshEvent
 *
 *  DESCRIPTION
 *      This user application function is called whenever a CSRmesh event
 *      is received by the system.
 *
 * PARAMETERS
 *      event_code csr_mesh_event_t
 *      data       Data associated with the event
 *      length     Length of the data
 *      state_data Pointer to the variable pointing to state data.
 *
 * RETURNS
 *      TRUE if the app has finished with the event data; the control layer
 *      will free the buffer.
 *----------------------------------------------------------------------------*/
extern void AppProcessCsrMeshEvent(csr_mesh_event_t event_code, uint8* data,
                                   uint16 length, void **state_data)
{
    bool start_nvm_timer = FALSE;
    bool update_lastetag = FALSE;

    switch(event_code)
    {
        case CSR_MESH_ASSOCIATION_REQUEST:
        {
            if( g_lightapp_data.assoc_state != app_state_association_started)
            {
                g_lightapp_data.assoc_state = app_state_association_started;
            }
            TimerDelete(g_lightapp_data.mesh_device_id_advert_tid);
            g_lightapp_data.mesh_device_id_advert_tid = TIMER_INVALID;

            /* Blink Light in Yellow to indicate association started */
            /*LightHardwareSetBlink(127, 127, 0, 32, 32);*/
        }
        break;

        case CSR_MESH_KEY_DISTRIBUTION:
        {
            /*DEBUG_STR("Association complete\r\n");*/
            g_lightapp_data.assoc_state = app_state_associated;
            
            PioSetMode(PIO_4, pio_mode_user);         /*000 ====================> State-0 */
            PioSetDir(PIO_4, PIO_DIRECTION_OUTPUT);
            PioSetPullModes((1UL<<PIO_4), pio_mode_strong_pull_up);
            PioSet(PIO_4,TRUE);

            /* Write association state to NVM */
            Nvm_Write((uint16 *)&g_lightapp_data.assoc_state, 1,
                                                  NVM_OFFSET_ASSOCIATION_STATE);

            /* Save the network key on NVM */
            Nvm_Write((uint16 *)data, sizeof(CSR_MESH_NETWORK_KEY_T), 
                                                        NVM_OFFSET_NETWORK_KEY);

            /* The association is complete set LE bearer to non-promiscuous.*/
            g_lightapp_data.bearer_data.bearerPromiscuous &= ~BLE_BEARER_MASK;
            CsrMeshEnablePromiscuousMode(
                                g_lightapp_data.bearer_data.bearerPromiscuous);

            /* Update Bearer Model Data to NVM */
            Nvm_Write((uint16 *)&g_lightapp_data.bearer_data,
                      sizeof(BEARER_MODEL_STATE_DATA_T), NVM_BEARER_DATA_OFFSET);

            /* Restore light settings after association */
            LightHardwareSetColor(g_lightapp_data.light_state.red,
                                  g_lightapp_data.light_state.green,
                                  g_lightapp_data.light_state.blue);

            /* Restore power settings after association */
            LightHardwarePowerControl(g_lightapp_data.power.power_state);

        }
        break;

        case CSR_MESH_ASSOCIATION_ATTENTION:
        {
            CSR_MESH_ASSOCIATION_ATTENTION_DATA_T *attn_data;

            attn_data = (CSR_MESH_ASSOCIATION_ATTENTION_DATA_T *)data;

            /* Delete attention timer if it exists */
            if (TIMER_INVALID != attn_tid)
            {
                TimerDelete(attn_tid);
                attn_tid = TIMER_INVALID;
            }
            /* If attention Enabled */
            if (attn_data->attract_attention)
            {
                /* Create attention duration timer if required */
                if(attn_data->duration != 0xFFFF)
                {
                    attn_tid = TimerCreate(attn_data->duration * MILLISECOND, 
                                                        TRUE, attnTimerHandler);
                }
                /* Enable Green light blinking to attract attention */
                /*LightHardwareSetBlink(0, 127, 0, 16, 16);*/
            }
            else
            {
                if(g_lightapp_data.assoc_state == app_state_not_associated)
                {
                    /* Blink blue to indicate not associated status */
                    /*LightHardwareSetBlink(0, 0, 127, 32, 32);*/
                    PioSetMode(PIO_4, pio_mode_user);         /*000 ====================> State-0 */
                    PioSetDir(PIO_4, PIO_DIRECTION_OUTPUT);
                    PioSetPullModes((1UL<<PIO_4), pio_mode_strong_pull_up);
                    PioSet(PIO_4,FALSE);
                }
                else
                {
                    /* Restore Light State */
                    LightHardwareSetColor(g_lightapp_data.light_state.red,
                                          g_lightapp_data.light_state.green,
                                          g_lightapp_data.light_state.blue);

                    /* Restore the light Power State */
                    LightHardwarePowerControl(g_lightapp_data.power.power_state);
                }
            }
        }
        break;

        case CSR_MESH_UPDATE_MSG_SEQ_NUMBER:
        {
            /* Sequence number has updated, store it in NVM */
            Nvm_Write((uint16 *)data, 2, NVM_OFFSET_SEQUENCE_NUMBER);
        }
        break;

        case CSR_MESH_CONFIG_RESET_DEVICE:
        {
            /*DEBUG_STR("Reset Device\r\n");*/
            PioSetMode(PIO_4, pio_mode_user);         /*000 ====================> State-0 */
            PioSetDir(PIO_4, PIO_DIRECTION_OUTPUT);
            PioSetPullModes((1UL<<PIO_4), pio_mode_strong_pull_up);
            PioSet(PIO_4,FALSE);
            
            /* Move device to dissociated state */
            g_lightapp_data.assoc_state = app_state_not_associated;

            /* Write association state to NVM */
            Nvm_Write((uint16 *)&g_lightapp_data.assoc_state,
                     sizeof(g_lightapp_data.assoc_state),
                     NVM_OFFSET_ASSOCIATION_STATE);

            /* Reset the supported model groups and save it to NVM */
            /* Light model */
            MemSet(light_model_groups, 0x0000, sizeof(light_model_groups));
            Nvm_Write((uint16 *)light_model_groups, sizeof(light_model_groups),
                                                 NVM_OFFSET_LIGHT_MODEL_GROUPS);

            /* Power model */
            MemSet(power_model_groups, 0x0000, sizeof(power_model_groups));
            Nvm_Write((uint16 *)power_model_groups, sizeof(power_model_groups),
                                                 NVM_OFFSET_POWER_MODEL_GROUPS);

            /* Attention model */
            MemSet(attention_model_groups, 0x0000, sizeof(attention_model_groups));
            Nvm_Write((uint16 *)attention_model_groups, sizeof(attention_model_groups),
                                            NVM_OFFSET_ATTENTION_MODEL_GROUPS);

#ifdef ENABLE_DATA_MODEL
            /* Data stream model */
            MemSet(data_model_groups, 0x0000, sizeof(data_model_groups));
            Nvm_Write((uint16 *)data_model_groups, sizeof(data_model_groups),
                                            NVM_OFFSET_DATA_MODEL_GROUPS);
#endif /* ENABLE_DATA_MODEL */

            /* Reset Light State */
            g_lightapp_data.light_state.red   = 0xFF;
            g_lightapp_data.light_state.green = 0xFF;
            g_lightapp_data.light_state.blue  = 0xFF;
            g_lightapp_data.light_state.power = POWER_STATE_OFF;
            g_lightapp_data.power.power_state = POWER_STATE_OFF;
            start_nvm_timer = TRUE;

            /* Enable promiscuous mode on un-associated devices so that they 
             * relay all the messages. This helps propagate messages(MCP) based 
             * on the newly assigned network key as they will be relayed also by
             * the devices not yet associated.
             */
            g_lightapp_data.bearer_data.bearerPromiscuous =
                               (BLE_BEARER_MASK | BLE_GATT_SERVER_BEARER_MASK);
            CsrMeshEnablePromiscuousMode(
                                g_lightapp_data.bearer_data.bearerPromiscuous);

            /* Update Bearer Model Data to NVM */
            Nvm_Write((uint16 *)&g_lightapp_data.bearer_data,
                      sizeof(BEARER_MODEL_STATE_DATA_T), NVM_BEARER_DATA_OFFSET);

            /* Start Mesh association again */
            initiateAssociation();
        }
        break;

        case CSR_MESH_CONFIG_DEVICE_IDENTIFIER:
        {
            Nvm_Write((uint16 *)data, 1, NVM_OFFSET_DEVICE_ID);
            /*DEBUG_STR("Device ID received:");
            DEBUG_U16(((uint16)data[0]));
            DEBUG_STR("\r\n");*/
        }
        break;

        case CSR_MESH_CONFIG_GET_VID_PID_VERSION:
        {
            if (state_data != NULL)
            {
                *state_data = (void *)&vid_pid_info;
            }
        }
        break;

        case CSR_MESH_CONFIG_GET_APPEARANCE:
        {
            if (state_data != NULL)
            {
                *state_data = (void *)&device_appearance;
            }
        }
        break;

        case CSR_MESH_GROUP_SET_MODEL_GROUPID:
        {
            update_lastetag = handleCsrMeshGroupSetMsg(data, length);
        }
        break;

        case CSR_MESH_LIGHT_SET_LEVEL:
        {
            /* Update State of RGB in application */
            g_lightapp_data.light_state.level = data[0];
            g_lightapp_data.light_state.power = POWER_STATE_ON;
            g_lightapp_data.power.power_state = POWER_STATE_ON;
            start_nvm_timer = TRUE;

            /* Set the light level in the latest RGB setting */
            LightHardwareSetLevel(g_lightapp_data.light_state.red, 
                                  g_lightapp_data.light_state.green,
                                  g_lightapp_data.light_state.blue,
                                  g_lightapp_data.light_state.level);

            /* Send Light State Information to Model */
            if (state_data != NULL)
            {
                *state_data = (void *)&g_lightapp_data.light_state;
            }

            /* Don't apply to hardware unless light is ON */
            /*DEBUG_STR("Set Level: ");
            DEBUG_U8(data[0]);
            DEBUG_STR("\r\n");*/
        }
        break;

        case CSR_MESH_LIGHT_SET_RGB:
        {
            /* Update State of RGB in application */
            g_lightapp_data.light_state.level = data[0];
            g_lightapp_data.light_state.red   = data[1];
            g_lightapp_data.light_state.green = data[2];
            g_lightapp_data.light_state.blue  = data[3];
            g_lightapp_data.light_state.power = POWER_STATE_ON;
            g_lightapp_data.power.power_state = POWER_STATE_ON;
            start_nvm_timer = TRUE;

            /* Set the light level in the latest RGB setting */
            LightHardwareSetLevel(g_lightapp_data.light_state.red, 
                                  g_lightapp_data.light_state.green,
                                  g_lightapp_data.light_state.blue,
                                  g_lightapp_data.light_state.level);

            /* Send Light State Information to Model */
            if (state_data != NULL)
            {
                *state_data = (void *)&g_lightapp_data.light_state;
            }

            /*DEBUG_STR("Set RGB : ");
            DEBUG_U8(data[1]);
            DEBUG_STR(",");
            DEBUG_U8(data[2]);
            DEBUG_STR(",");
            DEBUG_U8(data[3]);
            DEBUG_STR("\r\n");*/
        }
        break;

        case CSR_MESH_LIGHT_SET_COLOR_TEMP:
        {
#ifdef COLOUR_TEMP_ENABLED
            g_lightapp_data.light_state.color_temp = 
                                             ((uint16)data[1] << 8) | (data[0]);

            g_lightapp_data.power.power_state = POWER_STATE_ON;
            g_lightapp_data.light_state.power = POWER_STATE_ON;

            /* Set Colour temperature of light */
            LightHardwareSetColorTemp(g_lightapp_data.light_state.color_temp);

            /* Send Light State Information to Model */
            if (state_data != NULL)
            {
                *state_data = (void *)&g_lightapp_data.light_state;
            }

            /*DEBUG_STR("Set Colour Temperature: ");
            DEBUG_U16(g_lightapp_data.light_state.color_temp);
            DEBUG_STR("\r\n");*/
#endif /* COLOUR_TEMP_ENABLED */
        }
        break;

        case CSR_MESH_LIGHT_GET_STATE:
        {
            /* Send Light State Information to Model */
            if (state_data != NULL)
            {
                *state_data = (void *)&g_lightapp_data.light_state;
            }
        }
        break;

        case CSR_MESH_POWER_GET_STATE:
        {
            /* Send Power State Information to Model */
            if (state_data != NULL)
            {
                *state_data = (void *)&g_lightapp_data.power;
            }
        }
        break;

        case CSR_MESH_POWER_TOGGLE_STATE:
        case CSR_MESH_POWER_SET_STATE:
        {
            if (CSR_MESH_POWER_SET_STATE == event_code)
            {
                g_lightapp_data.power.power_state = data[0];
            }
            else
            {
                togglePowerState();
            }

            /*DEBUG_STR("Set Power: ");
            DEBUG_U8(g_lightapp_data.power.power_state);
            DEBUG_STR("\r\n");*/

            if (g_lightapp_data.power.power_state == POWER_STATE_OFF ||
                g_lightapp_data.power.power_state == POWER_STATE_STANDBY)
            {
                LightHardwarePowerControl(FALSE);
            }
            else if(g_lightapp_data.power.power_state == POWER_STATE_ON ||
                    g_lightapp_data.power.power_state == \
                                                POWER_STATE_ON_FROM_STANDBY)
            {
                LightHardwareSetColor(g_lightapp_data.light_state.red,
                                      g_lightapp_data.light_state.green,
                                      g_lightapp_data.light_state.blue);

                /* Turn on with old colour value restored */
                LightHardwarePowerControl(TRUE);
            }

            g_lightapp_data.light_state.power =
                                            g_lightapp_data.power.power_state;

            /* Send Power State Information to Model */
            if (state_data != NULL)
            {
                *state_data = (void *)&g_lightapp_data.power;
            }

            start_nvm_timer = TRUE;
        }
        break;

        case CSR_MESH_BEARER_SET_STATE:
        {
            uint8 *pData = data;
            g_lightapp_data.bearer_data.bearerRelayActive = BufReadUint16(&pData);
            g_lightapp_data.bearer_data.bearerEnabled     = BufReadUint16(&pData);
            g_lightapp_data.bearer_data.bearerPromiscuous = BufReadUint16(&pData);

            /* BLE Advert Bearer is always enabled on this device. */
            g_lightapp_data.bearer_data.bearerEnabled    |= BLE_BEARER_MASK;

            /* Filter the supported bearers from the bitmap received */
            g_lightapp_data.bearer_data.bearerRelayActive = 
                g_lightapp_data.bearer_data.bearerRelayActive & 
                    (BLE_BEARER_MASK | BLE_GATT_SERVER_BEARER_MASK);

            /* Filter the supported bearers from the bitmap received */
            g_lightapp_data.bearer_data.bearerEnabled = 
                g_lightapp_data.bearer_data.bearerEnabled & 
                    (BLE_BEARER_MASK | BLE_GATT_SERVER_BEARER_MASK);

            g_lightapp_data.bearer_data.bearerPromiscuous = 
                g_lightapp_data.bearer_data.bearerPromiscuous & 
                    (BLE_BEARER_MASK | BLE_GATT_SERVER_BEARER_MASK);

            /* Update the saved values */
            bearer_relay_active = g_lightapp_data.bearer_data.bearerRelayActive;
            bearer_promiscuous = g_lightapp_data.bearer_data.bearerPromiscuous;

            /* Update new bearer state */
            CsrMeshRelayEnable(g_lightapp_data.bearer_data.bearerRelayActive);
            CsrMeshEnablePromiscuousMode(
                             g_lightapp_data.bearer_data.bearerPromiscuous);

            /* Update Bearer Model Data to NVM */
            Nvm_Write((uint16 *)&g_lightapp_data.bearer_data,
                     sizeof(BEARER_MODEL_STATE_DATA_T), NVM_BEARER_DATA_OFFSET);

            if(g_lightapp_data.state != app_state_connected) 
            {
                if(g_lightapp_data.bearer_data.bearerEnabled 
                                                & BLE_GATT_SERVER_BEARER_MASK)
                {
                    AppSetState(app_state_advertising);
                }
                else
                {
                    AppSetState(app_state_idle);
                }
            }

            /* Update LastETag. */
            update_lastetag = TRUE;
        }
        /* Fall through */
        case CSR_MESH_BEARER_GET_STATE:
        {
            if (state_data != NULL)
            {
                *state_data = (void *)&g_lightapp_data.bearer_data;
            }
        }
        break;

#ifdef ENABLE_FIRMWARE_MODEL
        case CSR_MESH_FIRMWARE_GET_VERSION_INFO:
        {
            /* Send Firmware Version data to the model */
            if (state_data != NULL)
            {
                *state_data = (void *)&g_lightapp_data.fw_version;
            }
        }
        break;

        case CSR_MESH_FIRMWARE_UPDATE_REQUIRED:
        {
            BD_ADDR_T *pBDAddr = NULL;
#ifdef USE_STATIC_RANDOM_ADDRESS
            pBDAddr = &g_lightapp_data.random_bd_addr;
#endif /* USE_STATIC_RANDOM_ADDRESS */

            /*DEBUG_STR("\r\n FIRMWARE UPDATE IN PROGRESS \r\n");*/

            /* Write the value CSR_OTA_BOOT_LOADER to NVM so that
             * it starts in OTA mode upon reset
             */
            OtaWriteCurrentApp(csr_ota_boot_loader,
                               FALSE,   /* is bonded */
                               NULL,    /* Typed host BD Address */
                               0,       /* Diversifier */
                               pBDAddr, /* local_random_address */
                               NULL,    /* irk */
                               FALSE    /* service_changed_config */
                              );

           /* Defer OTA Reset for half a second to ensure that,
            * acknowledgements are sent before reset.
            */
           ota_rst_tid = TimerCreate(OTA_RESET_DEFER_DURATION, TRUE,
                                     issueOTAReset);

           /* Update LastETag. */
           update_lastetag = TRUE;
        }
        break;
#endif /* ENABLE_FIRMWARE_MODEL */

#ifdef ENABLE_BATTERY_MODEL
        case CSR_MESH_BATTERY_GET_STATE:
        {
            /* Initialise battery state. IOT  boards (H13323) are battery powered */
            g_lightapp_data.battery_data.battery_state = 
                                            BATTERY_MODEL_STATE_POWERING_DEVICE;
            /* Read Battery Level */
            g_lightapp_data.battery_data.battery_level = ReadBatteryLevel();

            if(g_lightapp_data.battery_data.battery_level == 0)
            {
                /* Voltage is below flat battery voltage. Set the needs 
                 * replacement flag in the battery state
                 */
                g_lightapp_data.battery_data.battery_state |=
                                          BATTERY_MODEL_STATE_NEEDS_REPLACEMENT;
            }
            /* Pass Battery state data to model */
            if (state_data != NULL)
            {
                *state_data = (void *)&g_lightapp_data.battery_data;
            }
        }
        break;
#endif /* ENABLE_BATTERY_MODEL */

        case CSR_MESH_ATTENTION_SET_STATE:
        {
            uint32 dur_us;
            /* Read the data */
            g_lightapp_data.attn_data.attract_attn  = BufReadUint8(&data);
            g_lightapp_data.attn_data.attn_duration = BufReadUint16(&data);

            /* Delete attention timer if it exists */
            if (TIMER_INVALID != attn_tid)
            {
                TimerDelete(attn_tid);
                attn_tid = TIMER_INVALID;
            }

            /* If attention Enabled */
            if (g_lightapp_data.attn_data.attract_attn)
            {
                /* Create attention duration timer if required */
                if (g_lightapp_data.attn_data.attn_duration != 0xFFFF)
                {
                    dur_us = (uint32)g_lightapp_data.attn_data.attn_duration * \
                                     MILLISECOND;
                    attn_tid = TimerCreate(dur_us, TRUE, attnTimerHandler);
                }

                /* Enable Red light blinking to attract attention */
                /*LightHardwareSetBlink(127, 0, 0, 32, 32);*/
            }
            else
            {
                /* Restore Light State */
                LightHardwareSetColor(g_lightapp_data.light_state.red,
                                      g_lightapp_data.light_state.green,
                                      g_lightapp_data.light_state.blue);

                /* Restore the light Power State */
                LightHardwarePowerControl(g_lightapp_data.power.power_state);
            }

            /* Send response data to model */
            if (state_data != NULL)
            {
                *state_data = (void *)&g_lightapp_data.attn_data;
            }

            /* Debug logs */
            /*DEBUG_STR("\r\n ATTENTION_SET_STATE : Enable :");
            DEBUG_U8(g_lightapp_data.attn_data.attract_attn);
            DEBUG_STR("Duration : ");
            DEBUG_U16(g_lightapp_data.attn_data.attn_duration);
            DEBUG_STR("\r\n");*/

        }
        break;

#ifdef ENABLE_DATA_MODEL
        /* Data stream model messages */
        case CSR_MESH_DATA_STREAM_SEND_CFM:
        {
            /*uint8 i;
                {
                    for(i=0;i<((CSR_MESH_STREAM_EVENT_T *)data)->data_len;i++)
                    {
                        DEBUG_C8(((CSR_MESH_STREAM_EVENT_T *)data)->data[i]);
                    }
                }*/
            
            
           /* handleCSRmeshDataStreamSendCfm((CSR_MESH_STREAM_EVENT_T *)data);*/

         }
        break;

        case CSR_MESH_DATA_STREAM_DATA_IND:
        {
            /*uint8 i ;
            
           
                {
                    for(i=0;i<((CSR_MESH_STREAM_EVENT_T *)data)->data_len;i++)
                    {
                        DEBUG_C8(((CSR_MESH_STREAM_EVENT_T *)data)->data[i]);
                    }
                }*/
                 
           /* handleCSRmeshDataStreamDataInd((CSR_MESH_STREAM_EVENT_T *)data);*/
        }
        break;

        /* Stream flush indication */
        case CSR_MESH_DATA_STREAM_FLUSH_IND:
        {
/*            handleCSRmeshDataStreamFlushInd((CSR_MESH_STREAM_EVENT_T *)data);*/
        }
        break;

        /* Received a single block of data */
        case CSR_MESH_DATA_BLOCK_IND:
        {
           uint8 i = 0;
           uint8 array[11] = {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'};
                 
                {
                    for(;i<((CSR_MESH_STREAM_EVENT_T *)data)->data_len;i++)
                    {
                        /*DEBUG_C8(((CSR_MESH_STREAM_EVENT_T *)data)->data[i]);*/
                        array[i]=((CSR_MESH_STREAM_EVENT_T *)data)->data[i];
                    }
                }
                UartWrite(&array[0],((CSR_MESH_STREAM_EVENT_T *)data)->data_len);
           
                /*handleCSRmeshDataBlockInd((CSR_MESH_STREAM_EVENT_T *)data);*/
          
        }
        break;
#endif /* ENABLE_DATA_MODEL */

        /* Received a raw message from lower-layers.
         * Notify to the control device if connected.
         */
        case CSR_MESH_RAW_MESSAGE:
        {
            if (g_lightapp_data.state == app_state_connected)
            {
                MeshControlNotifyResponse(g_lightapp_data.gatt_data.st_ucid,
                                          data, length);
            }
        }
        break;

        default:
        break;
    }

    /* Commit Update LastETag. */
    if (update_lastetag)
    {
        CsrMeshUpdateLastETag(&g_node_data.device_ETag);
        /* Save the device ETag on NVM */
        NvmWrite(g_node_data.device_ETag.ETag, sizeof(CSR_MESH_ETAG_T),
                                                        NVM_OFFSET_DEVICE_ETAG);
    }

    /* Start NVM timer if required */
    if (TRUE == start_nvm_timer)
    {
        /* Delete existing timer */
        if (TIMER_INVALID != g_lightapp_data.nvm_tid)
        {
            TimerDelete(g_lightapp_data.nvm_tid);
        }

        /* Re-start the timer */
        g_lightapp_data.nvm_tid = TimerCreate(NVM_WRITE_DEFER_DURATION,
                                              TRUE,
                                              lightDataNVMWriteTimerHandler);
    }
}

