/***************************************************************************//**
 * @file app.c
 * @brief Callbacks implementation and application specific code.
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include "app/framework/include/af.h"
#include "sl_simple_button_instances.h"
#include "sl_simple_led_instances.h"
#include "sl_simple_rgb_pwm_led_instances.h"
#include "pin_config.h"
#include "network-steering.h"

#define LIGHT_ENDPOINT                         1

// Zigbee Cluster Library: On/Off cluster = 0x0006, attribut OnOff = 0x0000
//#define ZCL_ON_OFF_CLUSTER_ID                  0x0006
//#define ZCL_ON_OFF_ATTRIBUTE_ID                0x0000

//GPIO_PinModeSet;


/*emberAfPluginNetworkSteeringStart;

GPIO_PinModeSet;

//static sl_zigbee_event_t xxx;
sl_zigbee_af_isr_event_init;
sl_zigbee_event_set_active;

emberAfStackStatusCallback;*/


extern const sl_led_t sl_led_led0; // LED_RED :contentReference[oaicite:7]{index=7}
extern const sl_led_t sl_led_led1; // Souvent LED verte

extern const sl_button_t sl_button_btn0;
extern const sl_button_t sl_button_btn1;

static sl_zigbee_event_t g_app_event;
static sl_zigbee_event_t g_app_isr_event;

//check LedBlinking


// ---------- Etat applicatif ----------
static volatile bool g_toggle_requested = false;
static volatile bool g_commission_requested = false;


//extern const sl_led_pwm_rgb_t sl_led_pwm_rgb; // Exemple: instance RGB


/** @brief Complete network steering.
 *
 * This callback is fired when the Network Steering plugin is complete.
 *
 * @param status On success this will be set to EMBER_SUCCESS to indicate a
 * network was joined successfully. On failure this will be the status code of
 * the last join or scan attempt. Ver.: always
 *
 * @param totalBeacons The total number of 802.15.4 beacons that were heard,
 * including beacons from different devices with the same PAN ID. Ver.: always
 * @param joinAttempts The number of join attempts that were made to get onto
 * an open Zigbee network. Ver.: always
 *
 * @param finalState The finishing state of the network steering process. From
 * this, one is able to tell on which channel mask and with which key the
 * process was complete. Ver.: always
 */
void emberAfPluginNetworkSteeringCompleteCallback(EmberStatus status,
                                                  uint8_t totalBeacons,
                                                  uint8_t joinAttempts,
                                                  uint8_t finalState)
{
  sl_zigbee_app_debug_println("%s network %s: 0x%02X", "Join", "complete", status);
}

/** @brief
 *
 * Application framework equivalent of ::emberRadioNeedsCalibratingHandler
 */
void emberAfRadioNeedsCalibratingCallback(void)
{
  sl_mac_calibrate_current_channel();
}

void emberAfMainInitCallback(void)
{

  sl_zigbee_af_event_init(&g_app_event, my_event_handler);
  sl_zigbee_af_isr_event_init(&my_isr_event, my_isr_event_handler);
  sl_zigbee_af_event_set_active(&g_app_event);

  //init led
  sl_led_init(&sl_led_led0);
  sl_led_init(&sl_led_led1);

  //sl_led_turn_on(&sl_led_led0);
  //sl_led_turn_on(&sl_led_led1);

  sl_led_turn_off(&sl_led_led0);
  sl_led_turn_off(&sl_led_led1);


  sl_button_init(&sl_button_btn0);
  sl_button_init(&sl_button_btn1);



}


// ---------- Helpers LEDs ----------
static void set_pairing_led(bool pairing)
{
  // LED rouge: appairage en cours
  if (pairing) {
    sl_led_turn_on(&sl_led_led0);
  } else {
    sl_led_turn_off(&sl_led_led0);
  }
}


void sl_button_on_change(const sl_button_t *handle)
{
  if (!sl_button_get_state(handle)) {
     return; // on ne traite que l'appui (pas le relâchement)
   }

   // Adapte les handles: souvent BTN0 = commissioning, BTN1 = On/Off
   if (handle == &sl_button_btn0) {
       g_commission_requested = true;
       sl_zigbee_event_set_active(&g_app_event);
       while(1){
           sl_led_toggle(&sl_led_led0);
           for(volatile int i=0; i<1000000;i++); //simple delay
       }
   }
   else if (handle == &sl_button_btn1) {
      g_toggle_requested = true;
      sl_zigbee_event_set_active(&g_app_event);
     }
}

// Suivi réseau: LEDs rouge/verte
void emberAfStackStatusCallback(EmberStatus status)
{
  // Mise à jour LED verte selon connexion réseau
  /*
  set_network_led(is_joined_network());

  // Si on vient de rejoindre, on coupe l'indication appairage
  if (is_joined_network()) {
    set_pairing_led(false);
  }
  */
}




//sl_led_turn_xxx
//sl_led_set_rgb_color
//emberLeaveNetwork
