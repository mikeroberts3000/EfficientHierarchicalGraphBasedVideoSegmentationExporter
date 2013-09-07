/*
 *  assert_log.h
 *  assert_log
 *
 *  Created by Matthias Grundmann on 5/22/09.
 *  Copyright 2009 Matthias Grundmann. All rights reserved.
 *
 *  Based on: http://www.ddj.com/cpp/184403861   ('Assertions' by Andrei Alexandrescu)
 *  Logging inspired by google testing framework.
 */

// Defines two macros
// ASSERT_LOG
// ASSURE_LOG

// Usage: [ASSERT|ERROR]_LOG(condition) << {Some message} << {another message} ...
// ASSERT_LOG will be ignored if NDEBUG is defined - otherwise it breaks at the current line.
// ASSURE_LOG is ALWAYS evaluated. 
// It aborts if NDEBUG is defined otherwise breaks at the current line.



#ifndef ASSERT_LOG_H__
#define ASSERT_LOG_H__

#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>

class AssertLog {
public:
  
  AssertLog(const std::string& file, const std::string& function, int line) :
    file_(file), function_(function), line_(line) {}
  
  AssertLog& operator()(bool condition) {
    cond_ = condition;
    return *this;
  }
  
  AssertLog(const AssertLog& log) : 
      cond_(log.cond_), file_(log.file_), function_(log.function_), line_(log.line_) {
    log_message_ << log.log_message_.str();
  }
  
  bool evaluate_condition() const {
    if (cond_) 
      return true;
    else {
      std::cerr << "\n[ERROR] " << file_ << " (" << line_ << ") " << "in " << function_
                << ": " << log_message_.str() << "\n";
      return false;
    }
  }
  
  template <class T>
  AssertLog& operator<<(const T& output) {  
    log_message_ << output;
    return *this;
  }
  
private:
  bool cond_;
  std::ostringstream log_message_;
  std::string file_;
  std::string function_;
  int line_;

};

#if defined __APPLE__
  #define DEBUG_BREAK { __asm {int 3} }
#elif defined _WIN32
  #include <windows.h>
  #define DEBUG_BREAK DebugBreak()
#elif defined __linux

  extern "C"{
    #include <signal.h>
  }

  #define DEBUG_BREAK raise(SIGINT)
#endif

// ASSERT_LOG definition.

#ifdef NDEBUG

#define ASSERT_LOG \
if (true) ; \
  else \
    struct Local { \
      Local(const AssertLog& assert_log) { \
        if (!assert_log.evaluate_condition()) { \
          DEBUG_BREAK; \
        } \
      } \
    } local_assert = AssertLog(__FILE__, __FUNCTION__, __LINE__)

#else

#define ASSERT_LOG \
if (false) ; \
else \
  struct Local { \
    Local(const AssertLog& assert_log) { \
      if (!assert_log.evaluate_condition()) { \
      DEBUG_BREAK; } \
    } \
  } local_assert =AssertLog(__FILE__, __FUNCTION__, __LINE__)

#endif  // NDEBUG


// ERROR_LOG definition


#ifdef NDEBUG 

#define ASSURE_LOG \
  if (false) ; \
    else \
  struct Local { \
    Local(const AssertLog& assert_log) { \
      if (!assert_log.evaluate_condition()) { \
      exit(1);  } \
    } \
  } local_assert = AssertLog(__FILE__, __FUNCTION__, __LINE__)

#else

#define ASSURE_LOG ASSERT_LOG

#endif  // NDEBUG

#endif  // ASSERT_LOG_H__
