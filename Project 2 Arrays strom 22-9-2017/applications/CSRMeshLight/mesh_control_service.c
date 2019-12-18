/******************************************************************************
 *  Copyright Cambridge Silicon Radio Limited 2015
 *  CSR Bluetooth Low Energy CSRmesh 1.3 Release
 *  Application version 1.3
 *
 *  FILE
 *      mesh_control_service.c
 *
 *  DESCRIPTION
 *      This file defines routines for using the Mesh Control Service.
 *
 *****************************************************************************/

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <gatt.h>
#include <gatt_prim.h>
#include <mem.h>
#include <string.h>
#include <buf_utils.h>

/*============================================================================*
 *  Local Header Files
 *============================================================================*/
#include "app_debug.h"
#include "app_gatt_db.h"
#include "app_gatt.h"
#include "mesh_control_service.h"
#include "nvm_access.h"

/*============================================================================*
 *  CSRmesh Header Files
 *============================================================================*/
#include "csr_mesh.h"

/*============================================================================*
 *  Private Data Types
 *============================================================================*/

typedef struct
{
    uint16 length;

    uint8 mesh_data[MESH_LONGEST_MSG_LEN];
}MESH_MSG_T;

/* Structure for the Lock Unlock service */
typedef struct
{
    /* NVM offset at which data is stored */
    uint16              nvm_offset;

    /* Client configuration for Mesh Control characteristic */
    gatt_client_config  mtl_cp_ccd;

    MESH_MSG_T mesh_data;

}MESH_SERVICE_DATA_T;

/*============================================================================*
 *  Private Data
 *============================================================================*/
MESH_SERVICE_DATA_T        g_mesh_svc_data;

