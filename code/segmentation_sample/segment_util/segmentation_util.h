/*
 *  segmentation_util.h
 *  seamcarving
 *
 *  Created by Matthias Grundmann on 6/15/09.
 *  Copyright 2009 Matthias Grundmann. All rights reserved.
 *
 */

#ifndef SEGMENTATION_UTIL_H__
#define SEGMENTATION_UTIL_H__

#include "segmentation.pb.h"
#include <vector>

#ifdef _WIN32
#include <hash_map>
#else
#include <ext/hash_map>
#endif

namespace Segment {
  typedef unsigned char uchar;
  using std::vector;
  #ifndef _WIN32
  using __gnu_cxx::hash_map;
  #else
  using stdext::hash_map;
  #endif

  // Common usage for all functions:
  // The level of the hierarchy is passed in hierarchy_level.
  // 0: denotes the over-segmentation.
  // >1 : denotes a hierarchical level which is saved in the protobuffer at hierarchy_level - 1.
  //
  // Functions will threshold the passed level to max. level present in the hierarchy.
  //
  // The pixel level segmentation in SegmentationDesc and the actual hierarchy can
  // be separated, as the hierarchy is saved only ONCE for the whole video volume.
  // If a hierarchy is not specified in seg_hier, it is assumed that if
  // hierarchy_level > 0, desc contains a valid hierarchy.
  
  // Converts Segmentation description to image by assigning each pixel its
  // corresponding region id.
  void SegmentationDescToIdImage(int* img,
                                 int width_step,
                                 int width,
                                 int height,
                                 int hierarchy_level,
                                 const SegmentationDesc& desc,
                                 const SegmentationDesc* seg_hier = 0);
  
  // Renders each region with a random color for 3-channel 8-bit input image.
  // If highlight_boundary is set, region boundary will be colored black.
  void RenderRegionsRandomColor(char* img,
                                int width_step,
                                int width,
                                int height,
                                int hierarchy_level,
                                bool highlight_boundary,
                                const SegmentationDesc& desc,
                                const SegmentationDesc* seg_hier = 0);
  
  // Returns region_id at corresponding (x, y) location in image,
  // return value -1 indicates error.
  int GetRegionIdFromPoint(int x,
                           int y,
                           int hierarchy_level,
                           const SegmentationDesc& seg,
                           const SegmentationDesc* seg_hier = 0);  
  
  // DEPRECATED
  // Render the specified region_ids with 1 channel color in multi-channel image.
  void RenderRegions(const vector<int>& region_ids,
                     uchar color,
                     uchar* img,
                     int width_step,
                     int width,
                     int height,
                     int num_colors,
                     int hierarchy_level,
                     const SegmentationDesc& desc,
                     const SegmentationDesc* seg_hier = 0);
  // DEPRECATED  
  // Render the specified regions region_ids with associated 1 channel color.
  void RenderRegions(const vector<std::pair<int, uchar> >& region_color_pairs,
                     uchar* img,
                     int width_step,
                     int width,
                     int height,
                     int num_colors,
                     int hierarchy_level,
                     const SegmentationDesc& desc,
                     const SegmentationDesc* seg_hier = 0);
  
}  // namespace Segment.

#endif  // SEGMENTATION_UTIL_H__