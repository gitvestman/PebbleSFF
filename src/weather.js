// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {    
    console.log('PebbleKit JS ready: ' + JSON.stringify(e.playload));
    
    // Get the initial stats and weather
    getFMKStats(function () {
      getWeather();
    });
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received: ' + JSON.stringify(e.playload));
    getFMKStats(function () {
      getWeather();
    });
  }                     
);

Pebble.addEventListener('showConfiguration', function(e) {
  // Show config page
  Pebble.openURL('http://nod.fullmakt.nu/statistics/');
});

Pebble.addEventListener('webviewclosed',
  function(e) {
    var configuration = JSON.parse(decodeURIComponent(e.response));
    console.log('Configuration window returned: ', JSON.stringify(configuration));
  }
);

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function translate(conditions) {
  switch(conditions) {
    case "Clouds":
      return "Mulet";
    case "Clear":
      return "Klart";
    case "Rain":
      return "Regn";
    default:
      return conditions;
  }  
}

function sendToPebble(dictionary, callback) {
  // Send to Pebble
  Pebble.sendAppMessage(dictionary,
    function(e) {
      console.log('Sent dictionary to Pebble successfully!');
      if (typeof callback === 'function') callback();
    },
    function(e) {
      console.log('Error sending dictonary to Pebble!');
      if (typeof callback === 'function') callback();
    }
  );  
}

function locationSuccess(pos) {
  // Construct URL
  var url = 'http://api.openweathermap.org/data/2.5/weather?lang=sv&lat=' +
      pos.coords.latitude + '&lon=' + pos.coords.longitude;
  console.log(url);

  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);
      console.log(responseText);

      // Temperature in Kelvin requires adjustment
      var temperature = Math.round(json.main.temp - 273.15);
      console.log('Temperature is ' + temperature);

      // Conditions
      var conditions = json.weather[0].main;      
      console.log('Conditions are ' + conditions);
      
      // Assemble dictionary using our keys
      var dictionary = {
        'KEY_TEMPERATURE': temperature,
        'KEY_CONDITIONS': translate(conditions)
      };
      
      // Send to Pebble
      sendToPebble(dictionary);
    }      
  );
}

function getInt32Bytes( x ) {
    var bytes = [];
    var i = 4;
    do {
      bytes[--i] = x & (255);
      x = x>>8;
    } while ( i );
    return bytes;
}

function getFMKStats(callback) {
  var url = '';
  console.log(url);

  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);
      console.log(responseText);

      var bestUser = json.BestUser;
      console.log('BestUser is ' + bestUser);
      var bestUserCount = json.BestUserCount;
      console.log('BestUserCount is ' + bestUserCount);
      var dailyTotal = json.DailyTotal;
      console.log('DailyTotal is ' + dailyTotal);      
      //var hourlyStats  = json.HourlyStats;
      var byteArray = [];
      if (json.HourlyStats) {
        for(var i = 0; i < json.HourlyStats.length; i += 2) {
          byteArray.push(json.HourlyStats[i]);
          byteArray.push.apply(byteArray, getInt32Bytes(json.HourlyStats[i+1]));
        }
      }
      //console.log('HourlyStats('+json.HourlyStats.length+'):'+json.HourlyStats[0]+", "+json.HourlyStats[1]);
      //console.log('Array('+byteArray.length+'):'+byteArray[0]+", "+byteArray[1]+", "+byteArray[2]+", "+byteArray[3]+", "+byteArray[4]);
      
      // Assemble dictionary using our keys
      var dictionary = {
        'KEY_BESTUSER': bestUser,
        'KEY_BESTUSERCOUNT': bestUserCount,
        'KEY_DAILYTOTAL': dailyTotal,
        'KEY_HOURLYSTATS': byteArray
      };

      // Send to Pebble
      sendToPebble(dictionary, callback);
    }      
  );
}

function locationError(err) {
  console.log('Error requesting location!');
}

function getWeather(callback) {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}
