# Experimental Implementation of Synergistic Traffic Assignment (STA)

This is an experimental implementation of the Synergistic Traffic Assignment (STA) algorithm written in C++ 17.
For information on the algorithm, we refer to the following paper:

* Thomas Bläsius, Adrian Feilhauer, Markus Jung, Moritz Laupichler, Peter Sanders, Michael Zündorf. 
  2025. 
  Synergistic Traffic Assignment. 
  In Proc. of the 24th International Conference on Autonomous Agents and Multiagent Systems (AAMAS 2025),
  Pages 352 - 360,
  Detroit, Michigan, USA, May 19 – 23, 2025, 
  IFAAMAS,
  https://dl.acm.org/doi/10.5555/3709347.3743549

If you use this code in your scientific publication, we ask that you cite the paper above. 

## License
All files in this repository are licensed under the MIT license.

## Building
To build the code, you need a modern C++ compiler.
For instance, using GCC (version 11.4.0 or later), run the following command at the top-level directory:
```
g++ -O3 cch/main.cpp -o sta
``` 

## Input data
The code expects input files to be placed in the `inputs` directory.
The following input files are expected (all in [RoutingKit](https://github.com/RoutingKit/RoutingKit) vector format):
* a road network given as an adjacency array consisting of index array `first_out` (one entry per vertex) and edge head array `head` (one entry per edge)
* travel times `travel_time` (one value per edge)
* a CCH node order `cch_order` (one value per vertex)
* O-D pairs as two arrays of vertex IDs `od_pairs/<basename>.origin` and `od_pairs/<basename>.destination`(one entry per O-D pair each); `<basename>` can be specified as a command line argument to the compiled program, see below

RoutingKit offers [functionality](https://github.com/RoutingKit/RoutingKit/blob/master/doc/OpenStreetMap.md) for generating road networks in the desired format using OSM data.
[InertialFlowCutter](https://github.com/kit-algo/InertialFlowCutter) is able to compute a CCH node order for a given road network in RoutingKit format.

## Running
To execute the STA algorithm, run the following command:
```
./sta <basename> <selfishness>
```

The first command line parameter `<basename>` refers to the name of the O-D pair files (see above).
The second command line parameter `<selfishness>` is the selfishness tuning parameter (explained in the paper). 
This must be a decimal number between `0.0` and `1.0`.

The program prints statistics on each iteration of the algorithm in standard out and places a file containing the final flow pattern in the `outputs` directory (one value per edge in RoutingKit vector format). 
