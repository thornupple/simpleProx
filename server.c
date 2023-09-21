/* Server.c */
#include <stdio.h>
#include <stdint.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "btstack.h"
#include "simpleProx.h"
#include "server.h"
#include "math.h"

#define APP_AD_FLAGS 0x06


int                 le_notification_enabled; 
hci_con_handle_t    con_handle;

u_int32_t           combinedDistanceMeasurement;    // takes the three ints and puts them into a single int as a hex
u_int8_t            proxCenter, proxLeft, proxRight;

void getDistance(void) {
    
    // VERY IMPORTANT  THE CLIENT MUST REVERSE THE BYTES - we are trying to do as much processing on the client as we can   
    // and the bytes are reversed when they are transmitted to the client - go figure 
    printMessage("getDistance");
    
    for (int i = 0; i < 3; i++)
    {        
        // Trigger the sensor by sending a pulse on the trigger pin
        gpio_put(GPIO_TRIGGER_PINS[i], 1);  // send

        sleep_us(10);  // sleep for 10 milliseconds; so we send the ultrasonic sound for just ten milliseconds and then turn it off
        
        gpio_put(GPIO_TRIGGER_PINS[i], 0);  // cancel send

        // Measure the duration of the echo pulse
        uint32_t start_time = time_us_32();
        
        while (gpio_get(GPIO_ECHO_PINS[i]) == 0) {
            if ((time_us_32() - start_time) > 3700) {
                return;
            }
        }
        uint32_t echo_start_time = time_us_32();

        while (gpio_get(GPIO_ECHO_PINS[i]) == 1) continue;
        
        uint32_t echo_end_time = time_us_32();

        // Calculate the duration of the echo pulse
        uint32_t echo_duration = echo_end_time - echo_start_time;
        
        // Calculate the distance in centimeters (Speed of sound: 343 m/s or 34300 cm/s)
        float distance_cm = (echo_duration / 2.0) * 0.0343;
        
        // we don't care if the distance is over 99 cm
        if (distance_cm > 99)
            distance_cm = 99;
        
        switch (i)
        {
            case CENTER_ECHO_PIN:
                // printf("Raw distance CENTER: %d\n",echo_duration);
                proxCenter = roundf(distance_cm);
                break;
            
            case LEFT_ECHO_PIN:
                // printf("Raw distance LEFT: %d\n",echo_duration);
                proxLeft = roundf(distance_cm);
                break;
            
            case RIGHT_ECHO_PIN:
                // printf("Raw distance RIGHT: %d\n",echo_duration);
                proxRight = roundf(distance_cm);
                break;
        }
    }   
        
        // // further from this, we just skip it
        // this was in one of the samples, but let's not use it for now
        // if (echo_duration > 3700)
        //     continue;
    

    // we want to send the data in a three byte hex
    combinedDistanceMeasurement = hexifyProxData(proxLeft, proxCenter,  proxRight);

    printf("Sending data Left: %d,  Center: %d,  Right: %d\n\n", proxLeft, proxCenter, proxRight);

}

/******************************
 * Advertising Data
 * ****************************/
static uint8_t adv_data[] = {
    // Flags general discoverable
    0x02, BLUETOOTH_DATA_TYPE_FLAGS, APP_AD_FLAGS,
    0x11, BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME, 'S', 'i', 'm', 'p', 'l', 'e', ' ', 'P', 'r', 'o', 'x', 'i', 'm', 'i', 't', 'y',
    0x03, BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS, 0x1a, 0x18,
};
static const uint8_t adv_data_len = sizeof(adv_data);

/******************************
 *  Print Message - Overloaded
 * ****************************/
void printMessage(const char* errorMessage) {
    printf("Debug Message: %s\n", errorMessage);
}

/******************************
 *  Packet Handler
 * ****************************/
void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {

printMessage("packet_handler");
  
    UNUSED(size);
    UNUSED(channel);

    if (packet_type != HCI_EVENT_PACKET)
        return;
    
    uint8_t event_type = hci_event_packet_get_type(packet);

    switch(event_type){
        case BTSTACK_EVENT_STATE:

            if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING) {
                return;
            }
            
            // setup advertisements
            uint16_t adv_int_min = 800;   // we want this small to reduce the power consumption
            uint16_t adv_int_max = 800;  //
            uint8_t adv_type = 0;
            bd_addr_t null_addr;
            gap_advertisements_set_params(adv_int_min, adv_int_max, adv_type, 0, null_addr, 0x07, 0x00);
            assert(adv_data_len <= 31); // ble limitation
            gap_advertisements_set_data(adv_data_len, (uint8_t*) adv_data);
            gap_advertisements_enable(1);
            break;
        case HCI_EVENT_DISCONNECTION_COMPLETE:
            le_notification_enabled = 0;
            break;
        case ATT_EVENT_CAN_SEND_NOW:
            printMessage("ATT_EVENT_CAN_SEND_NOW - SENDING server notify");
            att_server_notify(con_handle, ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_LOCATION_AND_SPEED_01_VALUE_HANDLE, (uint8_t*)&combinedDistanceMeasurement, sizeof(combinedDistanceMeasurement));
            break;
        default:
            break;
    }
}

/******************************
 *  ATT Read Callback
 * ****************************/
uint16_t att_read_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size) {

printMessage("att_read_callback");

    UNUSED(connection_handle);
   
    if (att_handle == ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_LOCATION_AND_SPEED_01_VALUE_HANDLE)
    {
        printMessage("att_read_callback handle blob");
        return att_read_callback_handle_blob((const uint8_t *)&combinedDistanceMeasurement, sizeof(combinedDistanceMeasurement), offset, buffer, buffer_size);
    }
    return 0;
}

/*******************************
 *  ATT Write Callback
 * ****************************/
int att_write_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size) {

    UNUSED(transaction_mode);
    UNUSED(offset);
    UNUSED(buffer_size);
    
    if (att_handle != ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_LOCATION_AND_SPEED_01_CLIENT_CONFIGURATION_HANDLE) 
        return 0;

    le_notification_enabled = little_endian_read_16(buffer, 0);

  // printf("att_write_callback notification enabled is: %d\n");
    con_handle = connection_handle;
   
    if (le_notification_enabled) {
        att_server_request_can_send_now_event(con_handle);
        printMessage("Server Send Notifications Enabled");
    }
    else
    {
        printMessage("Server Send Notifications Disabled   ");
        le_notification_enabled = 0;
    }
    return 0;
}

// the systen sends in reverse byte order.  We only need a 12 bit value for our three number and so let's combined them
// in reverse byte order, that is, the first byte is value 3...
u_int32_t hexifyProxData(unsigned int val1, unsigned int val2, unsigned int val3) {
    
    // this would put the values in first to last order
    //return (val1 << 16) | (val2 << 8) | val3;

    // thiks puts the third byte first, second next, etc. This supports three values
    return (val3 << 16) | (val2 << 8) | val1;
}

// test test
void getIntsFromHex(unsigned int inHexValue, unsigned int *out1, unsigned int *out2, unsigned int *out3){
    *out1 = (inHexValue >> 16) & 0xFF;
    *out2 = (inHexValue >> 8) & 0xFF;
    *out3 = inHexValue & 0xFF;
}
