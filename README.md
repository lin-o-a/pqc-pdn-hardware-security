# PQC PDN Fault Dynamics & Defense Research

Research repository evaluating power delivery network (PDN) physical limits, capacitor decoupling trade-offs, and early-warning interrupt (PVD) zeroization strategies during Post-Quantum Cryptography (ML-KEM) execution on embedded platforms.

## Overview

Applying bulk and ceramic decoupling capacitors to protect microcontrollers against PQC-induced voltage sags can introduce a fault-masking "gray zone." In this state, degraded supply rails allow logic timing violations during matrix operations before hardware Brownout Reset (BOR) triggers, leaving cryptographic state exposed.

This project analyzes:
- **PDN Decoupling Interactions:** Bulk vs. ceramic capacitance hierarchy ($470\ \mu\text{F}$ bulk hold-up vs. $0.1\ \mu\text{F}$ clock-edge $di/dt$ filtering).
- **PVD Zeroization Handlers:** Software-initiated emergency reset (`PVD_PVM_IRQHandler`) vs. hardware $V_{\text{BOR}}$ trip delays.
- **Hardware/Software Mitigations:** Dual-layer boot-time reset checks (`RCC_FLAG_BORRST` / `RCC_FLAG_SFTRST`) and memory zeroization.

## Hardware & Toolchain
- **Target MCU:** STMicroelectronics NUCLEO-L552ZE-Q / STM32F4 series
- **Toolchain:** STM32CubeIDE, ST-Link V2/V3
- **Test Equipment:** Oscilloscope (transient $dV/dt$ measurement), Logic Analyzer, ChipWhisperer Lite/Nano

## Repository Structure
- `/src` - STM32 C source files, PVD interrupt routines, and zeroization logic => Root(main) code file is /src/Core/Src/main.c
- `/docs` - Experimental setup diagrams, power rail decay measurements, and technical notes

## Workstation Settings
- Nucleo-L552ZE-Q STM32 with TrustZone
- Breadboard
- Resistor of 47 Ohm and 10 Ohm connected in series to create load to initiate board BOR reset
- Electrolytic capacitor 470muF and 2 ceramic capacitors 0.1 muF to support board voltage during its sag and to support board CPU during current spikes (while electrolytic capacitor supports CPU)
- OLED TZT 0.95 display to show the steps (PQC steps and reset catch by voltage detector(PVD))

## Connections
### Nucleo board and Display
- connect display VDD for power source to Nucleo board 3V3 on CN8 (use yellow female-male DUPONT wire)
- connect display GND to Nucleo board GND on CN7 (female header from the right 4th row, use brown female-male DUPONT wire)
- connect display SCK clock pin (to sync with board clock) to SCL on CN7 (1st row, female header on the right, use blue female-male DUPONT wire)
- connect display SDA data pin to Nucleo board's SDA (2nd row, right, use green female-male DUPONT wire)

### Nucleo board and Breadboard
- Nucleo's IDD pin 1 connect to 10a (use orange female-male DUPONT wire)
- Nucleo's IDD pin 2 connect to 20a (use red female-male DUPONT wire)
- Nucleo's GND on CN7 connect to blue line of breadboard any row, for e.g. 20 or 21 (use brown male-male DUPONT wire)

### Breadboard
- Add resistor 47 Ohm to 10b and 15b to increase voltage sag to get into BOR reset
- Add resistor 10 Ohm to 10d and 20d to increase voltage sag to get into BOR reset
- Add switcher to 10c and 13c to open and close the path of resistors that increase voltage sag (default state of resistor "on" i.e. a path without resistors is open)
- Add a yellow male-male DUPONT wire from 13e to 20e (we need it because the switcher is too small to sit from 10 row to 20 row to set the path made of resistors so we use dupont wire to enable the path when the switcher will be switch to "off")
- Add electrolytic capacitor 470 muF: anode(long leg to 20th row free cell) and cathode(short leg) to breadboard GND(blue line)
- Add the first ceramic 0.1 muF: anode to another free cell on 20th row and cathode to breadboard GND(blue line)
- Add the second ceramic 0.1 muF: anode to 10th row free cell and cathode to breadboard GND(blue line) 

