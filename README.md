# INFOMR-Project

## Getting Started

### Prerequisites

The UI for this project is created using Qt5. You can download and install Qt by following this [link](https://www.qt.io/download-qt-installer) and downloading the installer. During instalation make sure to select at least the component for your IDE and Qt Charts.

To build the project cmake version 3.8 or higher is required. Please make sure to have this installed by following [this link](https://cmake.org/download/).

### Building

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