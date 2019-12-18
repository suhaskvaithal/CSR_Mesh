/******************************************************************************
 *  Copyright Cambridge Silicon Radio Limited 2015
 *  CSR Bluetooth Low Energy CSRmesh 1.3 Release
 *****************************************************************************/
/*! \file csr_mesh.h
 *  \brief CSRmesh library configuration and control functions
 *
 *   This file contains the functions to provide the application with
 *   access to the CSRmesh library
 *
 *   NOTE: This library includes the Mesh Transport Layer, Mesh Control
 *   Layer and Mesh Association Layer functionality.
 *
 *   Copyright (c) CSR plc 2013
 */

#ifndef __CSR_MESH_H__
#define __CSR_MESH_H__

#include "types.h"
#include <bt_event_types.h>

/*! \addtogroup CSRmesh
 * @{
 */

/*============================================================================*
Public Definitions
*============================================================================*/
/*! \brief Number of timers required for CSRmesh library to be reserved by
 * the application.
 */
#define MAX_CSR_MESH_TIMERS         (6) /*!< \brief User application needs
                                          * to reserve these many timers along
                                          * with application timers.\n
                                          * Example:\n
                                          * \code #define MAX_APP_TIMERS (3 + MAX_CSR_MESH_TIMERS)
                                          * \endcode
                                          */

/*! \struct LM_EV_ADVERTISING_REPORT_T
 *  \brief  LM Advertisement report structure. Refer to CSR uEnergy SDK API
 *   documentation for details
 */

/*! \brief Bluetooth SIG Organization identifier for CSRmesh device appearance */
#define APPEARANCE_ORG_BLUETOOTH_SIG (0)

#define BLE_BEARER_MASK              (0x1 << CSR_MESH_BEARER_BLE)                  /*!< \brief Bearer Mask for BLE bearer. */
#define BLE_GATT_SERVER_BEARER_MASK  (0x1 << CSR_MESH_BEARER_BLE_GATT_SERVER)      /*!< \brief Bearer Mask for GATT server bearer. */

#define SUPPORTED_BEARER_MASK        (BLE_BEARER_MASK | BLE_GATT_SERVER_BEARER_MASK)

/*! \brief 128-bit UUID type */
typedef struct
{
    uint16 uuid[8]; /*!< \brief CSRmesh 128-bit UUID 
                     *     The array should be stored as word wise little endian
                     *     If the UUID is 0x0123456789ABCDEFFEDCBA9876543210
                     *     it should be set as: {0x3210, 0x7654, 0xBA98, 0xFEDC,
                     *      0xCDEF, 0x89AB, 0x4567, 0x0123};
                     */
}CSR_MESH_UUID_T;

/*! \brief 64 bit Authorisation Code type */
typedef struct
{
    uint16 auth_code[4]; /*!< \brief CSRmesh 64 bit Authorisation Code. */
}CSR_MESH_AUTH_CODE_T;

/*! \brief 128-bit network key */
typedef struct
{
    uint16 key[8]; /*!< \brief CSRmesh 128-bit Network-Key */
}CSR_MESH_NETWORK_KEY_T;

/*! \brief 64-bit Device ETag */
typedef struct
{
    uint16 ETag[4]; /*!< \brief CSRmesh 64-bit device ETag */
}CSR_MESH_ETAG_T;

/*! \brief CSRmesh Product ID, Vendor ID and Version Information. */
typedef struct
{
    uint16 vendor_id;   /*!< \brief Vendor Identifier.  */
    uint16 product_id;  /*!< \brief Product Identifier. */
    uint32 version;     /*!< \brief Version Number.     */
}CSR_MESH_VID_PID_VERSION_T;

/*! \brief CSRmesh Device Appearance. The Appearance is a 24-bit value that is 
 *   composed of an "organization" and an "organization appearance". 
 */
typedef struct
{
    uint8  organization; /*!< \brief Identifies the organization which assigns
                               device appearance values. Refer \ref XAP_8bit_handler_page
                          */
    uint16 value;        /*!< \brief Appearance value */
}CSR_MESH_APPEARANCE_T;

/*! \brief CSRmesh Diagnostic data type. */
typedef struct
{
    uint16 num_mesg_received; /*< \brief Number of messages received */
    uint16 num_mesg_repeated; /*< \brief Number of messages repeated. */
    uint16 num_mesg_unknown;  /*< \brief Number of messages with Unknown Key/IV. */
     int32 acc_rssi;          /*< \brief Accumulated RSSI of all the messages */
}CSR_MESH_DIAGNOSTIC_DATA_T;

/*! \brief CSRmesh Scan and Advertising Parameters */
typedef struct
{
    uint16 scan_duty_cycle;      /*!< \brief CSRmesh scan duty cycle \n
                                  *   0 - 100: Duty cycle percentage in 1 percent steps \n
                                  *   101-255: Duty cycle = (value-100)/10 percentage in 0.1 percent steps \n
                                  *            0.1 to 15.5%
                                  */
    uint16 min_scan_slot;        /*!< \brief CSRMesh minimum scan slot. \n
                                  *  0x0004-0x4000: The minimum slot for calculating the scan window 
                                  *                 and scan interval in multiples of 625uSec while
                                  *                 setting the scan duty cycle. \n
                                  *                 Min(Default) value : 4 - 2.5ms, Max Value : 0x4000- 10240ms
                                  */
    uint16 advertising_interval; /*!< \brief CSRmesh advertising interval in milliseconds */
    uint16 advertising_time;     /*!< \brief CSRmesh advertising time in milliseconds */
    uint8  relay_repeat_count;   /*!< \brief Number of times relay messages to be repeated 
                                  *    Refer \ref XAP_8bit_handler_page
                                  */
}CSR_MESH_ADVSCAN_PARAM_T;

/*! \brief CSRmesh Scan Parameters */
typedef struct
{
    uint16 scan_window;          /*!< \brief CSRmesh scan window in milliseconds */
    uint16 scan_interval;        /*!< \brief CSRmesh scan interval in milliseconds */
}CSR_MESH_SCAN_PARAM_T;

