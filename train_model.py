import random
import statistics

# --- CONFIGURATION ---
# Flame sensors usually output 0-1023.
# Low value (~100-300) = Close Fire
# High value (~800-1023) = No Fire (Ambient Light)

def generate_sensor_data(n_samples=50):
    data = []
    
    # 1. CLASS: NO_FIRE (All sensors high)
    for _ in range(n_samples):
        l = random.randint(800, 1023)
        c = random.randint(800, 1023)
        r = random.randint(800, 1023)
        data.append((l, c, r, "NO_FIRE"))

    # 2. CLASS: FIRE_LEFT (Left low, others high)
    for _ in range(n_samples):
        l = random.randint(50, 400)  # Fire detected
        c = random.randint(600, 1023)
        r = random.randint(600, 1023)
        data.append((l, c, r, "FIRE_LEFT"))

    # 3. CLASS: FIRE_CENTER (Center low, others high)
    for _ in range(n_samples):
        l = random.randint(600, 1023)
        c = random.randint(50, 400)  # Fire detected
        r = random.randint(600, 1023)
        data.append((l, c, r, "FIRE_CENTER"))

    # 4. CLASS: FIRE_RIGHT (Right low, others high)
    for _ in range(n_samples):
        l = random.randint(600, 1023)
        c = random.randint(600, 1023)
        r = random.randint(50, 400)  # Fire detected
        data.append((l, c, r, "FIRE_RIGHT"))

    return data

def calculate_stats(data, target_class):
    # Filter data for this specific class
    subset = [row for row in data if row[3] == target_class]
    
    # Extract columns
    left_vals = [row[0] for row in subset]
    center_vals = [row[1] for row in subset]
    right_vals = [row[2] for row in subset]
    
    # Calculate Mean and Standard Deviation
    mean_L = statistics.mean(left_vals)
    stdev_L = statistics.stdev(left_vals)
    mean_C = statistics.mean(center_vals)
    stdev_C = statistics.stdev(center_vals)
    mean_R = statistics.mean(right_vals)
    stdev_R = statistics.stdev(right_vals)

    print(f"// Class: {target_class}")
    print(f"const float mean_{target_class}_L = {mean_L:.2f}; const float stdev_{target_class}_L = {stdev_L:.2f};")
    print(f"const float mean_{target_class}_C = {mean_C:.2f}; const float stdev_{target_class}_C = {stdev_C:.2f};")
    print(f"const float mean_{target_class}_R = {mean_R:.2f}; const float stdev_{target_class}_R = {stdev_R:.2f};\n")

# --- MAIN EXECUTION ---
print("Generating synthetic flame sensor data...")
dataset = generate_sensor_data()
print("Training Naive Bayes Model (Calculating Gaussian Stats)...\n")
print("--- COPY THESE VALUES INTO C++ CODE ---")

calculate_stats(dataset, "NO_FIRE")
calculate_stats(dataset, "FIRE_LEFT")
calculate_stats(dataset, "FIRE_CENTER")
calculate_stats(dataset, "FIRE_RIGHT")