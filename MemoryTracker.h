#ifndef MEMORY_TRACKER_H

#define MEMORY_TRACKER_H

namespace NVSHARE
{

	enum MemoryType
	{
		MT_NEW,
		MT_NEW_ARRAY,
		MT_MALLOC,
		MT_FREE,
		MT_DELETE,
		MT_DELETE_ARRAY,
		MT_GLOBAL_NEW,
		MT_GLOBAL_NEW_ARRAY,
		MT_GLOBAL_DELETE,
		MT_GLOBAL_DELETE_ARRAY,
	};



class MemoryTracker
{
public:

  virtual void setLogLevel(bool logEveryAllocation,bool logEveyFrame) = 0;

  virtual void lock(void) = 0; // mutex lock.
  virtual void unlock(void) = 0; // mutex unlock

  // it will also fire asserts in a debug build.  The default is false for performance reasons.

  virtual void trackAlloc(void *mem,
                          unsigned int size,
                          MemoryType type,
                          const char *context,
                          const char *className,
                          const char *fileName,
                          int lineno) = 0;

  virtual void trackRealloc(void *oldMem,
	                        void *newMem,
							size_t newSize,
							MemoryType freeType,
							MemoryType allocType,
							const char *context,
							const char *className,
							const char *fileName,
							int lineno) = 0;

  virtual void trackFree(void *mem,
                         MemoryType type,
                         const char *context,
                         const char *fileName,
                         int lineno) = 0;

  virtual void trackFrame(void) = 0;


  virtual bool detectMemoryLeaks(const char *fname,bool reportAllLeaks=true) = 0; // detect memory leaks and, if any, write out a report to the filename specified.

};

void createMemoryTracker(const char *memoryTrackerDLL);

}; // end of namespace

#define MEMORY_TRACKER_VERSION 100

extern NVSHARE::MemoryTracker *gMemoryTracker;



#endif
