#include <pebble.h>

// We'll use these keys to send data from JS to C
#define MESSAGE_KEY_PRESSURE 0
#define MESSAGE_KEY_TEMPERATURE 1
#define MESSAGE_KEY_CONDITIONS 2
#define MESSAGE_KEY_HUMIDITY 3
#define MESSAGE_KEY_WIND 4
#define MESSAGE_KEY_PRECIP 5
#define MESSAGE_KEY_LOCATION 6
#define MESSAGE_KEY_PRESSURE_TREND 7
// Diagnostic/test key sent from the companion to indicate HTTPS/XHR test result
#define MESSAGE_KEY_PRESSURE_TEST 8
// Unit labels sent from JS to ensure correct display
#define MESSAGE_KEY_TEMP_UNIT 9
#define MESSAGE_KEY_WIND_UNIT 10  
#define MESSAGE_KEY_PRECIP_UNIT 11
#define MESSAGE_KEY_HOURLY_VIBRATION 12

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_location_layer;
static TextLayer *s_temp_cond_layer;
static TextLayer *s_pressure_layer;
static TextLayer *s_wind_precip_layer;
// Icon layer removed: conditions will be shown as text only

// Condition icon code: 0=clear,1=partly,2=cloudy,3=rain,4=snow,5=fog,6=unknown

// Draw a simple 32x32 monochrome icon for the current condition code
// icon_layer removed — using text-only conditions

// A buffer to hold the pressure string, e.g., "1012 hPa"
// Buffers for displayed strings
static char s_pressure_buffer[16];
static char s_temp_cond_buffer[48];
static char s_wind_precip_buffer[40];

// Hourly vibration settings
static bool s_hourly_vibration_enabled = false;
static int s_last_hour = -1;

// --- AppMessage Handlers --- //

