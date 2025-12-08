# Just Weather - Pebble Watch Face

![Just Weather Screenshot](https://github.com/digitalurban/just-weather-pebble-watchface/blob/main/JustWeather_loading.jpg?raw=true)

**[Get it on the Rebble App Store](https://apps.rebble.io/en_US/application/69034d22d004720008412cf1)**

---

A clean and simple watch face for Pebble smartwatches that provides the current time and essential weather information at a glance.

## What's New in Version 3.0.0

* **Storm Warning System:** Barometric pressure monitoring for weather alerts
  * Monitors pressure changes over 3-hour periods using historical data tracking
  * Vibration alerts when pressure drops 3mb or more in 3 hours
  * Displays warning message during significant pressure events
  * Optional setting to enable/disable storm warnings
* **Improved Display Layout:** Better temperature display for longer location names
  * Temperature shows on location line for short location names
  * Temperature moves to pressure line for longer location names to prevent overflow
  * Maintains pressure trend information on short locations
* **Enhanced Stability:** Variable scope improvements and boundary condition fixes

### Previous Updates (Version 2.3.0)

* Pressure drop detection with 3-hour historical tracking
* Smart alert logic to reduce unnecessary vibrations
* Visual storm warning indicators on the display

### Previous Updates (Version 2.0.0)

### Previous Updates (Version 2.0.0)

* **Smart Layout Optimization:** Improved display layout for better information organization
  * Temperature and atmospheric pressure displayed together
  * Weather conditions have dedicated display space
  * Pressure trend information maintained
  * Visual updates for improved readability
* **Data Source Optimization:** Uses current weather data for temperature, pressure, wind and humidity with 15-minute forecasts for conditions

### Previous Updates (Version 1.9.0)

* **Display Improvements:** Enhanced visual layout and text display
  * Improved step count spacing
  * Cleaner weather condition text presentation
  * Better text positioning throughout the interface

### Previous Updates (Version 1.8.0)

* **Data Source Selection:** Uses current weather data for real-time accuracy with 15-minute forecasts for conditions and daily rainfall totals

### Previous Updates (Version 1.7.0)

* **Rainfall Display:** Shows daily accumulated rainfall instead of instantaneous rates
* **Weather Forecasting:** Uses 15-minute forecast data for weather conditions
* **Multi-Endpoint API:** Optimized use of different weather data endpoints for appropriate granularity

### Previous Updates (Version 1.6.0)

* **Step Tracking Integration:** Real-time step count and distance display using Health API
  * Step count updates with minute-by-minute frequency
  * Distance display with consistent formatting
  * Miles and kilometers unit selection
  * Show/hide steps toggle setting
* **Smart Display Management:** Toggles between weather and fitness data based on user preference
* **Enhanced Settings:** New fitness preferences for step tracking display and distance units

## Features

* **Time:** Displays the current time in a large, easy-to-read font.
* **Weather Update Progress:** Visual indicator showing time until next weather refresh (15-minute cycle).
* **Hourly Vibration:** Optional vibration alert at the top of each hour.
* **Configurable Units:** Settings for temperature, wind speed, and precipitation units with immediate updates.
* **Geolocation:** Automatically detects your current location and displays the local place name.
* **Weather Data:** Current weather including temperature, conditions, pressure with trend, wind speed, and precipitation.
* **Step Tracking:** Real-time step count and distance display with mile/kilometer selection.
* **Storm Warnings:** Optional pressure-based alerts for significant pressure changes.

## Technical Details

This project consists of two main parts:

1.  **The Watch Face (C):** The native application that runs on the Pebble watch. It is responsible for displaying the UI, managing the layout, and updating the time.
2.  **The Companion App (JavaScript):** A JavaScript application that runs on the connected smartphone. It is responsible for fetching location data and making API calls to get weather information, which it then sends to the watch.

## How to Configure Settings

Access the settings page through your Pebble app:

* **Temperature Units:** Celsius (°C) or Fahrenheit (°F)
* **Wind Speed Units:** mph or km/h
* **Precipitation Units:** mm or inches
* **Hourly Vibration:** Enable or disable hourly alerts
* **Update Countdown:** Show or hide the weather update progress indicator
* **Step Tracking:** Enable or disable step count display
* **Distance Units:** miles (mi) or kilometers (km)
* **Storm Warnings:** Enable optional pressure-based weather alerts

Changes take effect immediately on your watch.

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
