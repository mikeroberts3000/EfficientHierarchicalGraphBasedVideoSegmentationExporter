/*
 *  segmentation_io.cpp
 *  segmentat_util
 *
 *  Created by Matthias Grundmann on 6/30/10.
 *  Copyright 2010 Matthias Grundmann. All rights reserved.
 *
 */

#include "segmentation_io.h"

#include <iostream>

namespace Segment {
  
  bool SegmentationWriter::OpenAndPrepareFileHeader() {
    // Open file to write
    ofs_.open(filename_.c_str(), 
              std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
    
    if (!ofs_) {
      std::cerr << "SegmentationWriter::PrepareFileHeader: " 
      << "Could not open " << filename_ << " to write!\n";
      return false;
    }
    
    // Write dummy header. To be filled on post process.
    int num_frames = 0;
    int64_t header_offset = 0;
    
    ofs_.write(reinterpret_cast<const char*>(&num_frames), sizeof(num_frames));
    ofs_.write(reinterpret_cast<const char*>(&header_offset), sizeof(header_offset));    
    
    return true;
  }
  
  void SegmentationWriter::WriteOffsetsAndClose() {
    // Header information.
    int num_frames = file_offsets_.size();
    int64_t header_offset = ofs_.tellp();
    
    //  Write file offsets to end.
    for (int i = 0; i < file_offsets_.size(); ++i) {
      ofs_.write(reinterpret_cast<const char*>(&file_offsets_[i]), sizeof(file_offsets_[i]));
      ofs_.write(reinterpret_cast<const char*>(&time_stamps_[i]), sizeof(time_stamps_[i]));
    }
    
    // Write header.
    ofs_.seekp(0);
    ofs_.write(reinterpret_cast<const char*>(&num_frames), sizeof(num_frames));
    ofs_.write(reinterpret_cast<const char*>(&header_offset), sizeof(header_offset));
    
    ofs_.close();    
  }
  
  void SegmentationWriter::FlushAndReopen(const string& filename) {
    WriteOffsetsAndClose();
    filename_ = filename;
    file_offsets_.clear();
    time_stamps_.clear();
    OpenAndPrepareFileHeader();
  }
  
  void SegmentationWriter::WriteSegmentation(const uchar* data, int sz, int64_t pts) {
    file_offsets_.push_back(ofs_.tellp());
    time_stamps_.push_back(pts);
    
    ofs_.write(reinterpret_cast<const char*>(&sz), sizeof(sz));
    ofs_.write(reinterpret_cast<const char*>(data), sz);
  }
  
  bool SegmentationReader::OpenFileAndReadHeader() {
    // Open file.
    ifs_.open(filename_.c_str(), std::ios_base::in | std::ios_base::binary);
    
    if (!ifs_) {
      std::cerr << "SegmentationReader::OpenFileAndReadHeader: "
      << "Could not open segmentation file " << filename_ << "\n";
      return false;
    }
    
    // Read offset for each segmentation frame from header.
    int num_seg_frames;
    int64_t seg_header_offset;
    
    ifs_.read(reinterpret_cast<char*>(&num_seg_frames), sizeof(num_seg_frames));
    ifs_.read(reinterpret_cast<char*>(&seg_header_offset), sizeof(seg_header_offset));
    
    int64_t start_pos = ifs_.tellg();
    ifs_.seekg(seg_header_offset);
    file_offsets_ = vector<int64_t>(num_seg_frames);
    time_stamps_ = vector<int64_t>(num_seg_frames);
    
    for (int i = 0; i < num_seg_frames; ++i) {
      int64_t pos;
      int64_t time_stamp;
      ifs_.read(reinterpret_cast<char*>(&pos), sizeof(pos));
      ifs_.read(reinterpret_cast<char*>(&time_stamp), sizeof(time_stamp));
      file_offsets_[i] = pos;
      time_stamps_[i] = time_stamp;
    }
    
    ifs_.seekg(start_pos);    
    return true;
  }
  
  void SegmentationReader::SeekToFrame(int frame) {
    ifs_.seekg(file_offsets_[frame]);
  }
  
  int SegmentationReader::ReadFrameSize() {
    ifs_.read(reinterpret_cast<char*>(&frame_sz_), sizeof(frame_sz_));
    return frame_sz_;
  }
  
  void SegmentationReader::ReadFrame(uchar* data) {
    ifs_.read(reinterpret_cast<char*>(data), frame_sz_);
  }  
  
}  // namespace Segment.