/*! \brief CSRmesh Node Data */
typedef struct
{
    bool associated;                /*!< \brief Set to TRUE if associated */
    bool use_authorisation;         /*!< \brief Set to TRUE if device authorisation is required
                                     *           for association 
                                     */
    CSR_MESH_UUID_T device_uuid;    /*!< \brief 128-bit Device UUID */
    CSR_MESH_AUTH_CODE_T auth_code; /*!< \brief 64-bit Authorisation Code. */
    CSR_MESH_NETWORK_KEY_T nw_key;  /*!< \brief Network key assigned to the device.It is
                                     *          received in \ref CSR_MESH_KEY_DISTRIBUTION message. */
    uint16 device_id;               /*!< \brief 16-bit Device Identifier assigned during association */
    uint32 seq_number;              /*!< \brief 24 bit sequence number. It is incremented for every 
                                     *          model message sent by the device. CSRmesh reports an 
                                     *          event \ref CSR_MESH_UPDATE_MSG_SEQ_NUMBER whenever 
                                     *          it is incremented by a threshold.
                                     */
    CSR_MESH_ETAG_T device_ETag;    /*!< \brief The last Etag of the device. The ETag should be updated
                                     *          whenever a device configuration changes. A new Etag is 
                                     *          returned when \ref CsrMeshUpdateLastETag is called.
                                     */
}CSR_MESH_NODE_DATA_T;

/*! \brief CSRmesh User Advertisement parameters */
typedef struct
{
    TYPED_BD_ADDR_T   bd_addr;      /*!< \brief Type and Address to use for the advertising */
    ls_advert_type    adv_type;     /*!< \brief Advertisement type to be used*/
    gap_role          role;         /*!< \brief GAP role */
    gap_mode_connect  connect_mode; /*!< \brief GAP connect mode */
    gap_mode_discover discover_mode;/*!< \brief GAP discover mode */
    gap_mode_bond     bond;         /*!< \brief GAP bond mode */
    gap_mode_security security_mode;/*!< \brief GAP security mode */
}CSR_MESH_USER_ADV_PARAMS_T;

/*! \brief Association attention request data type */
typedef struct
{
    uint8 attract_attention; /*!< \brief Enable or disable attracting attention\n
                              *          0 - Do not attract attention\n
                              *          1 - Attract attention
                              *    Refer \ref XAP_8bit_handler_page
                              */
    uint16 duration;         /*!< \brief Duration for which the attention is 
                              *          requested. 
                              */
} CSR_MESH_ASSOCIATION_ATTENTION_DATA_T;

/*! \brief CSRmesh device power state */
typedef enum
{
    POWER_STATE_OFF = 0,            /*!< \brief Device is in OFF state */
    POWER_STATE_ON = 1,             /*!< \brief Device is in ON state */
    POWER_STATE_STANDBY = 2,        /*!< \brief Device is in STANDBY state */
    POWER_STATE_ON_FROM_STANDBY = 3 /*!< \brief Device returned to ON state from STANDBY
                                     * state
                                     */
}POWER_STATE_T;

/*! \brief CSRmesh Bearers */
typedef enum
{
    CSR_MESH_BEARER_BLE              = 0,  /*!< \brief Bluetooth Low Energy Bearer. */
    CSR_MESH_BEARER_BLE_GATT_SERVER  = 1   /*!< \brief Bluetooth Low Energy GATT Server (CSRmesh Control Service) Bearer. */
} CSR_MESH_BEARER_T;

/*! \brief CSRmesh Message types */
typedef enum
{
    CSR_MESH_MESSAGE_ASSOCIATION, /*!< \brief CSRmesh Association message. */
    CSR_MESH_MESSAGE_CONTROL      /*!< \brief CSRmesh Control message. */
} CSR_MESH_MESSAGE_T;

/*! \brief CSRmesh transmit Queue status values. */
typedef enum
{
    CSR_MESH_TX_QUEUE_EMPTY = 0   /*!< \brief CSRmesh Transmit Queue is empty */
}CSR_MESH_TX_QUEUE_STATUS_T;

/*! \brief CSRmesh Advert types */
typedef enum
{
    CSR_MESH_MESSAGE_ADVERT,   /*!< \brief CSRmesh Message Advert. */
    CSR_MESH_BRIDGE_ADVERT,    /*!< \brief CSRmesh Bridge Advert.  */
    CSR_MESH_UNKNOWN_ADVERT    /*!< \brief CSRmesh Unknown Advert. */
} CSR_MESH_ADVERT_T;

/*! \brief CSRmesh Model types */
typedef enum
{
    CSR_MESH_WATCHDOG_MODEL = 0,
    CSR_MESH_CONFIG_MODEL = 1,
    CSR_MESH_GROUP_MODEL = 2,
    CSR_MESH_KEY_MODEL = 3,
    CSR_MESH_SENSOR_MODEL = 4,
    CSR_MESH_ACTUATOR_MODEL = 5,
    CSR_MESH_ASSET_MODEL = 6,
    CSR_MESH_LOCATION_MODEL = 7,
    CSR_MESH_DATA_MODEL = 8,
    CSR_MESH_FIRMWARE_MODEL = 9,
    CSR_MESH_DIAGNOSTIC_MODEL = 10,
    CSR_MESH_BEARER_MODEL = 11,
    CSR_MESH_PING_MODEL = 12,
    CSR_MESH_BATTERY_MODEL = 13,
    CSR_MESH_ATTENTION_MODEL = 14,
    CSR_MESH_IDENTIFIER_MODEL = 15,
    CSR_MESH_WALLCLOCK_MODEL = 16,
    CSR_MESH_SEMANTIC_MODEL = 17,
    CSR_MESH_EFFECT_MODEL = 18,
    CSR_MESH_POWER_MODEL = 19,
    CSR_MESH_LIGHT_MODEL = 20,
    CSR_MESH_SWITCH_MODEL = 21,
    CSR_MESH_EVENT_MODEL = 22,
    CSR_MESH_VOLUME_MODEL = 23,
    CSR_MESH_IV_MODEL = 24,
    CSR_MESH_REMOTE_MODEL = 25,
    CSR_MESH_USER_MODEL = 26,
    CSR_MESH_TIMER_MODEL = 27,
    CSR_MESH_ALL_MODELS = 255
}CSR_MESH_MODEL_TYPE_T;

