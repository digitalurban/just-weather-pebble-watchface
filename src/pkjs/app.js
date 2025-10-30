var MessageKeys;
try {
  MessageKeys = require('message_keys');
} catch (e) {
  MessageKeys = null;
}

Pebble.addEventListener('ready', function(e) {
  console.log('[JS] src/pkjs/app.js is ready.');

  // --- 1. Define Keys ---
  var PRESSURE_KEY = (MessageKeys && typeof MessageKeys.PRESSURE !== 'undefined') ? MessageKeys.PRESSURE : 0;
  var TEMPERATURE_KEY = (MessageKeys && typeof MessageKeys.TEMPERATURE !== 'undefined') ? MessageKeys.TEMPERATURE : 1;
  var CONDITIONS_KEY = (MessageKeys && typeof MessageKeys.CONDITIONS !== 'undefined') ? MessageKeys.CONDITIONS : 2;
  var HUMIDITY_KEY = (MessageKeys && typeof MessageKeys.HUMIDITY !== 'undefined') ? MessageKeys.HUMIDITY : 3;
  var WIND_KEY = (MessageKeys && typeof MessageKeys.WIND !== 'undefined') ? MessageKeys.WIND : 4;
  var PRECIP_KEY = (MessageKeys && typeof MessageKeys.PRECIP !== 'undefined') ? MessageKeys.PRECIP : 5;
  var LOCATION_KEY = (MessageKeys && typeof MessageKeys.LOCATION !== 'undefined') ? MessageKeys.LOCATION : 6;
  var PRESSURE_TREND_KEY = (MessageKeys && typeof MessageKeys.PRESSURE_TREND !== 'undefined') ? MessageKeys.PRESSURE_TREND : 7;
  var TEST_KEY = (MessageKeys && typeof MessageKeys.PRESSURE_TEST !== 'undefined') ? MessageKeys.PRESSURE_TEST : 8;

  // --- 2. Weather Sending Helper ---
  function sendWeatherToWatch(pressureValue, tempValue, condText, humidityValue, windValue, precipTenths, pressureTrend, locationName) {
    var dict = {};
    if (typeof pressureValue !== 'undefined') dict[PRESSURE_KEY] = Math.round(pressureValue);
    if (typeof pressureTrend !== 'undefined') dict[PRESSURE_TREND_KEY] = pressureTrend.toString();
    if (typeof tempValue !== 'undefined') dict[TEMPERATURE_KEY] = Math.round(tempValue);
    if (typeof condText !== 'undefined') dict[CONDITIONS_KEY] = condText.toString();
    if (typeof humidityValue !== 'undefined') dict[HUMIDITY_KEY] = Math.round(humidityValue);
    if (typeof windValue !== 'undefined') dict[WIND_KEY] = Math.round(windValue);
    if (typeof precipTenths !== 'undefined') dict[PRECIP_KEY] = Math.round(precipTenths);
    if (typeof locationName !== 'undefined') dict[LOCATION_KEY] = locationName.toString();
    
    Pebble.sendAppMessage(dict,
      function() { console.log('[JS] Weather sent: ' + JSON.stringify(dict)); },
      function(e) { console.log('[JS] Weather send failed: ' + JSON.stringify(e)); }
    );
  }

  // --- 3. Weather Mapping ---
  function weatherCodeToString(code) {
    if (code === 0) return 'Clear';
    if (code === 1 || code === 2 || code === 3) return 'Partly Cloudy';
    if ([45,48].indexOf(code) !== -1) return 'Fog';
    if ([51,53,55,56,57,61,63,65,66,67,80,81,82].indexOf(code) !== -1) return 'Rain';
    if ([71,73,75,77,85,86].indexOf(code) !== -1) return 'Snow';
    return 'Unknown';
  }

  // --- 3.5. Reverse Geocoding (NEW: Using Nominatim for better accuracy) ---
  function reverseGeocode(lat, lon, callback) {
    // Use Nominatim (OpenStreetMap) for more detailed, local results
    var url = 'https://nominatim.openstreetmap.org/reverse?format=jsonv2' +
              '&lat=' + encodeURIComponent(lat) +
              '&lon=' + encodeURIComponent(lon) +
              '&accept-language=en';
    
    console.log('[JS] Reverse geocoding (Nominatim): ' + url);

    var xhr = new XMLHttpRequest();
    xhr.timeout = 10000; // 10 second timeout
    
    xhr.open('GET', url, true);
    
    // IMPORTANT: Nominatim's usage policy requires a valid User-Agent.
    // We must set this header or we will be blocked.
    xhr.setRequestHeader('User-Agent', 'Just Weather Pebble Face/1.2 (https://github.com/digitalurban/just-weather-pebble-watchface)');

    xhr.onreadystatechange = function() {
      if (xhr.readyState !== 4) return;

      if (xhr.status >= 200 && xhr.status < 300) {
        try {
          var data = JSON.parse(xhr.responseText);
          var locationName = 'Unknown';

          // --- New Parsing Logic for Nominatim ---
          // Try to find the most local name possible
          if (data && data.address) {
            if (data.address.village) {
              locationName = data.address.village; // e.g., "Fincham"
            } else if (data.address.hamlet) {
              locationName = data.address.hamlet;
            } else if (data.address.suburb) {
              locationName = data.address.suburb;
            } else if (data.address.town) {
              locationName = data.address.town;
            } else if (data.address.city) {
              locationName = data.address.city; // e.g., "King's Lynn"
            } else if (data.address.county) {
              locationName = data.address.county;
            }
          } else if (data && data.display_name) {
            // Fallback to just the first part of the full name
            locationName = data.display_name.split(',')[0];
          }
          // --- End of New Parsing Logic ---

          console.log('[JS] Reverse geocoding result: ' + locationName);
          callback(locationName);
        } catch (ex) {
          console.log('[JS] Error parsing reverse geocode response: ' + ex);
          callback('Unknown');
        }
      } else {
        console.log('[JS] Reverse geocode failed: HTTP ' + xhr.status);
        callback('Unknown');
      }
    };

    xhr.ontimeout = function() {
      console.log('[JS] Reverse geocode timeout');
      callback('Unknown');
    };

    xhr.onerror = function(e) {
      console.log('[JS] Reverse geocode error');
      callback('Unknown');
    };
    
    xhr.send();
  }

  // --- 4. Fetch Function (No Promises) ---
  function fetchPressureFromOpenMeteo(lat, lon, locationName) {
    // Build minimized, direct HTTPS URL
    var url = 'https://api.open-meteo.com/v1/forecast?latitude=' + encodeURIComponent(lat) + '&longitude=' + encodeURIComponent(lon) + 
              '&hourly=surface_pressure' + 
              '&current=temperature_2m,weather_code,relative_humidity_2m,wind_speed_10m,precipitation' + 
              '&forecast_hours=24' + // Get 24 hours of forecast for trend
              '&timeformat=unixtime&timezone=auto';
    
    console.log('[JS] Fetching Open-Meteo: ' + url);

    var xhr = new XMLHttpRequest();
    xhr.timeout = 30000; // 30 second timeout

    xhr.onreadystatechange = function() {
      if (xhr.readyState !== 4) return; // Wait for request to be done

      console.log('[JS] XHR readyState=4 status=' + (xhr.status || 0) + ' url=' + url);

      if (xhr.status >= 200 && xhr.status < 300) {
        // SUCCESS
        try {
          var data = JSON.parse(xhr.responseText);
          console.log('[JS] Open-Meteo JSON received and parsed.');

          var currentTemp = undefined;
          var currentCondition = undefined;
          var humidity = undefined;
          var wind_mph = undefined;
          var precipTenths = undefined;
          var pressure = undefined;
          var trendStr = undefined;

          if (data && data.current) {
            currentTemp = data.current.temperature_2m;
            currentCondition = weatherCodeToString(data.current.weather_code);
            humidity = data.current.relative_humidity_2m;
            // Open-Meteo wind_speed_10m is km/h by default. Convert to mph.
            wind_mph = Math.round(data.current.wind_speed_10m * 0.621371); 
            precipTenths = Math.round(data.current.precipitation * 10);
          }

          if (data && data.hourly && data.hourly.surface_pressure && data.hourly.time) {
            var times = data.hourly.time;
            var pressures = data.hourly.surface_pressure;
            if (times.length > 0 && times.length === pressures.length) {
              // Find entry closest to now
              var now = Date.now();
              var bestIdx = 0;
              var bestDiff = Math.abs((times[0] * 1000) - now); // * 1000 because timeformat=unixtime (seconds)
              for (var i = 1; i < times.length; i++) {
                var diff = Math.abs((times[i] * 1000) - now);
                if (diff < bestDiff) {
                  bestDiff = diff;
                  bestIdx = i;
                }
              }
              pressure = pressures[bestIdx];
              console.log('[JS] Open-Meteo pressure (chosen index=' + bestIdx + '): ' + pressure + ' hPa');

              // Calculate 3-hour trend
              var threeHoursAgo = (now / 1000) - (3 * 3600);
              var pastIdx = -1;
              var pastDiff = Infinity;
              for (var j = 0; j < bestIdx; j++) {
                var d = Math.abs(times[j] - threeHoursAgo);
                if (d < pastDiff) {
                  pastDiff = d;
                  pastIdx = j;
                }
              }

              if (pastIdx !== -1) {
                var pastPressure = pressures[pastIdx];
                var trend = pressure - pastPressure;
                trendStr = (trend >= 0 ? '+' : '') + trend.toFixed(1);
                console.log('[JS] Pressure trend (3hr): ' + trend.toFixed(1) + ' hPa');
              }
            }
          }

          sendWeatherToWatch(pressure, currentTemp, currentCondition, humidity, wind_mph, precipTenths, trendStr, locationName);

        } catch (ex) {
          console.log('[JS] Error parsing Open-Meteo response: ' + ex);
          sendWeatherToWatch(1013, 20, 'Parse Error', undefined, undefined, undefined, undefined, locationName); // Send specific error
        }
      } else {
        // FAILURE (status 0, 404, 500, etc.)
        console.log('[JS] Open-Meteo fetch failed: HTTP status ' + xhr.status + ' -- sending fallback');
        sendWeatherToWatch(1013, 20, 'HTTP Fail ' + xhr.status, undefined, undefined, undefined, undefined, locationName); // Send specific error
      }
    };

    xhr.ontimeout = function() {
      console.log('[JS] XHR TIMEOUT after ' + xhr.timeout + 'ms url=' + url);
      sendWeatherToWatch(1013, 20, 'Timeout', undefined, undefined, undefined, undefined, locationName); // Send specific error
    };

    xhr.onerror = function(e) {
      console.log('[JS] XHR ERROR url=' + url);
      sendWeatherToWatch(1013, 20, 'Net Error', undefined, undefined, undefined, undefined, locationName); // Send specific error
    };
    
    xhr.open('GET', url, true);
    xhr.send();
  }

  // --- 5. Geolocation ---
  var DEFAULT_LAT = 51.5074;  // London
  var DEFAULT_LON = -0.1278;

  function updatePressure() {
    if (navigator.geolocation) {
      navigator.geolocation.getCurrentPosition(
        function(position) {
          var lat = position.coords.latitude;
          var lon = position.coords.longitude;
          console.log('[JS] Geolocation success: lat=' + lat + ' lon=' + lon);
          
          // CORRECT: The weather fetch is now INSIDE the callback.
          reverseGeocode(lat, lon, function(locationName) {
            fetchPressureFromOpenMeteo(lat, lon, locationName);
          });
        },
        function(error) {
          console.log('[JS] Geolocation error: ' + error.message + ' -- using default coords');
          fetchPressureFromOpenMeteo(DEFAULT_LAT, DEFAULT_LON, "London");
        },
        { timeout: 10000, enableHighAccuracy: false }
      );
    } else {
      console.log('[JS] Geolocation not available -- using default coords');
      fetchPressureFromOpenMeteo(DEFAULT_LAT, DEFAULT_LON, "London");
    }
  }

  // Initial immediate update
  updatePressure();

  // Then refresh periodically
  setInterval(updatePressure, 15 * 60 * 1000);
});

console.log('[JS] src/pkjs/app.js loaded.');