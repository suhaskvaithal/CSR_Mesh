/******************************************************************************
 *  Copyright Cambridge Silicon Radio Limited 2015
 *  CSR Bluetooth Low Energy CSRmesh 1.3 Release
 *****************************************************************************/
 /*! \file actuator_model.h
 *  \brief Defines functions implemented in the CSRmesh Actuator Model 
 *
 *   This file defines the functions implemented in CSRmesh Actuator Model
 *   
 *   Copyright (c) CSR plc 2015
 */
    
#ifndef __ACTUATOR_MODEL_H__
#define __ACTUATOR_MODEL_H__

/*! \addtogroup Actuator_Model
 * @{
 */

#include "sensor_types.h"

/*============================================================================*
Public Definitions
*============================================================================*/

/*! \brief CSRmesh Actuator state */
typedef struct
{
    sensor_type_t type;       /*!< \brief Sensor type. */
    uint16        value_size; /*!< \brief Size in octets of sensor value. */
    uint16        *value;     /*!< \brief Pointer to the value of the sensor. */
} ACTUATOR_STATE_DATA_T;

/*! \brief CSRmesh Actuator Model state */
typedef struct
{
    ACTUATOR_STATE_DATA_T* sensor_list;  /*!< \brief Pointer to sensor list. */
    uint16                 num_types;    /*!< \brief Number of Sensor Types. */
} ACTUATOR_MODEL_STATE_DATA_T;

/*============================================================================*
  Public Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 * ActuatorModelInit
 */
/*! \brief Initialise the Actuator Model
 *
 *  This function enables the Actuator model functionality on the device.\n
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
extern void ActuatorModelInit(uint16 *group_id_list, uint16 num_groups);

/*----------------------------------------------------------------------------*
 * ActuatorGetTypes
 */
/*! \brief Get the types of Actuators supported by a device.
 *
 *   Sends a Actuator get types command to the destination device.\n
 *   Response to the ping is reported to application in AppProcessCsrMeshEvent
 *   with the event CSR_MESH_ACTUATOR_TYPES.\n
 *   The device needs to be associated to the network before this function
 *   is called.
 *
 *  \param dev_id Device identifier of the target device.
 *
 *  \param first_type Actuator type.
 *
 *  \returns TRUE if the request was successful
 */
/*----------------------------------------------------------------------------*/
extern bool ActuatorGetTypes(uint16 dev_id,  sensor_type_t first_type);

/*----------------------------------------------------------------------------*
 * ActuatorSetValue
 */
/*! \brief Sets the value for a sensor type on the actuator device.
 *
 *   Sends a Actuator set value command to the destination device.\n
 *   Response to this ping is reported to application in AppProcessCsrMeshEvent
 *   with the event CSR_MESH_ACTUATOR_VALUE_ACK if the ack_req flag is set.\n
 *   The device needs to be associated to the network before this function
 *   is called.
 *
 *  \param dev_id Device identifier of the target device.
 *
 *  \param data Pointer to the data to be transmitted. Refer \ref XAP_8bit_handler_page
 *
 *  \param data_len Length of data in bytes.
 *
 *  \param ack_req Distinguishes the message to receive an acknowledgement or not.
 *
 *  \param type  Actuator type.
 *
 *  \returns TRUE if the sending of set value was successful
 */
/*----------------------------------------------------------------------------*/
extern bool ActuatorSetValue(uint16 dev_id,
                             uint8 *data, 
                             uint16 data_len,
                             bool ack_req,
                             sensor_type_t type);

/*!@} */
#endif /* __ACTUATOR_MODEL_H__ */
