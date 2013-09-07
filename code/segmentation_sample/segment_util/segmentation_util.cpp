/*
 *  segmentation_util.cpp
 *  seamcarving
 *
 *  Created by Matthias Grundmann on 6/15/09.
 *  Copyright 2009 Matthias Grundmann. All rights reserved.
 *
 */

#include "segmentation_util.h"
#include "assert_log.h"

#include <algorithm>
#include <google/protobuf/repeated_field.h>
using google::protobuf::RepeatedPtrField;

typedef unsigned char uchar;

#ifdef _WIN32
  #undef min
  #undef max
#endif

// Moved from imagefilter here to remove library dependency.
namespace {
  template <class T>
  T* PtrOffset(T* t, int offset) {
    return reinterpret_cast<T*>(reinterpret_cast<uchar*>(t) + offset);
  }
  
  template <class T>
  const T* PtrOffset(const T* t, int offset) {
    return reinterpret_cast<const T*>(reinterpret_cast<const uchar*>(t) + offset);
  }
  
  int ColorDiff_L1(const char* first, const char* second) {
    return abs((int)first[0] - (int)second[0]) +
           abs((int)first[1] - (int)second[1]) +
           abs((int)first[2] - (int)second[2]);
  }
  
}

namespace Segment {
  
  typedef SegmentationDesc::Region SegRegion;
  typedef SegmentationDesc::Region::Scanline Scanline;
  typedef SegmentationDesc::Region::Scanline::Interval ScanlineInterval;
  
  void SegmentationDescToIdImage(int* img,
                                 int width_step,
                                 int width,
                                 int height,
                                 int level,
                                 const SegmentationDesc& seg, 
                                 const SegmentationDesc* seg_hier) {
    
    if (level > 0 && seg.hierarchy_size() != 0) {
      // Is a hierarchy present at the current frame?
      seg_hier = &seg;
    }
    
    if (level)
      level = std::min(level, seg_hier->hierarchy_size());
    
    ASSURE_LOG(level == 0 || seg_hier) << "Hierarchy requested but not found.";
    ASSURE_LOG(level == 0 || level <= seg_hier->hierarchy_size()) << "Requested hierarchy exceeds"
        << " levels supplied in seg or seg_hier.";
    
    // Fill each region with it's id.
    const RepeatedPtrField<SegRegion>& regions = seg.region();
    for (RepeatedPtrField<SegRegion>::const_iterator r = regions.begin();
         r != regions.end();
         ++r) {
      
           
      // Get id.
      int region_id;
      if (level == 0)
        region_id = r->id();
      else {
        // Traverse to parent region.
        int parent_id = r->parent_id();
        
        for (int l = 0; l < level - 1; ++l) {
          ASSERT_LOG(seg_hier->hierarchy(l).region_size() >= parent_id);
          parent_id = seg_hier->hierarchy(l).region(parent_id).parent_id();
        }
        
        region_id = parent_id;
      }
      
      const RepeatedPtrField<Scanline>& scanlines = r->scanline();
      int* dst_ptr = PtrOffset(img, r->top_y() * width_step);
      
      for(RepeatedPtrField<Scanline>::const_iterator s = scanlines.begin();
          s != scanlines.end();
          ++s) {
        for (int i = 0, sz = s->interval_size(); i < sz; ++i) {
          const ScanlineInterval& inter = s->interval(i);
          int* out_ptr = dst_ptr + inter.left_x();
          for (int j = 0, len = inter.right_x() - inter.left_x() + 1; j < len; ++j, ++out_ptr) {
            *out_ptr = region_id;
          }
        }
        dst_ptr = PtrOffset(dst_ptr, width_step);
      }
    }  
  }
  
  
  void RenderRegionsRandomColor(char* img,
                                int width_step,
                                int width,
                                int height,
                                int level,
                                bool highlight_boundary,
                                const SegmentationDesc& seg,
                                const SegmentationDesc* seg_hier) {
    // Clear image.
    memset(img, 0, width_step * height);
    
    if (level > 0 && seg.hierarchy_size() != 0) {
      // Is a hierarchy present at the current frame?
      seg_hier = &seg;
    }
    
    if (level)
      level = std::min(level, seg_hier->hierarchy_size());
    
    ASSURE_LOG(level == 0 || seg_hier) << "Hierarchy requested but not found.";
    
    // Fill each region.
    const RepeatedPtrField<SegRegion>& regions = seg.region();
    for(RepeatedPtrField<SegRegion>::const_iterator r = regions.begin();
        r != regions.end();
        ++r) {
      // Get color.
      uchar color[3];
      
      int region_id;
      
      if (level == 0)
        region_id = r->id();
      else {
        // Traverse to parent region.
        int parent_id = r->parent_id();
        
        for (int l = 1; l < level; ++l) {
          assert(seg_hier->hierarchy(l-1).region_size() > parent_id);
          parent_id = seg_hier->hierarchy(l-1).region(parent_id).parent_id();
        }
        
        region_id = parent_id;
      }
      
      // Use region id as seed.
      srand(region_id);
      color[0] = (uchar) (rand() % 255);
      color[1] = (uchar) (rand() % 255);
      color[2] = (uchar) (rand() % 255);
      
      const RepeatedPtrField<Scanline>& scanlines = r->scanline();
      char* dst_ptr =img + width_step * r->top_y();
      
      for(RepeatedPtrField<Scanline>::const_iterator s = scanlines.begin();
          s != scanlines.end();
          ++s) {
        for (int i = 0, sz = s->interval_size(); i < sz; ++i) {
          const ScanlineInterval& inter = s->interval(i);
          char* out_ptr = dst_ptr + inter.left_x() * 3;
          for (int j = 0, len = inter.right_x() - inter.left_x() + 1;
               j < len;
               ++j, out_ptr += 3) {
            out_ptr[0] = color[0];
            out_ptr[1] = color[1];
            out_ptr[2] = color[2];
          }
        }
        dst_ptr += width_step;
      }
    }
    
    // Edge highlight post-process.
    if (highlight_boundary) {
      for (int i = 0; i < height - 1; ++i) {
        char* row_ptr = img + i * width_step;
        for (int j = 0; j < width - 1; ++j, row_ptr += 3) {
          if (ColorDiff_L1(row_ptr, row_ptr + 3) != 0 ||
              ColorDiff_L1(row_ptr, row_ptr + width_step) != 0) 
            row_ptr[0] = row_ptr[1] = row_ptr[2] = 0;
        }
        
        // Last column.
        if (ColorDiff_L1(row_ptr, row_ptr + width_step) != 0)
          row_ptr[0] = row_ptr[1] = row_ptr[2] = 0;
      }
      
      // Last row.
      char* row_ptr = img + width_step * (height - 1);
      for (int j = 0; j < width - 1; ++j, row_ptr += 3) {
        if (ColorDiff_L1(row_ptr, row_ptr + 3) != 0)
          row_ptr[0] = row_ptr[1] = row_ptr[2] = 0;
      }      
    }    
  }
  
