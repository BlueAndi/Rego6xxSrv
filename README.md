# Rego6xx Server
A REST API server which provides control over IVT heatpumps, which uses a Rego6xx controller.
It runs on the AVR-NET-IO board from Pollin.

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)
[![Repo Status](https://www.repostatus.org/badges/latest/wip.svg)](https://www.repostatus.org/#wip)
[![Release](https://img.shields.io/github/release/BlueAndi/Rego6xxSrv.svg)](https://github.com/BlueAndi/Rego6xxSrv/releases)
[![PlatformIO CI](https://github.com/BlueAndi/Rego6xxSrv/workflows/PlatformIO%20CI/badge.svg?branch=master)](https://github.com/BlueAndi/Rego6xxSrv/actions?query=workflow%3A%22PlatformIO+CI%22)

- [Rego6xx Server](#rego6xx-server)
- [Motivation](#motivation)
- [Electronic](#electronic)
- [Software](#software)
  - [IDE](#ide)
  - [Installation](#installation)
  - [Install bootloader](#install-bootloader)
  - [Build Project](#build-project)
  - [Update of the device](#update-of-the-device)
    - [Update via serial interface](#update-via-serial-interface)
  - [Used Libraries](#used-libraries)
- [REST API](#rest-api)
  - [Get temperature value (GET /api/sensors/<sensor>)](#get-temperature-value-get-apisensorssensor)
  - [Set temperature value (POST /api/sensors)](#set-temperature-value-post-apisensors)
  - [Send raw command (POST /api/debug)](#send-raw-command-post-apidebug)
  - [Get last error information (GET /api/lastError)](#get-last-error-information-get-apilasterror)
  - [Get frontpanel LED state (GET /api/frontPanel/<led>)](#get-frontpanel-led-state-get-apifrontpanelled)
- [Issues, Ideas And Bugs](#issues-ideas-and-bugs)
- [License](#license)
- [Contribution](#contribution)

# Motivation
I have a photovoltaic system (PV) on the roof of the house. If the PV provides enough energy, the idea is to store it in form of hot water with the heatpump.
Unfortunately my Junkers heatpump (original build by IVT) has no interface to do that. Therefore the service interface of the build-in reg6xx controller shall be used to change the target temperature of hot water.

The installed SolarEdge inverter provides a modbus tcp interface, which is used to retrieve how much energy is harvested. Additional there are s0 counter to retrieve the power consumption of the house.

The protocol for the Rego6xx controller was derived from http://rago600.sourceforge.net/.

# Electronic

* [Pollin AVR-NET-IO](https://www.pollin.de/p/avr-net-io-fertigmodul-810073)

Note, the AVR-NET-IO has original a ATmega32-16PU, which was replaced with a ATmega644p.

# Software

## IDE
The [PlatformIO IDE](https://platformio.org/platformio-ide) is used for the development. Its build on top of Microsoft Visual Studio Code.

## Installation
1. Install [VSCode](https://code.visualstudio.com/).
2. Install PlatformIO IDE according to this [HowTo](https://platformio.org/install/ide?install=vscode).
3. Close and start VSCode again.
4. Recommended is to take a look to the [quick-start guide](https://docs.platformio.org/en/latest/ide/vscode.html#quick-start).

## Install bootloader
Note, for the following steps I used the AtmelStudio v7.0 with my AVR-ISP programmer, because I didn't manage to get this setup work with VSCode + Platformio or the Arduino IDE.
1. Set fuse bits: lfuse = 0xf7, hfuse = 0xd6, efuse = 0xfd
2. The MightyCore provides in the platformio installation directy bootloaders. Choose ```.platformio/packages/framework-arduino-avr-mightycore/bootloaders/optiboot_flash/bootloaders/atmega644p/16000000L/optiboot_flash_atmega644p_UART0_115200_16000000L_B0_BIGBOOT.hex```
3. If the bootloader is active, two pulses are shown on pin B0 (= Arduino pin 0), which can be checked with a oscilloscope.

After the bootloader is installed and running, the typical Arduino upload can be used over the serial interface.

## Build Project
1. Load workspace in VSCode.
2. Change to PlatformIO toolbar.
3. _Project Tasks -> Build All_ or via hotkey ctrl-alt-b

## Update of the device

### Update via serial interface
1. Connect the AVR-NET-IO board to your PC via serial interface.
2. Build and upload the software via _Project Tasks -> Upload All_.
3. Note, if the AVR-NET-IO board is not modified, you need to keep it off until in the console ```Uploading .pio\build\MightyCore\firmware.hex``` is shown. Just in this moment power the board and the upload starts.

## Used Libraries
* [MightyCore](https://github.com/MCUdude/MightyCore) - Arduino core for ATmega644.
* [EtherCard](https://github.com/njh/EtherCard) - Driver for the Microchip ENC28J60 chip.
* [ArduinoHttpServer](https://github.com/QuickSander/ArduinoHttpServer) - Server side minimalistic object oriented HTTP protocol implementation.
* [ArduinoJSON](https://arduinojson.org/) - JSON library.

# REST API

## Get temperature value (GET /api/sensors/<sensor>)
Get a temperature value in °C from the heatpump.

```<sensor>```:
* gt1 - Radiator return temperature in °C
* gt2 - Outdoor temperature in °C
* gt3 - Hot water temperature in °C
* gt4 - Forward temperature in °C
* gt5 - Room temperature in °C
* gt8 - Heat fluid out temperature in °C
* gt9 - Heat fluid in temperature in °C
* gt10 - Cold fluid in  temperature in °C
* gt11 - Cold fluid out temperature in °C
* gt3x - External hot water temperature in °C
* gt3TargetValue - Hot water target temperature in °C
* gt3On - Hot water on temperature in °C
* gt3Off - Hot water off temperature in °C

Response:
```
{ "data": {
    "name": "gt1",
    "value": 30
  },
  "status":0
}
```

Status 0 means successful. If the request fails, it the status will be non-zero and data is empty.

## Set temperature value (POST /api/sensors)
Set the temperature target value in °C.

JSON parameter:
* name: Temperature name as string.
  * gt3Target
  * gt3On
  * gt3Off
* value: Temperature value in °C as float.

Example:
```
$ curl -H "Content-Type: application/json" --data '{"name": "gt3Target", "value": 20.4}' http://192.168.1.3/api/sensors
```

Response:
```
{ "status": 0 }
```

Status 0 means successful. If the request fails, it the status will be non-zero and data is empty.

## Send raw command (POST /api/debug)
Send a raw command to the heatpump controller for reverse engineering or debug purposes. Note, the response message comes back as string with hex numbers.

JSON parameter:
* cmdId: Rego6xx command id as integer.
* addr: Rego6xx register address.
* value: Rego6xx Register value.

Example:
```
$ curl -H "Content-Type: application/json" --data '{"cmdId": 2, "addr": 521, "value": 0 }' http://192.168.1.3/api/debug
```

Response:
```
{ "data": {
    "response": "8100022C2E"
  },
  "status": 0
}
```

## Get last error information (GET /api/lastError)
Get last error information from the heatpump.

Response:
```
{ "data": {
    "errorId": 22,
    "log": "021009 18:21:03",
    "description": "Varmetr. delta high"
  },
  "status": 0
}
```

## Get frontpanel LED state (GET /api/frontPanel/<led>)
Get state of frontpanel LED from the heatpump.

```<led>```:
* power - Power LED
* pump - Pump LED
* heating - Heating LED
* boiler - Boiler LED
* alarm - Alarm LED

Response:
```
{ "data": {
    "name": "power",
    "state": true
  },
  "status":0
}
```

Status 0 means successful. If the request fails, it the status will be non-zero and data is empty.

# Issues, Ideas And Bugs
If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/Rego6xxSrv/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

# License
The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).

# Contribution
Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, shall be licensed as above, without any
additional terms or conditions.
