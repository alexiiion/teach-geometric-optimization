# Geometric optimization


## Install
Install all of the following components well **before** the session. Note that some of these packages are **large** (multiple GB) and take time to download and install. Make sure to test everything to be ready to go.

If you are experienced in C++ and have everyting already setup, jump to [Building](#building) directly.

### (1) Development environment & C++ compiler
If you already have a C++ environment set up, skip this section and feel free to use your favorite editor, IDE, compilers, etc.  

If you don't have your toolchain set up already, I recommend installing IDEs that take of the toolchain for you:
* Windows: [Visual Studio](https://visualstudio.microsoft.com/vs/community/), which installs the MSVC compiler for you
* MacOS: [Xcode](https://developer.apple.com/xcode/), which install clang for you

### (2) CMake
Install [CMake](https://cmake.org/download/), which is the build system that creates the C++ project, sets up dependencies, etc. If you are new to CMake, I recommend using the CMake GUI.

### (3) Boost C++ libraries
* Windows: [Install instructions](https://robots.uc3m.es/installation-guides/install-boost.html#install-boost-windows). You might need to run the command prompt in admin mode. I recommend using the *Developer command prompt* that is installed by Visual Studio.
* MacOS: [Install instructions](https://robots.uc3m.es/installation-guides/install-boost.html#install-boost-ubuntu)


## Building

### (1) Clone the code recursively
This project uses [libigl](https://github.com/libigl/libigl/) for geometry processing. See the [libigl tutorial](https://libigl.github.io/tutorial/).
Clone this repository with all dependencies, configured as submodules, using:
```
git clone --recursive <repo>
```

### (2) Build using CMake
On Windows, open the CMake GUI (see the image below). 
1. Set the path to **this folder** (source code) and 
2. the **build folder** as they are on your system. The build folder should be contained in this folder. 
3. Click `Configure` and keep the settings (your Visual Studio version might be newer, keep `Use default native compilers` unless you know what you do.)

<img src="./_instructions/cmake-gui-initial-config.PNG" width="500"/>


On MacOS, you can also use the cmake-gui, as described above. Alterantively, from the root directory that contains `CMakeLists.txt`, run:

```
mkdir _build
cd _build
cmake ../
make
```

There are some warnings due to changes in the build system of libigl. It still works. After running cmake, the output should be `Configuring done`.

### (3) Run the code

* Windows: open the `*.sln` file, which opens Visual Studio. There, set the configuration to `RelWithDebInfo`. Right click on the `GeometricOptimization` project in the Solution Explorere and select `Set as Startup Project`.

* MacOS: execute in Xcode or in terminal.

This is the result of a successful compilation:

<img src="./_instructions/result.PNG" width="500"/>
