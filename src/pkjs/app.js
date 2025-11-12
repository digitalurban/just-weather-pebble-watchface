var MessageKeys;
try {
  MessageKeys = require('message_keys');
} catch (e) {
  MessageKeys = null;
}

// --- Settings Management ---
var settings = {
  temperature_unit: 'celsius',
  wind_unit: 'mph',
  precipitation_unit: 'mm',
  hourly_vibration: false,
  update_countdown: true,
  show_steps: false,
  step_unit: 'miles',
  storm_warning: false
};

// Store last weather data for immediate re-sending when units change
var lastWeatherData = null;

// Global function to resend weather data with current units
function resendWeatherWithCurrentUnits() {
  console.log('[JS] resendWeatherWithCurrentUnits called (global scope)');
  console.log('[JS] lastWeatherData exists: ' + (lastWeatherData ? 'YES' : 'NO'));
  console.log('[JS] Current settings: ' + JSON.stringify(settings));
  
  // Use real weather data if available, otherwise use test data
  var useData = lastWeatherData || {
    pressure: 1013,
    temperature: 20, // celsius
    wind: 15, // km/h  
    precipitation: 2.5, // mm
    conditions: 'Settings Update'
  };
  
  console.log('[JS] Sending weather update with current units (real data: ' + (lastWeatherData ? 'YES' : 'NO') + ')...');
  
  // Simple inline message construction and sending with unit conversions
  var dict = {};
  dict[0] = Math.round(useData.pressure); // PRESSURE_KEY = 0
  dict[1] = Math.round(settings.temperature_unit === 'fahrenheit' ? (useData.temperature * 9/5) + 32 : useData.temperature); // TEMPERATURE_KEY = 1
  dict[2] = useData.conditions || 'Settings Update'; // CONDITIONS_KEY = 2 - use real conditions if available
  dict[4] = Math.round(settings.wind_unit === 'mph' ? useData.wind * 0.621371 : useData.wind); // WIND_KEY = 4
  dict[5] = Math.round(settings.precipitation_unit === 'inches' ? useData.precipitation * 0.0393701 * 100 : useData.precipitation * 10); // PRECIP_KEY = 5
  dict[6] = useData.location || 'Settings'; // LOCATION_KEY = 6 - use real location if available
  dict[9] = settings.temperature_unit === 'fahrenheit' ? 'F' : 'C'; // TEMP_UNIT_KEY = 9
  dict[10] = settings.wind_unit === 'mph' ? 'mph' : 'kph'; // WIND_UNIT_KEY = 10
  dict[11] = settings.precipitation_unit === 'inches' ? 'in' : 'mm'; // PRECIP_UNIT_KEY = 11
  dict[12] = settings.hourly_vibration ? 1 : 0; // HOURLY_VIBRATION_KEY = 12
  dict[13] = settings.update_countdown ? 1 : 0; // UPDATE_COUNTDOWN_KEY = 13
  dict[14] = settings.show_steps ? 1 : 0; // SHOW_STEPS_KEY = 14  
  dict[15] = settings.step_unit === 'miles' ? 1 : 0; // STEP_UNIT_KEY = 15
  dict[16] = settings.storm_warning ? 1 : 0; // STORM_WARNING_KEY = 16
  
  console.log('[JS] Test message dict: ' + JSON.stringify(dict));
  
  Pebble.sendAppMessage(dict,
    function() { console.log('[JS] Settings update message sent successfully'); },
    function(e) { console.log('[JS] Settings update message failed: ' + JSON.stringify(e)); }
  );
}

// Load settings from localStorage
function loadSettings() {
  var saved = localStorage.getItem('just_weather_settings');
  if (saved) {
    try {
      var savedSettings = JSON.parse(saved);
      // Merge saved settings with defaults to ensure all properties exist
      if (savedSettings.temperature_unit) settings.temperature_unit = savedSettings.temperature_unit;
      if (savedSettings.wind_unit) settings.wind_unit = savedSettings.wind_unit;
      if (savedSettings.precipitation_unit) settings.precipitation_unit = savedSettings.precipitation_unit;
      if (typeof savedSettings.hourly_vibration !== 'undefined') settings.hourly_vibration = savedSettings.hourly_vibration;
      if (typeof savedSettings.update_countdown !== 'undefined') settings.update_countdown = savedSettings.update_countdown;
      if (typeof savedSettings.show_steps !== 'undefined') settings.show_steps = savedSettings.show_steps;
      if (savedSettings.step_unit) settings.step_unit = savedSettings.step_unit;
      console.log('[JS] Loaded settings:', JSON.stringify(settings));
    } catch (e) {
      console.log('[JS] Error loading settings, using defaults');
    }
  }
}

