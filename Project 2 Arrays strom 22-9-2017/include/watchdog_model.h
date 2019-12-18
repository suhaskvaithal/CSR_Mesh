/******************************************************************************
 *  Copyright Cambridge Silicon Radio Limited 2015
 *  CSR Bluetooth Low Energy CSRmesh 1.3 Release
 *****************************************************************************/
/*! \file watchdog_model.h
 *  \brief Defines the functions implemented in CSRmesh Watchdog Model
 *
 *   Copyright (c) CSR plc 2014
 */

#ifndef __WATCHDOG_MODEL_H__
#define __WATCHDOG_MODEL_H__

/*! \addtogroup Watchdog_Model
 * @{
 */

/*============================================================================*
  Public Definitions
 *============================================================================*/
/*! \brief CSRmesh Watchdog Model state */
typedef struct
{
    uint16 interval;     /*!< \brief Interval(in seconds) at which watchdog messages are sent. */
    uint16 active_after; /*!< \brief Duration(in seconds) for which device listens after sending message. */
} WATCHDOG_MODEL_STATE_DATA_T;

/*============================================================================*
  Public Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 * WatchdogModelInit
 */
/*! \brief Initialise the Watchdog Model of the CSRmesh library
 *
 *  This function enables the Watchdog model functionality on the device.\n
 *  The events from the model are reported to application in
 *  AppProcessCsrMeshEvent.\n
 *  Registers the model handler with the CSRmesh. Sets the CSRmesh to report
 *  num_groups as the maximum number of groups supported for the model\n
 *
 *  \param group_id_list Pointer to a uint16 array to hold assigned group_ids. 
 *                       This must be NULL if no groups are supported
 *  \param num_groups Size of the group_id_list. This must be 0 if no groups
 *                    are supported.
 *  
 *  \returns Nothing
 */
/*----------------------------------------------------------------------------*/
extern void WatchdogModelInit(uint16 *group_id_list, uint16 num_groups);

/*----------------------------------------------------------------------------*
 * WatchdogSendRandomMessage
 */
/*! \brief Sends a random message to a device.
 *
 *  This function sends a random message to a specified destination device id.
 *  The device needs to be associated to the network before this function
 *  is called.\n
 *
 *  \param destDevID Destination device id.
 *
 *  \param msg_len length of the message to be sent including response length.
 *                 (1 to 10 bytes)
 *
 *  \param rsp_len length of the expected response message (0 to 10 bytes).
 *                 If no response is needed, set this to zero.
 *
 *  \returns TRUE if the request was successful.
 */
/*----------------------------------------------------------------------------*/
extern bool WatchdogSendRandomMessage(uint16 destDevID, uint16 msg_len,
                                                        uint16 rsp_len);
/*!@} */
#endif /* __WATCHDOG_MODEL_H__ */

