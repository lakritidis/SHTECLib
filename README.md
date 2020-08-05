# SHTECLib
A C++ short text clustering library

This library contains efficient implementations of several generic and short text clustering algorithms. It can be applied to both general entities and products. Regarding products, SHTECLib replaces the [UPM Library](https://github.com/lakritidis/UPM-full) and contains the original implementation of the UPM algorithm.

SHTECLib can be built as both a command line tool and a standard C++ program. This is determined by the `COMMAND_LINE` declarative in main.cpp, line 12. If SHTECLib is to be built as a command line tool, the [TCLAP library](http://tclap.sourceforge.net/) must be present within the `lib/tclap/` directory. SHTECLib can also be directly compiled, provided that the [GSL - GNU Scientific Library] resides inside the `lib` folder. In that case, a link to the `libgsl.a` file must be declared.

The execution parameters of STECLib can all be set in the `src/Settings.cpp` file. The `algorithm` and `dataset` members of the `Settings` class determine the clustering algorithm that will be applied, and the set of records to be clustered respectively. In the same file the hyper parameters of the algorithms can be set.

