#!/usr/bin/env python3
"""
Test the step-to-mileage calculation for Pebble watch face
"""

def calculate_distance(steps, use_miles=True):
    """Calculate distance for given steps using the same logic as the C code"""
    
    # Step length: ~0.76 meters per step (as in C code)
    distance_meters = steps * 0.76
    
    if use_miles:
        # Convert to miles: meters * 0.000621371 = miles
        # Then multiply by 10 to store as tenths
        
        # OLD (truncating) version:
        miles_tenths_old = int(distance_meters * 0.000621371 * 10)
        
        # NEW (rounding) version:  
        miles_tenths_new = int(distance_meters * 0.000621371 * 10 + 0.5)
        
        return {
            'steps': steps,
            'meters': distance_meters,
            'miles_exact': distance_meters * 0.000621371,
            'miles_tenths_old': miles_tenths_old,
            'miles_display_old': f"{miles_tenths_old // 10}.{miles_tenths_old % 10}",
            'miles_tenths_new': miles_tenths_new, 
            'miles_display_new': f"{miles_tenths_new // 10}.{miles_tenths_new % 10}"
        }
    else:
        # Kilometers
        km_tenths_old = int(distance_meters / 100.0)
        km_tenths_new = int(distance_meters / 100.0 + 0.5)
        
        return {
            'steps': steps,
            'meters': distance_meters,
            'km_exact': distance_meters / 1000.0,
            'km_tenths_old': km_tenths_old,
            'km_display_old': f"{km_tenths_old // 10}.{km_tenths_old % 10}",
            'km_tenths_new': km_tenths_new,
            'km_display_new': f"{km_tenths_new // 10}.{km_tenths_new % 10}"
        }

if __name__ == "__main__":
    # Test with user's current step count
    steps = 2162
    
    print("=== STEP-TO-MILEAGE CALCULATION TEST ===")
    print(f"Testing with {steps} steps")
    print()
    
    result = calculate_distance(steps, use_miles=True)
    
    print("MILES:")
    print(f"  Steps: {result['steps']}")
    print(f"  Distance in meters: {result['meters']:.1f}m")
    print(f"  Exact miles: {result['miles_exact']:.4f}")
    print(f"  OLD (truncating): {result['miles_display_old']} miles")
    print(f"  NEW (rounding): {result['miles_display_new']} miles")
    print()
    
    # Test a few more values to show the pattern
    test_values = [1000, 1500, 2000, 2162, 2500, 3000]
    
    print("COMPARISON TABLE:")
    print("Steps    | Exact Miles | Old Display | New Display")
    print("---------|-------------|-------------|------------")
    
    for steps in test_values:
        r = calculate_distance(steps, use_miles=True)
        print(f"{steps:8} | {r['miles_exact']:11.4f} | {r['miles_display_old']:11} | {r['miles_display_new']:11}")
    
    print()
    print("ANALYSIS:")
    print("- Your watch shows 1.0 miles for 2162 steps")
    print("- Exact calculation: 1.0206 miles") 
    print("- OLD code (truncating): 1.0 miles ✓ (matches your watch)")
    print("- NEW code (rounding): 1.0 miles ✓ (would still show 1.0)")
    print()
    print("The fix will make mileage more accurate for edge cases")
    print("but won't change your current reading of 1.0 miles.")