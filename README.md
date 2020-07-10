# SHTECLib
A C++ short text clustering library

This library contains efficient implementations of several generic and short text clustering algorithms. It can be applied to both general entities and products. Regarding products, SHTECLib replaces the [UPM Library](https://github.com/lakritidis/UPM-full).

SHTECLib can be built as both a command line tool and a standard C++ program. This is determined by the `COMMAND_LINE` declarative in main.cpp, line 12. If SHTECLib is to be built as a command line tool, the TCLAP library must be present within the `lib/tclap/` directory.
