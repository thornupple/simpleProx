
//  Center: Black/Yellow
// static const uint CENTER_BLACK_YELLOW_TRIGGER_PIN_TRIGGER_PIN = 13;
// static const uint CENTER_BLACK_YELLOW_ECHO_PIN = 11;

//  Left: Black/Black
// static const uint LEFT_BLACK_BLACK_TRIGGER_PIN = 21;
// static const uint LEFT_BLACK_BLACK_PIN_ECHO_PIN = 20;

//  Right: Yellow/Yellow
// static const uint RIGHT_YELLOW_YELLOW_TRIGGER_PIN = 1;
// static const uint RIGHT_YELLOW_YELLOW_ECHO_PIN = 0;

#ifndef SERVER_H_
#define SERVER_H_

        extern int                      le_notification_enabled;
        extern hci_con_handle_t         con_handle;
        extern uint8_t const            profile_data[];

        void                            packet_handler(uint8_t, uint16_t, uint8_t *, uint16_t);
        uint16_t                        att_read_callback(hci_con_handle_t, uint16_t, uint16_t, uint8_t *, uint16_t);
        int                             att_write_callback(hci_con_handle_t, uint16_t, uint16_t, uint16_t, uint8_t *, uint16_t);
        void                            printMessage();
        u_int8_t                        measureDistance();
        void                            getDistance();
        u_int32_t                       hexifyProxData(unsigned int, unsigned int, unsigned int);
        void                            getIntsFromHex( unsigned int, unsigned int *, unsigned int *, unsigned int *);


        // the following are hardware specific; that is, the numbers depend on where you install the proximity sensor;
        static const int8_t             CENTER_ECHO_GPIO_PIN = 11;
        static const int8_t             LEFT_ECHO_GPIO_PIN = 20;
        static const int8_t             RIGHT_ECHO_GPIO_PIN = 0;

        static const int8_t             CENTER_TRIGGER_GPIO_PIN = 13;
        static const int8_t             LEFT_TRIGGER_GPIO_PIN = 21;
        static const int8_t             RIGHT_TRIGGER_GPIO_PIN = 1;

        static const int8_t             LEFT_ECHO_PIN = 0;      // Position in the array
        static const int8_t             CENTER_ECHO_PIN = 1; 
        static const int8_t             RIGHT_ECHO_PIN = 2;     

        static uint GPIO_TRIGGER_PINS[3] = 
                {LEFT_TRIGGER_GPIO_PIN, CENTER_TRIGGER_GPIO_PIN, RIGHT_TRIGGER_GPIO_PIN};

        static uint GPIO_ECHO_PINS[3] = 
                {LEFT_ECHO_GPIO_PIN, CENTER_ECHO_GPIO_PIN, RIGHT_ECHO_GPIO_PIN};

#endif