// This function runs every time the watch receives a message from the phone
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Find the tuple (data) for our key
  Tuple *pressure_tuple = dict_find(iterator, MESSAGE_KEY_PRESSURE);

  // If the companion sent a diagnostic/test key, show a short status immediately
  Tuple *test_tuple = dict_find(iterator, MESSAGE_KEY_PRESSURE_TEST);
  if (test_tuple) {
    // If it's an integer, 1=success, 0=failure. Otherwise show the raw cstring if present.
    if (test_tuple->type == TUPLE_INT) {
      int v = (int)test_tuple->value->int32;
      if (v) {
        text_layer_set_text(s_pressure_layer, "Conn OK");
      } else {
        text_layer_set_text(s_pressure_layer, "Conn FAIL");
      }
    } else if (test_tuple->type == TUPLE_CSTRING && test_tuple->value->cstring) {
      text_layer_set_text(s_pressure_layer, test_tuple->value->cstring);
    }
    // We don't return here — allow pressure/other keys in same message to still be processed
  }

  if (pressure_tuple) {
    // We have the pressure! Read it as an integer
    int pressure_val = (int)pressure_tuple->value->int32;

    // Log the received pressure for diagnostics
    APP_LOG(APP_LOG_LEVEL_INFO, "inbox_received_callback: received PRESSURE=%d", pressure_val);

    /* If the JS sent a PRESSURE_TREND tuple, prefer to display a short
       arrow next to the pressure (e.g. "1013 hPa ↑"). The JS currently
       sends a string like "↑+0.8hPa". We will append that string (or a
       single arrow derived from an integer trend) after the pressure. */
    char trend_suffix[24] = "";
    Tuple *trend_tuple = dict_find(iterator, MESSAGE_KEY_PRESSURE_TREND);
    if (trend_tuple) {
      if (trend_tuple->type == TUPLE_CSTRING && trend_tuple->value->cstring) {
        // Append a space then the provided trend string (expected like "+0.8")
        snprintf(trend_suffix, sizeof(trend_suffix), " %s", trend_tuple->value->cstring);
      } else if (trend_tuple->type == TUPLE_INT) {
        /* If JS sent an integer tenths-of-hPa value, format it as "+X.Y" */
        int t = (int)trend_tuple->value->int32; /* tenths */
        char sign = (t >= 0) ? '+' : '-';
        int a = t >= 0 ? t : -t;
        snprintf(trend_suffix, sizeof(trend_suffix), " %c%d.%d", sign, a / 10, a % 10);
      }
    }

    // Format the pressure into our buffer with the trend suffix
    snprintf(s_pressure_buffer, sizeof(s_pressure_buffer), "%d hPa%s", pressure_val, trend_suffix);

    // Set the text on our Pressure TextLayer
    text_layer_set_text(s_pressure_layer, s_pressure_buffer);
  } else {
    // Log for diagnosis (the message came in but no PRESSURE tuple was found)
    APP_LOG(APP_LOG_LEVEL_INFO, "inbox_received_callback: no PRESSURE tuple found in incoming message");
  }

  /* Additional tuples: temperature, conditions, location, wind, precip */
  Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_TEMPERATURE);
  Tuple *cond_tuple = dict_find(iterator, MESSAGE_KEY_CONDITIONS);
  Tuple *loc_tuple = dict_find(iterator, MESSAGE_KEY_LOCATION);
  Tuple *wind_tuple = dict_find(iterator, MESSAGE_KEY_WIND);
  Tuple *precip_tuple = dict_find(iterator, MESSAGE_KEY_PRECIP);
  
  APP_LOG(APP_LOG_LEVEL_INFO, "Keys found: temp=%s cond=%s wind=%s precip=%s", 
    temp_tuple ? "YES" : "NO", cond_tuple ? "YES" : "NO", wind_tuple ? "YES" : "NO", precip_tuple ? "YES" : "NO");

  
  // Unit labels
  Tuple *temp_unit_tuple = dict_find(iterator, MESSAGE_KEY_TEMP_UNIT);
  Tuple *wind_unit_tuple = dict_find(iterator, MESSAGE_KEY_WIND_UNIT);
  Tuple *precip_unit_tuple = dict_find(iterator, MESSAGE_KEY_PRECIP_UNIT);

  // Location
  if (loc_tuple && loc_tuple->type == TUPLE_CSTRING) {
    text_layer_set_text(s_location_layer, loc_tuple->value->cstring);
  }

  // Temperature and Conditions (combined display)
  if (temp_tuple && cond_tuple) {
    int temp_val = (int)temp_tuple->value->int32;
    char *cond_str = cond_tuple->value->cstring;
    
    // Get temperature unit from JS (default to C)
    char temp_unit[4] = "C";
    if (temp_unit_tuple && temp_unit_tuple->type == TUPLE_CSTRING && temp_unit_tuple->value->cstring) {
      snprintf(temp_unit, sizeof(temp_unit), "%s", temp_unit_tuple->value->cstring);
    }
    
    if (cond_str) {
      snprintf(s_temp_cond_buffer, sizeof(s_temp_cond_buffer), "%d%s • %s", temp_val, temp_unit, cond_str);
    } else {
      snprintf(s_temp_cond_buffer, sizeof(s_temp_cond_buffer), "%d%s", temp_val, temp_unit);
    }
    text_layer_set_text(s_temp_cond_layer, s_temp_cond_buffer);
  }

  // Wind and Precip (combined display)
  /* Handle wind and precip data - can be integers or strings, and display even if only one is available */
  char wind_display[20] = "";
  char precip_display[20] = "";
  
  // Handle wind data with explicit units
  if (wind_tuple) {
    if (wind_tuple->type == TUPLE_INT) {
      int wind_val = (int)wind_tuple->value->int32;
      
      // Get wind unit from JS (default to mph)
      char wind_unit[8] = "mph";
      if (wind_unit_tuple && wind_unit_tuple->type == TUPLE_CSTRING && wind_unit_tuple->value->cstring) {
        snprintf(wind_unit, sizeof(wind_unit), "%s", wind_unit_tuple->value->cstring);
      }
      
      snprintf(wind_display, sizeof(wind_display), "%d %s", wind_val, wind_unit);
    } else if (wind_tuple->type == TUPLE_CSTRING && wind_tuple->value->cstring) {
      snprintf(wind_display, sizeof(wind_display), "%s", wind_tuple->value->cstring);
    }
  }
  
  // Handle precip data with explicit units
  if (precip_tuple) {
    if (precip_tuple->type == TUPLE_INT) {
      int precip_val = (int)precip_tuple->value->int32;
      
      // Get precip unit from JS (default to mm)
      char precip_unit[8] = "mm";
      if (precip_unit_tuple && precip_unit_tuple->type == TUPLE_CSTRING && precip_unit_tuple->value->cstring) {
        snprintf(precip_unit, sizeof(precip_unit), "%s", precip_unit_tuple->value->cstring);
      }
      
      if (strcmp(precip_unit, "in") == 0) {
        // Handle inches (sent as hundredths) - use integer arithmetic for Pebble
        if (precip_val > 0) {
          int whole = precip_val / 100;
          int frac = precip_val % 100;
          snprintf(precip_display, sizeof(precip_display), "%d.%02d %s", whole, frac, precip_unit);
        } else {
          snprintf(precip_display, sizeof(precip_display), "0 %s", precip_unit);
        }
      } else {
        // Handle mm (sent as tenths) - use integer arithmetic for Pebble  
        if (precip_val > 0) {
          int whole = precip_val / 10;
          int frac = precip_val % 10;
          snprintf(precip_display, sizeof(precip_display), "%d.%d %s", whole, frac, precip_unit);
        } else {
          snprintf(precip_display, sizeof(precip_display), "0 %s", precip_unit);
        }
      }
    } else if (precip_tuple->type == TUPLE_CSTRING && precip_tuple->value->cstring) {
      snprintf(precip_display, sizeof(precip_display), "%s", precip_tuple->value->cstring);
    }
  }
  
  // Display wind and/or precip
  if (strlen(wind_display) > 0 && strlen(precip_display) > 0) {
    snprintf(s_wind_precip_buffer, sizeof(s_wind_precip_buffer), "%s • %s", wind_display, precip_display);
    text_layer_set_text(s_wind_precip_layer, s_wind_precip_buffer);
  } else if (strlen(wind_display) > 0) {
    text_layer_set_text(s_wind_precip_layer, wind_display);
  } else if (strlen(precip_display) > 0) {
    text_layer_set_text(s_wind_precip_layer, precip_display);
  }
  
  // Handle hourly vibration setting
  Tuple *vibration_tuple = dict_find(iterator, MESSAGE_KEY_HOURLY_VIBRATION);
  if (vibration_tuple) {
    if (vibration_tuple->type == TUPLE_INT) {
      s_hourly_vibration_enabled = (vibration_tuple->value->int32 != 0);
      APP_LOG(APP_LOG_LEVEL_INFO, "Hourly vibration setting: %s", 
              s_hourly_vibration_enabled ? "enabled" : "disabled");
    }
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

// --- Clock Update Handler --- //

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  // Use a static buffer so we don't keep it on the stack
  static char s_time_buffer[8]; // "00:00"

  // Format the time into the buffer
  if(clock_is_24h_style()) {
    strftime(s_time_buffer, sizeof(s_time_buffer), "%H:%M", tick_time);
  } else {
    strftime(s_time_buffer, sizeof(s_time_buffer), "%I:%M", tick_time);
  }

  // Set the text on our Time TextLayer
  text_layer_set_text(s_time_layer, s_time_buffer);
  
  // Check for hourly vibration
  if (s_hourly_vibration_enabled) {
    int current_hour = tick_time->tm_hour;
    int current_minute = tick_time->tm_min;
    
    // Vibrate at the top of each hour (minute 0) if hour changed
    if (current_minute == 0 && s_last_hour != current_hour) {
      // Short, subtle vibration
      vibes_short_pulse();
      APP_LOG(APP_LOG_LEVEL_INFO, "Hourly vibration at %d:00", current_hour);
    }
    
    // Update last hour
    s_last_hour = current_hour;
  }
}

