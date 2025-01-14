# Experimental Implementation of Synergistic Traffic Assignment (STA)

To compute an STA equilibrium using this code, compile (using GCC) and execute (main function in cch/main.cpp).
The code expects input files in <project-root>/inputs. 
The following input files are expected (all in [RoutingKit](https://github.com/RoutingKit/RoutingKit) vector format):
* a road network given as an adjacency array consisting of index array `first_out` (one entry per vertex) and edge head array `head` (one entry per edge)
* travel times `travel_time` (one value per edge)
* a CCH node order `cch_order` (one value per vertex); this can be computed, e.g., using [InertialFlowCutter](https://github.com/kit-algo/InertialFlowCutter)
* O-D pairs as two arrays of vertex IDs `od_pairs/<basename>.origin` and `od_pairs/<basename>.destination`(one entry per O-D pair each); <basename> can be specified as a command line argument to the compiled program

The selfishness parameter `r` is hardcoded (at the top of main.cpp) and can be changed before compiling. The default value is `r=0`. 
