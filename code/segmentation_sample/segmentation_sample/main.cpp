/*
 *  main.cpp
 *  segmentation_sample
 *
 *  Created by Matthias Grundmann on 6/17/2010.
 *  Copyright 2010 Matthias Grundmann. All rights reserved.
 *
 */

#include <iostream>
#include <string>
#include <cv.h>
#include <highgui.h>

#include "assert_log.h"
#include "segmentation_io.h"
#include "segmentation_util.h"

using namespace Segment;

// Slider positions.
int g_hierarchy_level;
int g_frame_pos;

// Frame width and height.
int g_frame_num_;
int g_frame_width;
int g_frame_height;

// This should be scoped_ptr's. Removed to remove boost dependency.
SegmentationReader* g_segment_reader;
SegmentationDesc* g_seg_hierarchy;

// Render target.
IplImage* g_frame_buffer = 0;

// Indicates if automatic playing is set.
bool g_playing;

// Returns number of hierarchy_levels.
void RenderCurrentFrame(int frame_number) {
  g_segment_reader->SeekToFrame(frame_number);
  
  // Read from file.
  vector<Segment::uchar> data_buffer(g_segment_reader->ReadFrameSize());
  g_segment_reader->ReadFrame(&data_buffer[0]);
  
  SegmentationDesc segmentation;
  segmentation.ParseFromArray(&data_buffer[0], data_buffer.size());
  
  // Allocate frame_buffer if necessary
  if (g_frame_buffer == NULL) {
    g_frame_buffer = cvCreateImage(cvSize(g_frame_width,
                                          g_frame_height), IPL_DEPTH_8U, 3);
  }
  
  // Render segmentation at specified level.
  RenderRegionsRandomColor(g_frame_buffer->imageData,
                           g_frame_buffer->widthStep,
                           g_frame_buffer->width,
                           g_frame_buffer->height,
                           g_hierarchy_level,
                           true,
                           segmentation,
                           g_seg_hierarchy);
}

void FramePosChanged(int pos) {
  g_frame_pos = pos;
  RenderCurrentFrame(g_frame_pos);
  cvShowImage("main_window", g_frame_buffer);
}

void HierarchyLevelChanged(int level) {
  g_hierarchy_level = level;
  RenderCurrentFrame(g_frame_pos);
  cvShowImage("main_window", g_frame_buffer);
}

int main(int argc, char** argv) {
  // Get filename from command prompt.
  if (argc != 2) {
    std::cout << "Usage: segmentation_sample FILE_NAME\n";
    return 1;
  }
  
  std::string filename(argv[1]);
  g_playing = false;
  
  // Read segmentation file.
  g_segment_reader = new SegmentationReader(filename);
  g_segment_reader->OpenFileAndReadHeader();
  g_frame_pos = 0;
  
  std::cout << "Segmentation file " << filename << " contains " 
            << g_segment_reader->FrameNumber() << " frames.\n";
  
  // Read first frame, it contains the hierarchy.
  vector<Segment::uchar> data_buffer(g_segment_reader->ReadFrameSize());
  g_segment_reader->ReadFrame(&data_buffer[0]);
  
  // Save hierarchy for all frames.
  g_seg_hierarchy = new SegmentationDesc;
  g_seg_hierarchy->ParseFromArray(&data_buffer[0], data_buffer.size());

  g_frame_num_ = g_segment_reader->FrameNumber();
  g_frame_width = g_seg_hierarchy->frame_width();
  g_frame_height = g_seg_hierarchy->frame_height();
  
  std::cout << "Video resolution: " << g_frame_width << "x" << g_frame_height << "\n";
  
  // Create OpenCV window.
  cvNamedWindow("main_window");
  
  RenderCurrentFrame(0);
  cvShowImage("main_window", g_frame_buffer);
  
  cvCreateTrackbar("frame_pos", 
                   "main_window", 
                   &g_frame_pos, 
                   g_segment_reader->FrameNumber() - 1,
                   &FramePosChanged);  

  cvCreateTrackbar("hier_level", 
                   "main_window", 
                   &g_hierarchy_level, 
                   g_seg_hierarchy->hierarchy_size(),
                   &HierarchyLevelChanged);  

  
  int key_value = 0;
  
  // Yotam Doron recommended this kind of loop.
  while (1) {
    key_value = cvWaitKey(30) & 0xFF;
    if (key_value == 27) {
      break;
    }

    if (g_playing) {
      FramePosChanged((g_frame_pos + 1) % g_frame_num_);
    }
    
    switch (key_value) {
      case 110:
        FramePosChanged((g_frame_pos + 1) % g_frame_num_);
        break;
      case 112:
        FramePosChanged((g_frame_pos - 1 + g_frame_num_) % g_frame_num_);
        break;
      case 32:
        g_playing = !g_playing;
      default:
        break;
    }
  }
  
  g_segment_reader->CloseFile();
  delete g_segment_reader;
  
  cvReleaseImage(&g_frame_buffer);
  delete g_seg_hierarchy;
  
   return 0;
}
