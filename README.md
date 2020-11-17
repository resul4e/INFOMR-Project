# INFOMR-Project

## Prerequisites

The UI for this project is created using Qt5. You can download and install Qt by following this [link](https://www.qt.io/download-qt-installer) and downloading the installer. During instalation select at least the following components:
* your IDE component
* Qt Charts

If you already have a recent Qt installation, please update it to have the Qt Charts component.

To build the project files cmake version 3.8 or higher is required. Please make sure to have it installed by following [this link](https://cmake.org/download/).

To run the query system, the [Princeton Shape Benchmark](https://shape.cs.princeton.edu/benchmark/download.cgi?file=download/psb_v1.zip) needs to be unzipped somewhere on the computer.  

## Building

To build the project through the console go to the root of the project folder and follow the steps to build.

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

Alternatively the project can be build in the visual GUI of CMake and compiled in Visual Studio (tested on 2019).

## Usage
After the program opens, a database can be loaded through the top-bar menu by picking Database -> Load PSB and pointing it to the `benchmark` folder.

If all is well, it will fill in the Database Shape Hierarchy list on the left of the application. From that list a model can be selected which will show it in the central model viewer. If the features on the right of the screen are filled in after selecting a model, then you are ready to launch a query. Otherwise, please make sure to put the FeatureDatabase folder next to the project executable.

A query can be launched by pressing `Search similar` and optionally changing the query size `k` located above the button.
