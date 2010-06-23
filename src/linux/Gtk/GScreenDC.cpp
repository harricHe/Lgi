/*hdr
**	FILE:			GScreenDC.cpp
**	AUTHOR:			Matthew Allen
**	DATE:			14/10/2000
**	DESCRIPTION:	GDC v2.xx header
**
**	Copyright (C) 2000, Matthew Allen
**		fret@memecode.com
*/

#include <stdio.h>
#include <math.h>

#include "Lgi.h"
using namespace Gtk;

class GScreenPrivate
{
public:
	int x, y, Bits;
	bool Own;
	COLOUR Col;
	GRect Client;

	OsView v;
	GdkDrawable *d;
	GdkGC *gc;

	GScreenPrivate()
	{
		x = y = Bits = 0;
		Own = false;
		Col = 0;
		v = 0;
		d = 0;
		gc = 0;
		Client.ZOff(-1, -1);
	}
	
	~GScreenPrivate()
	{
		if (gc)
			g_object_unref((Gtk::GObject*)g_type_check_instance_cast((Gtk::GTypeInstance*)gc, G_TYPE_OBJECT));
	}
};

// Translates are cumulative... so we undo the last one before resetting it.
#define UnTranslate()		// d->p.translate(OriginX, OriginY);
#define Translate()			// d->p.translate(-OriginX, -OriginY);

/////////////////////////////////////////////////////////////////////////////////////////////////////
GScreenDC::GScreenDC()
{
	d = new GScreenPrivate;
}

GScreenDC::GScreenDC(OsView View)
{
	d = new GScreenPrivate;
	d->v = View;
	d->d = View->window;	
	d->x = View->allocation.width;
	d->y = View->allocation.height;	
	if (d->gc = gdk_gc_new(View->window))
	{
	    GdkScreen *s = gdk_gc_get_screen(d->gc);
	    if (s)
	    {
	        GdkVisual *v = gdk_screen_get_system_visual(s);
	        if (v)
	        {
	            d->Bits = v->depth;
	        }
	    }
	}	
}

GScreenDC::GScreenDC(int x, int y, int bits)
{
	d = new GScreenPrivate;
	d->x = x;
	d->y = y;
	d->Bits = bits;
}

GScreenDC::GScreenDC(Gtk::GdkDrawable *Drawable)
{
	d = new GScreenPrivate;
	d->Own = false;
}

GScreenDC::GScreenDC(GView *view, void *param)
{
	d = new GScreenPrivate;
	if (view)
	{
		d->x = view->X();
		d->y = view->Y();
		d->Bits = 0;
		d->Own = false;

	    GdkScreen *s = gdk_display_get_default_screen(gdk_display_get_default());
	    if (s)
	    {
	        GdkVisual *v = gdk_screen_get_system_visual(s);
	        if (v)
	        {
	            d->Bits = v->depth;
	        }
	    }
    }
}

GScreenDC::~GScreenDC()
{
	UnTranslate();
	
	DeleteObj(d);
}

Gtk::cairo_t *GScreenDC::GetCairo()
{
	if (!Cairo)
	{
		Cairo = gdk_cairo_create(d->d);
	}
	return Cairo;
}

/*
Gtk::cairo_surface_t *GScreenDC::GetSurface(bool Render)
{
	if (!Surface)
	{
		if (Render)
		{
			Pango::xcb_render_pictforminfo_t format;
			
			Surface = Pango::cairo_xcb_surface_create_with_xrender_format(XcbConn(),
				      								GetDrawable(),
				      								XcbScreen(),
				      								&format,
				      								d->x,
				      								d->y);
			if (!Surface)
				printf("%s:%i - cairo_xcb_surface_create_with_xrender_format failed.\n", _FL);
		}
		else
		{
			Surface = Pango::cairo_xcb_surface_create(XcbConn(),
													GetDrawable(),
													get_root_visual_type(XcbScreen()),
													d->x,
													d->y);
			if (!Surface)
				printf("%s:%i - cairo_xcb_surface_create failed.\n", _FL);
		}
	}
	
	return Surface;
}
*/

GdcPt2 GScreenDC::GetSize()
{
	return GdcPt2(d->x, d->y);
}

Gtk::GdkDrawable *GScreenDC::GetDrawable()
{
  return 0;
}

