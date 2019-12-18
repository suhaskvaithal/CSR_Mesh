/******************************************************************************
 *  Copyright Cambridge Silicon Radio Limited 2015
 *  CSR Bluetooth Low Energy CSRmesh 1.3 Release
 *****************************************************************************/
/*! \file battery_model.h
 *  \brief Defines CSRmesh Battery Model specific functions
 *   the CSRmesh library
 *
 *   This file contains the functions defined in the CSRmesh battery model
 *   
 *   Copyright (c) CSR plc 2013
 */
    
#ifndef __BATTERY_MODEL_H__
#define __BATTERY_MODEL_H__

/*! \addtogroup Battery_Model
 * @{
 */

/* Bit field definitions of the supported battery state */
#define BATTERY_MODEL_STATE_POWERING_DEVICE    (1)      /*!< \brief Bit-0 - Battery is powering the device */
#define BATTERY_MODEL_STATE_CHARGING           (1 << 1) /*!< \brief Bit-1 - Battery is charging */
#define BATTERY_MODEL_STATE_EXTERNALLY_POWERED (1 << 2) /*!< \brief Bit-2 - Device is externally powered */
#define BATTERY_MODEL_STATE_SERVICE_REQUIRED   (1 << 3) /*!< \brief Bit-3 - Service is required for battery */
#define BATTERY_MODEL_STATE_NEEDS_REPLACEMENT  (1 << 4) /*!< \brief Bit-4 - Battery needs replacement */


/*============================================================================*
    Public Definitions
 *============================================================================*/

/*! \brief CSRmesh Battery Model state */
typedef struct
{
    uint8  battery_level;    /*!< \brief Percentage Level of the battery. */
                             /* Refer \ref XAP_8bit_handler_page. */
    uint8  battery_state;    /*!< \brief Battery State. */
                             /* Refer \ref XAP_8bit_handler_page. */
} BATTERY_MODEL_STATE_DATA_T;

/*============================================================================*
    Public Functions
 *============================================================================*/
/*----------------------------------------------------------------------------*
 * BatteryModelInit 
 */
/*! \brief Initialise the Battery Model of the CSRmesh library
 * 
 *  Initialises the battery model.\n
 *  Enables notification of battery model messages addressed to the device
 *
 *  \returns Nothing
 */
/*----------------------------------------------------------------------------*/
extern void BatteryModelInit(void);

/*!@} */
#endif /* __BATTERY_MODEL_H__ */

