# IoT Final Project: Smart Park

A smart IoT-based parking management system designed to monitor and automatize the parking gates and provide analytica via cloud Services such as Quicksight. The project uses ESP32 microcontrollers, and AWS. 

## Table of Contents

- [Hardware Requirements](#hardware-requirements)
- [Software Requirements](#software-requirements)
- [Installation](#installation)
- [Usage](#usage)
- [Project Structure](#project-structure)
- [Contributing](#contributing)
- [License](#license)

---
## Hardware Requirements

- Microcontroller (e.g. ESP32 or similar)
- 4 LDR Sensors
- 2 Servo Gates 

## Software Requirements

- C++ compiler (Arduino IDE, PlatformIO, etc.)
- Required libraries (see code for details):
      -ArduinoJson
      -ESP32Servo
      -PubSubClient
      -WiFiManager
- AWS Account 

## Installation

1. **Clone the repository:**
   ```bash
   git clone https://github.com/FernandoGarcia19/Iot-Final-Smart-Park.git
   cd Iot-Final-Smart-Park
   ```

2. **Open the project in your IDE**  
   (e.g., Arduino IDE or PlatformIO)

3. **Configure hardware connections**  
   Connect sensors and modules according to your hardware setup.
   
4. **Set up your Certificates.h and Config.h**
    Set up these files with your AWS Thing certificates and entry variables for the .ino files 

6. **Upload the code**  
   Upload the relevant `.ino ` (Entrance or Exit) to your microcontroller

## Usage

- Power the microcontroller and connected sensors.
- The system will connect to your AWS IoT Core MQTT Broker
- Sensors and actuators will communicate with the Cloud and receive Orders accordingly

## Project Structure

```
.
├── SmartParking.h     # Core logic for parking management
├── README.md          # Project documentation
└── [Other files...]
```

## Contributing

Contributions are welcome! Please open issues or pull requests for improvements or bug fixes.

## License

[MIT](LICENSE)  
Feel free to use, modify, and distribute this project.

---

**Contact:**  
FernandoGarcia19 on GitHub

---

**Note:**  
Update this README with specific details about your hardware setup, network requirements, and additional features as your project evolves.

---

Let me know if you'd like a more detailed section for setup, wiring diagrams, or code explanations!
