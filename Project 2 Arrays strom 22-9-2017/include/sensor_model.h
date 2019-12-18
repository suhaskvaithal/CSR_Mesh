/******************************************************************************
 *  Copyright Cambridge Silicon Radio Limited 2015
 *  CSR Bluetooth Low Energy CSRmesh 1.3 Release
 *****************************************************************************/
/*! \file sensor_model.h
 *  \brief Defines functions implemented in the CSRmesh Sensor Model 
 *
 *   This file defines the functions implemented in CSRmesh Sensor Model
 *   
 *   Copyright (c) CSR plc 2015
 */
    
#ifndef __SENSOR_MODEL_H__
#define __SENSOR_MODEL_H__

/*! \addtogroup Sensor_Model
 * @{
 */

#include "sensor_types.h"

/*============================================================================*
Public Definitions
*============================================================================*/

#define SENSOR_MODEL_DATA_SIZE_MAX 10 /*!< \brief Maximum size of sensor model
                                       * data that could be received
                                       */

/*! \brief CSRmesh Sensor state */
typedef struct
{
    sensor_type_t type;           /*!< \brief Sensor type. */
    uint16        value_size;     /*!< \brief Size in octets of sensor value. */
    uint16        *value;         /*!< \brief Pointer to the value of sensor. */
    uint8         repeat_interval;/*!< \brief Interval(seconds) between repeated 
                                   *    transmissions of sensor values.
                                   *    Refer \ref XAP_8bit_handler_page
                                   */
} SENSOR_STATE_DATA_T;

/*! \brief CSRmesh Sensor Model state */
typedef struct
{
    SENSOR_STATE_DATA_T* sensor_list;  /*!< \brief Pointer to sensor list. */
    uint16               num_types;    /*!< \brief Number of Sensor Types. */
} SENSOR_MODEL_STATE_DATA_T;

typedef struct
{
    uint16 source_id; /*!< \brief Device ID of the source device */
    uint16 dest_id;   /*!< \brief Device ID of the destination device */
    uint8 tid;        /*!< \brief Transaction ID received  \
                       *          Refer \ref XAP_8bit_handler_page
                       */
} SENSOR_MODEL_INFO_T;

/*! \brief Data stream event type */
typedef struct
{
    SENSOR_MODEL_INFO_T sensor_model_info;
                    /*!< \brief sensor model data in all sensor model events */
    uint16 msg_len; /*!< \brief Length of the msg present in the msg pointer.*/
    uint8 msg[SENSOR_MODEL_DATA_SIZE_MAX];
                    /*!< \brief Holds the msg of length specified in msg_len.
                     *       .  Refer \ref XAP_8bit_handler_page
                     */
}SENSOR_MODEL_EVENT_T;

/*============================================================================*
  Public Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 * SensorModelInit
 */
/*! \brief Initialise the Sensor Model
 *
 *  This function enables the Sensor model functionality on the device.\n
 *  The events from the model are reported to application in
 *  AppProcessCsrMeshEvent.\n
 *
 *  \param group_id_list Pointer to a uint16 array to hold assigned group_ids. 
 *                       This must be NULL if no groups are supported
 *  \param num_groups Size of the group_id_list. This must be 0 if no groups
 *                    are supported.
 *
 *  \returns Nothing
 */
/*----------------------------------------------------------------------------*/
extern void SensorModelInit(uint16 *group_id_list, uint16 num_groups);

/*----------------------------------------------------------------------------*
 * SensorGetTypes
 */
/*! \brief Get the types of sensors supported by a device.
 *
 *   Sends a sensor get types command to the destination device.\n
 *   Response to the ping is reported to application in AppProcessCsrMeshEvent
 *   with the event CSR_MESH_SENSOR_TYPES.\n
 *   The device needs to be associated to the network before this function
 *   is called.
 *
 *  \param dev_id Device identifier of the target device.
 *
 *  \param first_type Sensor type.
 *
 *  \returns TRUE if the request was successful
 */
/*----------------------------------------------------------------------------*/
extern bool SensorGetTypes(uint16 dev_id,  sensor_type_t first_type);

/*----------------------------------------------------------------------------*
 * SensorSetState
 */
/*! \brief Set the state of a sensor type supported by a device.
 *
 *   Sends a sensor set state command to the destination device.\n
 *   Response to the ping is reported to application in AppProcessCsrMeshEvent
 *   with the event CSR_MESH_SENSOR_STATE.\n
 *   The device needs to be associated to the network before this function
 *   is called.
 *
 *  \param dev_id Device identifier of the target device.
 *
 *  \param type Sensor type.
 *
 *  \param repeat_interval Repeat interval at which the sensor values to be
 *                         retransmitted. Refer \ref XAP_8bit_handler_page
 *
 *  \returns TRUE if the request was successful
 */
/*----------------------------------------------------------------------------*/
extern bool SensorSetState(uint16 dev_id, 
                           sensor_type_t type,
                           uint8 repeat_interval);


