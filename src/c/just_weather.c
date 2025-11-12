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
#define MESSAGE_KEY_UPDATE_COUNTDOWN 13
#define MESSAGE_KEY_SHOW_STEPS 14
#define MESSAGE_KEY_STEP_UNIT 15
#define MESSAGE_KEY_STEP_COUNT 16
#define MESSAGE_KEY_STEP_DISTANCE 17
#define MESSAGE_KEY_STORM_WARNING 18

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_location_layer;
static Layer *s_update_progress_layer;
static TextLayer *s_temp_cond_layer;
static TextLayer *s_pressure_layer;
static TextLayer *s_wind_precip_layer;
// Icon layer removed: conditions will be shown as text only

// Condition icon code: 0=clear,1=partly,2=cloudy,3=rain,4=snow,5=fog,6=unknown

// Draw a simple 32x32 monochrome icon for the current condition code
// icon_layer removed — using text-only conditions

// A buffer to hold the pressure string, e.g., "1012 hPa"
// Buffers for displayed strings
static char s_pressure_buffer[32];
static char s_temp_cond_buffer[48];
static char s_wind_precip_buffer[40];

// Hourly vibration setting
static bool s_hourly_vibration_enabled = false;
static bool s_update_countdown_enabled = true;
static bool s_storm_warning_enabled = false; // Experimental storm warning feature
static int s_last_hour = -1;

// Storm warning state tracking
static bool s_storm_warning_active = false;
static int s_last_storm_trend = 0;

// Update progress tracking
static time_t s_last_weather_update = 0;

// Step tracking settings and data
static bool s_show_steps_enabled = false;
static bool s_step_unit_miles = true; // true = miles, false = kilometers
static char s_step_display_buffer[64];
static int s_current_step_count = 0;
static int s_current_step_distance = 0;

// Step icon bitmap resources
static GBitmap *s_shoe_icon_bitmap = NULL;
static BitmapLayer *s_shoe_icon_layer = NULL;

// --- Function Declarations --- //
static void progress_layer_draw(Layer *layer, GContext *ctx);
static int calculate_progress_layer_y_position(void);
static void update_step_data(void);
static void update_step_display(void);
static void load_step_icon(void);
static void unload_step_icon(void);
static void destroy_step_icon_layer(void);

// --- AppMessage Handlers --- //

