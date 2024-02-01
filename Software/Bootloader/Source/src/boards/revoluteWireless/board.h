#ifndef _REVOLUTE_WIRELESS_H
#define _REVOLUTE_WIRELESS_H

#define _PINNUM(port, pin)    ((port)*32 + (pin))

/*------------------------------------------------------------------*/
/* LED
 *------------------------------------------------------------------*/
#define LEDS_NUMBER       1
#define LED_PRIMARY_PIN   _PINNUM(0, 03) // Blue
#define LED_STATE_ON      1

//#define LED_NEOPIXEL          _PINNUM(0, 16)
//#define NEOPIXELS_NUMBER      1
//#define BOARD_RGB_BRIGHTNESS  0x040404

/*------------------------------------------------------------------*/
/* BUTTON
 *------------------------------------------------------------------*/
#define BUTTONS_NUMBER        2
#define BUTTON_1              _PINNUM(0, 19)
#define BUTTON_2              _PINNUM(0, 10)
#define BUTTON_PULL           NRF_GPIO_PIN_PULLUP

//--------------------------------------------------------------------+
// BLE OTA
//--------------------------------------------------------------------+
#define BLEDIS_MANUFACTURER   "Tongtong Inc"
#define BLEDIS_MODEL          "RevoluteWireless"

//--------------------------------------------------------------------+
// USB
//--------------------------------------------------------------------+
#define USB_DESC_VID           0x239A
#define USB_DESC_UF2_PID       0x0029 // TODO change later
#define USB_DESC_CDC_ONLY_PID  0x002A // TODO change later

//------------- UF2 -------------//
#define UF2_PRODUCT_NAME      "RevoluteWireless"
#define UF2_VOLUME_LABEL      "RevoluteWireless"
#define UF2_BOARD_ID          "RevoluteWireless-nrf52833"
#define UF2_INDEX_URL         "https://www.tongtonginc.com/revolute"

#endif