/*! \brief CSRmesh event types */
typedef enum
{
    /* Device association event types */
    CSR_MESH_ASSOCIATION_REQUEST = 0x0001,               /*!< \brief Received when a control device sends association request*/
    CSR_MESH_KEY_DISTRIBUTION = 0x0002,                  /*!< \brief Received when association is complete and control device assigns a network key  */

    /* CSRmesh Transmit queue event */
    CSR_MESH_TX_QUEUE_EVENT = 0x0004,                    /*!< \brief Received when the CSRmesh transmit Queue status changes.
                                                          *          This message is received only if the application queried for the status.
                                                          */
    /* CSRmesh request Attention */
    CSR_MESH_ASSOCIATION_ATTENTION = 0x000F,             /*!< \brief Received when a control device seeks the device's attention even before .
                                                          *          the device is associated.
                                                          */
    /* CSRmesh Update sequence number */
    CSR_MESH_UPDATE_MSG_SEQ_NUMBER = 0x0020,             /*!< \brief Received when the sequence number is incremented by a threshold. Application has
                                                          *          to save this value and set it on the CSRmesh upon a device restart.
                                                          */
    /* Watchdog Model Events. */
    CSR_MESH_WATCHDOG_SET_INTERVAL = 0x0100,             /*!< \brief Received when control device sets watchdog parameters. */
    /* Config Model Events */
    CSR_MESH_CONFIG_LAST_SEQUENCE_NUMBER = 0x0101,       /*!< \brief Reserved for future use */
    CSR_MESH_CONFIG_RESET_DEVICE = 0x0102,               /*!< \brief Received when control device sends reset command */
    CSR_MESH_CONFIG_DISCOVER_DEVICE = 0x0103,            /*!< \brief Reserved for future use */
    CSR_MESH_CONFIG_SET_DEVICE_IDENTIFIER = 0x0104,      /*!< \brief Received when a control device sets a device identifier*/
    CSR_MESH_CONFIG_DEVICE_IDENTIFIER = 0x0105,          /*!< \brief Reserved for future use */
    CSR_MESH_CONFIG_GET_VID_PID_VERSION = 0x0106,        /*!< \brief Received when a controller device queries for Product ID, Vendor ID or Version Number. */
    CSR_MESH_CONFIG_GET_APPEARANCE      = 0x0107,        /*!< \brief Received when a controller device queries for Device Apprearnce. */

    /* Group Model Events */
    CSR_MESH_GROUP_GET_NUMBER_OF_MODEL_GROUPIDS = 0x0110,/*!< \brief Reserved for future use */
    CSR_MESH_GROUP_NUMBER_OF_MODEL_GROUPIDS = 0x0111,    /*!< \brief Reserved for future use */
    CSR_MESH_GROUP_SET_MODEL_GROUPID = 0x0112,           /*!< \brief Received when a control device sends a group assignment command */
    CSR_MESH_GROUP_GET_MODEL_GROUPID = 0x0113,           /*!< \brief Reserved for future use */
    CSR_MESH_GROUP_MODEL_GROUPID = 0x0114,               /*!< \brief Reserved for future use */

    /* Ping Model Events */
    CSR_MESH_PING_RESPONSE        = 0x0119,              /*!< \brief Received in response to the ping sent from the device */

    /* Diagnostic Model Events */
    CSR_MESH_DIAGNOSTIC_TRAFFIC_STATS         = 0x0a1a,  /*!< \brief Received in response to the DIAGNOSTIC_RESET or DIAGNOSTIC_READ command sent from the device */
    CSR_MESH_DIAGNOSTIC_GET_STATE             = 0x0a1b,  /*!< \brief Reserved for Future Use */
    CSR_MESH_DIAGNOSTIC_SET_STATE             = 0x0a1c,  /*!< \brief Reserved for Future Use */
    CSR_MESH_DIAGNOSTIC_STATE                 = 0x0a1d,  /*!< \brief Reserved for Future Use */
    CSR_MESH_DIAGNOSTIC_GET_RSSI_BINS         = 0x0a1e,  /*!< \brief Reserved for Future Use */
    CSR_MESH_DIAGNOSTIC_RSSI_BINS             = 0x0a1f,  /*!< \brief Reserved for Future Use */
    CSR_MESH_DIAGNOSTIC_RESET_RSSI_BINS       = 0x0a20,  /*!< \brief Reserved for Future Use */
    CSR_MESH_DIAGNOSTIC_GET_RSSI_TABLE_LENGTH = 0x0a21,  /*!< \brief Reserved for Future Use */
    CSR_MESH_DIAGNOSTIC_RSSI_TABLE_LENGTH     = 0x0a22,  /*!< \brief Reserved for Future Use */
    CSR_MESH_DIAGNOSTIC_GET_RSSI_TABLE_ENTRY  = 0x0a23,  /*!< \brief Reserved for Future Use */
    CSR_MESH_DIAGNOSTIC_RSSI_TABLE_ENTRY      = 0x0a24,  /*!< \brief Reserved for Future Use */
    CSR_MESH_DIAGNOSTIC_DEVICE_TX_PARAM       = 0x0a25,  /*!< \brief Reserved for Future Use */

    /* Attention Model Events */
    CSR_MESH_ATTENTION_SET_STATE = 0x011b,               /*!< \brief Received on a device when a control device sends an ATTENTION_SET_STATE command */

    /* Power Model events */
    CSR_MESH_POWER_SET_STATE = 0x0131,                   /*!< \brief Received when a control device sends a power set state command */
    CSR_MESH_POWER_TOGGLE_STATE = 0x0133,                /*!< \brief Received when a control device sends a power toggle state command */
    CSR_MESH_POWER_GET_STATE = 0x0134,                   /*!< \brief Received when control device requests for device state */
    CSR_MESH_POWER_STATE = 0x0135,                       /*!< \brief Received on a control device when a device sends its own state in response
                                                          *   to a Power Get State or Power Set State commands
                                                          */
    /* Light Model Events */
    CSR_MESH_LIGHT_SET_LEVEL = 0x0141,                   /*!< \brief Received when a control device sends a Light Set Level command */
    CSR_MESH_LIGHT_SET_RGB = 0x0143,                     /*!< \brief Received when a control device sends a Light Set RGB command */
    CSR_MESH_LIGHT_SET_POWER_LEVEL = 0x0145,             /*!< \brief Received when a control device sends a light Set power level command */
    CSR_MESH_LIGHT_GET_STATE = 0x0146,                   /*!< \brief Received when a control device requests for the current light state of the device */
    CSR_MESH_LIGHT_STATE = 0x0147,                       /*!< \brief Received on a control device in response to a Light Set command or Light Get state command */
    CSR_MESH_LIGHT_SET_COLOR_TEMP = 0x0148,              /*!< \brief Received when a control device sends a Light Set Colour Temperature command */

    /* Asset Model Events */
    CSR_MESH_ASSET_STATE = 0x0150,
    CSR_MESH_ASSET_IDENTIFIER = 0x0151,
    CSR_MESH_ASSET_BROADCAST = 0x0152,
    CSR_MESH_ASSET_FOUND = 0x0153,
    CSR_MESH_ASSET_REPORT = 0x0154,
    CSR_MESH_ASSET_LOCATION = 0x155,

    /* Data Stream Model Events */
    CSR_MESH_DATA_STREAM_DATA_IND = 0x0171,              /*!< \brief Data stream message is received. */
    CSR_MESH_DATA_STREAM_SEND_CFM = 0x0172,              /*!< \brief Received when the receiver acknowledges the data stream message sent
                                                          *    by calling the \ref StreamSendData 
                                                          */
    CSR_MESH_DATA_BLOCK_IND = 0x0173,                    /*!< \brief Received a block of data */
    CSR_MESH_DATA_STREAM_FLUSH_IND = 0x0174,             /*!< \brief Indicates the Start or End of a data stream */

    /* Firmware Model Events */
    CSR_MESH_FIRMWARE_GET_VERSION_INFO = 0x01C0,         /*!< \brief Reserved for future use */
    CSR_MESH_FIRMWARE_VERSION_INFO = 0x01C1,             /*!< \brief Reserved for future use */
    CSR_MESH_FIRMWARE_UPDATE_REQUIRED = 0x01C2,          /*!< \brief Received when a control device requests for a firmware upgrade of the device */
    CSR_MESH_FIRMWARE_UPDATE_ACKNOWLEDGED = 0x01C3,      /*!< \brief Reserved for future use */
    CSR_MESH_FIRMWARE_BLOCK_REQ = 0x01C4,                /*!< \brief Reserved for future use */
    CSR_MESH_FIRMWARE_BLOCK = 0x01C5,                    /*!< \brief Reserved for future use */
    CSR_MESH_FIRMWARE_CHECKSUM = 0x01C6,                 /*!< \brief Reserved for future use */
    CSR_MESH_FIRMWARE_RESTART = 0x01C7,                  /*!< \brief Reserved for future use */

    /* Bearer Model Events */
    CSR_MESH_BEARER_SET_STATE = 0x0128,                  /*!< \brief Received on the device when a control device sends BEARER_SET_STATE command */
    CSR_MESH_BEARER_GET_STATE = 0x0129,                  /*!< \brief Received on the device when a control device sends BEARER_GET_STATE command */
    CSR_MESH_BEARER_STATE = 0x012A,                      /*!< \brief Received on a control device when a device sends its bearer status
                                                          *   in response to a BEARER_SET_STATE or BEARER_GET_STATE commands.
                                                          */

    CSR_MESH_BATTERY_GET_STATE = 0x012C,                 /*!< \brief Received when a control device requests for the battery status */

    /* Sensor Model Events. */
    CSR_MESH_SENSOR_GET_TYPES      = 0x0220,             /*!< \brief Received when a control device wants to know the types the sensor supports */
    CSR_MESH_SENSOR_TYPES          = 0x0221,             /*!< \brief Received in response to a CSR_MESH_SENSOR_GET_TYPES message sent containing the supported sensor types */
    CSR_MESH_SENSOR_SET_STATE      = 0x0222,             /*!< \brief when a control device wants to change the repeat_interval for the sensor */
    CSR_MESH_SENSOR_GET_STATE      = 0x0223,             /*!< \brief Received when a control device wants to know the the repeat interval value used by the sensor */
    CSR_MESH_SENSOR_STATE          = 0x0224,             /*!< \brief Received in response to a CSR_MESH_SENSOR_GET_STATE with the current repeat interval value for the sensor type */
    CSR_MESH_SENSOR_WRITE_VALUE    = 0x0225,             /*!< \brief Received when a control device wants to change the sensor values and expects an acknowledgement back */
    CSR_MESH_SENSOR_WRITE_VALUE_NO_ACK = 0x0226,         /*!< \brief Received when a control device wants to change the sensor values and does not expect an acknowledgement back */
    CSR_MESH_SENSOR_READ_VALUE     = 0x0227,             /*!< \brief Received when a control device wants to read the value of specific sensor types */
    CSR_MESH_SENSOR_VALUE          = 0x0228,             /*!< \brief Received in response to a CSR_MESH_SENSOR_READ_VALUE or CSR_MESH_SENSOR_WRITE_VALUE containg the supported sensor values */
    CSR_MESH_SENSOR_MISSING        = 0x0229,             /*!< \brief Received when a device is expecting a value to be written and no value has been written within a reasonable period of time */

    /* Actuator Model Events. */
    CSR_MESH_ACTUATOR_GET_TYPES         = 0x0230,        /*!< \brief Received when a control device wants to know the types the actuator supports */
    CSR_MESH_ACTUATOR_TYPES             = 0x0231,        /*!< \brief Received in response to a CSR_MESH_ACTUATOR_GET_TYPES message sent */
    CSR_MESH_ACTUATOR_SET_VALUE_NO_ACK  = 0x0232,        /*!< \brief Received when a control device wants to change the value for a sensor type in the actuator without expecting an acknowledgement */
    CSR_MESH_ACTUATOR_SET_VALUE         = 0x0233,        /*!< \brief Received when a control device wants to change the value for a sensor type in the actuator and expects an acknowledgement back*/
    CSR_MESH_ACTUATOR_VALUE_ACK         = 0x0234,        /*!< \brief Received in response to a CSR_MESH_ACTUATOR_SET_VALUE message as an acknowledgement */

    CSR_MESH_RAW_MESSAGE = 0xFFFF                        /*!< \brief Received when a CSRmesh message is received if the application has enabled
                                                          *   notification of raw messages
                                                          */
}csr_mesh_event_t;

