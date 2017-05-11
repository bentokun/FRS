#pragma once

#include "Rules.h"

#ifdef _WIN32
#ifdef FRSV_EXPORTS
#define TFSAPI __declspec(dllexport)
#else
#define TFSAPI __declspec(dllimport)
#endif
#endif

namespace FRS {

	class TFSAPI ObserverCommitter {
		virtual void Notify() = 0;
	};

}