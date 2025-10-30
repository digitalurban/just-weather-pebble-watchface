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
static char s_location_buffer[32];

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
        int whole = a / 10;
        int tenth = a % 10;
        snprintf(trend_suffix, sizeof(trend_suffix), " %c%d.%d", sign, whole, tenth);
      }
    }

    // Format the string for display (pressure plus optional trend)
    snprintf(s_pressure_buffer, sizeof(s_pressure_buffer), "%d hPa%s", pressure_val, trend_suffix);

    // Set the text on our pressure layer
    text_layer_set_text(s_pressure_layer, s_pressure_buffer);
  } else {
    // Log for diagnostics when the expected key isn't present
    APP_LOG(APP_LOG_LEVEL_INFO, "inbox_received_callback: no PRESSURE tuple found in incoming message");
  }
  // Also handle temperature and conditions (may arrive together or separately)
  Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_TEMPERATURE);
  Tuple *cond_tuple = dict_find(iterator, MESSAGE_KEY_CONDITIONS);

  // Handle temperature and conditions on the same line
  if (temp_tuple && cond_tuple && cond_tuple->value && cond_tuple->value->cstring) {
    int temp_val = (int)temp_tuple->value->int32;
    const char *cond_str = cond_tuple->value->cstring;
    snprintf(s_temp_cond_buffer, sizeof(s_temp_cond_buffer), "%d°C, %s", temp_val, cond_str);
    text_layer_set_text(s_temp_cond_layer, s_temp_cond_buffer);
  } else if (temp_tuple) {
    int temp_val = (int)temp_tuple->value->int32;
    snprintf(s_temp_cond_buffer, sizeof(s_temp_cond_buffer), "%d°C", temp_val);
    text_layer_set_text(s_temp_cond_layer, s_temp_cond_buffer);
  } else if (cond_tuple && cond_tuple->value && cond_tuple->value->cstring) {
    const char *cond_str = cond_tuple->value->cstring;
    snprintf(s_temp_cond_buffer, sizeof(s_temp_cond_buffer), "%s", cond_str);
    text_layer_set_text(s_temp_cond_layer, s_temp_cond_buffer);
  }
  // Location
  Tuple *loc_tuple = dict_find(iterator, MESSAGE_KEY_LOCATION);
  if (loc_tuple && loc_tuple->value && loc_tuple->value->cstring) {
    snprintf(s_location_buffer, sizeof(s_location_buffer), "%s", loc_tuple->value->cstring);
    text_layer_set_text(s_location_layer, s_location_buffer);
  }
  // Also handle wind and precipitation (humidity removed)
  Tuple *wind_tuple = dict_find(iterator, MESSAGE_KEY_WIND);
  Tuple *precip_tuple = dict_find(iterator, MESSAGE_KEY_PRECIP);

  if (wind_tuple && precip_tuple) {
    int wind_val = (int)wind_tuple->value->int32;
    int precip_tenths = (int)precip_tuple->value->int32;
    int whole = precip_tenths / 10;
    int tenth = precip_tenths % 10;
    if (tenth == 0) {
      snprintf(s_wind_precip_buffer, sizeof(s_wind_precip_buffer), "%d mph • %dmm", wind_val, whole);
    } else {
      if (tenth < 0) tenth = -tenth;
      snprintf(s_wind_precip_buffer, sizeof(s_wind_precip_buffer), "%d mph • %d.%dmm", wind_val, whole, tenth);
    }
    text_layer_set_text(s_wind_precip_layer, s_wind_precip_buffer);
  } else if (wind_tuple) {
    int wind_val = (int)wind_tuple->value->int32;
    snprintf(s_wind_precip_buffer, sizeof(s_wind_precip_buffer), "%d mph", wind_val);
    text_layer_set_text(s_wind_precip_layer, s_wind_precip_buffer);
  } else if (precip_tuple) {
    int precip_tenths = (int)precip_tuple->value->int32;
    int whole = precip_tenths / 10;
    int tenth = precip_tenths % 10;
    if (tenth == 0) {
      snprintf(s_wind_precip_buffer, sizeof(s_wind_precip_buffer), "%dmm", whole);
    } else {
      if (tenth < 0) tenth = -tenth;
      snprintf(s_wind_precip_buffer, sizeof(s_wind_precip_buffer), "%d.%dmm", whole, tenth);
    }
    text_layer_set_text(s_wind_precip_layer, s_wind_precip_buffer);
  }
}

// --- Clock Handlers --- //