/*============================================================================*
Public Function Implementations
*============================================================================*/

/*----------------------------------------------------------------------------*
 * CsrMeshInit
 */
/*! \brief Initialise the CSRmesh library.
 *
 *  This function initialises the CSRmesh library with the given node data.
 *
 *  \param  p_node Pointer to the structure containing the node data.
 *
 *  \returns TRUE if the request was successful.
 */
/*----------------------------------------------------------------------------*/
extern bool CsrMeshInit(CSR_MESH_NODE_DATA_T *p_node);

/*----------------------------------------------------------------------------*
 * CsrMeshGetDeviceUUID
 */
/*! \brief Returns the CSRmesh library 128 bit UUID.
 *
 *  \param[in] uuid pointer to a CSR_MESH_UUID_T type
 *  \param[out] uuid pointer to the CSR_MESH_UUID_T of the device
 *
 *  \returns TRUE if request was successful.
 */
/*----------------------------------------------------------------------------*/
extern bool CsrMeshGetDeviceUUID(CSR_MESH_UUID_T *uuid);

/*----------------------------------------------------------------------------*
 * CsrMeshSetDeviceUUID
 */
/*! \brief Sets the 128-bit CSRmesh UUID of the device
 *
 *  This function sets 128-bit UUID of the device. The CSRmesh library uses
 *  this value to advertise itself for Mesh association.
 *
 *  \param uuid pointer to a CSR_MESH_UUID_T type
 *
 *  \returns Nothing
 */
