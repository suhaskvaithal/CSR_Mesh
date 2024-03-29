###########################################################
# Makefile generated by xIDE for uEnergy                   
#                                                          
# Project: csr_mesh_light_csr102x
# Configuration: Release
# Generated: Fri May 4 12:39:07 2018
#                                                          
# WARNING: Do not edit this file. Any changes will be lost 
#          when the project is rebuilt.                    
#                                                          
###########################################################

XIDE_PROJECT=csr_mesh_light_csr102x
XIDE_CONFIG=Release
OUTPUT=csr_mesh_light_csr102x
OUTDIR=C:/Users/Pradeep/Desktop/Arraystorm2.1 -12-13-2017/Arraystorm2.1 -12-13-2017 Version 3/Arraystorm2.1 -12-13-2017 (modified)/applications/csr_mesh_light
DEFS=-DCLIENT -DSERVER -DCENTRAL -DPERIPHERAL -DOBSERVER -DGAIA_OTAU_RELAY_SUPPORT -DNVM_DONT_PAD 

OUTPUT_TYPE=0
USE_FLASH=0
ERASE_NVM=1
CSFILE_CSR101x_A05=
MASTER_DB=app_gatt_db.db
LIBPATHS=..\mesh_common\mesh\libraries\csr_102x
INCPATHS=..\mesh_common\...
STRIP_SYMBOLS=0
OTAU_BOOTLOADER=0
OTAU_CSFILE=
OTAU_NAME=
OTAU_SECRET=
OTAU_VERSION=7

LIBS=csrmesh asset_server asset_client tracker_server tracker_client action_server tuning_server time_server time_client light_server power_server ping_server attention_server data_server data_client battery_server largeobjecttransfer_server largeobjecttransfer_client 
DBS=\
\
      ../mesh_common/server/gap/gap_service_db.db\
      ../mesh_common/server/gatt/gatt_service_db.db\
      ../mesh_common/server/mesh_control/mesh_control_service_db.db\
      ../mesh_common/server/gaia/gaia_db.db\
      app_gatt_db.db

INPUTS=\
      ../mesh_common/client/gaia/gaia_client.c\
      ../mesh_common/client/gaia/gaia_client_service.c\
      ../mesh_common/client/gaia/gaia_otau_client.c\
      ../mesh_common/peripheral/conn_param_update.c\
      ../mesh_common/components/nvm_manager/nvm_access.c\
      ../mesh_common/components/connection_manager/cm_common.c\
      ../mesh_common/components/connection_manager/cm_hal.c\
      ../mesh_common/components/connection_manager/cm_private.c\
      ../mesh_common/components/connection_manager/cm_security.c\
      ../mesh_common/components/connection_manager/cm_server.c\
      ../mesh_common/components/connection_manager/cm_observer.c\
      ../mesh_common/components/connection_manager/cm_peripheral.c\
      ../mesh_common/components/connection_manager/cm_central.c\
      ../mesh_common/components/connection_manager/cm_client.c\
      ../mesh_common/mesh/drivers/battery_hw.c\
      ../mesh_common/mesh/drivers/fast_pwm.c\
      ../mesh_common/mesh/drivers/iot_hw.c\
      ../mesh_common/mesh/drivers/csr_mesh_ps_ifce.c\
      ../mesh_common/mesh/handlers/attention_model/attention_model_handler.c\
      ../mesh_common/mesh/handlers/battery_model/battery_model_handler.c\
      ../mesh_common/mesh/handlers/core_mesh/core_mesh_handler.c\
      ../mesh_common/mesh/handlers/data_model/data_model_handler.c\
      ../mesh_common/mesh/handlers/light_model/light_model_handler.c\
      ../mesh_common/mesh/handlers/power_model/power_model_handler.c\
      ../mesh_common/mesh/handlers/common/app_util.c\
      ../mesh_common/mesh/handlers/connection/connection_handler.c\
      ../mesh_common/mesh/handlers/advertisement/advertisement_handler.c\
      ../mesh_common/mesh/handlers/time_model/time_model_handler.c\
      ../mesh_common/mesh/handlers/action_model/action_model_handler.c\
      ../mesh_common/mesh/handlers/largeobjecttransfer_model/largeobjecttransfer_model_handler.c\
      ../mesh_common/mesh/handlers/asset_model/asset_model_handler.c\
      ../mesh_common/mesh/handlers/tracker_model/tracker_model_handler.c\
      ../mesh_common/mesh/handlers/otau/app_otau_handler.c\
      ../mesh_common/mesh/handlers/scan/scan_handler.c\
      ../mesh_common/server/gap/gap_service.c\
      ../mesh_common/server/gatt/gatt_service.c\
      ../mesh_common/server/mesh_control/mesh_control_service.c\
      ../mesh_common/server/gaia/byte_utils.c\
      ../mesh_common/server/gaia/gaia.c\
      ../mesh_common/server/gaia/gaia_otau.c\
      ../mesh_common/server/gaia/gaia_service.c\
      main_app.c\
      app_mesh_handler.c\
      app_hw.c\
      pio_ctrlr_code.asm\
      $(DBS)

KEYR=

# Project-specific options
build_type=0
cs_htf=csr_mesh_light_csr102x.htf
custom_store_config=app_store_config_smem_a.stores
data_ram=1
enable_otau=1
ota_upd=mesh_release.upd
store_config=4

-include csr_mesh_light_csr102x.mak
include $(SDK)/genmakefile.uenergy