  int GetRegionIdFromPoint(int x, int y, int level, const SegmentationDesc& seg,
                           const SegmentationDesc* seg_hier) {
    
    if (level > 0 && seg.hierarchy_size() != 0) {
      // Is a hierarchy present at the current frame?
      seg_hier = &seg;
    }
    
    if (level)
      level = std::min(level, seg_hier->hierarchy_size());
    
    ASSURE_LOG(level == 0 || seg_hier) << "Hierarchy requested but not found.";
    ASSURE_LOG(level == 0 || level <= seg_hier->hierarchy_size()) << "Requested hierarchy exceeds"
    << " levels supplied in seg or seg_hier.";    
    
    const RepeatedPtrField<SegRegion>& regions = seg.region();
    for(RepeatedPtrField<SegRegion>::const_iterator r = regions.begin();
        r != regions.end();
        ++r) {
      
      // Is y within the regions range?
      if (y >= r->top_y() && y < r->top_y() + r->scanline_size()) {
        // Jump to specific scanline.
        const Scanline& s = r->scanline(y - r->top_y());
        
        // Is x in range?
        for (int i = 0, sz = s.interval_size(); i < sz; ++i) {
          const ScanlineInterval& inter = s.interval(i);
          if (x >= inter.left_x() && x <= inter.right_x()) {
            // Get my id and return.
            if (level == 0)
              return r->id();
            else {
              int parent_id = r->parent_id();
              
              for (int l = 0; l < level-1; ++l) {
                ASSERT_LOG(seg_hier->hierarchy(l).region_size() > parent_id);
                parent_id = seg_hier->hierarchy(l).region(parent_id).parent_id();
              }
              
              return parent_id;
            }
          }
        }
      }
    }
    return -1;
  }
  
  
  void RenderRegions(const vector<int>& region_ids,
                     uchar color,
                     uchar* img,
                     int width_step,
                     int width,
                     int height,
                     int num_colors,
                     int level,
                     const SegmentationDesc& seg,
                     const SegmentationDesc* seg_hier) {
    
    // Make sure region_ids is sorted.
    vector<int> region_ids_sorted(region_ids);
    std::sort(region_ids_sorted.begin(), region_ids_sorted.end());
    
    if (level > 0 && seg.hierarchy_size() != 0) {
      // Is a hierarchy present at the current frame?
      seg_hier = &seg;
    }
    
    if (level)
      level = std::min(level, seg_hier->hierarchy_size());
    
    ASSURE_LOG(level == 0 || seg_hier) << "Hierarchy requested but not found.";
    ASSURE_LOG(level == 0 || level <= seg_hier->hierarchy_size()) << "Requested hierarchy exceeds"
       << " levels supplied in seg or seg_hier.";
    

    // Mark each region found in region_ids with color.
    const RepeatedPtrField<SegRegion>& regions = seg.region();
    for(RepeatedPtrField<SegRegion>::const_iterator r = regions.begin();
        r != regions.end();
        ++r) {
      
      // Get id.
      int region_id;
      if (level == 0)
        region_id = r->id();
      else {
        // Traverse to parent region.
        int parent_id = r->parent_id();
        
        for (int l = 0; l < level-1; ++l) {
          ASSERT_LOG(seg_hier->hierarchy(l).region_size() > parent_id);
          parent_id = seg_hier->hierarchy(l).region(parent_id).parent_id();
        }
        
        region_id = parent_id;
      }
      
      vector<int>::const_iterator pos = std::lower_bound(region_ids_sorted.begin(),
                                                         region_ids_sorted.end(), region_id);
      if (pos != region_ids_sorted.end() && *pos == region_id) {
      
        const RepeatedPtrField<Scanline>& scanlines = r->scanline();
        uchar* dst_ptr = PtrOffset(img, r->top_y() * width_step);
      
        for(RepeatedPtrField<Scanline>::const_iterator s = scanlines.begin();
            s != scanlines.end();
            ++s) {
          for (int i = 0, sz = s->interval_size(); i < sz; ++i) {
            const ScanlineInterval& inter = s->interval(i);
            uchar* out_ptr = dst_ptr + inter.left_x() * num_colors;
            for (int j = 0, len = inter.right_x() - inter.left_x() + 1;
                 j < len;
                 ++j, out_ptr+=num_colors) {
              *out_ptr = color;
            }
          }

          dst_ptr = PtrOffset(dst_ptr, width_step);
        }
      }
    }  
  }
  