// --- Window Load/Unload --- //

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // This is a simple variable to add padding from the top of the screen.
  // Reduce padding to move text up and prevent clipping
  const int vertical_padding = -12;

  // Define heights for each text layer - increased to prevent font clipping
  int time_h = 52; // Height for the time font
  int location_h = 28;  // Increased from 24 to prevent clipping
  int temp_cond_h = 28; // Increased from 24 to prevent clipping  
  int pressure_h = 28;  // Increased from 24 to prevent clipping
  int wind_precip_h = 28; // Increased from 24 to prevent clipping
  int gap = 2; // Gap between data fields

  // --- Time Layer ---
  // Y position starts with padding, but move time down by 5 pixels
  int current_y = vertical_padding + 5;
  s_time_layer = text_layer_create(GRect(0, current_y, bounds.size.w, time_h));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_font(s_time_layer, fonts_get_system_font(
    PBL_IF_COLOR_ELSE(
      FONT_KEY_BITHAM_42_BOLD,
      FONT_KEY_LECO_42_NUMBERS
    )
  ));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  // --- Data Layers ---
  // Reset current_y to original position for data layers (ignore time offset)
  current_y = vertical_padding + time_h;

  // Location
  s_location_layer = text_layer_create(GRect(0, current_y, bounds.size.w, location_h));
  text_layer_set_background_color(s_location_layer, GColorClear);
  text_layer_set_text_color(s_location_layer, GColorBlack);
  text_layer_set_font(s_location_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_location_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(s_location_layer, GTextOverflowModeTrailingEllipsis);
  text_layer_set_text(s_location_layer, "");
  layer_add_child(window_layer, text_layer_get_layer(s_location_layer));
  current_y += location_h + gap;

  // Temperature and conditions
  s_temp_cond_layer = text_layer_create(GRect(0, current_y, bounds.size.w, temp_cond_h));
  text_layer_set_background_color(s_temp_cond_layer, GColorClear);
  text_layer_set_text_color(s_temp_cond_layer, GColorBlack);
  text_layer_set_font(s_temp_cond_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_temp_cond_layer, GTextAlignmentCenter);
  text_layer_set_text(s_temp_cond_layer, "");
  layer_add_child(window_layer, text_layer_get_layer(s_temp_cond_layer));
  current_y += temp_cond_h + gap;

  // Pressure
  s_pressure_layer = text_layer_create(GRect(0, current_y, bounds.size.w, pressure_h));
  text_layer_set_background_color(s_pressure_layer, GColorClear);
  text_layer_set_text_color(s_pressure_layer, GColorBlack);
  text_layer_set_font(s_pressure_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_pressure_layer, GTextAlignmentCenter);
  text_layer_set_text(s_pressure_layer, "Loading..."); // Default text
  layer_add_child(window_layer, text_layer_get_layer(s_pressure_layer));
  current_y += pressure_h + gap;

  // Wind and precipitation
  s_wind_precip_layer = text_layer_create(GRect(0, current_y, bounds.size.w, wind_precip_h));
  text_layer_set_background_color(s_wind_precip_layer, GColorClear);
  text_layer_set_text_color(s_wind_precip_layer, GColorBlack);
  text_layer_set_font(s_wind_precip_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_wind_precip_layer, GTextAlignmentCenter);
  text_layer_set_text(s_wind_precip_layer, "");
  layer_add_child(window_layer, text_layer_get_layer(s_wind_precip_layer));
}

static void main_window_unload(Window *window) {
  // Destroy the TextLayers to free up memory
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_location_layer);
  text_layer_destroy(s_temp_cond_layer);
  text_layer_destroy(s_pressure_layer);
  text_layer_destroy(s_wind_precip_layer);
}

// --- Main App Init/Deinit --- //

static void init() {
  // Create the main Window
  s_main_window = window_create();

  // Set the window's load/unload handlers
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window
  window_stack_push(s_main_window, true /* Animated */);

  // Subscribe to the clock service
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // Register AppMessage handlers
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  
  // Open AppMessage to be ready to receive data
  // Use smaller, reasonable buffer sizes to avoid large heap usage.
  // Our payload is small (several integers and short strings), so 2KB inbox / 512B outbox is sufficient.
  app_message_open(2048, 512);
}

static void deinit() {
  // Destroy the Window
  window_destroy(s_main_window);
}

// The main entry point
int main(void) {
  init();
  app_event_loop();
  deinit();
}