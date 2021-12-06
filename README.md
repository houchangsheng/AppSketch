# AppSketch
- AppSketch: A Sketch Framework for Lightweight and Real-time Application Traffic Analysis

## AppSketch
- AppSketch interface for IP-Trace data

### Files
- libprotoident: Libprotoident library related files
- sketch.hpp sketch.cpp: the implementation of AppSketch
- sketch2.hpp sketch2.cpp: the implementation of Enhanced TCM
- main.hpp main.cpp: the interface of traffic analysis using AppSketch\Enhanced TCM

### Required Libraries
- libtrace 4.0.1 or later
-  available from http://research.wand.net.nz/software/libtrace.php
- libflowmanager 3.0.0 or later
-  available from http://research.wand.net.nz/software/libflowmanager.php

### Compile and Run
- Compile with make
```
$ make
```
- Run the examples, and the program will output some statistics about the accuracy and efficiency. 
```
$ ./appsketch
```
- You can give the path of the IP-trace dataset.
```
$ ./appsketch ./traces/ipv4.202011262000.pcap
```
- Note that you can change the configuration of AppSketch\Enhanced TCM, e.g. the depth, length and width of the sketch.
