#include <Arduino.h>
#include "view.h"
#include "networking.h"
#include "sideled.h"
#include "images/image_cherry.h"
#include "images/image_lemon.h"
#include "images/image_grapes.h"
#include "ImageRoller.h"


void event_handler_checkbox(struct _lv_obj_t * obj, lv_event_t event);
void event_handler_button(struct _lv_obj_t * obj, lv_event_t event);
void init_image_roller();
void mqtt_callback(char* topic, byte* payload, unsigned int length);

unsigned long next_lv_task = 0;

ImageRoller * rip = new ImageRoller();

void event_handler_button(struct _lv_obj_t * obj, lv_event_t event) {
  if(event == LV_EVENT_PRESSED && rip->is_stopped()) {
    rip->start();
  }
}


void init_image_roller() {
  rip->add_image(add_image(&image_cherry, 0, 0, event_handler_button));
  rip->add_image(add_image(&image_lemon, 0, 0, event_handler_button));
  rip->add_image(add_image(&image_grapes, 0, 0, event_handler_button));
}


// ----------------------------------------------------------------------------
// MQTT callback
// ----------------------------------------------------------------------------

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  // Parse Payload into String
  char * buf = (char *)malloc((sizeof(char)*(length+1)));
  memcpy(buf, payload, length);
  buf[length] = '\0';
  String payloadS = String(buf);
  payloadS.trim();

  Serial.println(topic);
  Serial.println(payloadS);

  if(String(topic) == "imageroller/action") {
    if(payloadS == "roll") {
      if(rip->is_stopped()) {
        rip->start();
      }
    }
  }
}


// ----------------------------------------------------------------------------
// UI event handlers
// ----------------------------------------------------------------------------

String buffer = "";
lv_obj_t * mbox;


// ----------------------------------------------------------------------------
// MAIN LOOP
// ----------------------------------------------------------------------------
unsigned long next_image_task = 0;


void loop() {
  if(next_lv_task < millis()) {
    lv_task_handler();
    next_lv_task = millis() + 5;
  }
  if(next_image_task < millis()) {
    rip->next();
    next_image_task = rip->next_timeout();
  }
  // Uncomment the following lines to enable MQTT
  //mqtt_loop();
}

// ----------------------------------------------------------------------------
// MAIN SETUP
// ----------------------------------------------------------------------------

void init_networking() {
  lv_obj_t * wifiConnectingBox = show_message_box_no_buttons("Connecting to WiFi...");
  lv_task_handler();
  delay(5);
  setup_wifi();
  mqtt_init(mqtt_callback);
  close_message_box(wifiConnectingBox);
}

void setup() {
  init_m5();
  init_display();
  Serial.begin(115200);
  
  // Uncomment the following lines to enable WiFi and MQTT (and the mqtt_loop() line in the loop function)
  //init_networking();

  init_image_roller();
  init_sideled();
  rip->start();
}