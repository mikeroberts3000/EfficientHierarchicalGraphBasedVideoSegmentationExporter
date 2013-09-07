/*
 *  segmentation_io.h
 *  segmentat_util
 *
 *  Created by Matthias Grundmann on 6/30/10.
 *  Copyright 2010 Matthias Grundmann. All rights reserved.
 *
 */

// Segmentation Reader and Writer to be used with segmentation.pb files.
// Moved from segmentation_unit.h for external use.

// The binary file format we use has the following form, and contains a header 
// at the end of the file to support random seeking.
// Format:
//
// Number of frames : sizeof(int32)
// Offset to header at end of file : sizeof(int64)
// For every frame
//    Size of protobuffer in bytes : sizeof(int32)
//    Protobuffer serialized to binary format : from above member
// Header, for every frame:
//    FileOffset in file : sizeof(int64)
//    TimeStamp of frame in pts : sizeof(int64)

#ifndef SEGMENTATION_IO_H
#define SEGMENTATION_IO_H

#include <fstream>
#include <string>
#ifdef __linux
  #include <stdint.h>
#endif

#include <vector>

#ifdef _WIN32
  typedef __int64 int64_t;
#endif

namespace Segment {
  
  typedef unsigned char uchar;
  using std::string;
  using std::vector;

  class SegmentationWriter {
  public:
    SegmentationWriter(const string& filename) : filename_(filename) {}
    
    bool OpenAndPrepareFileHeader();
    void WriteOffsetsAndClose();
    
    void FlushAndReopen(const string& filename);
    
    void WriteSegmentation(const uchar* data, int sz, int64_t pts = 0);
    
  private:
    string filename_;
    std::ofstream ofs_;                         
    
    vector<int64_t> file_offsets_;
    vector<int64_t> time_stamps_; 
  };
  
  class SegmentationReader {
  public:
    SegmentationReader(const string& filename) : filename_(filename) {}
    
    bool OpenFileAndReadHeader();
    
    // For each frame, first call ReadFrameSize
    // and subsequently ReadFrame.
    int ReadFrameSize();
    void ReadFrame(uchar* data);
    
    const vector<int64_t>& TimeStamps() { return time_stamps_; }
    void SeekToFrame(int frame);
    int FrameNumber() const { return file_offsets_.size(); }
    void CloseFile() { ifs_.close(); }
    
  private:
    vector<int64_t> file_offsets_;
    vector<int64_t> time_stamps_;
    
    int frame_sz_;
    
    string filename_;
    std::ifstream ifs_;
  };

}  // namespace Segment.

#endif