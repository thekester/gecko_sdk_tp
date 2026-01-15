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


// led 0LED_RED :contentReference[oaicite:7]{index=7}
// LED verte

static sl_zigbee_event_t g_app_event;
static sl_zigbee_event_t g_app_isr_event;

static sl_zigbee_event_t ledBlinkEvent;

void app_start_join(void)
{
  //(void) sl_zigbee_af_network_steering_start(); //emberAfPluginNetworkSteeringStart
  emberAfPluginNetworkSteeringStart();
}


static void ledBlinkEventHandler(sl_zigbee_event_t *event)
{
  sl_led_toggle(&sl_led_led0);  // ou sl_led_led1 selon ta carte
  app_start_join();

}



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
  //Appairement au Zigbee
  sl_zigbee_app_debug_println("%s network %s: 0x%02X", "Join", "complete", status);
  // status == EMBER_SUCCESS means joined successfully
  if (status == EMBER_SUCCESS) {

      // Joined a Zigbee network successfully -> turn LED1 on
      sl_led_turn_on(&sl_led_led1);
      sl_led_turn_off(&sl_led_led0);

    }
}

void emberAfPostAttributeChangeCallback(uint8_t endpoint,
                                        EmberAfClusterId clusterId,
                                        EmberAfAttributeId attributeId,
                                        uint8_t mask,
                                        uint16_t manufacturerCode,
                                        uint8_t type,
                                        uint8_t size,
                                        uint8_t *value)
{
  if (clusterId == ZCL_ON_OFF_CLUSTER_ID
      && attributeId == ZCL_ON_OFF_ATTRIBUTE_ID
      && mask == CLUSTER_MASK_SERVER) {
    bool onOff;
    EmberAfStatus readStatus;

    readStatus = emberAfReadServerAttribute(endpoint,
                                            ZCL_ON_OFF_CLUSTER_ID,
                                            ZCL_ON_OFF_ATTRIBUTE_ID,
                                            (uint8_t *)&onOff,
                                            sizeof(onOff));

    if (readStatus == EMBER_ZCL_STATUS_SUCCESS) {
         if (onOff) {
           // ON : allumer la LED
           uint16_t red = 65535;
           uint16_t green = 0;
           uint16_t blue = 65535;
           sl_led_set_rgb_color(&sl_simple_rgb_pwm_led_rgb_led0, red, green, blue);
           sl_led_turn_on((sl_led_t *)&sl_simple_rgb_pwm_led_rgb_led0);
         } else {
           // OFF : éteindre la LED
           sl_led_turn_off((sl_led_t *)&sl_simple_rgb_pwm_led_rgb_led0);
         }
       } else {
         // Ici seulement en cas d’erreur de lecture
         // (par ex. log ou clignotement d’erreur)
         sl_led_toggle(&sl_simple_rgb_pwm_led_rgb_led0);
       }
  }
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


  //init led and buttons in autogen

  GPIO_PinModeSet(gpioPortJ, 14, gpioModePushPull, 1);
  GPIO_PinModeSet(gpioPortI, 0, gpioModePushPull, 1);

  sl_led_turn_off(&sl_led_led0);
  sl_led_turn_off(&sl_led_led1);

  //pas de event init sans isr sinon on peut pas gérer l'interruption
  sl_zigbee_af_isr_event_init(&ledBlinkEvent, ledBlinkEventHandler);


}


void sl_button_on_change(const sl_button_t *handle)
{
  if (!sl_button_get_state(handle)) {
     return; // on ne traite que l'appui (pas le relâchement)
   }

   // Adapte les handles: souvent BTN0 = commissioning, BTN1 = On/Off
   if (handle == &sl_button_btn0) {
       sl_zigbee_event_set_active(&ledBlinkEvent);


       /*while(status != EMBER_SUCCESS){
           app_start_join();
           sl_led_toggle(&sl_led_led0);
           for(volatile int i=0; i<1000000;i++); //simple delay
       }*/
   }
   if (handle == &sl_button_btn1) {
       sl_led_toggle(&sl_simple_rgb_pwm_led_rgb_led0);
   }
}


void emberAfStackStatusCallback(EmberStatus status)
{

    sl_led_turn_off(&sl_led_led0);

    sl_led_turn_on(&sl_led_led1);

}



//sl_led_turn_xxx
//sl_led_set_rgb_color
//emberLeaveNetwork
