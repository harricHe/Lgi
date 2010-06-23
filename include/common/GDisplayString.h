
#ifndef _GDISPLAY_STRING_H_
#define _GDISPLAY_STRING_H_

#ifdef LINUX
namespace Pango
{
#include "pango/pango.h"
#include "pango/pangocairo.h"
}
#endif

/// \brief Cache for text measuring, glyph substitution and painting
///
/// To paint text onto the screen several stages need to be implemented to
/// properly support unicode on multiple platforms. This class addresses all
/// of those needs and then allows you to cache the results to reduce text
/// related workload.
///
/// The first stage is converting text into the native format for the 
/// OS's API. This usually involved converting the text to wide characters for
/// Linux or Windows, or Utf-8 for BeOS. Then the text is converted into runs of
/// characters that can be rendered in the same font. If glyph substitution is
/// required to render the characters a separate run is used with a different
/// font ID. Finally you can measure or paint these runs of text. Also tab characters
/// are expanded to the current tab size setting.
class LgiClass GDisplayString
{
	GSurface *pDC;
	OsChar *Str;
	GFont *Font;
	int x, y, len, TabOrigin;
	int Blocks;
	class CharInfo *Info;
	uint8 Flags;
	
	#if defined MAC
	
	ATSUTextLayout Hnd;
	ATSUTextMeasurement fAscent;
	ATSUTextMeasurement fDescent;
	
	#elif defined __GTK_H__
	
	Gtk::PangoLayout *Hnd;
	
	#endif

	void Layout();

public:
	/// Constructor
	GDisplayString
	(
		/// The base font. Must not be destroyed during the lifetime of this object.
		GFont *f,
		/// Utf-8 input string
		char *s,
		/// Number of bytes in the input string or -1 for NULL terminated.
		int l = -1,
		GSurface *pdc = 0,
		int tabOrigin = 0
	);
	/// Constructor
	GDisplayString
	(
		/// The base font. Must not be destroyed during the lifetime of this object.
		GFont *f,
		/// A wide character input string
		char16 *s,
		/// The number of characters in the input string (NOT the number of bytes) or -1 for NULL terminated
		int l = -1,
		GSurface *pdc = 0,
		int tabOrigin = 0
	);
	virtual ~GDisplayString();
	
	/// Returns the ShowVisibleTab setting.
	/// Treats Unicode-2192 (left arrow) as a tab char
	bool ShowVisibleTab();
	/// Sets the ShowVisibleTab setting.
	/// Treats Unicode-2192 (left arrow) as a tab char
	void ShowVisibleTab(bool i);

	/// \returns the font used to create the layout
	GFont *GetFont() { return Font; };

	/// Fits string into 'width' pixels, truncating with '...' if it's not going to fit
	void TruncateWithDots
	(
		/// The number of pixels the string has to fit
		int Width
	);
	/// Returns true if the string is trucated
	bool IsTruncated();

	/// Returns the chars in the OsChar string
	int Length();
	/// Sets the number of chars in the OsChar string
	void Length(int NewLen);

	/// Returns the pointer to the native string
	operator OsChar*() { return Str; }

	/// Returns the width of the whole string
	int X();
	/// Returns the height of the whole string
	int Y();
	/// Returns the width and height of the whole string
	void Size(int *x, int *y);
	/// Returns the number of characters that fit in 'x' pixels.
	int CharAt(int x);

	/// Draws the string onto a device surface
	void Draw
	(
		/// The output device
		GSurface *pDC,
		/// The x coordinate of the top left corner of the output box
		int x,
		/// The y coordinate of the top left corner of the output box
		int y,
		/// An optional clipping rectangle. If the font is not transparent this rectangle will be filled with the background colour.
		GRect *r = 0
	);
};

#endif