# INFOMR-Project

## Prerequisites

The UI for this project is created using Qt5. You can download and install Qt by following this [link](https://www.qt.io/download-qt-installer) and downloading the installer. During instalation select at least the following components:
* your IDE component
* Qt Charts

To build the project files cmake version 3.8 or higher is required. Please make sure to have it installed by following [this link](https://cmake.org/download/).

To run the query system, the [Princeton Shape Benchmark](https://shape.cs.princeton.edu/benchmark/download.cgi?file=download/psb_v1.zip) needs to be unzipped somewhere on the computer.  

## Building

To build the project go to the root of the project folder and follow the steps to build.

1. Create the build directory and access it
```sh
mkdir build
cd build
```
2. Build the project files using cmake
```sh
cmake ..
```
3. Build the project
```
cmake --build . --config Release
```
4. Run the project
```
cd ..
Binary/Release/INFOMR-Project.exe
```

## Usage

