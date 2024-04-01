// The SFThread is mostly a simple wrapper around the Windows API thread
//   functions. It also provides a mechamism for communicating between
//   threads using Windows thread messages.

#if !defined __SFTHREADINC
  #define __SFTHREADINC 1

class SFThread
{public:
	SFThread::SFThread(unsigned int (__stdcall *address)(void *), void  *args,
			long stksize);

	void *args;
	uint  thrdid;
	ulong thrdhndl;
};

#endif
