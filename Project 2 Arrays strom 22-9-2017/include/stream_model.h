/******************************************************************************
 *  Copyright Cambridge Silicon Radio Limited 2015
 *  CSR Bluetooth Low Energy CSRmesh 1.3 Release
 *****************************************************************************/
/*! \file stream_model.h
 *  \brief Defines CsrMesh Stream Model specific functions of
 *   the CSRMesh library
 *
 *   This file contains the functions defined in the CSRMesh Data Stream model.\n
 *
 *   The Data stream library provides API to send and receive a data stream. It 
 *   supports one instance of a stream at a time.
 *
 *   Copyright (c) CSR plc 2013
 */

#ifndef __STREAM_MODEL_H__
#define __STREAM_MODEL_H__

/*! \addtogroup Stream_Model
 * @{
 */

/*============================================================================*
    Public Definitions
 *============================================================================*/
#define SINGLE_DATA_BLOCK_SIZE_MAX 10 /*!< \brief Maximum size of a single block 
                                       *    of data sent without acknowledgement
                                       */
#define STREAM_DATA_BLOCK_SIZE_MAX  8 /*!< \brief Maximum size of a data block 
                                       *    of a data stream with receiver
                                       *    acknowledgement
                                       */

/*============================================================================*
    Public Data types
 *============================================================================*/
/*! \brief Common data for all the CSR mesh events */
typedef struct
{
    uint16 source_id; /*!< \brief Device ID of the source */
    uint16 dest_id; /*!< \brief Device ID of the destination device */
} CSR_MESH_EVENT_COMMON_T;

/*! \brief Data stream event type */
typedef struct
{
    CSR_MESH_EVENT_COMMON_T common_data;
                      /*!< \brief Common data in all CSRmesh data model events */
    uint16 data_len;  /*!< \brief Length of the data associated with the event.
                       *          In case of \ref CSR_MESH_DATA_STREAM_SEND_CFM
                       *          event, it is the length of the data that the
                       *          receiver confirmed to have received
                       */
    uint8 data[SINGLE_DATA_BLOCK_SIZE_MAX];
                      /*!< \brief Holds the data of length specified in
                       *          \ref data_len for the 
                       *          \ref CSR_MESH_DATA_STREAM_DATA_IND and \ref
                       *          CSR_MESH_DATA_BLOCK_IND events
                       *          Refer \ref XAP_8bit_handler_page
                       */
}CSR_MESH_STREAM_EVENT_T;

/*============================================================================*
    Public Function Definitions
 *============================================================================*/

/*----------------------------------------------------------------------------*
 * StreamModelInit
 */
/*! \brief Initialise the Stream Model of the CSRMesh library
 * 
 *   Registers the model handler with the CSRmesh. Sets the CSRmesh to report
 *   num_groups as the maximum number of groups supported for the model
 *
 *  \param group_id_list Pointer to a uint16 array to hold assigned group_ids. 
 *                       This can be NULL if no groups are supported
 *  \param num_groups Size of the group_id_list. This must be 0 if groups are
 *                    not supported for the model.
 *
 *  \returns Nothing
 */
/*----------------------------------------------------------------------------*/
extern void StreamModelInit(uint16 *group_id_list, uint16 num_groups);


/*----------------------------------------------------------------------------*
 *  StreamModelStartSender
 */
/*! \brief Starts a data stream sender to stream data to a specified device. 
 *
 *   Starts a data stream sender to stream data to a specified device.
 *
 *  \param dest_id Destination ID of the CSRmesh device or a group of devices
                   which to stream the data
 *
 *  \returns Nothing
 */
/*----------------------------------------------------------------------------*/
extern void StreamStartSender(uint16 dest_id);

/*----------------------------------------------------------------------------*
 * StreamSendData
 */
/*! \brief Send the specified amount of data on the Stream
 *   
 *   This function sends the stream data over CSRmesh. It can send a maximum 
 *   of \ref STREAM_DATA_BLOCK_SIZE_MAX. The send confirmation is notified 
 *   to the application using the \ref CSR_MESH_DATA_STREAM_SEND_CFM event when
 *   the receiver acknowledges the data stream message.\n\n 
 *
 *   Multiple \ref CSR_MESH_DATA_STREAM_SEND_CFM messages may be received when 
 *   the data stream is sent to a group, since all the devices which received
 *   the data packet will acknowledge.\n\n
 * 
 *   This function sends the data and advances the stream sequence number. 
 *   It must be called only once per packet. To retry sending the last packet
 *   \ref StreamResendLastData must be called.\n\n
 *
 *   This function takes care of sending the STREAM_FLUSH message to indicate
 *   start of a stream if the stream is not alreay started. The application 
 *   need not call the \ref StreamFlush() to start a stream.
 *
 *  \param data Pointer to buffer containing data.
 *              Refer \ref XAP_8bit_handler_page.
 *  \param length Amount of data to send in bytes
 *  
 *  \returns Length of the data streamed over CSRmesh
 */
/*----------------------------------------------------------------------------*/
extern uint16 StreamSendData(uint8 *data, uint16 length);

/*----------------------------------------------------------------------------*
 *  StreamResendLastData
 */
/*! \brief Resends last sent data stream packet over the mesh
 * 
 *   This function resends last sent data stream packet over the mesh. This 
 *   function must be called by the application to retry sending a packet if
 *   the application did not receive an acknowledgement.
 *
 *   \returns Nothing
 */
/*----------------------------------------------------------------------------*/
extern void StreamResendLastData(void);

/*----------------------------------------------------------------------------*
 * StreamFlush
 */
/*! \brief Sends a stream flush message to the receiver
 *
 *   This function sends a stream flush message to the receiver to indicate
 *   completion of the data stream transfer or to start a new stream.
 *   Application must call this function when all the data to be streamed has 
 *   been transferred successfully
 *
 *  \returns Nothing
 */
/*----------------------------------------------------------------------------*/
extern void StreamFlush(void);

/*----------------------------------------------------------------------------*
 * StreamReset
 */
/*! \brief Resets a stream being received
 *   
 *   This function resets a stream being received.\n
 *   The application must call this function to reset a stream in progress
 *   in case of a timeout. This will reset the receive status of the stream
 *   model so that it is ready to receive a new stream.
 *
 *  \returns Nothing
 */
/*----------------------------------------------------------------------------*/
extern void StreamReset(void);


/*----------------------------------------------------------------------------*
 * StreamSendDataBlock
 */
/*! \brief Sends a single block of data to the specified device
 *
 *   Sends a single block of data to the speicified device
 *   There will be no acknowledgement from the receiver for receiving the data 
 *   block.
 * 
 *  \param dest_dev_id Device ID of the CSRmesh device to which data block needs
 *                     to be sent
 *  \param data Pointer to a buffer containing the data.
 *              Refer \ref XAP_8bit_handler_page
 *  \param length Length of the data to be sent. Maximum value can be 
 *                \ref SINGLE_DATA_BLOCK_SIZE_MAX
 *
 *  \returns Number of bytes of data that were sent
 */
/*----------------------------------------------------------------------------*/
extern uint16 StreamSendDataBlock(uint16 dest_dev_id, uint8 *data, uint16 length);


/*!@} */
#endif /* __STREAM_MODEL_H__ */

