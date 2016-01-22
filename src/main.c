#include <pebble.h>
#include <stdio.h>
#include <string.h>

#include "under_weather/under_weather.h"

static Window *s_window;
static TextLayer *s_text_layer;
static const char AM[3]="AM";
static const char PM[3]="PM";
 
static void weather_callback(OWMWeatherInfo *info, OWMWeatherStatus status) {
  char originalString[] = "Haves;123;Stonebank School, WI;345;";
  char parseStr[5][40];
  int i=0;
  int j=1;
  int indexLoc[5]={-1,0,0,0,0};

  switch(status) {
    case OWMWeatherStatusAvailable:
    {
      static char s_buffer[256];

      char *ampm;
      int dispHr;
      
      while (info->conditions[i] != '\0') {
         if (info->conditions[i]==';') {
            indexLoc[j]=i;
            info->conditions[i]=0;
            memcpy(parseStr[j-1], info->conditions + indexLoc[j-1]+1, indexLoc[j]-indexLoc[j-1]);
            j++;
         }
         i++;
      }

      if (info->timestamp->tm_hour > 12) {
        dispHr = info->timestamp->tm_hour - 12;
        ampm=(char *)PM;
      }
      else {
        dispHr = info->timestamp->tm_hour;
        ampm=(char *)AM;
      }

      snprintf(s_buffer, sizeof(s_buffer),
        "Temp: %ddF\nDescrpt:%s\n%s\n%s\nWind speed: %d\nLocation: \n%s\n%02d:%02d%s",
        info->temp_f, info->description,parseStr[0],parseStr[1], info->wind_speed, info->conditions, dispHr, info->timestamp->tm_min,ampm);
      text_layer_set_text(s_text_layer, s_buffer);
    }
      break;
    case OWMWeatherStatusNotYetFetched:
      text_layer_set_text(s_text_layer, "OWMWeatherStatusNotYetFetched");
      break;
    case OWMWeatherStatusBluetoothDisconnected:
      text_layer_set_text(s_text_layer, "OWMWeatherStatusBluetoothDisconnected");
      break;
    case OWMWeatherStatusPending:
      text_layer_set_text(s_text_layer, "OWMWeatherStatusPending");
      break;
    case OWMWeatherStatusFailed:
      text_layer_set_text(s_text_layer, "OWMWeatherStatusFailed");
      break;
    case OWMWeatherStatusBadKey:
      text_layer_set_text(s_text_layer, "OWMWeatherStatusBadKey");
      break;
    case OWMWeatherStatusLocationUnavailable:
      text_layer_set_text(s_text_layer, "OWMWeatherStatusLocationUnavailable");
      break;
  }
}

static void js_ready_handler(void *context) {
  owm_weather_fetch(weather_callback);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_text_layer, "Down pressed!");
  js_ready_handler(NULL);
}

static void click_config_provider(void *context) {
  // Register the ClickHandlers, BUTTON_ID_UP,BUTTON_ID_SELECT,BUTTON_ID_DOWN
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_text_layer = text_layer_create(PBL_IF_ROUND_ELSE(
    grect_inset(bounds, GEdgeInsets(20, 0, 0, 0)),
    bounds));
  text_layer_set_text(s_text_layer, "Ready.");
  text_layer_set_text_alignment(s_text_layer, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft));
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(s_text_layer);

  window_destroy(window);
}

static void init() {
  char stringBuffer[100];

  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);

  // Replace this with your own API key from OpenWeatherMap.org
  char *api_key = "9120e81acb7c7f27";
  char *api_type = "conditions";
  strcpy(stringBuffer, api_key);
  strcat(stringBuffer, "/");
  strcat(stringBuffer, api_type);
  owm_weather_init(stringBuffer);
    
  window_set_click_config_provider(s_window, click_config_provider);
  
  app_timer_register(3000, js_ready_handler, NULL);
}

static void deinit() { 
  owm_weather_deinit();
}

int main() {
  init();
  app_event_loop();
  deinit();
}
