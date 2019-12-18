/*
 * THIS FILE IS AUTOGENERATED, DO NOT EDIT!
 *
 * generated by gattdbgen from depend_Release_CSR101x_A05/app_gatt_db.db_
 */
#ifndef __APP_GATT_DB_H
#define __APP_GATT_DB_H

#include <main.h>

#define HANDLE_GATT_SERVICE             (0x0001)
#define HANDLE_GATT_SERVICE_END         (0x0004)
#define ATTR_LEN_GATT_SERVICE           (2)
#define HANDLE_SERVICE_CHANGED          (0x0003)
#define ATTR_LEN_SERVICE_CHANGED        (0)
#define HANDLE_SERVICE_CHANGED_CLIENT_CONFIG (0x0004)
#define ATTR_LEN_SERVICE_CHANGED_CLIENT_CONFIG (0)
#define HANDLE_GAP_SERVICE              (0x0005)
#define HANDLE_GAP_SERVICE_END          (0x000b)
#define ATTR_LEN_GAP_SERVICE            (2)
#define HANDLE_DEVICE_NAME              (0x0007)
#define ATTR_LEN_DEVICE_NAME            (1)
#define HANDLE_DEVICE_APPEARANCE        (0x0009)
#define ATTR_LEN_DEVICE_APPEARANCE      (1)
#define HANDLE_DEVICE_PREF_CONN_PARAMS  (0x000b)
#define ATTR_LEN_DEVICE_PREF_CONN_PARAMS (6)
#define HANDLE_CSR_OTA_SERVICE          (0x000c)
#define HANDLE_CSR_OTA_SERVICE_END      (0x0015)
#define ATTR_LEN_CSR_OTA_SERVICE        (16)
#define HANDLE_CSR_OTA_CURRENT_APP      (0x000e)
#define ATTR_LEN_CSR_OTA_CURRENT_APP    (1)
#define HANDLE_CSR_OTA_READ_CS_BLOCK    (0x0010)
#define ATTR_LEN_CSR_OTA_READ_CS_BLOCK  (1)
#define HANDLE_CSR_OTA_DATA_TRANSFER    (0x0012)
#define ATTR_LEN_CSR_OTA_DATA_TRANSFER  (20)
#define HANDLE_CSR_OTA_DATA_TRANSFER_CLIENT_CONFIG (0x0013)
#define ATTR_LEN_CSR_OTA_DATA_TRANSFER_CLIENT_CONFIG (0)
#define HANDLE_CSR_OTA_VERSION          (0x0015)
#define ATTR_LEN_CSR_OTA_VERSION        (1)
#define HANDLE_MESH_CONTROL_SERVICE     (0x0016)
#define HANDLE_MESH_CONTROL_SERVICE_END (0xffff)
#define ATTR_LEN_MESH_CONTROL_SERVICE   (2)
#define HANDLE_NETWORK_KEY              (0x0018)
#define ATTR_LEN_NETWORK_KEY            (1)
#define HANDLE_DEVICE_UUID              (0x001a)
#define ATTR_LEN_DEVICE_UUID            (16)
#define HANDLE_DEVICE_ID                (0x001c)
#define ATTR_LEN_DEVICE_ID              (1)
#define HANDLE_MTL_CONTINUATION_CP      (0x001e)
#define ATTR_LEN_MTL_CONTINUATION_CP    (1)
#define HANDLE_MTL_CP_CLIENT_CONFIG     (0x001f)
#define ATTR_LEN_MTL_CP_CLIENT_CONFIG   (0)
#define HANDLE_MTL_COMPLETE_CP          (0x0021)
#define ATTR_LEN_MTL_COMPLETE_CP        (1)
#define HANDLE_MTL_CP2_CLIENT_CONFIG    (0x0022)
#define ATTR_LEN_MTL_CP2_CLIENT_CONFIG  (0)
#define HANDLE_MTL_TTL                  (0x0024)
#define ATTR_LEN_MTL_TTL                (1)
#define HANDLE_MESH_APPEARANCE          (0x0026)
#define ATTR_LEN_MESH_APPEARANCE        (1)

extern uint16 *GattGetDatabase(uint16 *len);

#endif

/* End-of-File */