# Programming framework for the course "Scientific Visualization".

## Supported Operating Systems

In the following you will be guided through the process of installing and setting up everything necessary to work on the programming exercise. The instructions mainly focus on Windows but the process on Linux should be similar with a few adaptations. If you need help with setting up on Linux we are happy to help. Unfortunately, we do not support macOS officially but we have seen students who were able to run it in previous years.


## Install IDE
If you are working on windows we recommend using Visual Studio as your development environment. You can download it from [https://visualstudio.microsoft.com/de/downloads/](https://visualstudio.microsoft.com/de/downloads/). Simply follow the install instructions and make sure to also install the package "Desktop development with C++".

## Install CMake
CMake is a cross-platform build tool that sets up a project for a given compiler and development environment. Visit [https://cmake.org/download/](https://cmake.org/download/) and download the latest version of CMake. You can use the installers. It is not necessary to build CMake from source.

## Download VTK
VTK is among the most popular libraries for scientific visualization. Throughout the programming exercises, we will use VTK for rendering. Visit [http://www.vtk.org/download/](http://www.vtk.org/download/) and download the latest version of VTK (9.4.2 at the time of writing). VTK has to be built from source. Thus, scroll down to the sources and download them. It is not necessary to download the data sets

For Linux users we recommend to use a newer version than the versions that are currently available for download since they address a performance issue on Linux machines. To do so it is necessary to get the source code from the official VTK GitLab repository [https://gitlab.kitware.com/vtk/vtk](https://gitlab.kitware.com/vtk/vtk). From there you can either download the sources in a zip file or clone the repository. We tested the framework on commit *54b70aeb8b69ad185cf765ea3040a2da91697b78*. We recommend to navigate to that commit before downloading the zip file or checking out that commit after cloning the repository. The paths in the following tasks have to be adapted to match the path to your downloaded directory.

## Build VTK using CMake
* Start *cmake-gui*. 
* Enter into the text box that is labeled with 'Where is the source code:' the path to VTK. For instance: *D:/VTK-9.4.2*
* Enter into the text box that is labeled with 'Where to build the binaries:' another arbitrary folder. For instance: *D:/VTK-9.4.2/build*
* Press *Configure*. If the build folder does not exist yet, CMake will ask you, whether it should create the folder for you. Say, yes.
* Select compiler. A window will open with a list of all supported compilers. Select your favorite compiler. (E.g., *Visual Studio 17 2022*. Also, if you have a 64-bit system, select the *x64* as optional platform for generator.) After selecting the compiler, CMake will run a few diagnostics and test your compiler. If you immediately get an error, you probably selected a compiler that cannot be found on your computer. Check again, whether you selected the right version of your compiler. Expect that the first configuration might take a while.
* Press *Configure* again. After the first run, CMake will have found and listed numerous configuration options. Every newly found configuration option is highlighted in red. After the first run, that's all of them. In a second run, the red highlights should disappear, since the configuration options have already been found. They only remain red if there are problems, for instance if a dependent library could not be found. If all red highlights disappeared, you can continue to the next step. 
* Press *Generate*. This button will generate the project files for your selected compiler. If you selected Visual Studio, a solution file  (sln) will be created for you in your build folder.
* Press *Open Project*. This button will open the project using the default IDE.

The following steps are for Visual Studio. For other IDEs the procedure is probably somewhat similar.
* Select the configuration *'Release'*.
* Build the project *'ALL_BUILD'*. This will compile every library of VTK. This will take a while. (Get coffee and hope that no compiler errors have shown up while you were gone.)
* (optional) Select the configuration *'Debug'*.
* Build the project *'ALL_BUILD'*. Yes, time for another coffee.

## Building the Exercise Program
In the following, we setup the programming framework.
* Clone this repository or download the zipped version from this repository (Coed -> Download source code -> zip). If you did the latter you also need to extract the package.
* Start *cmake-gui*. 
* Enter into the text box that is labeled with 'Where is the source code:' the path to the cloned repository or the unpacked folder. For instance: *D:/scivis-ss25* (Make sure that this folder contains the CMakeLists.txt file, which configures the build procedure of CMake.)
* Enter into the text box that is labeled with 'Where to build the binaries:' another arbitrary folder. For instance: *D:/scivis-ss25/build*.
* Press *Configure*. If the build folder does not exist yet, CMake will ask you, whether it should create the folder for you. Say, yes.
* Select compiler. A window will open with a list of all supported compilers. Select your favorite compiler. Make sure that you use the same compiler as you used above for compiling VTK. CMake will run a few diagnostics and test your compiler.
* Specify *VTK build path*. You get an error message. CMake was probably not able to find your VTK build path on its own, so you have to help. Set as *'VTK_DIR'* the path where you built VTK. For instance, *D:/VTK-9.4.2/build*.
* Press *Configure* again. Now, VTK should be found and the configuration can continue.
* Press *Generate*. This button will generate the project files for your selected compiler. If you selected Visual Studio, a solution file (sln) will be created for you in your build folder.
* Press *Open Project*. This button will open the project using the default IDE.

## Running an Exercise Program
The following steps are for Visual Studio again. If you use another IDE, the steps should be somewhat similar.
* Always be aware whether you compile in Debug or Release mode.
* Set *startup project*. Do a right click on the project *scivis* in the *Solution Explorer*. Select *Set as startup project*. The project name should now be printed in bold letters. Hitting F5 will now build and run our program. (If you do not set the startup project, Visual Studio will try to run the *'ALL_BUILD'* project, which is not an executable, resulting in an error message.)
* Add *VTK to PATH*. If you try to run the program, you will probably receive some error messages that tell you that VTK DLLs cannot be found. Do a right click on the project *scivis* in the *Solution Explorer* and select *Properties*. Go to *Debugging* and set as *Environment* the following line (adapt it to your build path of VTK): *PATH=%PATH;D:/VTK-9.4.2/build/bin/$(Configuration)*
* Run the program. You should see the grid and two spheres.