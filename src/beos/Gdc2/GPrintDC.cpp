#include "Lgi.h"
#include "GdiLeak.h"

class GPrintDCPrivate
{
public:
	bool PageOpen;
	bool DocOpen;

	GPrintDCPrivate()
	{
		PageOpen = false;
		DocOpen = false;
	}
};

GPrintDC::GPrintDC(void *Handle, char *PrintJobName)
{
	d = NEW(GPrintDCPrivate);
}

GPrintDC::~GPrintDC()
{
	EndPage();
}

int GPrintDC::X()
{
	return 0;
}

int GPrintDC::Y()
{
	return 0;
}

int GPrintDC::GetBits()
{
	return 0;
}

int GPrintDC::DpiX()
{
	return 0; 
}

int GPrintDC::DpiY()
{
	return 0;
}

bool GPrintDC::StartPage()
{
	bool Status = false;

	return Status;
}

void GPrintDC::EndPage()
{
}