// Save settings to localStorage
function saveSettings() {
  localStorage.setItem('just_weather_settings', JSON.stringify(settings));
  console.log('[JS] Saved settings:', JSON.stringify(settings));
  
  // Immediately update display with new units when settings are saved
  console.log('[JS] Settings saved - immediately updating display with new units...');
  try {
    console.log('[JS] About to call resend function...');
    
    // Check if the function exists
    if (typeof resendWeatherWithCurrentUnits === 'function') {
      console.log('[JS] resendWeatherWithCurrentUnits function exists, calling it...');
      resendWeatherWithCurrentUnits();
    } else {
      console.log('[JS] ERROR: resendWeatherWithCurrentUnits function not found!');
      console.log('[JS] typeof resendWeatherWithCurrentUnits: ' + typeof resendWeatherWithCurrentUnits);
    }
  } catch (error) {
    console.log('[JS] ERROR calling resend function: ' + error.toString());
  }
}

// Unit conversion functions
function convertTemperature(celsius, targetUnit) {
  if (targetUnit === 'fahrenheit') {
    return (celsius * 9/5) + 32;
  }
  return celsius; // celsius
}

function convertWindSpeed(kmh, targetUnit) {
  if (targetUnit === 'mph') {
    return kmh * 0.621371; // km/h to mph
  }
  return kmh; // kph
}

function convertPrecipitation(mm, targetUnit) {
  if (targetUnit === 'inches') {
    return mm * 0.0393701; // mm to inches
  }
  return mm; // mm
}

function getTemperatureLabel() {
  return settings.temperature_unit === 'fahrenheit' ? 'F' : 'C';
}

function getWindLabel() {
  return settings.wind_unit === 'mph' ? 'mph' : 'kph';
}

function getPrecipitationLabel() {
  return settings.precipitation_unit === 'inches' ? 'in' : 'mm';
}



