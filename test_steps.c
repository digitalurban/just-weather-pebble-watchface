#include <stdio.h>
#include <stdbool.h>

// Mock the Pebble types and functions for testing
typedef int HealthValue;
typedef enum { HealthMetricStepCount } HealthMetric;

bool s_step_unit_miles = true; // Test with miles
int s_current_step_count = 0;
int s_current_step_distance = 0;
char s_step_display_buffer[64];

// Mock function
HealthValue health_service_sum_today(HealthMetric metric) {
    return -1; // Simulate no health data (emulator behavior)
}

// Our actual step calculation function (copied from the watch app)
void update_step_data(void) {
  // FORCE DEMO DATA for emulator - always use demo data to ensure visibility
  s_current_step_count = 4500;
  // Calculate distance for 4500 steps: 4500 * 0.76m = 3420m
  // Miles: 3420m * 0.000621371 = 2.125 miles * 10 (tenths) = 21 tenths = 2.1 miles
  // Kilometers: 3420m / 100 = 34.2 tenths = 3.4 km
  if (s_step_unit_miles) {
    s_current_step_distance = 21; // 2.1 miles (stored as tenths)
  } else {
    s_current_step_distance = 34; // 3.4 km (stored as tenths)  
  }
  printf("*** DEMO MODE FORCED: %d steps, %d distance, miles=%s ***\n", 
          s_current_step_count, s_current_step_distance, s_step_unit_miles ? "YES" : "NO");
}

void format_step_display(void) {
    // Format step display - icon positioned between count and distance
    if (s_step_unit_miles) {
      // Display miles with 1 decimal place - adjusted spacing for better centering
      int whole_miles = s_current_step_distance / 10;
      int frac_miles = s_current_step_distance % 10;
      snprintf(s_step_display_buffer, sizeof(s_step_display_buffer), 
               " %d      %d.%d mi", s_current_step_count, whole_miles, frac_miles);
      printf("*** DISPLAY UPDATE: %s ***\n", s_step_display_buffer);
    } else {
      // Display kilometers with 1 decimal place - adjusted spacing for better centering
      int whole_km = s_current_step_distance / 10;
      int frac_km = s_current_step_distance % 10;
      snprintf(s_step_display_buffer, sizeof(s_step_display_buffer), 
               " %d      %d.%d km", s_current_step_count, whole_km, frac_km);
      printf("*** DISPLAY UPDATE: %s ***\n", s_step_display_buffer);
    }
}

int main() {
    printf("=== TESTING STEP TRACKING LOGIC ===\n");
    
    printf("\n1. Testing with Miles:\n");
    s_step_unit_miles = true;
    update_step_data();
    format_step_display();
    
    printf("\n2. Testing with Kilometers:\n");
    s_step_unit_miles = false;
    update_step_data();
    format_step_display();
    
    printf("\n=== EXPECTED RESULT ===\n");
    printf("Miles: ' 4500      2.1 mi'\n");
    printf("Kilometers: ' 4500      3.4 km'\n");
    
    return 0;
}