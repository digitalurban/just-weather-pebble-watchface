# Just Weather - Pebble Watch Face

![Just Weather Screenshot](https://github.com/digitalurban/just-weather-pebble-watchface/blob/main/Screenshot%202025-10-30%20at%2011.36.49.png?raw=true)

**[Get it on the Rebble App Store](https://apps.rebble.io/en_US/application/69034d22d004720008412cf1)**

---

A clean and simple watch face for Pebble smartwatches that provides the current time and essential weather information at a glance.

## What's New in Version 1.2.0

* **Enhanced Geolocation System:** Upgraded to Nominatim (OpenStreetMap) for more accurate and detailed location detection
* **Improved Wind & Rain Display:** Fixed display issues - now shows wind and precipitation data reliably at the bottom of the watch face
* **Better Layout & Typography:** Optimized text positioning to prevent font clipping and ensure all data fits properly on screen
* **Enhanced Data Handling:** Improved compatibility with different data formats for more reliable weather updates
* **Smart Location Names:** Prioritizes the most relevant local place names (villages, neighborhoods) over generic city names

## Features

* **Time:** Displays the current time in a large, easy-to-read font with optimized spacing.
* **Enhanced Geolocation:** Automatically detects your current location with improved accuracy:
    * Uses Nominatim (OpenStreetMap) for precise reverse geocoding
    * Smart location detection prioritizing local names (village → hamlet → suburb → town → city → county)
    * Robust error handling with London fallback coordinates
    * Proper timeout handling (10 seconds) for reliable operation
* **Current Weather:**
    * **Location:** Shows the most relevant local place name for your current position.
    * **Temperature:** Current temperature in Celsius with weather conditions.
    * **Conditions:** A text description of the current weather (e.g., "Clear", "Partly Cloudy", "Rain").
    * **Pressure:** Current atmospheric pressure in hPa, with 3-hour trend indicator showing changes.
    * **Wind & Precipitation:** Current wind speed in mph and precipitation amount in mm (displays even if only one is available).

## Technical Details

This project consists of two main parts:

1.  **The Watch Face (C):** The native application that runs on the Pebble watch. It is responsible for displaying the UI, managing the layout, and updating the time.
2.  **The Companion App (JavaScript):** A JavaScript application that runs on the connected smartphone. It is responsible for fetching location data and making API calls to get weather information, which it then sends to the watch.

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
