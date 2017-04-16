#ifndef ASSERTSSE_H
#define ASSERTSSE_H

#include <iostream>
#include <string>
#include <Windows.h>

#ifdef _DEBUG

#define FRS_S_ASSERT(x) \
             if ((x)) \
               { \
                 std::cout << "[Line: "<<__LINE__<<" ; File: "<<__FILE__<<"]: ERROR: "<< #x <<std::endl; \
               }

#define FRS_ASSERT(x,message)\
	 if ((x)) \
	 { \
		 std::cout << "[Line: " << __LINE__ << " ; File: " << __FILE__ << "]: ERROR: " << #message << std::endl; \
	 }

#define FRS_ASSERT_WV(x,message,v1,v2)\
	 if ((x)) \
	 { \
		 std::cout << "[Line: " << __LINE__ << " ; File: " << __FILE__ << "]: ERROR: " << #message <<". Value: "<<v1<<","<<v2 << std::endl; \
	 }

#define FRS_MESSAGE(x) \
             if ((x)) \
               { \
                 std::cout << "[Line: "<<__LINE__<<" ; File: "<<__FILE__<<"]: ERROR: "<< x <<std::endl; \
               }

#else
#define FRSML_ASSERT(x,message)\
	if (x) {}\

#define FRSML_ASSERT_WV(x,message,v1,v2)
if (x) {}\

#define FRSML_S_ASSERT(x)
if (x) {}\

#define FRS_MESSAGE(x)

#endif

#if _WIN32
#define FRS_FATAL_ERROR(message)\
	 { \
		std::cout << "[Line: " << __LINE__ << " ; File: " << __FILE__ << "]: FATAL ERROR: " << message << std::endl; \
		throw std::runtime_error(message); \
	 }
#endif
#endif

