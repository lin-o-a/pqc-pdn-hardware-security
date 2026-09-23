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

