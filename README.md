# Just Weather - Pebble Watch Face

![Just Weather Screenshot](https://github.com/digitalurban/just-weather-pebble-watchface/blob/main/JustWeather_loading.jpg?raw=true)

**[Get it on the Rebble App Store](https://apps.rebble.io/en_US/application/69034d22d004720008412cf1)**

---

A clean and simple watch face for Pebble smartwatches that provides the current time and essential weather information at a glance.

## What's New in Version 1.4.0

* 📊 **Visual Progress Indicator:** Real-time countdown showing time until next weather update (15-minute cycle)
  * Elegant horizontal line with dot indicators showing elapsed time
  * Customizable through settings - enable/disable as preferred
  * **Enabled by default** to showcase the feature
* 🌤️ **Fresh Weather Data:** Now uses forecast data 15 minutes ahead instead of current conditions for more up-to-date information
* ⏰ **Hourly Vibration Feature:** Optional gentle vibration at the top of each hour for time awareness
* ⚙️ **Enhanced Configurable Settings:** Expanded settings page with new options:
  * Temperature units: Celsius (°C) or Fahrenheit (°F)
  * Wind speed: mph or km/h  
  * Precipitation: millimeters (mm) or inches (in)
  * Hourly vibration: Enable/disable hourly time alerts
  * **Update countdown:** Show/hide the weather update progress indicator
* 🔄 **Dynamic Layout Management:** Progress indicator can be toggled on/off in real-time with perfect spacing
* 🐛 **Enhanced Settings Persistence:** Improved settings loading/saving for reliable preference storage
* 🌤️ **Comprehensive Weather Conditions:** Expanded to 25+ specific weather descriptions from Open-Meteo

## Features

* **Time:** Displays the current time in a large, easy-to-read font with optimized spacing.
* **Weather Update Progress:** Visual countdown indicator showing time until next weather refresh (15-minute cycle):
    * Horizontal progress line with dot indicators  
    * Shows elapsed time since last update with filled/empty dots
    * Perfect spacing that integrates seamlessly with the watch face design
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
* **Fresh Weather Data:** Enhanced data freshness using forecast-based updates:
    * Uses hourly forecast data 15 minutes ahead instead of current conditions
    * Provides more up-to-date weather information throughout the day
    * Visual progress indicator shows time remaining until next update
* **Current Weather:**
    * **Location:** Shows the most relevant local place name for your current position.
    * **Temperature:** Current temperature with weather conditions (displayed in your preferred unit).
    * **Conditions:** Detailed weather descriptions covering 25+ specific conditions from Open-Meteo (Clear Sky, Thunderstorm, Heavy Rain, etc.).
    * **Pressure:** Current atmospheric pressure in hPa, with 3-hour trend indicator showing changes.
    * **Wind & Precipitation:** Current wind speed and precipitation amount (displayed in your preferred units).

## Technical Details

This project consists of two main parts:

1.  **The Watch Face (C):** The native application that runs on the Pebble watch. It is responsible for displaying the UI, managing the layout, and updating the time.
2.  **The Companion App (JavaScript):** A JavaScript application that runs on the connected smartphone. It is responsible for fetching location data and making API calls to get weather information, which it then sends to the watch.

## How to Configure Settings

Access the settings page through your Pebble app or by long-pressing the select button on your watch:

* **Temperature Units:** Choose between Celsius (°C) and Fahrenheit (°F)
* **Wind Speed Units:** Select mph or km/h for wind speed display
* **Precipitation Units:** Pick millimeters (mm) or inches (in) for rainfall measurements
* **Hourly Vibration:** Enable or disable gentle vibration alerts at the top of each hour
* **Update Countdown:** Show or hide the weather update progress indicator (enabled by default)

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
