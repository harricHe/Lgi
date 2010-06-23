#include "Lgi.h"
#include <errno.h>

////////////////////////////////////////////////////////////////////////////
void *ThreadEntryPoint(void *i)
{
	if (i)
	{
		GThread *Thread = (GThread*) i;

		// Make sure we have finished executing the setup
		while (Thread->State == LGITHREAD_INIT)
		{
			LgiSleep(5);
		}
		
		pthread_detach(Thread->hThread);

		// Do thread's work
		Thread->OnBeforeMain();
		Thread->ReturnValue = Thread->Main();
		Thread->OnAfterMain();

		// mark thread over...
		Thread->State = LGITHREAD_EXITED;

		if (Thread->DeleteOnExit)
		{
			DeleteObj(Thread);
		}

		pthread_exit(0);
	}
	return 0;
}

GThread::GThread()
{
	State = LGITHREAD_ASLEEP;
	ReturnValue = -1;
	hThread = 0;
	DeleteOnExit = false;
}

GThread::~GThread()
{
	if (!IsExited())
	{
		Terminate();
	}
}

int GThread::ExitCode()
{
	return ReturnValue;
}

bool GThread::IsExited()
{
	return State == LGITHREAD_EXITED;
}

void GThread::Run()
{
	if (!hThread)
	{
		State = LGITHREAD_INIT;

		static int Creates = 0;
		int e;
		if (!(e = pthread_create(&hThread, NULL, ThreadEntryPoint, (void*)this)))
		{
			Creates++;
			State = LGITHREAD_RUNNING;
		}
		else
		{
			const char *Err = "(unknown)";
			switch (e)
			{
				case EAGAIN: Err = "EAGAIN"; break;
				case EINVAL: Err = "EINVAL"; break;
				case EPERM: Err = "EPERM"; break;
				case ENOMEM: Err = "ENOMEM"; break;
			}
			printf("%s,%i - pthread_create failed with the error %i (%s) (After %i creates)\n", __FILE__, __LINE__, e, Err, Creates);
			
			State = LGITHREAD_EXITED;
		}
	}
}

void GThread::Terminate()
{
	if (hThread AND
		pthread_cancel(hThread) == 0)
	{
		State = LGITHREAD_EXITED;
	}
}

int GThread::Main()
{
	return 0;
}

