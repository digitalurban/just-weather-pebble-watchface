# Just Weather - Pebble Watch Face

![Just Weather Screenshot](https://github.com/digitalurban/just-weather-pebble-watchface/blob/main/Screenshot%202025-10-30%20at%2011.36.49.png?raw=true)

**[Get it on the Rebble App Store](https://apps.rebble.io/en_US/application/69034d22d004720008412cf1)**

---

A clean and simple watch face for Pebble smartwatches that provides the current time and essential weather information at a glance.

## Features

* **Time:** Displays the current time in a large, easy-to-read font.
* **Geolocation:** Automatically detects your current location to provide relevant weather data.
* **Current Weather:**
    * **Location:** Shows the name of your current city.
    * **Temperature:** Current temperature in Celsius.
    * **Conditions:** A text description of the current weather (e.g., "Clear", "Partly Cloudy", "Rain").
    * **Pressure:** Current atmospheric pressure in hPa, with a trend indicator.
    * **Wind:** Current wind speed in mph.
    * **Precipitation:** Expected precipitation in mm.

## Technical Details

This project consists of two main parts:

1.  **The Watch Face (C):** The native application that runs on the Pebble watch. It is responsible for displaying the UI, managing the layout, and updating the time.
2.  **The Companion App (JavaScript):** A JavaScript application that runs on the connected smartphone. It is responsible for fetching location data and making API calls to get weather information, which it then sends to the watch.

### APIs Used

* **Weather Data:** [Open-Meteo API](https://open-meteo.com/)
* **Reverse Geocoding:** [BigDataCloud API](https://www.bigdatacloud.com/)

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