Pebble.addEventListener('ready', function(e) {
  console.log('[JS] src/pkjs/app.js is ready.');
  
  // Load settings on startup
  loadSettings();
  
  // Start polling for settings changes every 2 seconds
  setInterval(function() {
    var currentSettingsString = JSON.stringify(settings);
    var savedSettingsString = localStorage.getItem('just_weather_settings');
    
    console.log('[JS] Polling check - current: ' + currentSettingsString.substring(0, 50) + '...');
    console.log('[JS] Polling check - saved: ' + (savedSettingsString ? savedSettingsString.substring(0, 50) + '...' : 'NULL'));
    
    if (savedSettingsString && savedSettingsString !== currentSettingsString) {
      console.log('[JS] Settings changed detected! Old: ' + currentSettingsString);
      console.log('[JS] New settings: ' + savedSettingsString);
      
      // Load the new settings
      loadSettings();
      
      // Immediately update display with new units
      console.log('[JS] Immediately updating display with new units...');
      resendWeatherWithCurrentUnits();
    }
  }, 3000);

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
  var TEMP_UNIT_KEY = (MessageKeys && typeof MessageKeys.TEMP_UNIT !== 'undefined') ? MessageKeys.TEMP_UNIT : 9;
  var WIND_UNIT_KEY = (MessageKeys && typeof MessageKeys.WIND_UNIT !== 'undefined') ? MessageKeys.WIND_UNIT : 10;
  var PRECIP_UNIT_KEY = (MessageKeys && typeof MessageKeys.PRECIP_UNIT !== 'undefined') ? MessageKeys.PRECIP_UNIT : 11;
  var HOURLY_VIBRATION_KEY = (MessageKeys && typeof MessageKeys.HOURLY_VIBRATION !== 'undefined') ? MessageKeys.HOURLY_VIBRATION : 12;
  var UPDATE_COUNTDOWN_KEY = (MessageKeys && typeof MessageKeys.UPDATE_COUNTDOWN !== 'undefined') ? MessageKeys.UPDATE_COUNTDOWN : 13;
  var SHOW_STEPS_KEY = (MessageKeys && typeof MessageKeys.SHOW_STEPS !== 'undefined') ? MessageKeys.SHOW_STEPS : 14;
  var STEP_UNIT_KEY = (MessageKeys && typeof MessageKeys.STEP_UNIT !== 'undefined') ? MessageKeys.STEP_UNIT : 15;

  // --- 2. Weather Sending Helper ---
  function sendWeatherToWatch(pressureValue, tempValue, condText, humidityValue, windValue, precipValue, pressureTrend, locationName) {
    console.log('[JS] sendWeatherToWatch called with args:', {p: pressureValue, t: tempValue, w: windValue, pr: precipValue});
    
    // Store the raw data for re-sending when units change
    lastWeatherData = {
      pressure: pressureValue,
      temperature: tempValue, 
      conditions: condText,
      humidity: humidityValue,
      wind: windValue,
      precipitation: precipValue,
      trend: pressureTrend,
      location: locationName
    };
    
    var dict = {};
    if (typeof pressureValue !== 'undefined') dict[PRESSURE_KEY] = Math.round(pressureValue);
    if (typeof pressureTrend !== 'undefined') dict[PRESSURE_TREND_KEY] = pressureTrend.toString();
    
    // Apply unit conversions with debug logging
    if (typeof tempValue !== 'undefined') {
      var convertedTemp = convertTemperature(tempValue, settings.temperature_unit);
      console.log('[JS] Temperature conversion: ' + tempValue + 'C -> ' + convertedTemp + getTemperatureLabel());
      dict[TEMPERATURE_KEY] = Math.round(convertedTemp);
    }
    
    if (typeof condText !== 'undefined') dict[CONDITIONS_KEY] = condText.toString();
    if (typeof humidityValue !== 'undefined') dict[HUMIDITY_KEY] = Math.round(humidityValue);
    
    if (typeof windValue !== 'undefined') {
      var convertedWind = convertWindSpeed(windValue, settings.wind_unit);
      console.log('[JS] Wind conversion: ' + windValue + 'km/h -> ' + convertedWind + getWindLabel());
      dict[WIND_KEY] = Math.round(convertedWind);
    }
    
    if (typeof precipValue !== 'undefined') {
      var convertedPrecip = convertPrecipitation(precipValue, settings.precipitation_unit);
      console.log('[JS] Precip conversion: ' + precipValue + 'mm -> ' + convertedPrecip + getPrecipitationLabel());
      if (settings.precipitation_unit === 'inches') {
        // Send as hundredths of an inch (multiply by 100)
        dict[PRECIP_KEY] = Math.round(convertedPrecip * 100);
      } else {
        // Send as tenths of mm (multiply by 10)
        dict[PRECIP_KEY] = Math.round(convertedPrecip * 10);
      }
    }
    
    if (typeof locationName !== 'undefined') dict[LOCATION_KEY] = locationName.toString();
    
    // Send unit labels so watch displays correctly
    dict[TEMP_UNIT_KEY] = getTemperatureLabel();
    dict[WIND_UNIT_KEY] = getWindLabel();
    dict[PRECIP_UNIT_KEY] = getPrecipitationLabel();
    
    // Send settings
    dict[HOURLY_VIBRATION_KEY] = settings.hourly_vibration ? 1 : 0;
    dict[UPDATE_COUNTDOWN_KEY] = settings.update_countdown ? 1 : 0;
    dict[SHOW_STEPS_KEY] = settings.show_steps ? 1 : 0;
    dict[STEP_UNIT_KEY] = settings.step_unit === 'miles' ? 1 : 0; // 1 = miles, 0 = kilometers
    
    console.log('[JS] Final message with units: temp=' + dict[TEMP_UNIT_KEY] + ', wind=' + dict[WIND_UNIT_KEY] + ', precip=' + dict[PRECIP_UNIT_KEY] + ', vibration=' + dict[HOURLY_VIBRATION_KEY] + ', countdown=' + dict[UPDATE_COUNTDOWN_KEY] + ', steps=' + dict[SHOW_STEPS_KEY] + ', step_unit=' + dict[STEP_UNIT_KEY]);
    console.log('[JS] Calling Pebble.sendAppMessage...');
    
    Pebble.sendAppMessage(dict,
      function() { console.log('[JS] Weather sent SUCCESS: ' + JSON.stringify(dict)); },
      function(e) { console.log('[JS] Weather send FAILED: ' + JSON.stringify(e)); }
    );
    console.log('[JS] sendAppMessage call completed (async)');
  }

  // --- 2.6 Test function to send sample data with current unit settings ---
  function sendTestDataWithCurrentUnits() {
    console.log('[JS] Sending test data with current units: ' + JSON.stringify(settings));
    
    // Always start with base metric values and let the conversion functions handle it
    var baseTempC = 20; // 20C
    var baseWindKmh = 16; // 16 km/h  
    var basePrecipMm = 2.5; // 2.5 mm
    
    console.log('[JS] Base values: temp=' + baseTempC + 'C, wind=' + baseWindKmh + 'kmh, precip=' + basePrecipMm + 'mm');
    console.log('[JS] Target units: temp=' + settings.temperature_unit + ', wind=' + settings.wind_unit + ', precip=' + settings.precipitation_unit);
    
    console.log('[JS] About to call sendWeatherToWatch with converted units...');
    
    try {
      // The sendWeatherToWatch function will convert these to the user's preferred units
      sendWeatherToWatch(
        1013, // pressure
        baseTempC, // temperature in Celsius (will be converted)
        'Test Mode',
        50, // humidity
        baseWindKmh, // wind in km/h (will be converted)
        basePrecipMm, // precip in mm (will be converted)
        '+0.5', // trend
        'Settings Test'
      );
      
      console.log('[JS] sendWeatherToWatch call completed');
    } catch (error) {
      console.log('[JS] ERROR in sendWeatherToWatch: ' + error.toString());
    }
  }

  // --- 3. Weather Mapping ---
  function weatherCodeToString(code) {
    // Open-Meteo WMO Weather interpretation codes (WW)
    // https://open-meteo.com/en/docs
    
    if (code === 0) return 'Clear Sky';
    if (code === 1) return 'Mainly Clear';
    if (code === 2) return 'Partly Cloudy';
    if (code === 3) return 'Overcast';
    
    if (code === 45) return 'Fog';
    if (code === 48) return 'Depositing Rime';
    
    if (code === 51) return 'Light Drizzle';
    if (code === 53) return 'Moderate Drizzle';
    if (code === 55) return 'Dense Drizzle';
    if (code === 56) return 'Light Freezing Drizzle';
    if (code === 57) return 'Dense Freezing Drizzle';
    
    if (code === 61) return 'Slight Rain';
    if (code === 63) return 'Moderate Rain';
    if (code === 65) return 'Heavy Rain';
    if (code === 66) return 'Light Freezing Rain';
    if (code === 67) return 'Heavy Freezing Rain';
    
    if (code === 71) return 'Slight Snow';
    if (code === 73) return 'Moderate Snow';
    if (code === 75) return 'Heavy Snow';
    if (code === 77) return 'Snow Grains';
    
    if (code === 80) return 'Slight Rain Showers';
    if (code === 81) return 'Moderate Rain Showers';
    if (code === 82) return 'Violent Rain Showers';
    if (code === 85) return 'Slight Snow Showers';
    if (code === 86) return 'Heavy Snow Showers';
    
    if (code === 95) return 'Thunderstorm';
    if (code === 96) return 'Thunderstorm w/ Slight Hail';
    if (code === 99) return 'Thunderstorm w/ Heavy Hail';
    
    return 'Unknown (' + code + ')';
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
    // Build API URL to get current data for temperature/pressure/wind, 15-minute forecast for conditions, daily for rainfall
    var url = 'https://api.open-meteo.com/v1/forecast?latitude=' + encodeURIComponent(lat) + '&longitude=' + encodeURIComponent(lon) + 
              '&current=temperature_2m,relative_humidity_2m,wind_speed_10m,surface_pressure' + // Current conditions
              '&minutely_15=weather_code&forecast_minutely_15=1' + // 15-min forecast for conditions only
              '&daily=precipitation_sum&forecast_days=1' + // Daily accumulated rainfall
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
          var windSpeed = undefined;
          var precipitation = undefined;
          var pressure = undefined;
          var trendStr = undefined;

          console.log('[JS] Using current data for temperature/pressure/wind, 15-minute forecast for conditions, daily for rainfall');
          
          // Extract current weather data (most accurate for present conditions)
          if (data.current) {
            var current = data.current;
            if (current.temperature_2m !== undefined) {
              currentTemp = current.temperature_2m;
            }
            if (current.relative_humidity_2m !== undefined) {
              humidity = current.relative_humidity_2m;
            }
            if (current.wind_speed_10m !== undefined) {
              windSpeed = current.wind_speed_10m;
            }
            if (current.surface_pressure !== undefined) {
              pressure = current.surface_pressure;
              console.log('[JS] Current pressure: ' + pressure + ' hPa');
            }
          }
          
          // Extract weather conditions from 15-minute forecast (next 15 minutes)
          if (data.minutely_15 && data.minutely_15.weather_code && data.minutely_15.weather_code.length > 0) {
            currentCondition = weatherCodeToString(data.minutely_15.weather_code[0]);
            console.log('[JS] 15-minute forecast conditions: ' + currentCondition);
          }
            
            // Extract daily accumulated rainfall
            if (data.daily && data.daily.precipitation_sum && data.daily.precipitation_sum.length > 0) {
              precipitation = data.daily.precipitation_sum[0];
              console.log('[JS] Daily accumulated rainfall: ' + precipitation + ' mm');
            }
          
          sendWeatherToWatch(pressure, currentTemp, currentCondition, humidity, windSpeed, precipitation, trendStr, locationName);
          
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

// --- Settings Event Handlers ---
Pebble.addEventListener('showConfiguration', function() {
  // Load settings first to ensure current state is reflected
  loadSettings();
  console.log('[JS] showConfiguration - Current settings: ' + JSON.stringify(settings));
  
  // For now, we'll use a simple data: URL with embedded configuration
  var configHtml = `
<!DOCTYPE html>
<html>
<head><title>Just Weather Settings</title>
<meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1">
<style>
body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;margin:20px;background-color:#f5f5f5;line-height:1.6}
.container{max-width:600px;margin:0 auto;background:white;padding:30px;border-radius:10px;box-shadow:0 2px 10px rgba(0,0,0,0.1)}
h1{color:#333;text-align:center;margin-bottom:30px}
.setting-group{margin-bottom:25px;padding:20px;border:1px solid #e0e0e0;border-radius:8px;background-color:#fafafa}
.setting-label{font-weight:bold;margin-bottom:10px;color:#555}
.radio-group{display:flex;gap:20px;flex-wrap:wrap}
.radio-option{display:flex;align-items:center;gap:8px}
input[type="radio"]{margin:0}
.button-group{text-align:center;margin-top:30px;gap:15px;display:flex;justify-content:center}
button{padding:12px 25px;font-size:16px;border:none;border-radius:6px;cursor:pointer;font-weight:500}
.save-btn{background-color:#4CAF50;color:white}.save-btn:hover{background-color:#45a049}
.cancel-btn{background-color:#f44336;color:white}.cancel-btn:hover{background-color:#da190b}
.description{color:#666;font-size:14px;margin-top:5px;font-style:italic}
</style>
</head>
<body>
<div class="container">
<h1>⛅ Just Weather Settings</h1>
<form id="settingsForm">
<div class="setting-group">
<div class="setting-label">Temperature Units</div>
<div class="radio-group">
<div class="radio-option"><input type="radio" id="temp_celsius" name="temperature_unit" value="celsius" ${settings.temperature_unit === 'celsius' ? 'checked' : ''}><label for="temp_celsius">Celsius (°C)</label></div>
<div class="radio-option"><input type="radio" id="temp_fahrenheit" name="temperature_unit" value="fahrenheit" ${settings.temperature_unit === 'fahrenheit' ? 'checked' : ''}><label for="temp_fahrenheit">Fahrenheit (°F)</label></div>
</div>
<div class="description">Choose how temperature is displayed on your watch</div>
</div>
<div class="setting-group">
<div class="setting-label">Wind Speed Units</div>
<div class="radio-group">
<div class="radio-option"><input type="radio" id="wind_mph" name="wind_unit" value="mph" ${settings.wind_unit === 'mph' ? 'checked' : ''}><label for="wind_mph">Miles per hour (mph)</label></div>
<div class="radio-option"><input type="radio" id="wind_kph" name="wind_unit" value="kph" ${settings.wind_unit === 'kph' ? 'checked' : ''}><label for="wind_kph">Kilometers per hour (km/h)</label></div>
</div>
<div class="description">Choose wind speed measurement unit</div>
</div>
<div class="setting-group">
<div class="setting-label">Precipitation Units</div>
<div class="radio-group">
<div class="radio-option"><input type="radio" id="precip_mm" name="precipitation_unit" value="mm" ${settings.precipitation_unit === 'mm' ? 'checked' : ''}><label for="precip_mm">Millimeters (mm)</label></div>
<div class="radio-option"><input type="radio" id="precip_in" name="precipitation_unit" value="inches" ${settings.precipitation_unit === 'inches' ? 'checked' : ''}><label for="precip_in">Inches (in)</label></div>
</div>
<div class="description">Choose rainfall measurement unit</div>
</div>
<div class="setting-group">
<div class="setting-label">Hourly Vibration</div>
<div class="radio-option">
<input type="checkbox" id="hourly_vibration" name="hourly_vibration" ${settings.hourly_vibration ? 'checked' : ''}>
<label for="hourly_vibration">Vibrate on the hour</label>
</div>
<div class="description">Watch will vibrate briefly at the top of each hour</div>
</div>
<div class="setting-group">
<div class="setting-label">Update Countdown</div>
<div class="radio-option">
<input type="checkbox" id="update_countdown" name="update_countdown" ${settings.update_countdown ? 'checked' : ''}>
<label for="update_countdown">Show weather update progress</label>
</div>
<div class="description">Display a progress line showing time until next weather update (15 minutes)</div>
</div>
<div class="setting-group">
<div class="setting-label">Step Tracking</div>
<div class="radio-option">
<input type="checkbox" id="show_steps" name="show_steps" ${settings.show_steps ? 'checked' : ''}>
<label for="show_steps">Show step count and distance</label>
</div>
<div class="description">Replace wind/precipitation data with daily step count and distance (requires Health permission)</div>
</div>
<div class="setting-group">
<div class="setting-label">Distance Units</div>
<div class="radio-group">
<div class="radio-option"><input type="radio" id="step_miles" name="step_unit" value="miles" ${settings.step_unit === 'miles' ? 'checked' : ''}><label for="step_miles">Miles (mi)</label></div>
<div class="radio-option"><input type="radio" id="step_km" name="step_unit" value="kilometers" ${settings.step_unit === 'kilometers' ? 'checked' : ''}><label for="step_km">Kilometers (km)</label></div>
</div>
<div class="description">Choose distance measurement unit for step tracking</div>
</div>
<div class="setting-group">
<div class="setting-label">⚠️ Storm Warning (Experimental)</div>
<div class="radio-option">
<input type="checkbox" id="storm_warning" name="storm_warning" ${settings.storm_warning ? 'checked' : ''}>
<label for="storm_warning">Enable storm warnings</label>
</div>
<div class="description">Get vibration alerts and watch warnings when barometric pressure drops -3mb in 3 hours, indicating potential severe weather</div>
</div>
<div class="button-group">
<div class="button-group">
<button type="submit" class="save-btn">Save Settings</button>
<button type="button" class="cancel-btn" onclick="document.location='pebblejs://close#'">Cancel</button>
</div>
</form>
</div>
<script>
document.getElementById('settingsForm').addEventListener('submit', function(e) {
  e.preventDefault();
  var settings = {
    temperature_unit: document.querySelector('input[name="temperature_unit"]:checked').value,
    wind_unit: document.querySelector('input[name="wind_unit"]:checked').value,
    precipitation_unit: document.querySelector('input[name="precipitation_unit"]:checked').value,
    hourly_vibration: document.getElementById('hourly_vibration').checked,
    update_countdown: document.getElementById('update_countdown').checked,
    show_steps: document.getElementById('show_steps').checked,
    step_unit: document.querySelector('input[name="step_unit"]:checked').value
  };
  document.location = 'pebblejs://close#' + encodeURIComponent(JSON.stringify(settings));
});
</script>
</body>
</html>`;

  var url = 'data:text/html;charset=utf-8,' + encodeURIComponent(configHtml);
  console.log('[JS] Opening settings page');
  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  try {
    console.log('[JS] webviewclosed event fired, response: ' + (e.response ? 'YES' : 'NO'));
    if (e.response) {
    console.log('[JS] Raw response: ' + e.response);
    try {
      var newSettings = JSON.parse(decodeURIComponent(e.response));
      console.log('[JS] Received new settings: ' + JSON.stringify(newSettings));
      console.log('[JS] Old settings: ' + JSON.stringify(settings));
      
      // Update settings
      if (newSettings.temperature_unit) settings.temperature_unit = newSettings.temperature_unit;
      if (newSettings.wind_unit) settings.wind_unit = newSettings.wind_unit;  
      if (newSettings.precipitation_unit) settings.precipitation_unit = newSettings.precipitation_unit;
      if (typeof newSettings.hourly_vibration !== 'undefined') {
        console.log('[JS] Updating hourly_vibration from ' + settings.hourly_vibration + ' to ' + newSettings.hourly_vibration);
        settings.hourly_vibration = newSettings.hourly_vibration;
      } else {
        console.log('[JS] hourly_vibration not found in new settings');
      }
      if (typeof newSettings.update_countdown !== 'undefined') {
        console.log('[JS] Updating update_countdown from ' + settings.update_countdown + ' to ' + newSettings.update_countdown);
        settings.update_countdown = newSettings.update_countdown;
      } else {
        console.log('[JS] update_countdown not found in new settings');
      }
      if (typeof newSettings.show_steps !== 'undefined') {
        console.log('[JS] Updating show_steps from ' + settings.show_steps + ' to ' + newSettings.show_steps);
        settings.show_steps = newSettings.show_steps;
      } else {
        console.log('[JS] show_steps not found in new settings');
      }
      if (newSettings.step_unit) {
        console.log('[JS] Updating step_unit from ' + settings.step_unit + ' to ' + newSettings.step_unit);
        settings.step_unit = newSettings.step_unit;
      } else {
        console.log('[JS] step_unit not found in new settings');
      }
      if (typeof newSettings.storm_warning !== 'undefined') {
        console.log('[JS] Updating storm_warning from ' + settings.storm_warning + ' to ' + newSettings.storm_warning);
        settings.storm_warning = newSettings.storm_warning;
      } else {
        console.log('[JS] storm_warning not found in new settings');
      }
      
      console.log('[JS] Updated settings: ' + JSON.stringify(settings));
      
      // Save settings
      saveSettings();
      
      // Send a test message with new units to verify they work
      console.log('[JS] Testing units: temp=' + getTemperatureLabel() + ', wind=' + getWindLabel() + ', precip=' + getPrecipitationLabel());
      
      // Immediately resend last weather data with new units
      console.log('[JS] Units changed - resending weather data with new units...');
      resendWeatherWithCurrentUnits();
      
      // Also refresh real weather data after a short delay
      console.log('[JS] Settings updated, refreshing weather data in 2 seconds...');
      setTimeout(function() {
        updatePressure();
      }, 2000);
      
    } catch (ex) {
      console.log('[JS] Error parsing settings response: ' + ex);
      console.log('[JS] Raw response: ' + e.response);
    }
  } else {
    console.log('[JS] Settings cancelled by user');
  }
  } catch (error) {
    console.log('[JS] ERROR in webviewclosed handler: ' + error.toString());
  }
});

console.log('[JS] src/pkjs/app.js loaded.');