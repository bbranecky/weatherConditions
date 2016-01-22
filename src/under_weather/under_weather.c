#include "under_weather.h"
 
typedef enum {
  OWMWeatherAppMessageKeyRequest = 0,
  OWMWeatherAppMessageKeyReply,
  OWMWeatherAppMessageKeyConditions,
  OWMWeatherAppMessageKeyDescription,
  OWMWeatherAppMessageKeyTempF,
  OWMWeatherAppMessageKeyWindSpeed,
  OWMWeatherAppMessageKeyBadKey = 91,
  OWMWeatherAppMessageKeyLocationUnavailable = 92
} OWMWeatherAppMessageKey;

static OWMWeatherInfo *s_info;
static OWMWeatherCallback *s_callback;
static OWMWeatherStatus s_status;

static char s_api_key[33];

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  /*
  Tuple *tupleM;
  uint8_t i;
  
  for (i=1;i<8;i++){
  //i=1;
    tupleM = dict_find(iter, i);
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "tupleType %d",tupleM->type);
    if (tupleM->type ==3)    APP_LOG(APP_LOG_LEVEL_DEBUG, "i=%d,tuple %d",i,tupleM->value->uint8);
    if (tupleM->type ==1)    APP_LOG(APP_LOG_LEVEL_DEBUG, "i=%d,tuple %s",i,tupleM->value->cstring);
  }
*/
  Tuple *reply_tuple = dict_find(iter, OWMWeatherAppMessageKeyReply);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "OWMWeatherAppMessageKeyReply %d",reply_tuple->value->uint8);
  if(reply_tuple) {
    Tuple *locName_tuple = dict_find(iter, OWMWeatherAppMessageKeyConditions);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "OWMWeatherAppMessageKeyConditions %s",locName_tuple->value->cstring);
    strncpy(s_info->conditions, locName_tuple->value->cstring, OWM_WEATHER_BUFFER_SIZE);

    Tuple *desc_tuple = dict_find(iter, OWMWeatherAppMessageKeyDescription);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "OWMWeatherAppMessageKeyDescription %s",desc_tuple->value->cstring);
    strncpy(s_info->description, desc_tuple->value->cstring, OWM_WEATHER_BUFFER_SIZE);
 
    Tuple *temp_tuple = dict_find(iter, OWMWeatherAppMessageKeyTempF);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "OWMWeatherAppMessageKeyTempF %i",(int)temp_tuple->value->int32);
    s_info->temp_f = temp_tuple->value->int32;
    
    time_t t = time(NULL);
    // struct tm *tmp = gmtime(&t);
    s_info->timestamp = localtime(&t);    
    APP_LOG(APP_LOG_LEVEL_DEBUG, "OWMWeatherAppMessageKeyTimeH %d",(int)s_info->timestamp->tm_hour);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "OWMWeatherAppMessageKeyTimeM %d",(int)s_info->timestamp->tm_min);
 
    Tuple *wind_speed_tuple = dict_find(iter, OWMWeatherAppMessageKeyWindSpeed);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "OWMWeatherAppMessageKeyWindSpeed %i",(int)wind_speed_tuple->value->int32);
    s_info->wind_speed = wind_speed_tuple->value->int32;

    s_status = OWMWeatherStatusAvailable;
    app_message_deregister_callbacks();
    s_callback(s_info, s_status);
  }

  Tuple *err_tuple = dict_find(iter, OWMWeatherAppMessageKeyBadKey);
  if(err_tuple) {
    s_status = OWMWeatherStatusBadKey;
    s_callback(s_info, s_status);
  }

  err_tuple = dict_find(iter, OWMWeatherAppMessageKeyLocationUnavailable);
  if(err_tuple) {
    s_status = OWMWeatherStatusLocationUnavailable;
    s_callback(s_info, s_status);
  }
}

static void fail_and_callback() {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to send request!");
  s_status = OWMWeatherStatusFailed;
  s_callback(s_info, s_status);
}

static bool fetch() {
  DictionaryIterator *out;
  AppMessageResult result = app_message_outbox_begin(&out);
  if(result != APP_MSG_OK) {
    fail_and_callback();
    return false;
  }

  dict_write_cstring(out, OWMWeatherAppMessageKeyRequest, s_api_key);

  result = app_message_outbox_send();
  if(result != APP_MSG_OK) {
    fail_and_callback();
    return false;
  }

  s_status = OWMWeatherStatusPending;
  s_callback(s_info, s_status);
  return true;
}

void owm_weather_init(char *api_key) {
  if(s_info) {
    free(s_info);
  }

  if(!api_key) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "API key was NULL!");
    return;
  }

  strncpy(s_api_key, api_key, sizeof(s_api_key));

  s_info = (OWMWeatherInfo*)malloc(sizeof(OWMWeatherInfo));
  s_status = OWMWeatherStatusNotYetFetched;
}

bool owm_weather_fetch(OWMWeatherCallback *callback) {
  if(!s_info) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "OWM Weather library is not initialized!");
    return false;
  }

  if(!callback) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "OWMWeatherCallback was NULL!");
    return false;
  }

  s_callback = callback;

  if(!bluetooth_connection_service_peek()) {
    s_status = OWMWeatherStatusBluetoothDisconnected;
    s_callback(s_info, s_status);
    return false;
  }

  app_message_deregister_callbacks();
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(2026, 656);

  return fetch();
}

void owm_weather_deinit() {
  if(s_info) {
    free(s_info);
    s_info = NULL;
    s_callback = NULL;
  }
}

OWMWeatherInfo* owm_weather_peek() {
  if(!s_info) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "OWM Weather library is not initialized!");
    return NULL;
  }

  return s_info;
}
