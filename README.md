# ENIGMA Project
**Multi HEIG-VD Project**  
**2026**

The goal of this project is to design and build a fully functional and accurate prototype of the **Enigma machine** using modern embedded technologies. Originally used from the 1930s until the end of the Second World War, the Enigma machine encrypted and decrypted messages for both commercial and military purposes. Today, it remains one of the most iconic devices in the history of cryptography. This project combines software development, embedded programming, and hardware integration to recreate the Enigma machine's functionality while providing an educational demonstration of its operation. The project is structured to allow for modular testing of each hardware component before full integration, ensuring a robust and reliable final product.

---

## Table of Contents

- [ENIGMA Project](#enigma-project)
  - [Table of Contents](#table-of-contents)
  - [Project Overview](#project-overview)
  - [Objectives](#objectives)
  - [Historical Background](#historical-background)
  - [System Features](#system-features)
    - [Software Features](#software-features)
    - [Hardware Features](#hardware-features)
    - [Engineering Features](#engineering-features)
  - [Technologies Used](#technologies-used)
    - [Software](#software)
    - [Embedded Hardware](#embedded-hardware)
    - [Libraries](#libraries)
  - [Project Architecture](#project-architecture)
    - [1. Software Layer](#1-software-layer)
    - [2. Hardware Layer](#2-hardware-layer)
    - [3. Test-Oriented Design](#3-test-oriented-design)
  - [Repository Structure](#repository-structure)

---

## Project Overview

This project aims to recreate the **Enigma machine** as a modern embedded system while preserving the core principles of its original electromechanical operation. The project combines:

- **cryptographic algorithm design**
- **embedded programming**
- **hardware integration**
- **modular testing of physical components**

The result is an academic prototype capable of simulating the historical encryption and decryption process while interacting with real hardware such as a keyboard matrix, LEDs, sensors, and stepper motors.

---

## Objectives

The main objectives of the project are:

- Reproduce the logic of the original Enigma machine
- Implement the encryption algorithm in **C (C11)**
- Design a modular embedded architecture using **Arduino**
- Integrate physical input and output components
- Validate each hardware subsystem independently before full integration
- Provide a faithful and educational demonstration of historical cryptography using modern tools. 

---

## Historical Background

The Enigma machine was a rotor-based cipher device used extensively during the 20th century. Its operation relied on:

- a keyboard for user input
- a set of rotating wired rotors
- a reflector to send the signal back through the rotor stack
- a lampboard to display the encrypted letter

One of the most remarkable properties of the Enigma system is that the encryption process is **reversible**: the same machine configuration can be used both to encrypt and decrypt a message. This project recreates that principle with a combination of software logic and embedded hardware.

---

## System Features

### Software Features
- Complete implementation of the Enigma encryption algorithm in **C**
- Rotor data structures with:
  - wiring
  - position
  - offset
- Reflector modeling
- Forward rotor traversal
- Reflection
- Backward rotor traversal
- Automatic rotor stepping during encryption

### Hardware Features
- Matrix keyboard scanning for character input
- WS2812B LED output for encrypted letter display
- Stepper motor control to simulate physical rotor motion
- Magnetic and analog sensing to detect positions and identify rotors

### Engineering Features
- Separation between algorithmic logic and hardware testing
- Independent validation of subsystems before integration
- Modular project organization for easier maintenance and debugging

---

## Technologies Used

### Software
- **C (C11 standard)** for the Enigma algorithm
- **GCC** compiler
- **Makefile** for build automation
- **VS Code** for development and debugging 

### Embedded Hardware
- **Arduino** platforms
- **Stepper motors**
- **WS2812B LEDs**
- **Matrix keyboards**
- **Magnetic and analog sensors**

### Libraries
- **Adafruit NeoPixel** for LED strip control

---

## Project Architecture

The system is divided into two main layers.

### 1. Software Layer
The Enigma algorithm is implemented as a standalone software module in C. The machine is modeled using structures that represent:

- rotors
- rotor offsets
- rotor positions
- rotor wirings
- reflector configuration

The encryption process follows the historical Enigma sequence:

1. Rotor stepping
2. Forward traversal through the rotors
3. Reflection
4. Backward traversal through the rotors
5. Output character generation

### 2. Hardware Layer
The embedded layer connects the software model to physical components:

- the **keyboard** provides the input
- the **LEDs** display the encrypted character
- the **stepper motors** reproduce the rotor movement
- the **sensors** validate the physical state of the system 

### 3. Test-Oriented Design
Before full integration, each hardware block is tested individually. This approach reduces debugging complexity and improves reliability during final assembly.

---

## Repository Structure

```text
.
├── README.md
└── Tests/
    ├── algorithme/
    │   ├── enigma.h
    │   ├── enigma.c
    │   ├── main.c
    │   └── Makefile
    ├── Keyboard_LED/
    │   └── Keyboard_LED.ino
    ├── LED_simple/
    │   └── LED_simple.ino
    ├── magnetic_sensor/
    │   └── magnetic_sensor.ino
    ├── Rotor_ID/
    │   └── Rotor_ID.ino
    └── Stepper/
        └── Stepper.ino