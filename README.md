# Just Weather - Pebble Watch Face

![Just Weather Screenshot](https://github.com/digitalurban/just-weather-pebble-watchface/blob/main/JustWeather_loading.jpg?raw=true)

**[Get it on the Rebble App Store](https://apps.rebble.io/en_US/application/69034d22d004720008412cf1)**

---

A clean and simple watch face for Pebble smartwatches that provides the current time and essential weather information at a glance.

## What's New in Version 1.9.0

* 🎨 **Display Refinements:** Enhanced visual layout and text display
  * Fixed step count spacing - distance numbers no longer overlap with shoe icon
  * Removed unnecessary ellipsis from weather conditions (e.g., "Slight Rain" displays without dots)
  * Improved text positioning for better readability at zero steps/distance
  * Cleaner presentation across all display elements

### Previous Updates (Version 1.8.0)

* 🎯 **Optimized Data Sources:** Perfect balance of accuracy and responsiveness
  * **Current data** for temperature, pressure, wind speed, and humidity (most accurate present conditions)
  * **15-minute forecast** for weather conditions only (upcoming weather changes)
  * **Daily accumulated rainfall** for meaningful precipitation totals
  * Eliminates forecast lag for measurements while maintaining predictive conditions
* ⚡ **Enhanced API Efficiency:** Streamlined data retrieval with optimal endpoints
  * Reduced API complexity while maintaining comprehensive weather information
  * Faster response times with current weather measurements
  * More accurate short-term condition forecasting

### Previous Updates (Version 1.7.0)

* 🌧️ **Enhanced Rainfall Display:** Improved precipitation accuracy with daily accumulated rainfall
  * Shows total rainfall accumulated throughout the current day instead of instantaneous rates
  * More meaningful precipitation data for better weather awareness
  * Maintains millimeter/inch unit preferences with accurate daily totals
* 🎯 **Optimized Weather Forecasting:** Enhanced forecast accuracy using proper 15-minute data
  * Uses actual 15-minute forecast data instead of hourly approximations for current conditions
  * Improved weather condition detection with more responsive updates
  * Better alignment between displayed conditions and real-time weather
* ⚡ **Smart API Integration:** Multi-endpoint weather data optimization
  * 15-minute forecast for current conditions (most accurate short-term data)
  * Hourly forecast for atmospheric pressure (appropriate granularity)
  * Daily forecast for accumulated rainfall (meaningful precipitation totals)
  * Reduced API calls while maintaining data accuracy

### Previous Updates (Version 1.6.0)

* �‍♂️ **Step Tracking Integration:** Complete fitness tracking with Health API integration
  * Real-time step count and distance display with minute-by-minute updates
  * Authentic Google Material Design walking icon positioned between step count and distance
  * Miles/kilometers unit selection with consistent decimal formatting (1 decimal place for both units)
  * Show/hide steps toggle setting - replaces wind/precipitation line when enabled
  * Minimal battery impact - leverages existing minute timer for efficient updates
* 🎯 **Smart Display Management:** Dynamic layout switching between weather and fitness data
  * Seamlessly toggles between wind/precipitation and step tracking based on user preference
  * Clean icon positioning in the center of the step display line
  * Optimized text spacing to prevent overlap with the walking icon
* ⚙️ **Enhanced Settings:** New fitness preferences in the settings page:
  * **Show Steps:** Enable/disable step tracking display
  * **Distance Units:** Choose between miles (mi) and kilometers (km) for step distance
  * Real-time settings updates with immediate display refresh

## Features

* **Time:** Displays the current time in a large, easy-to-read font with optimized spacing.
* **Weather Update Progress:** Visual countdown indicator showing time until next weather refresh (15-minute cycle):
    * Horizontal progress line with dot indicators  
    * Shows elapsed time since last update with filled/empty dots
    * Clean spacing that integrates seamlessly with the watch face design
    * **Optional:** Can be disabled for a cleaner look through settings
* **Hourly Vibration:** Optional gentle vibration alert at the top of each hour for time awareness during meetings or focused work.
* **Configurable Units:** Comprehensive settings for personalizing your weather display:
    * **Temperature:** Switch between Celsius (°C) and Fahrenheit (°F)
    * **Wind Speed:** Choose miles per hour (mph) or kilometers per hour (km/h)  
    * **Precipitation:** Select millimeters (mm) or inches (in)
    * **Immediate Updates:** All unit changes take effect instantly on your watch
* **Enhanced Geolocation:** Automatically detects your current location with improved accuracy:
    * Uses Nominatim (OpenStreetMap) for precise reverse geocoding
    * Smart location detection prioritizing local names (village → hamlet → suburb → town → city → county)
    * Robust error handling with London fallback coordinates
    * Proper timeout handling (10 seconds) for reliable operation
* **Fresh Weather Data:** Optimized data sources for maximum accuracy and responsiveness:
    * Current weather data for temperature, pressure, wind speed, and humidity (real-time accuracy)
    * 15-minute forecast for weather conditions to show upcoming changes
    * Daily accumulated rainfall for meaningful precipitation totals
    * Visual progress indicator shows time remaining until next update
* **Current Weather:**
    * **Location:** Shows the most relevant local place name for your current position.
    * **Temperature:** Current temperature with weather conditions (displayed in your preferred unit).
    * **Conditions:** Detailed weather descriptions covering 25+ specific conditions from Open-Meteo (Clear Sky, Thunderstorm, Heavy Rain, etc.).
    * **Pressure:** Current atmospheric pressure in hPa, with 3-hour trend indicator showing changes.
    * **Wind & Precipitation:** Current wind speed and daily accumulated rainfall (displayed in your preferred units).
* **Step Tracking (NEW):**
    * **Step Count:** Real-time daily step counter using Pebble Health API with minute-by-minute updates.
    * **Distance:** Calculated walking distance with consistent formatting (1 decimal place for both miles and kilometers).
    * **Google Material Design Icon:** Authentic directions_walk icon positioned cleanly between step count and distance.
    * **Smart Display:** Replaces wind/precipitation line when step tracking is enabled for clean, focused layout.
    * **Minimal Battery Impact:** Efficient updates leveraging existing minute timer - less than 1% additional battery drain.

## Technical Details

This project consists of two main parts:

1.  **The Watch Face (C):** The native application that runs on the Pebble watch. It is responsible for displaying the UI, managing the layout, and updating the time.
2.  **The Companion App (JavaScript):** A JavaScript application that runs on the connected smartphone. It is responsible for fetching location data and making API calls to get weather information, which it then sends to the watch.

## How to Configure Settings

Access the settings page through your Pebble app or by long-pressing the select button on your watch:

* **Temperature Units:** Choose between Celsius (°C) and Fahrenheit (°F)
* **Wind Speed Units:** Select mph or km/h for wind speed display
* **Precipitation Units:** Pick millimeters (mm) or inches (in) for daily rainfall total display
* **Hourly Vibration:** Enable or disable gentle vibration alerts at the top of each hour
* **Update Countdown:** Show or hide the weather update progress indicator (enabled by default)
* **Step Tracking:** Enable or disable step count and distance display (replaces wind/precipitation when enabled)
* **Distance Units:** Choose between miles (mi) and kilometers (km) for step distance display

All changes take effect immediately with dynamic layout adjustment - no need to restart the app!

### APIs Used

* **Weather Data:** [Open-Meteo API](https://open-meteo.com/) - Provides current weather conditions, atmospheric pressure, wind speed, and precipitation data
* **Reverse Geocoding:** [Nominatim (OpenStreetMap)](https://nominatim.openstreetmap.org/) - Converts GPS coordinates to human-readable location names with high accuracy

## Project Structure

* `src/c/just_weather.c`: The main C source file for the watch face UI and logic.
* `src/pkjs/app.js`: The companion JavaScript app for fetching data.
* `package.json`: Contains project metadata, dependencies, and Pebble-specific settings, such as the app's UUID and target platforms.
* `wscript`: The Python-based build script used by the Pebble SDK to compile and bundle the application.
* `js/message_keys.json`: Defines the keys used for communication between the watch and the phone.

## How to Build

To build and run this watch face, you will need to have the Pebble SDK installed and configured in your environment.

1.  **Build the project:**
    ```bash
    pebble build
    ```

2.  **Install on an emulator (e.g., Basalt):**
    ```bash
    pebble install --emulator basalt
    ```
3.  **View logs:**
    To see logs from both the C application and the JavaScript companion app, run:
    ```bash
    pebble logs
    ```