/*----------------------------------------------------------------------------*/
extern void CsrMeshSetDeviceUUID(CSR_MESH_UUID_T *uuid);

/*----------------------------------------------------------------------------*
 * CsrMeshSetAuthorisationCode
 */
/*! \brief Sets the 64-bit Authorisation Code for CSRmesh network association.
 *
 *  This function sets the 64-bit authorisation code to be used during the
 *  CSRmesh  network association procedure.
 *
 *  \param code Pointer to a CSR_MESH_AUTH_CODE_T
 *
 *  \returns TRUE if the request was successful.
 */
/*----------------------------------------------------------------------------*/
extern bool CsrMeshSetAuthorisationCode(CSR_MESH_AUTH_CODE_T *code);

/*----------------------------------------------------------------------------*
 * CsrMeshGetDeviceID
 */
/*! \brief Gets the 16-bit Device Identifier of the CSRmesh device
 *
 *  Gets the 16-bit Device Identifier of the CSRmesh device.
 *
 *  \returns 16-bit Device Identifier. 0 if the device is not associated.
 */
/*----------------------------------------------------------------------------*/
extern uint16 CsrMeshGetDeviceID(void);

/*----------------------------------------------------------------------------*
 * CsrMeshReset
 */
/*! \brief Reset the CSRmesh library
 *
 *  Resets the CSRmesh library and clears all the network association data.\n
 *  If the device was associated, the association with the network will be lost
 *  and it cannot communicate with the network unless it is associated again
 *
 *  \returns TRUE if the request was successful.
 */
/*----------------------------------------------------------------------------*/
extern bool CsrMeshReset(void);

/*----------------------------------------------------------------------------*
 * CsrMeshStart
 */
/*! \brief Start the CSRmesh system
 *
 *  Starts processing of received CSRmesh messages.
 *
 *  \returns TRUE if the request was successful.
 */
/*----------------------------------------------------------------------------*/
extern bool CsrMeshStart(void);

/*----------------------------------------------------------------------------*
 * CsrMeshStop
 */
/*! \brief Stop the CSRmesh system
 *
 *  Stops the CSRmesh activity.
 *
 *  \param force_stop When this flag is set all the CSRmesh activities
 *  in progress would be stopped and it will return TRUE always.\n
 *  When this flag is cleared, CSRmesh activities will be stopped only
 *  when it is listening. Return status will indicate if it is stopped or not.
 *
 *  \returns TRUE if the request was successful.
 */
/*----------------------------------------------------------------------------*/
extern bool CsrMeshStop(bool force_stop);

/*----------------------------------------------------------------------------*
 * CsrMeshSendMessage
 */
/*! \brief Sends a CSRmesh message
 *
 *  This function transmits the given message over the Mesh
 *
 *  \param message Pointer to the message to be sent. Refer \ref XAP_8bit_handler_page
 *  \param message_length Length of the message to be sent
 *
 *  \returns TRUE if the request was successful
 */
/*----------------------------------------------------------------------------*/
extern bool CsrMeshSendMessage(uint8  *message, uint16  message_length);

/*----------------------------------------------------------------------------*
 * CsrMeshProcessMessage
 */
/*! \brief Called to process a CSRmesh Message.
 *
 *   This function extracts the CSRmesh messages from the advertisement reports
 *   The application calls this function when it receives a
 *   LM_EV_ADVERTISING_REPORT LM event. This function processes the
 *   message if the report contains a Mesh message belonging to a known network
 *   otherwise returns FALSE. The application can process the report if it
 *   is not a CSRmesh report.
 *
 *  \param report Advertisement report from the LM_EV_ADVERTISING_REPORT event
 *  received while scanning
 *
 *  \returns Returns the type of advert as listed in CSR_MESH_ADVERT_T.
 *  It can be a CSRmesh message or CSRmesh bridge connectible advert or
 *  other non-CSRmesh advert.
 */
 /*---------------------------------------------------------------------------*/
extern CSR_MESH_ADVERT_T CsrMeshProcessMessage(LM_EV_ADVERTISING_REPORT_T* report);

/*----------------------------------------------------------------------------*
 * CsrMeshProcessRawMessage
 */
