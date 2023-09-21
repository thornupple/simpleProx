/*  Sartup.c 
    To build, create a subdirectory called /build and start with running the following command:
        cmake -DPICO_BOARD=pico_w ..
    After that, build.  The "j" should reflect the number of cpu cores.  such as:
        make -j8

    In the build directory, you should have a *.uf2 file that is the file you would copy to the Pico W
*/
#include <stdio.h>
#include "btstack.h"
#include "pico/cyw43_arch.h"
#include "pico/btstack_cyw43.h"
#include "pico/stdlib.h"
#include <stdint.h>
#include "server.h"

// Read the proximity settings every 3/4 second
#define                                         HEARTBEAT_PERIOD_MS 750

static btstack_timer_source_t                   heartbeatTimer;
static btstack_packet_callback_registration_t   hci_event_callback_registration;

/******************************
 *  Heartbeat Handler
 * ****************************/
static void heartbeat_handler(struct btstack_timer_source *ts) {

    if (le_notification_enabled) {
        // if client has subscribed - this will send a notification out
        att_server_request_can_send_now_event(con_handle);
    }
    // Invert the led
    static int led_on = true;
    led_on = !led_on;
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);

    // Restart timer
    btstack_run_loop_set_timer(ts, HEARTBEAT_PERIOD_MS);
    btstack_run_loop_add_timer(ts);  
}

/******************************
 *  Main Entry Point
 * ****************************/
int main() {

    stdio_init_all();

    // sleep_ms(2000); // Delay to allow serial console to start - not needed for production
    
    // set the pins; this assumes three proximity sensors
    for (int i = 0; i < 3; i++)
    {
        gpio_init(GPIO_ECHO_PINS[i]);
        gpio_set_dir(GPIO_ECHO_PINS[i], GPIO_IN);
    
        gpio_init(GPIO_TRIGGER_PINS[i]);
        gpio_set_dir(GPIO_TRIGGER_PINS[i],  GPIO_OUT);
    }

    
    // initialize CYW43 driver architecture (will enable BT if/because CYW43_ENABLE_BLUETOOTH == 1)
    if (cyw43_arch_init()) {
        printf("failed to initialise cyw43_arch\n");
        return -1;
    }

    l2cap_init();
    
    sm_init();

    att_server_init(profile_data, att_read_callback, att_write_callback);    

    // inform about BTstack state
    hci_event_callback_registration.callback = &packet_handler;

    hci_add_event_handler(&hci_event_callback_registration);

    // register for ATT event
    att_server_register_packet_handler(packet_handler);

    // set one-shot btstack timer
    heartbeatTimer.process = &heartbeat_handler;

    btstack_run_loop_set_timer(&heartbeatTimer, HEARTBEAT_PERIOD_MS);

    btstack_run_loop_add_timer(&heartbeatTimer);

    // turn on bluetooth!
    hci_power_control(HCI_POWER_ON);

    // need to keep the app running... this will do it
    while(true) {      
        sleep_ms(HEARTBEAT_PERIOD_MS);
        getDistance();
    }
    // the getDistance is handled on the main thread and needs to stay here
    // the bt heartbeat handler is enabled to send out the data when the client subscribes
    return 0;
}