void GScreenDC::SetClient(GRect *c)
{
	if (c)
	{
		d->Client = *c;
	}
	else
	{
		d->Client.ZOff(-1, -1);
	}
	
	OriginX = -d->Client.x1;
	OriginY = -d->Client.y1;
}

GRect *GScreenDC::GetClient()
{
	return &d->Client;
}

OsPainter GScreenDC::Handle()
{
	return 0;
}

uint GScreenDC::LineStyle()
{
	return GSurface::LineStyle();
}

uint GScreenDC::LineStyle(uint32 Bits, uint32 Reset)
{
	return GSurface::LineStyle(Bits);
}

int GScreenDC::GetFlags()
{
	return 0;
}

void GScreenDC::GetOrigin(int &x, int &y)
{
	return GSurface::GetOrigin(x, y);
}

void GScreenDC::SetOrigin(int x, int y)
{
	UnTranslate();
	GSurface::SetOrigin(x, y);
	Translate();
}

GRect GScreenDC::ClipRgn()
{
	return Clip;
}

GRect GScreenDC::ClipRgn(GRect *c)
{
	GRect Prev = Clip;
	UnTranslate();

	if (c)
	{
		Clip = *c;

		/*
		xcb_rectangle_t r = {c->x1-OriginX, c->y1-OriginY, c->X(), c->Y()};
		xcb_void_cookie_t c =
			xcb_set_clip_rectangles_checked (
				XcbConn(),
				XCB_CLIP_ORDERING_UNSORTED,
				*d->Gc,
				0,
				0,
				1,
				&r);
		XcbCheck(c);
		*/
	}
	else
	{
		/*
		xcb_rectangle_t r = {0, 0, X(), Y()};
		xcb_void_cookie_t c =
			xcb_set_clip_rectangles_checked (
				XcbConn(),
				XCB_CLIP_ORDERING_UNSORTED,
				*d->Gc,
				0,
				0,
				1,
				&r);
		XcbCheck(c);
		*/
	}

	Translate();
	return Prev;
}

COLOUR GScreenDC::Colour()
{
	return d->Col;
}

COLOUR GScreenDC::Colour(COLOUR c, int Bits)
{
	COLOUR Prev = Colour();

	d->Col = c;

	c = CBit(32, c, Bits>0 ? Bits : GdcD->GetBits());
	if (d->gc)
	{
		GdkColor col = { 0, (R32(c)<<8)|R32(c), (G32(c)<<8)|G32(c), (B32(c)<<8)|B32(c) };
		gdk_gc_set_rgb_fg_color(d->gc, &col);
		gdk_gc_set_rgb_bg_color(d->gc, &col);
	}

	return Prev;
}

int GScreenDC::Op(int ROP)
{
	int Prev = Op();

	switch (ROP)
	{
		case GDC_SET:
		{
			//d->p.setRasterOp(XPainter::CopyROP);
			break;
		}
		case GDC_OR:
		{
			//d->p.setRasterOp(XPainter::OrROP);
			break;
		}
		case GDC_AND:
		{
			//d->p.setRasterOp(XPainter::AndROP);
			break;
		}
		case GDC_XOR:
		{
			//d->p.setRasterOp(XPainter::XorROP);
			break;
		}
	}

	return Prev;
}

int GScreenDC::Op()
{
	/*
	switch (d->p.rasterOp())
	{
		case XPainter::CopyROP:
		{
			return GDC_SET;
			break;
		}
		case XPainter::OrROP:
		{
			return GDC_OR;
			break;
		}
		case XPainter::AndROP:
		{
			return GDC_AND;
			break;
		}
		case XPainter::XorROP:
		{
			return GDC_XOR;
			break;
		}
	}
	*/

	return GDC_SET;
}

int GScreenDC::X()
{
	return d->Client.Valid() ? d->Client.X() : d->x;
}

int GScreenDC::Y()
{
	return d->Client.Valid() ? d->Client.Y() : d->y;
}

int GScreenDC::GetBits()
{
	return d->Bits;
}

GPalette *GScreenDC::Palette()
{
	return 0;
}

void GScreenDC::Palette(GPalette *pPal, bool bOwnIt)
{
}

void GScreenDC::Set(int x, int y)
{
	gdk_draw_point(d->d, d->gc, x-OriginX, y-OriginY);
}

COLOUR GScreenDC::Get(int x, int y)
{
	return 0;
}