/*! \brief Called to process a Raw CSRmesh Message.
 *
 *   This function processes the given message as complete CSRmesh packet.
 *   The application must call this function to process a raw CSRmesh message
 *   received over a GATT connection, in the bridge role.
 *
 *  \param msg Pointer to message buffer.Refer \ref XAP_8bit_handler_page
 *  \param len length of message in bytes. Refer \ref XAP_8bit_handler_page
 *
 *  \returns TRUE if the received packet is a CSRmesh message and
 *   belongs to a known network. FALSE otherwise
 */
 /*---------------------------------------------------------------------------*/
extern bool CsrMeshProcessRawMessage(uint8 *msg, uint8 len);

/*----------------------------------------------------------------------------*
 * CsrMeshRelayEnable
 */
/*! \brief Enable or disable relaying of CSRmesh messages.
 *
 *  This function enables/disables relaying of CSRmesh Messages on supported 
 *  bearers. The bearer mask bit needs to be set to 1 for enabling the relay
 *  on that particular bearer. \n
 *  If the CSRmesh relay is enabled, the device retransmits the received
 *  messages after decrementing the ttl(time to live) field of message.\n
 *  This can enabled when messages need to be propagated in the CSRmesh
 *  where devices are located beyond the radio range of individual devices.
 *
 *  \param bearer_mask The bitmap bearer mask for enabling/disabling the relay
 *                     on a particular bearer.
 *
 *  \returns Nothing
 */
/*-----------------------------------------------------------------------------*/
extern void CsrMeshRelayEnable(uint16 bearer_mask);

/*----------------------------------------------------------------------------*
 * CsrMeshIsRelayEnabled
 */
/*! \brief Called to receive the relay enabled bearers on the device.
 *
 *  Returns the bearer mask of relay enabled bearers on the device
 *
 *  \returns mask of the current relay enabled bearers.
 */
/*-----------------------------------------------------------------------------*/
extern uint16 CsrMeshIsRelayEnabled(void);

/*----------------------------------------------------------------------------*
 * CsrMeshEnablePromiscuousMode
 */
/*! \brief Enable or disable Promiscuous setting on supported bearers of the node.
 *
 *  This function enables/disables Promiscuous setting on supported bearers. The 
 *  bearer promiscuous mask bit needs to be set to 1 for enabling the promiscuous  
 *  mode on that particular bearer. \n
 *  When promiscuous mode is disabled only messages authenticated against a 
 *  known network key are relayed. Otherwise all messages are relayed 
 *  regardless of authentication.
 *
 *  \param bearer_promiscuous_mask The bitmap bearer mask for enabling/disabling
 *                                 the promiscuous mode on a particular bearer.
 *  \returns Nothing
 */
/*-----------------------------------------------------------------------------*/
extern void CsrMeshEnablePromiscuousMode(uint16 bearer_promiscuous_mask);

/*----------------------------------------------------------------------------*
 * CsrMeshIsPromiscuousModeEnabled
 */
/*! \brief Called to receive the promiscuous enabled bearers on the device.
 *
 *  Returns the berarer_mask of Promiscuous enabled bearers on the device.
 *
 *  \returns mask of the current promiscuous setting.
 */
/*-----------------------------------------------------------------------------*/
extern uint16 CsrMeshIsPromiscuousModeEnabled(void);

/*----------------------------------------------------------------------------*
 * CsrMeshEnableRawMsgEvent
 */
/*! \brief Enables or disables the notification of CSR_MESH_RAW_MESSAGE event.
 *
 *   Enables/disables the notification of \ref CSR_MESH_RAW_MESSAGE
 *   event. This event is sent when a CSRmesh message not belonging to any
 *   enabled models is received. This can be used by the application for its own
 *   processing.
 *
 *   Typically Raw message event is enabled on a GATT bridge which supports
 *   Mesh Control service and notifies the received CSRmesh messages to the
 *   connected CSRmesh Control application.
 *
 *  \param enable TRUE enables the notification of the event
 *
 *  \returns Nothing
 */
/*----------------------------------------------------------------------------*/
extern void CsrMeshEnableRawMsgEvent(bool enable);

/*----------------------------------------------------------------------------*
 * CsrMeshAssociateToANetwork
 */
/*! \brief Advertises a CSRmesh device identification message.
 *
 *   Advertises a CSRmesh device identification message to enable the
 *   device to be associated to a CSRmesh Network.
 *   The application ready to be associated to a network must call this
 *   function periodically to advertise itself till it receives a
 *   \ref CSR_MESH_ASSOCIATION_REQUEST event.
 *
 *  \returns TRUE if the request was successful
 */
/*----------------------------------------------------------------------------*/
extern bool CsrMeshAssociateToANetwork(void);

/*----------------------------------------------------------------------------*
 * CsrMeshAdvertiseDeviceAppearance
 */
/*! \brief Advertises CSRmesh device appearance.
 *
 *   Advertises the CSRmesh device appearance value to enable a control device
 *   to identify the type of device even before associating the device.
 *   The application may call this function periodically before association to 
 *   allow a control device to identify the type of this device.
 *
 *  \param appearance Pointer to the CSRmesh device appearance value
 *  \param short_name Short name of this device. Maximum 9 characters.
 *                    Refer \ref XAP_8bit_handler_page
 *  \param name_len   Length of the short name. Refer \ref XAP_8bit_handler_page
 *
 *  \returns TRUE if the request was successful
 */
/*----------------------------------------------------------------------------*/
extern bool CsrMeshAdvertiseDeviceAppearance(CSR_MESH_APPEARANCE_T *appearance,
                                             uint8 *short_name, uint8 name_len);

/*----------------------------------------------------------------------------*
 * AppProcessCsrMeshEvent
 */
/*! \brief Application function called to notify a CSRmesh event.
 *
 *  This application function is called by the CSRmesh library to notify a
 *  CSRmesh message.\n
 *
 *  \param event_code CSRmesh Event code
 *  \param data Data associated with the event.Refer \ref XAP_8bit_handler_page
 *  \param length Length of the associated data in bytes
 *  \param state_data Pointer to the variable pointing to state data.
 *
 *  \returns Nothing
 */