// This function is called every minute
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  // Create a buffer to hold the time string
  static char s_time_buffer[8]; // "00:00"

  // Format the time into the buffer
  if(clock_is_24h_style()) {
    strftime(s_time_buffer, sizeof(s_time_buffer), "%H:%M", tick_time);
  } else {
    strftime(s_time_buffer, sizeof(s_time_buffer), "%I:%M", tick_time);
  }

  // Set the text on our Time TextLayer
  text_layer_set_text(s_time_layer, s_time_buffer);
}

// --- Window Load/Unload --- //

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create the Time TextLayer (make visually larger by increasing layer height).
  // We'll aim for a tall time area (160px) to make the time much bigger, but
  // clamp it so it never overflows smaller screens. The location will sit
  // directly below the time area and other weather layers will flow after.
    /* Compute available space so the enlarged time area doesn't push other
       UI elements off-screen. Reserve space for location + pressure + temp +
       extra rows and small gaps. Then clamp the time height to a reasonable
       min/max so it still looks good on very small screens. We'll anchor the
       weather/info rows to the bottom so they never get clipped. */
  int location_h = 24;
  int temp_cond_h = 24;
  int pressure_h = 24;
  int wind_precip_h = 24;
    int gap = 2; // gap between elements
    int top_margin = 0;
    int bottom_margin = 0;
  /* Bottom area includes all data layers and bottom margin */
  int bottom_area = location_h + gap + temp_cond_h + gap + pressure_h + gap + wind_precip_h + bottom_margin;
    /* Max time is what's left after reserving bottom area */
    int max_time = (int)bounds.size.h - bottom_area;
    if (max_time < 42) max_time = 42; /* ensure minimum room */
    int desired_time_height = max_time;
    s_time_layer = text_layer_create(GRect(0, top_margin, bounds.size.w, desired_time_height));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  /* Some platforms (older/newer Pebble SDK targets) may not define
     FONT_KEY_BITHAM_50_BOLD. Guard and provide fallbacks so the code
     compiles across platforms (emery/basalt/etc). */
#if defined(FONT_KEY_LECO_42_NUMBERS)
  /* Prefer the LECO numeric font if present — use the 42-number variant */
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS));
  const char *s_font_used = "LECO_42";
#elif defined(FONT_KEY_BITHAM_42_LIGHT) /* BITHAM_50_BOLD is not a standard SDK font */
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
  const char *s_font_used = "BITHAM_42_LIGHT";
#elif defined(FONT_KEY_BITHAM_42_BOLD)
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  const char *s_font_used = "BITHAM_42_BOLD";
#else
  /* Last-resort: use a commonly-available Gothic font */
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  const char *s_font_used = "GOTHIC_28_BOLD";
#endif
  APP_LOG(APP_LOG_LEVEL_INFO, "Time layer height=%d, font=%s", desired_time_height, s_font_used);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  // --- Data Layers ---
  int base_y = desired_time_height - 20; // Move up to create space

  // Location
  s_location_layer = text_layer_create(GRect(0, base_y, bounds.size.w, location_h));
  text_layer_set_background_color(s_location_layer, GColorClear);
  text_layer_set_text_color(s_location_layer, GColorBlack);
  text_layer_set_font(s_location_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_location_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(s_location_layer, GTextOverflowModeTrailingEllipsis);
  text_layer_set_text(s_location_layer, "");
  layer_add_child(window_layer, text_layer_get_layer(s_location_layer));

  // Temperature and conditions
  s_temp_cond_layer = text_layer_create(GRect(0, base_y + location_h + gap, bounds.size.w, temp_cond_h));
  text_layer_set_background_color(s_temp_cond_layer, GColorClear);
  text_layer_set_text_color(s_temp_cond_layer, GColorBlack);
  text_layer_set_font(s_temp_cond_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_temp_cond_layer, GTextAlignmentCenter);
  text_layer_set_text(s_temp_cond_layer, "");
  layer_add_child(window_layer, text_layer_get_layer(s_temp_cond_layer));

  // Pressure
  s_pressure_layer = text_layer_create(GRect(0, base_y + location_h + gap + temp_cond_h + gap, bounds.size.w, pressure_h));
  text_layer_set_background_color(s_pressure_layer, GColorClear);
  text_layer_set_text_color(s_pressure_layer, GColorBlack);
  text_layer_set_font(s_pressure_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_pressure_layer, GTextAlignmentCenter);
  text_layer_set_text(s_pressure_layer, "Loading..."); // Default text
  layer_add_child(window_layer, text_layer_get_layer(s_pressure_layer));

  // Wind and precipitation
  s_wind_precip_layer = text_layer_create(GRect(0, base_y + location_h + gap + temp_cond_h + gap + pressure_h + gap, bounds.size.w, wind_precip_h));
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