# Real-Time Thread-Safe Robotic Sensor Registry
A modular, thread-safe sensor telemetry registry designed to synchronize raw hardware polling cycles with asynchronous consumer navigation loops. This module prevents memory-corrupting data races using modern C++ mutual exclusions (mutexes) and RAII locks.

---

## 🛰️ How It Works

In multi-threaded robotic architectures, high-frequency hardware drivers must write sensor updates concurrently while safety and path planning systems read those same parameters:

```text
   [ Telemetry Producer (100Hz) ]
                │
                ▼ (Acquire Lock)
   ┌───────────────────────────────────┐
   │    SafeSensorRegistry (Mutex)     │ <──  [ Navigation Consumer (80Hz) ]
   └───────────────────────────────────┘      (Acquire Lock / Read State)
                ▲
                │ (Release Lock on Exit)