/*----------------------------------------------------------------------------*/
extern void AppProcessCsrMeshEvent(csr_mesh_event_t event_code, uint8* data,
                                   uint16 length, void **state_data);

/*----------------------------------------------------------------------------*
 * CsrMeshHandleDataInConnection
 */
/*! \brief Synchronizes the CSRmesh activity with the connection events.
 *
 *   Allows the CSRmesh to synchronise the Mesh advertisements with the
 *   connection events when the application is connected to a central device.\n\n
 *   The application must call this function when \n
 *   1. A GATT connection is established with the device(indicated by the
 *      LM_EV_CONNECTION_COMPLETE event).\n
 *         2. Connection parameters of a connection are updated (indicated by
 *      the LS_CONNECTION_PARAM_UPDATE_IND event).\n
 *         3. An existing GATT connection is dropped (indicated by the
 *      LM_EV_DISCONNECT_COMPLETE event).\n
 *
 *  \param ucid          The connection identifier passed by the firmware to the
 *               application during connection establishment.\n
 *                       If the function is being called because of GATT
 *                       disconnection, this parameter should be
 *                       GATT_INVALID_UCID(0xFFFF).
 *  \param conn_interval The connection interval being used for the connection.
 *
 *  \returns Nothing
 */
/*----------------------------------------------------------------------------*/
extern void CsrMeshHandleDataInConnection(uint16 ucid, uint16 conn_interval);

/*----------------------------------------------------------------------------*
 * CsrMeshHandleRadioEvent
 */
/*! \brief Adjusts the CSRmesh advertisement timings based on the connection
 *  radio events.
 *
 *  This function allows the CSRmesh to adjust the CSRmesh advertisements and
 *  scan timing with the connection radio events.\n
 *  The application must call this function when a LS_RADIO_EVENT_IND is received.
 *
 *  \returns Nothing
 */
/*----------------------------------------------------------------------------*/
extern void CsrMeshHandleRadioEvent(void);

/*----------------------------------------------------------------------------*
 * CsrMeshGetAdvScanParam
 */
/*! \brief Read the current CSRmesh Advertise and Scan timing Parameters.
 *
 *  This function reads advertising and scanning parameters from CSRmesh
 *  library.
 *
 *  \param   param Pointer to a structure of Advertising and Scan timing
 *           parameters to be read into.The param contains the relay repeat count
 *           which specifies the number of times the relay messages are advertised.
 *
 *  \returns Nothing
 */
/*----------------------------------------------------------------------------*/
extern void CsrMeshGetAdvScanParam(CSR_MESH_ADVSCAN_PARAM_T *param);

/*----------------------------------------------------------------------------*
 * CsrMeshSetAdvScanParam
 */
/*! \brief Set the CSRmesh Advertise and Scan Timing parameters
 *
 *  Application can set advertising and scanning parameters to CSRmesh
 *  library using this function. The scan duty cycle parameter can be set between
 *  0 to 100 percent. The current implementation supports high resolution scan
 *  duty cycle setting. Application can also set the advertisement time and 
 *  advertisement interval through which the repeat count of the self 
 *  initiated message is calculated. It can also set the relay repeat count
 *  which specifies the number of times each relayed message is retransmitted.
 *
 *  \param   param Pointer to a structure containing Advertising and Scan
 *           timing parameters to be set.
 *
 *  \returns Nothing
 */
/*----------------------------------------------------------------------------*/
extern void CsrMeshSetAdvScanParam(CSR_MESH_ADVSCAN_PARAM_T *param);

/*----------------------------------------------------------------------------*
 * CsrMeshGetScanParameters
 */
/*! \brief Reads the current Scan Parameters.
 *
 *  This function reads the scanning parameters from CSRmesh library. This 
 *  function returns the last set scan window and scan interval parameters.
 *
 *  \param   param Pointer to a structure of Scanning parameters to be read into
 *           The param contains the scan window and the scan interval currently
 *           used in the mesh library.
 *
 *  \returns Nothing
 */
/*----------------------------------------------------------------------------*/
extern void CsrMeshGetScanParameters(CSR_MESH_SCAN_PARAM_T *param);

/*----------------------------------------------------------------------------*
 * CsrMeshSetScanParameters
 */
/*! \brief Sets the new Scan Parameters.
 *
 *  This function sets the scanning parameters in the CSRmesh library. This
 *  function could be called to set the scan interval and scan window
 *  parameters in the csrMesh library.
 *
 *  \param   param Pointer to a structure of Scanning parameters to be set into
 *           The param contains the scan window and the scan interval which
 *           should be set in the mesh library.
 *
 *  \returns Nothing
 */
/*----------------------------------------------------------------------------*/
extern void CsrMeshSetScanParameters(CSR_MESH_SCAN_PARAM_T *param);

/*----------------------------------------------------------------------------*
 * CsrMeshStoreUserAdvData
 */
/*! \brief Stores the advertising data for user adverts.
 *
 *  This function is called by the application to add either advertising or scan 
 *  response data. Each call to the function will add a single AD Structure 
 *  (refer to Bluetooth specification Vol.3 Part C Section 11). Repeated calls 
 *  will append new structures, to build up the data content.
 *  The application should not include the "length" parameter within the supplied 
 *  octet array - the GAP layer will add the length field in the appropriate 
 *  position. The first octet of the array should be the AD Type field
 *
 *  \param len  The number of bytes of data supplied.
 *  \param data A byte array of the AD Structure to add. The first byte will be 
 *              the AD Type field (see ad_type). It is the caller's responsibility 
 *              to ensure that the remainder of the data is correctly formatted 
 *              and consistent with the specified AD Type. 
 *              Refer \ref XAP_8bit_handler_page
 *  \param src  A flag indicating whether the data supplied is advertising data 
 *              (ad_src_advertise) or scan response data (ad_src_scan_rsp).
 *
 *  \returns boolean - TRUE if the user advertising data was stored correctly.
 */
/*----------------------------------------------------------------------------*/
extern bool CsrMeshStoreUserAdvData(uint16 len, uint8 *data, ad_src const src);

