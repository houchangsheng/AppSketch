# AppSketch
- AppSketch: A Sketch Framework for Lightweight and Real-time Application Traffic Analysis

## AppSketch
- AppSketch interface for IP-Trace data

### Files
- libprotoident: Libprotoident library related files
- AppSketch.h: the implementation of AppSketch
- USS.h: the implementation of USS
- DMatrix.h: the implementation of DMatrix
- WavingSketch.h: the implementation of WavingSketch
- HeavyGuardian.h: the implementation of HeavyGuardian
- ColdFilter.h: the implementation of Cold Filter
- BenchMark.h: the interface of traffic analysis using AppSketch and other methods
- main.cpp: the experiments on AppSketch and other methods

### Required Libraries
- libtrace 4.0.1 or later
  - available from http://research.wand.net.nz/software/libtrace.php
- libflowmanager 3.0.0 or later
  - available from http://research.wand.net.nz/software/libflowmanager.php

### Compile and Run
- Compile with make
```
$ make
```
- Run the examples, and the program will output some statistics about the accuracy and efficiency. 
```
$ ./appsketch
```
- Note that you can change the configuration of AppSketch and other methods.
