/*hdr
**	FILE:			Gdc2.h
**	AUTHOR:			Matthew Allen
**	DATE:			20/2/97
**	DESCRIPTION:	GDC v2.xx header
**
**	Copyright (C) 1997, Matthew Allen
**		fret@memecode.com
*/

#include <stdio.h>
#include <math.h>
#include "Gdc2.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////
class GScreenPrivate
{
public:
	OsView View;
	int Depth;
	bool ClientClip;
	GRect Client;
	GRect Clip;
	
	GScreenPrivate()
	{
		View = 0;
		ClientClip = false;
		Depth = GdcD->GetBits();
	}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
GScreenDC::GScreenDC(OsView view, void *param)
{
	LgiAssert(view);

	d = NEW(GScreenPrivate);
	d->View = view;
}

GScreenDC::~GScreenDC()
{
	DeleteObj(d);
}

int GScreenDC::GetFlags()
{
	return 0;
}

void GScreenDC::SetClient(GRect *r)
{
	/* FIXME
	// Unset previous client
	if (d->Client.Valid())
	{
		d->View->SetOrigin(0, 0);
		d->View->ConstrainClippingRegion(0);
	}
	
	if (r)
	{
		d->Client = *r;
	}
	else
	{
		d->Client.ZOff(-1, -1);
	}

	// Set new client
	if (d->Client.Valid())
	{
		GRect b = d->Client;
		b.Offset(-b.x1, -b.y1);
		BRegion r(b);
		d->View->ConstrainClippingRegion(&r);
		d->View->SetOrigin(d->Client.x1, d->Client.y1);
	}
	*/
}

OsView GScreenDC::Handle()
{
	return d->View;
}

bool GScreenDC::IsScreen()
{
	return true;
}

GPalette *GScreenDC::Palette()
{
	return 0;
}

void GScreenDC::Palette(GPalette *pPal, bool bOwnIt)
{
}

uint GScreenDC::LineStyle()
{
	return 0;
}

uint GScreenDC::LineStyle(uint Bits, uint32 Reset)
{
	return 0;
}

void GScreenDC::GetOrigin(int &x, int &y)
{
	BPoint p = d->View->Origin();
	x = -p.x;
	y = -p.y;
}

void GScreenDC::SetOrigin(int x, int y)
{
	d->View->SetOrigin(-x, -y);
}

GRect GScreenDC::ClipRgn()
{
	/*
	if (d->View->LockLooper())
	{
		BRegion r;
		d->View->GetClippingRegion(&r);
		BRect rc = r.Frame();
		d->View->UnlockLooper();
		return GRect(rc);
	}
	
	GRect r(0, 0, -1, -1);
	return r;
	*/
	
	return d->Clip;
}

/*
void GScreenDC::_SetClient(GRect *c)
{
	d->ClientClip = c != 0;
	if (c)
	{
		d->_Client = *c;

		BRegion br;
		br.Set(d->_Client);
		d->View->ConstrainClippingRegion(&br);
	}
	else
	{
		d->View->ConstrainClippingRegion(0);
	}
}
*/

GRect GScreenDC::ClipRgn(GRect *Rgn)
{
	if (Rgn)
	{
		BRegion b;
		b.Set(*Rgn);
		d->View->ConstrainClippingRegion(&b);
		d->Clip = *Rgn;
	}
	else
	{
		d->View->ConstrainClippingRegion(NULL);
		d->Clip.ZOff(-1, -1);
	}
}

COLOUR GScreenDC::Colour()
{
	rgb_color Rgb = d->View->HighColor();
	return CBit(GetBits(), Rgb24(Rgb.red, Rgb.green, Rgb.blue), 24);
}

COLOUR GScreenDC::Colour(COLOUR c, int Bits)
{
	COLOUR Prev = Colour();

	if (Bits < 1) Bits = GetBits();
	c = CBit(24, c, Bits);
	
	rgb_color Rgb;
	Rgb.red = R24(c);
	Rgb.green = G24(c);
	Rgb.blue = B24(c);
	d->View->SetHighColor(Rgb);
	
	return Prev;
}

int GScreenDC::Op()
{
	int Mode = d->View->DrawingMode();
	switch (Mode)
	{
		case B_OP_ERASE:
		{
			return GDC_AND;
			break;
		}
		case B_OP_OVER:
		{
			return GDC_OR;
			break;
		}
		case B_OP_INVERT:
		{
			return GDC_XOR;
			break;
		}
	}
	return GDC_SET;
}

int GScreenDC::Op(int NewOp)
{
	int Prev = Op();
	drawing_mode Mode = B_OP_COPY;
	switch (NewOp)
	{
		case GDC_AND:
		{
			Mode = B_OP_ERASE;
			break;
		}
		case GDC_OR:
		{
			Mode = B_OP_OVER;
			break;
		}
		case GDC_XOR:
		{
			Mode = B_OP_INVERT;
			break;
		}
	}
	d->View->SetDrawingMode(Mode);
	return Prev;
}

int GScreenDC::X()
{
	BRect r = d->View->Bounds();
	return r.right - r.left + 1;
}

int GScreenDC::Y()
{
	BRect r = d->View->Bounds();
	return r.bottom - r.top + 1;
}

int GScreenDC::GetBits()
{
	return d->Depth;
}

void GScreenDC::Set(int x, int y)
{
	d->View->StrokeLine(BPoint(x, y), BPoint(x, y));
}

COLOUR GScreenDC::Get(int x, int y)
{
	// you can't get the value of a pixel!
	// fix this Be!!!!!!!!!!
	return 0;
}

void GScreenDC::HLine(int x1, int x2, int y)
{
	d->View->StrokeLine(BPoint(x1, y), BPoint(x2, y));
}

void GScreenDC::VLine(int x, int y1, int y2)
{
	d->View->StrokeLine(BPoint(x, y1), BPoint(x, y2));
}

void GScreenDC::Line(int x1, int y1, int x2, int y2)
{
	d->View->StrokeLine(BPoint(x1, y1), BPoint(x2, y2));
}

void GScreenDC::Circle(double cx, double cy, double radius)
{
	d->View->StrokeEllipse(BPoint(cx, cy), radius, radius);
}

void GScreenDC::FilledCircle(double cx, double cy, double radius)
{
	d->View->FillEllipse(BPoint(cx, cy), radius, radius);
}

void GScreenDC::Arc(double cx, double cy, double radius, double start, double end)
{
	d->View->StrokeArc(BPoint(cx, cy), radius, radius, start, end - start);
}

void GScreenDC::FilledArc(double cx, double cy, double radius, double start, double end)
{
	d->View->FillArc(BPoint(cx, cy), radius, radius, start, end - start);
}

void GScreenDC::Ellipse(double cx, double cy, double x, double y)
{
	d->View->StrokeEllipse(BPoint(cx, cy), x, y);
}

void GScreenDC::FilledEllipse(double cx, double cy, double x, double y)
{
	d->View->FillEllipse(BPoint(cx, cy), x, y);
}

void GScreenDC::Box(int x1, int y1, int x2, int y2)
{
	d->View->StrokeRect(BRect(x1, y1, x2, y2));
}

void GScreenDC::Box(GRect *a)
{
	BRect r;
	if (a)
	{
		r = *a;
	}
	else
	{
		r.left = r.top = 0;
		r.right = X()-1;
		r.bottom = Y()-1;
	}
	
	d->View->StrokeRect(r);
}

void GScreenDC::Rectangle(int x1, int y1, int x2, int y2)
{
	d->View->FillRect(BRect(x1, y1, x2, y2));
}

void GScreenDC::Rectangle(GRect *a)
{
	BRect r;
	if (a)
	{
		r = *a;
	}
	else
	{
		r.left = r.top = 0;
		r.right = X()-1;
		r.bottom = Y()-1;
	}
	
	d->View->FillRect(r);
}

void GScreenDC::Blt(int x, int y, GSurface *Src, GRect *a)
{
	if (Src)
	{
		if (Src->IsScreen())
		{
			// screen to screen Blt
		}
		else
		{
			/* FIXME
			// memory to screen Blt
			GMemDC *Dc = dynamic_cast<GMemDC*>(Src);
			BBitmap *Bmp = Dc ? Dc->GetBitmap() : 0;
			if (Bmp)
			{
				BRect S;
				if (a)
				{
					S = *a;
				}
				else
				{
					S.left = S.top = 0;
					S.right = Src->X() - 1;
					S.bottom = Src->Y() - 1;
				}
				
				BRect D(x, y, x+S.Width(), y+S.Height());
				
				d->View->DrawBitmap(Bmp, S, D);
			}
			*/
		}
	}
}

void GScreenDC::StretchBlt(GRect *d, GSurface *Src, GRect *s)
{
}

void GScreenDC::Polygon(int Points, GdcPt2 *Data)
{
	if (Points > 0 AND Data)
	{
		BPoint *p = NEW(BPoint[Points]);
		if (p)
		{
			for (int i=0; i<Points; i++)
			{
				p[i].x = Data[i].x;
				p[i].y = Data[i].y;
			}
			
			d->View->StrokePolygon(p, Points);
			DeleteArray(p);
		}
	}
}

void GScreenDC::Bezier(int Threshold, GdcPt2 *Pt)
{
}

void GScreenDC::FloodFill(int x, int y, int Mode, COLOUR Border, GRect *Bounds)
{
}