// This function runs every time the watch receives a message from the phone
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // First, declare all tuples we'll need
  Tuple *pressure_tuple = dict_find(iterator, MESSAGE_KEY_PRESSURE);
  Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_TEMPERATURE);
  Tuple *cond_tuple = dict_find(iterator, MESSAGE_KEY_CONDITIONS);
  Tuple *loc_tuple = dict_find(iterator, MESSAGE_KEY_LOCATION);
  Tuple *wind_tuple = dict_find(iterator, MESSAGE_KEY_WIND);
  Tuple *precip_tuple = dict_find(iterator, MESSAGE_KEY_PRECIP);
  
  // Unit labels from JS
  Tuple *temp_unit_tuple = dict_find(iterator, MESSAGE_KEY_TEMP_UNIT);
  Tuple *wind_unit_tuple = dict_find(iterator, MESSAGE_KEY_WIND_UNIT);  
  Tuple *precip_unit_tuple = dict_find(iterator, MESSAGE_KEY_PRECIP_UNIT);

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

    // Update the weather update timestamp
    s_last_weather_update = time(NULL);
    
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

    // Format the pressure with temperature into our buffer with the trend suffix
    // Get temperature from the global temp value if available
    if (temp_tuple) {
      int temp_val = (int)temp_tuple->value->int32;
      char temp_unit[4] = "C";
      if (temp_unit_tuple && temp_unit_tuple->type == TUPLE_CSTRING && temp_unit_tuple->value->cstring) {
        snprintf(temp_unit, sizeof(temp_unit), "%s", temp_unit_tuple->value->cstring);
      }
      
      // Check for storm warning conditions (experimental feature)
      bool storm_warning = false;
      if (s_storm_warning_enabled && trend_tuple && trend_tuple->type == TUPLE_INT) {
        int trend_tenths = (int)trend_tuple->value->int32;
        // Storm warning: pressure drop of 30+ tenths (3.0+ mb) in 3 hours
        if (trend_tenths <= -30) {
          storm_warning = true;
        }
      }
      
      if (storm_warning) {
        snprintf(s_pressure_buffer, sizeof(s_pressure_buffer), "%d%s • %d mb%s ⚠️", temp_val, temp_unit, pressure_val, trend_suffix);
      } else {
        snprintf(s_pressure_buffer, sizeof(s_pressure_buffer), "%d%s • %d mb%s", temp_val, temp_unit, pressure_val, trend_suffix);
      }
    } else {
      snprintf(s_pressure_buffer, sizeof(s_pressure_buffer), "%d mb%s", pressure_val, trend_suffix);
    }

    // Set the text on our Pressure TextLayer
    text_layer_set_text(s_pressure_layer, s_pressure_buffer);
  } else {
    // Log for diagnosis (the message came in but no PRESSURE tuple was found)
    APP_LOG(APP_LOG_LEVEL_INFO, "inbox_received_callback: no PRESSURE tuple found in incoming message");
  }

  // Temperature and Conditions - now just conditions since temp moved to pressure line
  
  APP_LOG(APP_LOG_LEVEL_INFO, "Keys found: temp=%s cond=%s wind=%s precip=%s", 
    temp_tuple ? "YES" : "NO", cond_tuple ? "YES" : "NO", wind_tuple ? "YES" : "NO", precip_tuple ? "YES" : "NO");

  // Location
  if (loc_tuple && loc_tuple->type == TUPLE_CSTRING) {
    text_layer_set_text(s_location_layer, loc_tuple->value->cstring);
  }

  // Temperature and Conditions - now just conditions since temp moved to pressure line
  // Check for storm warning override (experimental feature)
  bool show_storm_warning = false;
  
  if (s_storm_warning_enabled && pressure_tuple && pressure_tuple->type == TUPLE_INT) {
    Tuple *trend_tuple = dict_find(iterator, MESSAGE_KEY_PRESSURE_TREND);
    if (trend_tuple && trend_tuple->type == TUPLE_INT) {
      int trend_tenths = (int)trend_tuple->value->int32;
      if (trend_tenths <= -50) {
        // Severe storm warning (5.0+ mb drop)
        snprintf(s_temp_cond_buffer, sizeof(s_temp_cond_buffer), "🌩️ SEVERE STORM WARNING");
        show_storm_warning = true;
        
        // Vibrate if this is a new or worsening severe warning
        if (!s_storm_warning_active || trend_tenths < s_last_storm_trend) {
          vibes_double_pulse();
          s_storm_warning_active = true;
          s_last_storm_trend = trend_tenths;
        }
      } else if (trend_tenths <= -30) {
        // Storm warning (3.0+ mb drop)
        snprintf(s_temp_cond_buffer, sizeof(s_temp_cond_buffer), "⚠️ STORM WARNING");
        show_storm_warning = true;
        
        // Vibrate if this is a new warning (only on initial trigger)
        if (!s_storm_warning_active) {
          vibes_double_pulse();
          s_storm_warning_active = true;
          s_last_storm_trend = trend_tenths;
        }
      } else {
        // Reset storm warning state when pressure stabilizes
        s_storm_warning_active = false;
        s_last_storm_trend = 0;
      }
    }
  } else {
    // Reset storm warning state when feature is disabled
    s_storm_warning_active = false;
    s_last_storm_trend = 0;
  }
  
  if (!show_storm_warning) {
    // Normal conditions display
    if (cond_tuple && cond_tuple->type == TUPLE_CSTRING && cond_tuple->value->cstring) {
      char *cond_str = cond_tuple->value->cstring;
      snprintf(s_temp_cond_buffer, sizeof(s_temp_cond_buffer), "%s", cond_str);
    } else {
      snprintf(s_temp_cond_buffer, sizeof(s_temp_cond_buffer), "Loading...");
    }
  }
  
  text_layer_set_text(s_temp_cond_layer, s_temp_cond_buffer);

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
  
  // Handle step tracking settings
  Tuple *show_steps_tuple = dict_find(iterator, MESSAGE_KEY_SHOW_STEPS);
  if (show_steps_tuple) {
    if (show_steps_tuple->type == TUPLE_INT) {
      s_show_steps_enabled = (show_steps_tuple->value->int32 != 0);
      APP_LOG(APP_LOG_LEVEL_INFO, "Show steps setting: %s", 
              s_show_steps_enabled ? "enabled" : "disabled");
      
      // When steps are enabled/disabled, update the display
      update_step_display();
    }
  }
  
  Tuple *step_unit_tuple = dict_find(iterator, MESSAGE_KEY_STEP_UNIT);
  if (step_unit_tuple) {
    if (step_unit_tuple->type == TUPLE_INT) {
      s_step_unit_miles = (step_unit_tuple->value->int32 != 0); // 1 = miles, 0 = kilometers
      APP_LOG(APP_LOG_LEVEL_INFO, "Step unit setting: %s", 
              s_step_unit_miles ? "miles" : "kilometers");
      
      // Update display if steps are enabled
      if (s_show_steps_enabled) {
        update_step_display();
      }
    }
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
  
  // Handle update countdown setting
  Tuple *countdown_tuple = dict_find(iterator, MESSAGE_KEY_UPDATE_COUNTDOWN);
  if (countdown_tuple) {
    if (countdown_tuple->type == TUPLE_INT) {
      bool old_countdown_enabled = s_update_countdown_enabled;
      s_update_countdown_enabled = (countdown_tuple->value->int32 != 0);
      APP_LOG(APP_LOG_LEVEL_INFO, "Update countdown setting: %s", 
              s_update_countdown_enabled ? "enabled" : "disabled");
      
      // If countdown setting changed, adjust the UI dynamically
      if (old_countdown_enabled != s_update_countdown_enabled) {
        APP_LOG(APP_LOG_LEVEL_INFO, "Countdown setting changed - adjusting UI dynamically");
        if (s_update_countdown_enabled) {
          // Add progress layer
          if (!s_update_progress_layer) {
            Layer *window_layer = window_get_root_layer(s_main_window);
            GRect bounds = layer_get_bounds(window_layer);
            // Calculate proper Y position to maintain spacing
            int progress_y = calculate_progress_layer_y_position();
            s_update_progress_layer = layer_create(GRect(0, progress_y, bounds.size.w, 6));
            layer_set_update_proc(s_update_progress_layer, progress_layer_draw);
            layer_add_child(window_layer, s_update_progress_layer);
            
            // Move temperature/conditions layer down to add spacing below progress line
            GRect temp_frame = layer_get_frame(text_layer_get_layer(s_temp_cond_layer));
            int progress_h = 6;
            int gap = 0; // No gap below progress line to move it visually closer to conditions
            temp_frame.origin.y = progress_y + progress_h + gap; // Minimal gap below progress line
            layer_set_frame(text_layer_get_layer(s_temp_cond_layer), temp_frame);
            
            // Also move the remaining layers down by the same amount
            int shift_down = progress_h + gap; // Total space used: progress height + gap below
            GRect pressure_frame = layer_get_frame(text_layer_get_layer(s_pressure_layer));
            pressure_frame.origin.y += shift_down;
            layer_set_frame(text_layer_get_layer(s_pressure_layer), pressure_frame);
            
            GRect wind_frame = layer_get_frame(text_layer_get_layer(s_wind_precip_layer));
            wind_frame.origin.y += shift_down;
            layer_set_frame(text_layer_get_layer(s_wind_precip_layer), wind_frame);
            
            layer_mark_dirty(s_update_progress_layer);
          }
        } else {
          // Remove progress layer and move temperature up to maintain equal spacing
          if (s_update_progress_layer) {
            // Move temperature/conditions to where progress line was (location + 2pt gap)
            GRect location_frame = layer_get_frame(text_layer_get_layer(s_location_layer));
            int gap = 2; // Standard gap used throughout layout
            int new_temp_y = location_frame.origin.y + location_frame.size.h + gap;
            
            // Calculate how much we're moving up from current position
            GRect temp_frame = layer_get_frame(text_layer_get_layer(s_temp_cond_layer));
            int shift_up = temp_frame.origin.y - new_temp_y;
            
            // Move temperature layer to maintain equal spacing
            temp_frame.origin.y = new_temp_y;
            layer_set_frame(text_layer_get_layer(s_temp_cond_layer), temp_frame);
            
            // Move other layers up by the same amount to maintain their relative spacing
            GRect pressure_frame = layer_get_frame(text_layer_get_layer(s_pressure_layer));
            pressure_frame.origin.y -= shift_up;
            layer_set_frame(text_layer_get_layer(s_pressure_layer), pressure_frame);
            
            GRect wind_frame = layer_get_frame(text_layer_get_layer(s_wind_precip_layer));
            wind_frame.origin.y -= shift_up;
            layer_set_frame(text_layer_get_layer(s_wind_precip_layer), wind_frame);
            
            layer_destroy(s_update_progress_layer);
            s_update_progress_layer = NULL;
          }
        }
      }
    }
  }
  
  // Handle storm warning setting (experimental)
  Tuple *storm_warning_tuple = dict_find(iterator, MESSAGE_KEY_STORM_WARNING);
  if (storm_warning_tuple) {
    if (storm_warning_tuple->type == TUPLE_INT) {
      s_storm_warning_enabled = (storm_warning_tuple->value->int32 != 0);
      APP_LOG(APP_LOG_LEVEL_INFO, "Storm warning %s", 
              s_storm_warning_enabled ? "ENABLED" : "disabled");
    }
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

// --- Update Progress Handler --- //

static void progress_layer_draw(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  
  // Draw a thin horizontal line across the width with centered progress dots
  int line_y = bounds.size.h / 2; // Center vertically in the layer
  int margin = 20; // Margins from left and right edges
  int line_width = bounds.size.w - (2 * margin);
  
  // Set drawing color to black
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_fill_color(ctx, GColorBlack);
  
  // Draw the full base line from margin to margin
  graphics_draw_line(ctx, GPoint(margin, line_y), GPoint(margin + line_width, line_y));
  
  // Calculate dot positioning - center 15 dots within the line
  int dots_total_width = 14 * 8; // 14 spaces between 15 dots, 8px apart
  int dots_start_x = margin + (line_width - dots_total_width) / 2; // Center the dots
  
  // Calculate how many dots to fill based on elapsed time
  int filled_dots = 0;
  if (s_last_weather_update != 0) {
    time_t now = time(NULL);
    int elapsed_minutes = (int)((now - s_last_weather_update) / 60);
    filled_dots = elapsed_minutes % 15; // 0-14 filled dots
  }
  
  // Draw progress dots centered on the line
  for (int i = 0; i < 15; i++) {
    int dot_x = dots_start_x + (i * 8); // 8px spacing between dots
    if (i < filled_dots) {
      // Filled dot - small filled circle
      graphics_fill_circle(ctx, GPoint(dot_x, line_y), 2);
    } else {
      // Empty dot - small circle outline
      graphics_draw_circle(ctx, GPoint(dot_x, line_y), 1);
    }
  }
}

static int calculate_progress_layer_y_position(void) {
  // Position progress line for visual centering - match window_load positioning
  GRect location_frame = layer_get_frame(text_layer_get_layer(s_location_layer));
  int gap = 3; // 2pt standard gap + 1pt extra for visual centering
  
  int progress_y = location_frame.origin.y + location_frame.size.h + gap;
  
  return progress_y;
}

static void update_progress_bar() {
  // Just trigger a redraw of the progress layer
  if (s_update_progress_layer) {
    layer_mark_dirty(s_update_progress_layer);
  }
}

// --- Step Tracking Functions --- //

static void update_step_data(void) {
  #if defined(PBL_HEALTH)
  // Get current step count for today
  HealthMetric metric = HealthMetricStepCount;
  HealthValue step_count = health_service_sum_today(metric);
  
  if (step_count >= 0) {
    s_current_step_count = (int)step_count;
    
    // Calculate distance based on step count
    // Average step length assumptions: ~2.5 feet per step = ~0.76 meters
    // So: steps * 0.76 meters = total meters
    // Convert to miles: meters * 0.000621371 = miles
    // Convert to kilometers: meters / 1000 = km
    
    int distance_meters = (int)(s_current_step_count * 0.76f);
    
    if (s_step_unit_miles) {
      // Convert to miles and store as tenths (for 1 decimal place display)
      s_current_step_distance = (int)(distance_meters * 0.000621371f * 10);
    } else {
      // Convert to kilometers and store as tenths
      s_current_step_distance = (int)(distance_meters / 100.0f); // Convert to km then multiply by 10 for tenths
    }
    
    APP_LOG(APP_LOG_LEVEL_INFO, "Steps: %d, Distance: %d (%s)", 
            s_current_step_count, s_current_step_distance, 
            s_step_unit_miles ? "miles*100" : "km*10");
  } else {
    s_current_step_count = 0;
    s_current_step_distance = 0;
    APP_LOG(APP_LOG_LEVEL_WARNING, "Health data unavailable");
  }
  #else
  // Health API not available on this platform
  s_current_step_count = 0;
  s_current_step_distance = 0;
  APP_LOG(APP_LOG_LEVEL_WARNING, "Health API not available on this platform");
  #endif
}

static void update_step_display(void) {
  if (s_show_steps_enabled) {
    // Show step icon
    if (s_shoe_icon_layer) {
      layer_set_hidden(bitmap_layer_get_layer(s_shoe_icon_layer), false);
    }
    
    // Update step data from Health API
    update_step_data();
    
    // Format step display - icon positioned between count and distance
    if (s_step_unit_miles) {
      // Display miles with 1 decimal place - extra spacing to prevent overlap
      int whole_miles = s_current_step_distance / 10;
      int frac_miles = s_current_step_distance % 10;
      snprintf(s_step_display_buffer, sizeof(s_step_display_buffer), 
               " %d        %d.%d mi", s_current_step_count, whole_miles, frac_miles);
    } else {
      // Display kilometers with 1 decimal place - extra spacing to prevent overlap
      int whole_km = s_current_step_distance / 10;
      int frac_km = s_current_step_distance % 10;
      snprintf(s_step_display_buffer, sizeof(s_step_display_buffer), 
               " %d        %d.%d km", s_current_step_count, whole_km, frac_km);
    }
    
    // Update icon position to center it properly in the gap
    if (s_shoe_icon_layer) {
      // Get the wind/precip layer position for vertical alignment
      GRect text_frame = layer_get_frame(text_layer_get_layer(s_wind_precip_layer));
      
      // Position icon in the center of the screen horizontally
      int icon_x = (144 - 16) / 2; // Center the 16px icon horizontally
      
      GRect icon_frame = GRect(icon_x, text_frame.origin.y + 9, 16, 16); // Align with text baseline
      layer_set_frame(bitmap_layer_get_layer(s_shoe_icon_layer), icon_frame);
    }
    
    // Replace the wind/precip layer with step data
    text_layer_set_text(s_wind_precip_layer, s_step_display_buffer);
    APP_LOG(APP_LOG_LEVEL_INFO, "Step display updated: %s", s_step_display_buffer);
  } else {
    // Steps disabled - hide icon and show weather data
    if (s_shoe_icon_layer) {
      layer_set_hidden(bitmap_layer_get_layer(s_shoe_icon_layer), true);
    }
    APP_LOG(APP_LOG_LEVEL_INFO, "Step display disabled - showing weather data");
  }
}

// --- Step Icon Functions --- //

static void load_step_icon(void) {
  if (s_shoe_icon_bitmap == NULL) {
    s_shoe_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SHOE_ICON);
  }
}

static void unload_step_icon(void) {
  if (s_shoe_icon_bitmap) {
    gbitmap_destroy(s_shoe_icon_bitmap);
    s_shoe_icon_bitmap = NULL;
  }
}



static void destroy_step_icon_layer(void) {
  if (s_shoe_icon_layer) {
    bitmap_layer_destroy(s_shoe_icon_layer);
    s_shoe_icon_layer = NULL;
  }
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
  
  // Update the progress bar
  update_progress_bar();
  
  // Update step display every minute if enabled (minimal battery impact)
  if (s_show_steps_enabled) {
    update_step_display();
  }
  
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
  // Move data up by 5 pixels for better spacing
  current_y = vertical_padding + time_h - 5;

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

  // Update Progress Line - positioned for visual centering between location and conditions
  if (s_update_countdown_enabled) {
    int progress_h = 6; // Very thin line with minimal height
    current_y += 1; // Extra 1pt gap above progress line for visual centering
    s_update_progress_layer = layer_create(GRect(0, current_y, bounds.size.w, progress_h));
    layer_set_update_proc(s_update_progress_layer, progress_layer_draw);
    layer_add_child(window_layer, s_update_progress_layer);
    current_y += progress_h; // No gap below - conditions will be placed right after
  } else {
    s_update_progress_layer = NULL; // No progress layer when disabled
  }

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
  
  // Load step icon (will be shown/hidden based on settings)
  load_step_icon();
  
  // Create step icon layer (positioned between step count and distance)
  if (s_shoe_icon_bitmap) {
    // Position icon to the left of center to fit between step count and distance
    GRect bounds = layer_get_bounds(window_layer);
    int icon_x = (bounds.size.w / 2) - 16;  // Offset left from center to avoid overlap
    GRect icon_frame = GRect(icon_x, current_y + 9, 16, 16);  // Align with text baseline
    
    s_shoe_icon_layer = bitmap_layer_create(icon_frame);
    bitmap_layer_set_bitmap(s_shoe_icon_layer, s_shoe_icon_bitmap);
    layer_add_child(window_layer, bitmap_layer_get_layer(s_shoe_icon_layer));
    
    // Hide icon initially (will be shown when steps are enabled)
    layer_set_hidden(bitmap_layer_get_layer(s_shoe_icon_layer), true);
  }
}

static void main_window_unload(Window *window) {
  // Destroy the TextLayers and custom layers to free up memory
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_location_layer);
  if (s_update_progress_layer) {
    layer_destroy(s_update_progress_layer);
  }
  text_layer_destroy(s_temp_cond_layer);
  text_layer_destroy(s_pressure_layer);
  text_layer_destroy(s_wind_precip_layer);
  
  // Clean up step icon resources
  destroy_step_icon_layer();
  unload_step_icon();
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
  
  // Initialize step tracking if enabled
  if (s_show_steps_enabled) {
    update_step_display();
  }
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