  typedef std::pair<int, uchar> RegionColor;
  struct RegionColorComp : public std::binary_function<bool, RegionColor, RegionColor> {
        
    bool operator()(const RegionColor& r1, const RegionColor& r2) {
      return r1.first < r2.first;
    }
    
  };  
  
  void RenderRegions(const vector<std::pair<int, uchar> >& region_color_pairs,
                     uchar* img,
                     int width_step,
                     int width,
                     int height,
                     int num_colors,
                     int level,
                     const SegmentationDesc& seg,
                     const SegmentationDesc* seg_hier) {
    
    // Make sure region_ids is sorted.
    vector<RegionColor> region_ids_sorted(region_color_pairs);
    std::sort(region_ids_sorted.begin(), region_ids_sorted.end(), RegionColorComp());
    
    if (level > 0 && seg.hierarchy_size() != 0) {
      // Is a hierarchy present at the current frame?
      seg_hier = &seg;
    }
    
    if (level)
      level = std::min(level, seg_hier->hierarchy_size());
    
    ASSURE_LOG(level == 0 || seg_hier) << "Hierarchy requested but not found.";
    ASSURE_LOG(level == 0 || level <= seg_hier->hierarchy_size()) << "Requested hierarchy exceeds"
    << " levels supplied in seg or seg_hier.";
    
    
    // Mark each region found in region_ids with color.
    const RepeatedPtrField<SegRegion>& regions = seg.region();
    for(RepeatedPtrField<SegRegion>::const_iterator r = regions.begin();
        r != regions.end();
        ++r) {
      
      // Get id.
      int region_id;
      if (level == 0)
        region_id = r->id();
      else {
        // Traverse to parent region.
        int parent_id = r->parent_id();
        
        for (int l = 0; l < level - 1; ++l) {
          ASSERT_LOG(seg_hier->hierarchy(l).region_size() > parent_id);
          parent_id = seg_hier->hierarchy(l).region(parent_id).parent_id();
        }
        
        region_id = parent_id;
      }
      
      vector<std::pair<int, uchar> >::const_iterator pos =
          std::lower_bound(region_ids_sorted.begin(), region_ids_sorted.end(),
                           std::make_pair<int, uchar>(region_id, 0), RegionColorComp());
      if (pos != region_ids_sorted.end() && pos->first == region_id) {
        uchar color = pos->second;
        const RepeatedPtrField<Scanline>& scanlines = r->scanline();
        uchar* dst_ptr = PtrOffset(img, r->top_y() * width_step);
        
        for(RepeatedPtrField<Scanline>::const_iterator s = scanlines.begin();
            s != scanlines.end();
            ++s) {
          for (int i = 0, sz = s->interval_size(); i < sz; ++i) {
            const ScanlineInterval& inter = s->interval(i);
            uchar* out_ptr = dst_ptr + inter.left_x() * num_colors;
            for (int j = 0, len = inter.right_x() - inter.left_x() + 1;
                 j < len;
                 ++j, out_ptr+=num_colors) {
              *out_ptr = color;
            }
          }
          
          dst_ptr = PtrOffset(dst_ptr, width_step);
        }
      }
    }  
  }
}