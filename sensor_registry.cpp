#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <iomanip>

// Represents a global state registry shared across different robotic threads
class SafeSensorRegistry {
private:
    double m_latest_imu_yaw{0.0};
    double m_latest_lidar_distance{0.0};
    mutable std::mutex m_registry_mutex; // Mutex to protect state across threads [16]

public:
    SafeSensorRegistry() = default;

    // Thread-safe update of sensor data [15, 16]
    void updateTelemetry(double yaw, double lidar_dist) {
        // std::lock_guard locks the mutex on instantiation and automatically 
        // releases it when it goes out of scope (RAII lock management) [16]
        std::lock_guard<std::mutex> lock(m_registry_mutex); 
        m_latest_imu_yaw = yaw;
        m_latest_lidar_distance = lidar_dist;
    }

    // Thread-safe retrieval of sensor data [15, 16]
    void readTelemetry(double& out_yaw, double& out_lidar_dist) const {
        std::lock_guard<std::mutex> lock(m_registry_mutex);
        out_yaw = m_latest_imu_yaw;
        out_lidar_dist = m_latest_lidar_distance;
    }
};

// Thread worker function simulating a high-speed hardware reader (e.g., IMU/LiDAR polling)
void telemetryProducer(SafeSensorRegistry& registry) {
    double mock_yaw = 0.0;
    double mock_dist = 10.0;

    for (int i = 1; i <= 5; ++i) {
        mock_yaw += 5.5;   // Simulate changing orientation
        mock_dist -= 0.5;  // Simulate approaching an obstacle
        
        registry.updateTelemetry(mock_yaw, mock_dist);
        
        std::cout << "[Producer Thread]  Telemetry Sent: Yaw=" << mock_yaw 
                  << "°, Dist=" << mock_dist << "m\n";

        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Poll at 10Hz
    }
}

// Thread worker function simulating a motion controller / path planner consuming data
void navigationConsumer(const SafeSensorRegistry& registry) {
    double current_yaw = 0.0;
    double current_dist = 0.0;

    for (int i = 1; i <= 5; ++i) {
        // Safely pull latest reading
        registry.readTelemetry(current_yaw, current_dist);

        std::cout << "  [Consumer Thread]  Path Planner read: Yaw=" 
                  << current_yaw << "°, Dist=" << std::fixed << std::setprecision(2) 
                  << current_dist << "m\n";

        std::this_thread::sleep_for(std::chrono::milliseconds(120)); // Read at ~8Hz
    }
}

int main() {
    std::cout << "===  Real-Time Multithreaded Robot Controller Started  ===\n\n";

    SafeSensorRegistry shared_registry;

    // Spawn concurrent execution units [14]
    // One thread produces data, while the other consumes it
    std::thread producer(telemetryProducer, std::ref(shared_registry));
    std::thread consumer(navigationConsumer, std::ref(shared_registry));

    // Wait for both threads to safely finish execution before exiting main() [14]
    producer.join();
    consumer.join();

    std::cout << "\n===  Robot Controller Safely Shut Down (No Thread Collisions!)  ===\n";
    return 0;
}
