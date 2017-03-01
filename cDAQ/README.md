## Description
C code to implement laser scanning microscope by calling **NI-DAQmx ANSI C** Library. 
Code is implemented with **NI DAQ** to generate scan waveform to control galvo mirrors and to acquire PMT signals.
Scan waveform generation was referred to ScanImage and WiscScan and in **analog mode** only. Data acquision is currently in **analog mode** only with NI DAQ.
Currently only supports **galvo-galvo** scanning.

## Hardware Requirement
1. NI X series DAQ card (e.g., PXIe-6356) and chassis (e.g., PXIe-1073).
2. Basic components for laser scanning micrsocpe, i.e., pulsing laser, galvos, PMT, etc..
3. Oscilloscope to monitor scan waveform. -- OPTIONAL

## Software Requirement
1. NI-DAQmx 15.1.1. Skip this step if ScanImage 5.1 was installed and running fine.
2. Microsoft Visual Studio 2012.
3. Windows 7 or 10, 64bit OS (Currently software running on 64bit Windows 10). 

## Contributors
Rui Xu (rxu56@wisc.edu) 
Bing Dai (bdai6@wisc.edu) 
