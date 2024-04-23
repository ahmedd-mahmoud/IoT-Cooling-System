# IoT Cooling System

This project implements an IoT cooling system using ESP8266 microcontroller, ThingSpeak and Node-RED. The system allows you to monitor and control the cooling process remotely.

## Features

- Real-time temperature monitoring
- Automatic cooling control based on temperature thresholds
- Data visualization using Node-RED dashboard

## Prerequisites

Before running the project, make sure you have the following:

- ESP8266 microcontroller
- Temperature sensor (e.g., DHT11, DS18B20)
- ThingSpeak account
- Node-RED installed on your IoT device

## Installation

1. Clone or download this repository to your IoT device.
2. Install the required dependencies by running the following command:
   ```
   npm install
   ```
3. Configure the ThingSpeak channel and API keys in the `config.js` file.
4. Start the Node-RED server by running the following command:
   ```
   node-red
   ```
5. Import the Node-RED flow from the `flows.json` file.
6. Deploy the flow and access the Node-RED dashboard.

## Usage

1. Connect the temperature sensor to your IoT device.
2. Run the Node-RED server and access the dashboard.
3. Monitor the real-time temperature readings on the dashboard.
4. Set the desired temperature thresholds for automatic cooling control.
5. Manually control the cooling system using the dashboard controls.