/*----------------------------------------------------------------------------*
 * SensorGetState
 */
/*! \brief Get the state of a sensor type supported by a device.
 *
 *   Sends a sensor get state command to the destination device.\n
 *   Response to the ping is reported to application in AppProcessCsrMeshEvent
 *   with the event CSR_MESH_SENSOR_STATE.\n
 *   The device needs to be associated to the network before this function
 *   is called.
 *
 *  \param dev_id Device identifier of the target device.
 *
 *  \param type Sensor type.
 *
 *  \returns TRUE if the request was successful
 */
/*----------------------------------------------------------------------------*/
extern bool SensorGetState(uint16 dev_id, sensor_type_t type);

/*----------------------------------------------------------------------------*
 * SensorReadValue
 */
/*! \brief Read the value of a sensor type supported by a device.
 *
 *   Sends a sensor read value command to the destination device.\n
 *   Response to the ping is reported to application in AppProcessCsrMeshEvent
 *   with the event CSR_MESH_SENSOR_VALUE.\n
 *   The device needs to be associated to the network before this function
 *   is called.
 *
 *  \param dev_id Device identifier of the target device.
 *
 *  \param type An array of  sensor types whose values need to be read. 
 *
 *  \param type_len The length of the sensor type array. A maximum of 2 values 
 *                  could be accomodated in a single SensorReadValue command.
 *                  Refer \ref XAP_8bit_handler_page
 *
 *  \returns TRUE if the request was successful
 */
/*----------------------------------------------------------------------------*/
extern bool SensorReadValue(uint16 dev_id, 
                            sensor_type_t *type,
                            uint8 type_len);

/*----------------------------------------------------------------------------*
 * SensorWriteValue
 */
/*! \brief Writes sensor values.
 *
 *   Writes sensor values to the destination device.\n
 *   Response to the ping is reported to application in AppProcessCsrMeshEvent
 *   with the event CSR_MESH_SENSOR_VALUE.\n
 *   The device needs to be associated to the network before this function
 *   is called.
 *
 *  \param dev_id Device identifier of the target device.
 *
 *  \param ack_req Specifies whether acknowledgement is required or not.
 *
 *  \param type The array of sensor types whose values need to be written. The
 *              application should only send the known sensor types and the 
 *              sensor types should be sorted in order. The model sends maximum
 *              sensor type values that could be accomodated in single msg.
 *
 *  \param type_count The number of sensor types in the sensor type array.
 *                    Refer \ref XAP_8bit_handler_page
 *
 *  \param p_data Pointer to Sensor State data.
 *
 *  \returns TRUE if the request was successful
 */
/*----------------------------------------------------------------------------*/
extern bool SensorWriteValue(uint16 dev_id, 
                              bool ack_req,
                              sensor_type_t* type,
                              uint8 type_count,
                              SENSOR_MODEL_STATE_DATA_T *p_data);

/*----------------------------------------------------------------------------*
 * ReportSensorMissing
 */
/*! \brief Request for the missing sensor value for sensor types by a device.
 *
 *   Sends a sensor missing command to the destination device.\n
 *   Response to the ping is reported to application in AppProcessCsrMeshEvent
 *   with the event CSR_MESH_SENSOR_VALUE.\n
 *   The device needs to be associated to the network before this function
 *   is called.
 *
 *  \param dev_id Device identifier of the target device.
 *
 *  \param type An array of sensor types whose values could be missing.
 *
 *  \param type_count The number of sensor types in the sensor type array.
 *                    A maximum of 4 values could be accomodated in a single 
 *                    ReportSensorMissing cmd.Refer \ref XAP_8bit_handler_page
 *
 *  \returns TRUE if the request was successful
 */
/*----------------------------------------------------------------------------*/
extern bool ReportSensorMissing(uint16 dev_id, 
                                sensor_type_t *type,
                                uint8 type_count);

/*----------------------------------------------------------------------------*
 * SensorValue
 */
/*! \brief sensor values.
 *
 *   Sends sensor values to the destination device.\n
 *   The device needs to be associated to the network before this function
 *   is called.
 *
 *  \param dev_id Device identifier of the target device.
 *
 *  \param type The array of sensor types whose values need to be written. The
 *              application should only send the known sensor types and the 
 *              sensor types should be sorted in order. The model sends maximum
 *              sensor type values that could be accomodated in single msg.
 *
 *  \param type_count The number of sensor types in the sensor type array.
 *                    Refer \ref XAP_8bit_handler_page
 *
 *  \param tid The transaction id for the message.
 *             Refer \ref XAP_8bit_handler_page
 *
 *  \param p_data Pointer to Sensor State data.
 *
 *  \returns TRUE if the request was successful
 */
/*----------------------------------------------------------------------------*/
extern bool SensorValue(uint16 dev_id, 
                        sensor_type_t* type,
                        uint8 type_count,
                        uint8 tid,
                        SENSOR_MODEL_STATE_DATA_T *p_data);


/*!@} */
#endif /* __SENSOR_MODEL_H__ */
