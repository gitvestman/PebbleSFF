// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log('PebbleKit JS ready!');
    
    // Get the initial weather
    getWeather();
    getPPFundList();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received!');
    getWeather();
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
      var iconurl = 'http://openweathermap.org/img/w/' + json.weather[0].icon + '.png';
      console.log(iconurl);

      // Temperature in Kelvin requires adjustment
      var temperature = Math.round(json.main.temp - 273.15);
      console.log('Temperature is ' + temperature);

      // Conditions
      var conditions = json.weather[0].main;      
      console.log('Conditions are ' + conditions);
      
      // Assemble dictionary using our keys
      var dictionary = {
        'KEY_TEMPERATURE': temperature,
        'KEY_CONDITIONS': conditions
      };
      
      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log('Weather info sent to Pebble successfully!');
        },
        function(e) {
          console.log('Error sending weather info to Pebble!');
        }
      );
    }      
  );
}

function getPPFundList() {
  // Construct URL
  var url = 'http://demo.fullmakt.nu/ppp_individ_ios/services/PrivateMobileService.asmx';
  
  var req = new XMLHttpRequest();
  req.open('POST', url, true);
  req.onload = function(e) {
    if (req.readyState == 4 && req.status == 200) {
      if(req.status == 200) {
        console.log(req.responseText);
      } else { console.log('Error'); }
    }
  };
  req.setRequestHeader("Content-type","text/xml; charset-utf-8");
  req.setRequestHeader('SOAPAction','http://pppension.se/GetPPFundList');
  req.send(
  '<?xml version="1.0" encoding="utf-8"?>' + 
  '<soap12:Envelope xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:soap12="http://www.w3.org/2003/05/soap-envelope">'+
    '<soap12:Body>'+
      '<GetPPFundList xmlns="http://pppension.se/">'+
        '<product>ITP2</product>'+
      '</GetPPFundList>'+
    '</soap12:Body>' +
  '</soap12:Envelope>');
}

function locationError(err) {
  console.log('Error requesting location!');
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}
