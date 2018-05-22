#ifndef MIDI_SERVICE_H
#define MIDI_SERVICE_H

#include "ble.h"
#include "ble_srv_common.h"

#ifdef __cplusplus
extern "C" {
#endif
    
    
// MIDI Service UUID: 03B80E5A-EDE8-4B33-A751-6CE34EC4C700
// MIDI Data I/O Characteristic UUID: 7772E5DB-3868-4112-A1A9-F2669D106BF3
// The bytes need to be in reverse order to match the UUID from the spec
// Note: They are stored in little endian, meaning that the Least Significant Byte
// is stored first. 
    
#define BLE_UUID_MIDI_SERVICE_BASE_UUID         {0x00, 0xC7, 0xC4, 0x4E, 0xE3, 0x6C, 0x51, 0xA7,    \
                                                 0x33, 0x4B, 0xE8, 0xED, 0x5A, 0x0E, 0xB8, 0x03}

#define BLE_UUID_MIDI_SERVICE_UUID               0x0E5A

#define BLE_UUID_MIDI_DATA_IO_CHAR_BASE_UUID {0xF3, 0x6B, 0x10, 0x9D, 0x66, 0xF2, 0xA9,             \
                                              0xA1, 0x12, 0x41, 0x68, 0x38, 0xDB, 0xE5, 0x72, 0x77}

#define BLE_UUID_MIDI_DATA_IO_CHAR_UUID       0xE5DB


typedef struct ble_midi_service_s   ble_midi_service_t;

typedef void (*ble_midi_data_io_write_handler_t)(ble_midi_service_t *p_midi_service, uint8_t new_state);

typedef struct  
{
    ble_midi_data_io_write_handler_t    data_io_write_handler;
}ble_midi_service_init_t;

struct ble_midi_service_s
{
    uint16_t                            service_handle;
    ble_gatts_char_handles_t            data_io_char_handles;
    uint8_t                             uuid_type;
    uint16_t                            conn_handle;
    ble_midi_data_io_write_handler_t    data_io_write_handler;
};


uint32_t ble_midi_service_init(ble_midi_service_t * p_midi_service, const ble_midi_service_init_t * p_midi_service_init);

void ble_midi_service_on_ble_evt(ble_midi_service_t * p_midi_service, ble_evt_t const * p_ble_evt);


#ifdef __cplusplus
}
#endif









#endif /*MIDI_SERVICE_H*/