void GScreenDC::HLine(int x1, int x2, int y)
{
	gdk_draw_line(d->d, d->gc, x1-OriginX, y-OriginY, x2-OriginX, y-OriginY);
}

void GScreenDC::VLine(int x, int y1, int y2)
{
	gdk_draw_line(d->d, d->gc, x-OriginX, y1-OriginY, x-OriginX, y2-OriginY);
}

void GScreenDC::Line(int x1, int y1, int x2, int y2)
{
	gdk_draw_line(d->d, d->gc, x1-OriginX, y1-OriginY, x2-OriginX, y2-OriginY);
}

void GScreenDC::Circle(double cx, double cy, double radius)
{
	//d->p.drawArc(cx, cy, radius);
}

void GScreenDC::FilledCircle(double cx, double cy, double radius)
{
	//d->p.fillArc(cx, cy, radius);
}

void GScreenDC::Arc(double cx, double cy, double radius, double start, double end)
{
	//d->p.drawArc(cx, cy, radius, start, end);
}

void GScreenDC::FilledArc(double cx, double cy, double radius, double start, double end)
{
	//d->p.fillArc(cx, cy, radius, start, end);
}

void GScreenDC::Ellipse(double cx, double cy, double x, double y)
{
}

void GScreenDC::FilledEllipse(double cx, double cy, double x, double y)
{
}

void GScreenDC::Box(int x1, int y1, int x2, int y2)
{
	gdk_draw_rectangle(d->d, d->gc, false, x1-OriginX, y1-OriginY, x2-x1, y2-y1);
}

void GScreenDC::Box(GRect *a)
{
	if (a)
	{
		Box(a->x1, a->y1, a->x2, a->y2);
	}
	else
	{
		Box(0, 0, X()-1, Y()-1);
	}
}

void GScreenDC::Rectangle(int x1, int y1, int x2, int y2)
{
	if (x2 >= x1 &&
		y2 >= y1)
	{
		gdk_draw_rectangle(d->d, d->gc, true, x1-OriginX, y1-OriginY, x2-x1+1, y2-y1+1);
	}
}

void GScreenDC::Rectangle(GRect *a)
{
	if (a)
	{
		if (a->X() > 0 &&
			a->Y() > 0)
		{
			gdk_draw_rectangle(d->d, d->gc, true, a->x1-OriginX, a->y1-OriginY, a->X(), a->Y());
		}
	}
	else
	{
		gdk_draw_rectangle(d->d, d->gc, true, -OriginX, -OriginY, X(), Y());
	}
}

void GScreenDC::Blt(int x, int y, GSurface *Src, GRect *a)
{
	if (!Src)
	{
		printf("%s:%i - No source.\n", _FL);
		return;
	}
	
	if (Src->IsScreen())
	{
		printf("%s:%i - Can't do screen->screen blt.\n", _FL);
		return;
	}
		
	// memory -> screen blt
	GBlitRegions br(this, x, y, Src, a);
	if (!br.Valid())
	{
		/*
		printf("%s:%i - Invalid clip region:\n"
				"\tDst=%i,%i @ %i,%i Src=%i,%i @ %s\n",
				_FL,
				X(), Y(),
				x, y,
				Src->X(), Src->Y(),
				a ? a->GetStr() : 0);
		br.Dump();
		*/
		return;
	}

	GMemDC *Mem;
	
	/*
	if (Src->GetBits() != GetBits())
	{
		// Do on the fly depth conversion...
		GMemDC Tmp(br.SrcClip.X(), br.SrcClip.Y(), GetBits());
		Tmp.Blt(0, 0, Src, &br.SrcClip);
		Blt(x, y, &Tmp);
	}
	else */
	if (Mem = dynamic_cast<GMemDC*>(Src))
	{
		gdk_draw_image( d->d,
		                d->gc,
                        Mem->GetImage(),
                        br.SrcClip.x1, br.SrcClip.y1,
                        x-OriginX, y-OriginY,
                        br.SrcClip.X(), br.SrcClip.Y());
	}
}

void GScreenDC::StretchBlt(GRect *d, GSurface *Src, GRect *s)
{
}

void GScreenDC::Polygon(int Points, GdcPt2 *Data)
{
}

void GScreenDC::Bezier(int Threshold, GdcPt2 *Pt)
{
}

void GScreenDC::FloodFill(int x, int y, int Mode, COLOUR Border, GRect *Bounds)
{
}

