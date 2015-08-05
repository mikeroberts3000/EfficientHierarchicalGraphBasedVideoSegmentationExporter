This repository contains C++ code to export the video segmentations from the system described in the paper <a href='http://www.cc.gatech.edu/cpl/projects/videosegmentation/'>Efficient Hierarchical Graph-Based Video Segmentation</a>. The system described in this paper returns segmentations as Protocol Buffer files. The exporter contained in this repository converts these Protocol Buffer files into image sequences.

Note that most of this code was written by <a href='http://www.mgrundmann.com/'>Matthias Grundmann</a> and distributed on his <a href='http://www.cc.gatech.edu/cpl/projects/videosegmentation/'>project page</a>. I just hacked his code to export the segmentations rather than visualize them in a GUI.

### Requirements

* __OpenCV__. I used OpenCV 2.4.6.1, but any recent version of OpenCV should work.
* __Protobuf__. I used ProtoBuf 2.5.0, but any recent version of Protobuf should work.
* __The CMake GUI__. I used CMake 2.8.10, but any recent version of CMake should work.
  * http://www.cmake.org/cmake/resources/software.html

On Mac OSX, OpenCV and Protobuf are easy to obtain via MacPorts. From a terminal window, simply type the following:

```
$ sudo port install opencv
$ sudo port install protobuf-cpp
```

### Build Instructions

* Point the CMake GUI to the code/segmentation_exporter/segmentation_exporter folder and specify a build folder.
* Hit Configure. Select the default compiler options. On Mac OSX, CMake should find all the build dependencies automatically. If CMake doesn't find these dependencies automatically, enter in the missing OpenCV and Protobuf CMake variables manually and hit Configure again.
* Hit Generate.
* Navigate to the build folder in a terminal window and type make.
* Now you can run the segmentation_exporter executable.

### Usage

* Obtain a video segmentation from <a href='http://neumann.cc.gt.atl.ga.us/segmentation/'>here</a>.
* Run the segmentation_exporter executable from this repository as follows:

```
$ ./segmentation_exporter input_segmentation.pb output_folder
```
