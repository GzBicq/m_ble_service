#include "midi_service.h"
#include "nordic_common.h"
#include "ble_srv_common.h"
#include "app_util.h"
#include <string.h>


static void on_connect(ble_midi_service_t *p_midi_service, ble_evt_t const * p_ble_evt)
{
    p_midi_service->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}
                     
static void on_disconnect(ble_midi_service_t *p_midi_service, ble_evt_t const * p_ble_evt)
{   
    UNUSED_PARAMETER(p_ble_evt);
    p_midi_service->conn_handle = BLE_CONN_HANDLE_INVALID;

}

static void on_write(ble_midi_service_t *p_midi_service, ble_evt_t const * p_ble_evt)
{
    if(p_ble_evt->evt.gatts_evt.params.write.handle == p_midi_service->data_io_char_handles.value_handle 
       && p_ble_evt->evt.gatts_evt.params.write.len == 1 
       && p_midi_service->data_io_write_handler != NULL)
    {
        p_midi_service->data_io_write_handler(p_midi_service, p_ble_evt->evt.gatts_evt.params.write.data[0]);
    }
}


 
void ble_midi_service_on_ble_evt(ble_midi_service_t * p_midi_service, ble_evt_t const * p_ble_evt)
{
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_midi_service, p_ble_evt);
            break;
            
        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_midi_service, p_ble_evt);
            break;
            
        case BLE_GATTS_EVT_WRITE:
            on_write(p_midi_service, p_ble_evt);
            break;
            
        default:
            // No implementation needed.
            break;
    }
}
 
/**@brief Function for adding the Data I/O characteristic.
 *
 */
static uint32_t data_io_char_add(ble_midi_service_t * p_midi_service, const ble_midi_service_init_t * p_midi_service_init)
{
    uint32_t   err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
 
    memset(&cccd_md, 0, sizeof(cccd_md));
 
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    cccd_md.vloc = BLE_GATTS_VLOC_STACK;
    
    memset(&char_md, 0, sizeof(char_md));
    
    char_md.char_props.read          = 1;
    char_md.char_props.write_wo_resp = 1;
    char_md.char_props.notify        = 1;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = &cccd_md;
    char_md.p_sccd_md                = NULL;
    
    // Add the MIDI Data I/O Characteristic UUID
    ble_uuid128_t base_uuid = {BLE_UUID_MIDI_DATA_IO_CHAR_BASE_UUID};
    err_code = sd_ble_uuid_vs_add(&base_uuid, &p_midi_service->uuid_type);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    ble_uuid.type = p_midi_service->uuid_type;
    ble_uuid.uuid = BLE_UUID_MIDI_DATA_IO_CHAR_UUID;
    
    memset(&attr_md, 0, sizeof(attr_md));
 
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;
    
    memset(&attr_char_value, 0, sizeof(attr_char_value));
 
    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = sizeof(uint8_t);
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = sizeof(uint8_t);
    attr_char_value.p_value      = NULL;
    
    return sd_ble_gatts_characteristic_add(p_midi_service->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_midi_service->data_io_char_handles);
}
 
uint32_t ble_midi_service_init(ble_midi_service_t * p_midi_service, const ble_midi_service_init_t * p_midi_service_init)
{
    uint32_t   err_code;
    ble_uuid_t ble_uuid;
 
    // Initialize service structure
    p_midi_service->conn_handle       = BLE_CONN_HANDLE_INVALID;
    p_midi_service->data_io_write_handler = p_midi_service_init->data_io_write_handler;
    
    // Add service
    ble_uuid128_t base_uuid = {BLE_UUID_MIDI_SERVICE_BASE_UUID};
    err_code = sd_ble_uuid_vs_add(&base_uuid, &p_midi_service->uuid_type);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    ble_uuid.type = p_midi_service->uuid_type;
    ble_uuid.uuid = BLE_UUID_MIDI_SERVICE_UUID;
 
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_midi_service->service_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    err_code = data_io_char_add(p_midi_service, p_midi_service_init);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    return NRF_SUCCESS;
}
 
// This function should be implemented to handle the notification event for updates to the 
// MIDI Data I/O characteristic
// The following example assumes that the data being notified to the Client is a uint8 (unsigned
// integer of size 1 byte). You will need to implement this function to follow your use case
// or adhere to the official MIDI spec.
// This function should be called when you want the application to notify the Client that the data
// has been updated. You will call this function from main.c
// For the purposes of this example, this function is not being called from main.c so even if the Client
// enables Notifications, nothing will get sent from the Server.
uint32_t ble_lbs_on_button_change(ble_midi_service_t * p_midi_service, uint8_t data)
{
    ble_gatts_hvx_params_t params;
    uint16_t len = sizeof(data);
    
    memset(&params, 0, sizeof(params));
    params.type = BLE_GATT_HVX_NOTIFICATION;
    params.handle = p_midi_service->data_io_char_handles.value_handle;
    params.p_data = &data;
    params.p_len = &len;
    
    return sd_ble_gatts_hvx(p_midi_service->conn_handle, &params);
}
 
