/*************************** Weather library start ****************************/

var owmWeatherAPIKey = '';

function owmWeatherXHR(url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
}

function owmWeatherSendToPebble(json) {
  var typeOfCall = '';
  typeOfCall = owmWeatherAPIKey.substr(17,owmWeatherAPIKey.length);
  console.log(owmWeatherAPIKey.substr(17,owmWeatherAPIKey.length));
  
  if (typeOfCall == 'conditions'){
    Pebble.sendAppMessage({
      'OWMWeatherAppMessageKeyReply': 1,
      'OWMWeatherAppMessageKeyConditions': json.current_observation.observation_location.city +';'+
      json.current_observation.temp_f+';'+json.current_observation.wind_mph,
      'OWMWeatherAppMessageKeyDescription': json.current_observation.weather,
      'OWMWeatherAppMessageKeyTempF': Math.round(json.current_observation.temp_f),
      'OWMWeatherAppMessageKeyWindSpeed': Math.round(json.current_observation.wind_mph),
    });    
  }
  
}

function owmWeatherLocationSuccess(pos) {
  var url = 'http://api.wunderground.com/api/' + owmWeatherAPIKey + '/q/' +
    pos.coords.latitude + ',' + pos.coords.longitude + '.json' ;
  console.log('owm-weather: Location success. Contacting OpenWeatherMap.org...');
  console.log( url);

  owmWeatherXHR(url, 'GET', function(responseText) {
    console.log('owm-weather: Got API response!');
    if(responseText.length > 100) {
      owmWeatherSendToPebble(JSON.parse(responseText));
    } else {
      console.log('owm-weather: API response was bad. Wrong API key?');
      Pebble.sendAppMessage({
        'OWMWeatherAppMessageKeyBadKey': 1
      });
    }
  });
}

function owmWeatherLocationError(err) {
  console.log('owm-weather: Location error');
  Pebble.sendAppMessage({
    'OWMWeatherAppMessageKeyLocationUnavailable': 1
  });
}

function owmWeatherHandler(dict) {
  if(dict.payload['OWMWeatherAppMessageKeyRequest']) {
    owmWeatherAPIKey = dict.payload['OWMWeatherAppMessageKeyRequest'];
    console.log('owm-weather: Got fetch request from C app');
    
    navigator.geolocation.getCurrentPosition(owmWeatherLocationSuccess, owmWeatherLocationError, {
      timeout: 15000,
      maximumAge: 60000
    });
  }
}

/**************************** Weather library end *****************************/

Pebble.addEventListener('ready', function(e) {
  console.log('PebbleKit JS ready!');
});

Pebble.addEventListener('appmessage', function(e) {
  console.log('appmessage: ' + JSON.stringify(e.payload));
  owmWeatherHandler(e);
});