/*----------------------------------------------------------------------------*
 * CsrMeshSendUserAdvert
 */
/*! \brief Set the current CSRmesh Scan Parameters.
 *
 *  This function sets the scanning parameters in the CSRmesh library.
 *
 *  \param   ad_params Pointer to a structure of advertising parameters to be set
 *
 *  \returns boolean - TRUE if the user advertising event was scheduled.
 */
/*----------------------------------------------------------------------------*/
extern bool CsrMeshSendUserAdvert(CSR_MESH_USER_ADV_PARAMS_T *ad_params);

/*----------------------------------------------------------------------------*
 * CsrMeshGetMessageTTL
 */
/*!  \brief Obtains the TTL value of the CSRmesh messages.
 *
 *   Returns the Time To Live value for Control or Association messages.
 *   Refer \ref XAP_8bit_handler_page
 *
 *   \param msg_type Type of CSRmesh (Association or Control) message.
 *
 *   \returns TTL value of CSRmesh messages.
 */
/*----------------------------------------------------------------------------*/
extern uint8 CsrMeshGetMessageTTL(CSR_MESH_MESSAGE_T msg_type);

/*----------------------------------------------------------------------------*
 * CsrMeshSetMessageTTL
 */
/*!  \brief Sets the TTL value of the CSRmesh messages to be sent.
 *
 *   Modifies the Time To Live value for Control or Association messages.
 *
 *   \param msg_type Type of CSRmesh (Association or Control) message.
 *
 *   \param ttl TTL value to be used for CSRmesh messages.
 *              Refer \ref XAP_8bit_handler_page
 *
 *   \returns Nothing.
 */
/*----------------------------------------------------------------------------*/
extern void CsrMeshSetMessageTTL(CSR_MESH_MESSAGE_T msg_type, uint8 ttl);

/*----------------------------------------------------------------------------*
 * CsrMeshGetTxPower
 */
/*!  \brief Obtain the Transmit Power level in dBm.
 *
 *   Returns the Transmit Power of CSRmesh messages in dBm.
 *   Refer \ref XAP_8bit_handler_page
 *
 *   \returns Power level in dBm
 */
/*----------------------------------------------------------------------------*/
extern int8 CsrMeshGetTxPower(void);

/*----------------------------------------------------------------------------*
 * CsrMeshSetTxPower
 */
/*!  \brief Sets the Transmit Power to the nearest value possible.
 *
 *   Modifies the Transmit Power of CSRmesh messages. CSR device supports
 *   a specific set of transmit power levels from -18dBm  to +8dBm.\n
 *   This function sets the transmit power to supported value nearest to the
 *   requested value and returns the set transmit power.\n
 *
 *   \param power Transmit power level in dBm. Refer \ref XAP_8bit_handler_page
 *
 *   \returns Set power level in dBm Refer \ref XAP_8bit_handler_page
 */
/*----------------------------------------------------------------------------*/
extern int8 CsrMeshSetTxPower(int8 power);

/*----------------------------------------------------------------------------*
 * CsrMeshResetDiagnosticData
 */
/*! \brief Resets the CSRmesh Diagnostic Data.
 *
 *   Resets the diagnostic statistic counters.
 *
 *  \returns Returns Nothing.
 */
/*----------------------------------------------------------------------------*/
extern void CsrMeshResetDiagnosticData(void);

/*----------------------------------------------------------------------------*
 * CsrMeshGetDiagnosticData
 */
/*! \brief Returns Diagnostic Data in the pointer to the
 *         Diagnostic Data structure.
 *
 *   This function reads the Diagnostic Statistics Counter values.
 *
 *  \param pDiagData Pointer to a variable of type \ref CSR_MESH_DIAGNOSTIC_DATA_T
 *                   to receive the diagnostic data
 *
 *  \returns Returns Nothing.
 */
/*----------------------------------------------------------------------------*/
extern void CsrMeshGetDiagnosticData(CSR_MESH_DIAGNOSTIC_DATA_T *pDiagData);

/*----------------------------------------------------------------------------*
 * CsrMeshIsListeningEnabled
 */
/*! \brief Check if listening to CSRmesh is enabled on device.
 *
 *   Returns TRUE if scanning is enabled on CSRmesh device.
 *
 *  \returns Returns the status of listening to CSRmesh on device.
 */
/*----------------------------------------------------------------------------*/
extern bool CsrMeshIsListeningEnabled(void);

/*----------------------------------------------------------------------------*
 * CsrMeshEnableListening
 */
/*! \brief Enable/Disable listening to CSRmesh.
 *
 *   Controls scanning on CSRmesh device.
 *
 *  \param enable Set TRUE to enable listening and FALSE to disable.
 *
 *  \returns Returns Nothing.
 */
/*----------------------------------------------------------------------------*/
extern void CsrMeshEnableListening(bool enable);

/*----------------------------------------------------------------------------*
 * CsrMeshUpdateLastETag
 */
/*! \brief Updates LastETag of CSRmesh device.
 *
 *   Updates the LastETag value of the CSRmesh device. This function needs
 *   to be called whenever a model or device configuration information is
 *   updated in application.
 *
 *  \param   new_ETag Pointer to the buffer to receive a new Etag
 *
 *  \returns Nothing
 */
/*----------------------------------------------------------------------------*/
extern void CsrMeshUpdateLastETag(CSR_MESH_ETAG_T *new_ETag);

/*----------------------------------------------------------------------------*
 * CsrMeshIsTxQueueEmpty
 */
/*! \brief Checks if the CSRmesh transmit queue is empty.
 *
 *   This function returns TRUE if transmit queue is empty. Returns FALSE
 *   otherwise and enables the \ref CSR_MESH_TX_QUEUE_EVENT to be 
 *   notified when the queue becomes empty.
 *
 *  \note The relay messages and the response messages are also queued on the
 *   tx queue. So the event when the transmit queue becomes empty may be 
 *   delayed by these messages.
 *
 *  \returns TRUE if transmit queue is empty.
 */
/*----------------------------------------------------------------------------*/
extern bool CsrMeshIsTxQueueEmpty(void);

/*!@} */
#endif /* __CSR_MESH_H__ */