/*============================================================================*
 *  Public Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      MeshControlServiceDataInit
 *
 *  DESCRIPTION
 *      This function is used to initialise Mesh Control Service data 
 *      structure.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void MeshControlServiceDataInit(void)
{
    /* Initialise Mesh Control Service Client Configuration Characterisitic
     * descriptor value to none.
     */
    g_mesh_svc_data.mtl_cp_ccd = gatt_client_config_none;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      MeshControlHandleAccessRead
 *
 *  DESCRIPTION
 *      This function handles read operations on the Mesh Control
 *      attributes maintained by the application and responds with the
 *      GATT_ACCESS_RSP message.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void MeshControlHandleAccessRead(GATT_ACCESS_IND_T *p_ind)
{
    uint16 length = 0;
    uint8  *p_value = NULL;
    sys_status rc = sys_status_success;
    CSR_MESH_UUID_T dev_uuid;
    uint8 val[16];


    switch(p_ind->handle)
    {
        case HANDLE_DEVICE_UUID:
        {
            CsrMeshGetDeviceUUID(&dev_uuid);
            p_value = val;
            MemCopyUnPack(val, dev_uuid.uuid, 16);
            length = 16;
        }
        break;

        case HANDLE_DEVICE_ID:
        {
            /* Do Nothing for now */
        }
        break;

        case HANDLE_MTL_TTL:
        {
            uint8 ttl = CsrMeshGetMessageTTL(CSR_MESH_MESSAGE_CONTROL);
            p_value = val;
            BufWriteUint8(&p_value, ttl);
            length = 1;
        }
        break;

        case HANDLE_MTL_CP_CLIENT_CONFIG:
        {
            p_value = val;
            BufWriteUint16(&p_value, g_mesh_svc_data.mtl_cp_ccd);
            p_value = val;
            length = 2;
        }

        default:
            /* No more IRQ characteristics */
            rc = gatt_status_read_not_permitted;
        break;

    }

    GattAccessRsp(p_ind->cid, p_ind->handle, rc,
                  length, p_value);

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      MeshControlNotifyResponse
 *
 *  DESCRIPTION
 *      This function notifies responses received on the mesh to the GATT
 *      client.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void MeshControlNotifyResponse(uint16 ucid, uint8 *mtl_msg, uint8 length)
{
    /* Update the connected host if notifications are configured */
    if((ucid != GATT_INVALID_UCID) &&
       (g_mesh_svc_data.mtl_cp_ccd == gatt_client_config_notification))
    {
        /* If message is less than or equal to ATT_WRITE_MAX_DATA_LEN,
         * notify it using MTL_COMPLETE_CP. If it is greater than
         * ATT_WRITE_MAX_DATA_LEN notify first ATT_WRITE_MAX_DATA_LEN bytes
         * with MTL_CONTINUATION_CP and rest with MTL_COMPLETE_CP.
         */
        if (length <= ATT_WRITE_MAX_DATALEN)
        {
            GattCharValueNotification(ucid, HANDLE_MTL_COMPLETE_CP,
                                      length, mtl_msg);
        }
        else
        {
            /* Send first ATT_WRITE_MAX_DATALEN with MTL_CONTINUATION_CP */
            GattCharValueNotification(ucid, HANDLE_MTL_CONTINUATION_CP,
                                      ATT_WRITE_MAX_DATALEN, mtl_msg);

            /* Send rest of the message with MTL_COMPLETE_CP */
            GattCharValueNotification(ucid, HANDLE_MTL_COMPLETE_CP,
                                      length - ATT_WRITE_MAX_DATALEN,
                                      &mtl_msg[ATT_WRITE_MAX_DATALEN]);
        }
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      MeshControlHandleAccessWrite
 *
 *  DESCRIPTION
 *      This function handles write operations on the Mesh Control
 *      service attributes maintained by the application and responds with the
 *      GATT_ACCESS_RSP message.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void MeshControlHandleAccessWrite(GATT_ACCESS_IND_T *p_ind)
{
    sys_status rc = sys_status_success;
    uint8  *pValue;
    bool csr_mesh_send_msg = FALSE;

    switch(p_ind->handle)
    {
        case HANDLE_MTL_CP_CLIENT_CONFIG:
        case HANDLE_MTL_CP2_CLIENT_CONFIG:
        {
            pValue = p_ind->value;
            g_mesh_svc_data.mtl_cp_ccd = BufReadUint16(&pValue);

            /* Reset the reserved bits in any case */
            g_mesh_svc_data.mtl_cp_ccd &= ~gatt_client_config_reserved;

        }
        break;

        case HANDLE_MTL_CONTINUATION_CP:
        {
            pValue = p_ind->value;

            /* Reset the length of the mesh message */
            g_mesh_svc_data.mesh_data.length = 0;

            if(p_ind->size_value)
            {
                MemCopy(g_mesh_svc_data.mesh_data.mesh_data + p_ind->offset,
                        p_ind->value, p_ind->size_value);
                g_mesh_svc_data.mesh_data.length += p_ind->size_value;
            }
        }
        break;

        case HANDLE_MTL_COMPLETE_CP:
        {
            pValue = p_ind->value;

            if(p_ind->size_value && ((p_ind->size_value + 
                     g_mesh_svc_data.mesh_data.length) <= MESH_LONGEST_MSG_LEN))
            {
                MemCopy(g_mesh_svc_data.mesh_data.mesh_data + \
                        g_mesh_svc_data.mesh_data.length,
                        p_ind->value, p_ind->size_value);
                g_mesh_svc_data.mesh_data.length += p_ind->size_value;

                /* Send message to CSRmesh Library. */
                csr_mesh_send_msg = TRUE;
            }

        }
        break;

        case HANDLE_MTL_TTL:
        {
            uint8 ttl = 0x00;
            pValue = p_ind->value;
            ttl =  BufReadUint8(&pValue);

            CsrMeshSetMessageTTL(CSR_MESH_MESSAGE_CONTROL, (ttl & 0xFF));
        }
        break;

        default:
        {
            /* No more IRQ characteristics */
            rc = gatt_status_write_not_permitted;
        }
        break;
    }

    GattAccessRsp(p_ind->cid, p_ind->handle, rc, 0, NULL);

    if(csr_mesh_send_msg)
    {
        /* Send the MTL data as it is on the mesh */
        /*DEBUG_STR("Send GATT Msg\r\n");*/
        CsrMeshProcessRawMessage(g_mesh_svc_data.mesh_data.mesh_data,
                                   g_mesh_svc_data.mesh_data.length);

        /* Reset the length of the mesh message */
        g_mesh_svc_data.mesh_data.length = 0;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      MeshControlCheckHandleRange
 *
 *  DESCRIPTION
 *      This function is used to check if the handle belongs to the Mesh Control
 *      service
 *
 *  RETURNS
 *      Boolean - Indicating whether handle falls in range or not.
 *
 *---------------------------------------------------------------------------*/
extern bool MeshControlCheckHandleRange(uint16 handle)
{
    return ((handle >= HANDLE_MESH_CONTROL_SERVICE) &&
            (handle <= HANDLE_MESH_CONTROL_SERVICE_END))
            ? TRUE : FALSE;
}
