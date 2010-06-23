#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "GMem.h"
#include "LgiOsDefs.h"
#include "GString.h"
#include "GContainers.h"
#include "LgiCommon.h"

char WhiteSpace[] = " \t\r\n";

// 8 Bit
char *strnchr(char *s, char c, int Len)
{
	if (s)
	{
		for (int i=0; i<Len; i++)
		{
			if (s[i] == c)
			{
				return s + i;
			}
		}
	}

	return 0;
}

char *strnstr(char *a, char *b, int n)
{
	if (a && b)
	{
		int SLen = strlen(b);
		int DLen = 0;
		while (DLen < n && a[DLen])
		{
		    DLen++;
		}

		while (SLen <= DLen && n >= SLen)
		{
			int i;
			for (i=0; i<SLen && a[i] == b[i]; i++);
			if (i == SLen) return a;

			n--;
			DLen--;
			a++;
		}
	}

	return NULL;
}

char *strnistr(char *a, char *b, int n)
{
	if (a && b)
	{
		int SLen = strlen(b);
		while (n >= SLen)
		{
			int i;
			for (i=0; a[i] && i<SLen && tolower(a[i]) == tolower(b[i]); i++);
			if (i == SLen) return a;
			if (a[i] == 0) return 0;

			n--;
			a++;
		}
	}

	return NULL;
}

int strnicmp(char *a, char *b, int i)
{
	int Cmp = -1;
	if (a && b && i > 0)
	{
		for (Cmp = 0; i-- && Cmp == 0; )
		{
			Cmp += tolower(*a) - tolower(*b);
			if (!*a || !*b) break;
			a++;
			b++;
		}
	}
	
	return Cmp;
}

char *stristr(char *a, char *b)
{
	if (a && b)
	{
		while (a && *a)
		{
			if (tolower(*a) == tolower(*b))
			{
				int i;
				for (i=0; a[i] && tolower(a[i]) == tolower(b[i]); i++)
					;

				if (b[i] == 0) return a;
			}
			a++;
		}
	}

	return NULL;
}

char *strsafecpy(char *dst, char *src, int len)
{
	char *d = dst;

	if (dst && src && len > 0)
	{
		while (*src && len > 1)
		{
			*dst++ = *src++;
			len--;
		}
		
		*dst++ = 0;
	}
	
	return d;
}

char *strsafecat(char *dst, char *src, int len)
{
	char *d = dst;

	if (dst && src && len > 0)
	{
		// Seek past the dst string
		while (*dst && len > 1)
		{
			dst++;
			len--;
		}
		
		// Copy the source onto the end
		while (*src && len > 1)
		{
			*dst++ = *src++;
			len--;
		}
		
		*dst++ = 0;
	}
	
	return d;
}

int strcmp(char *a, char *b)
{
	int c = -1;
	if (a && b)
	{
		c = 0;
		while (!c)
		{
			c = *a - *b;
			if (!*a || !*b) break;
			a++;
			b++;
		}
	}
	return c;
}

int stricmp(char *a, char *b)
{
	int c = -1;
	if (a && b)
	{
		c = 0;
		while (!c)
		{
			c = tolower(*a) - tolower(*b);
			if (!*a || !*b) break;
			a++;
			b++;
		}
	}
	return c;
}

#ifndef WIN32
char *strupr(char *a)
{
	for (char *s = a; s && *s; s++)
	{
		*s = tolower(*s);
	}

	return a;
}

char *strlwr(char *a)
{
	for (char *s = a; s && *s; s++)
	{
		*s = tolower(*s);
	}

	return a;
}
#endif

char *TrimStr(char *s, char *Delim)
{
	if (s)
	{
		char *Start = s;
		while (*Start && strchr(Delim, *Start))
		{
			Start++;
		}

		int StartLen = strlen(Start);
		if (StartLen > 0)
		{
			char *End = Start + strlen(Start) - 1;
			while (*End && End > Start && strchr(Delim, *End))
			{
				End--;
			}

			if (*Start)
			{
				int Len = (uint) End - (uint) Start + 1;
				char *n = new char[Len+1];
				if (n)
				{
					memcpy(n, Start, Len);
					n[Len] = 0;
					return n;
				}
			}
		}
	}

	return NULL;
}

bool ValidStr(char *s)
{
	if (s)
	{
		while (*s)
		{
			if (*s != ' ' &&
				*s != '\t' &&
				*s != '\r' &&
				*s != '\n' &&
				((uchar)*s) != 0xa0)
			{
				return true;
			}

			s++;
		}
	}

	return false;
}

char *NewStr(char *s, int Len)
{
	if (s)
	{
		if (Len < 0) Len = strlen(s);
		char *Ret = new char[Len+1];
		if (Ret)
		{
			if (Len > 0) memcpy(Ret, s, Len);
			Ret[Len] = 0;
			return Ret;
		}
	}
	return NULL;
}

#ifdef BEOS
#define toupper(c) ( ((c)>='a' && (c)<='z') ? (c)-'a'+'A' : (c) )
#define tolower(c) ( ((c)>='A' && (c)<='Z') ? (c)-'A'+'a' : (c) )
#endif

bool MatchStr(char *Template, char *Data)
{
	if (!Template)
	{
		return false;
	}

	if (stricmp(Template, (char*)"*") == 0)
	{
		// matches anything
		return true;
	}

	if (!Data)
	{
		return false;
	}

	while (*Template && *Data)
	{
		if (*Template == '*')
		{
			Template++;

			if (*Template)
			{
				char *EndA;
				for (EndA = Template; *EndA && *EndA!='?' && *EndA!='*'; EndA++);
				int SegLen = (int)EndA-(int)Template;
				char *Seg = NewStr(Template, SegLen);
				if (!Seg) return false;

				// find end of non match
				while (*Data)
				{
					if (strnicmp(Data, Seg, SegLen) == 0)
					{
						break;
					}
					Data++;
				}

				DeleteArray(Seg);

				if (*Data)
				{
					Template += SegLen;
					Data += SegLen;
				}
				else
				{
					// can't find any matching part in Data
					return false;
				}
			}
			else
			{
				// '*' matches everything
				return true;
			}

		}
		else if (*Template == '?')
		{
			Template++;
			Data++;
		}
		else
		{
			if (tolower(*Template) != tolower(*Data))
			{
				return false;
			}

			Template++;
			Data++;
		}
	}

	return	((*Template == 0) || (stricmp(Template, (char*)"*") == 0)) &&
			(*Data == 0);
}

int htoi(char *a)
{
	int Status = 0;

	for (; a && *a; a++)
	{
		if (*a >= '0' && *a <= '9')
		{
			Status <<= 4;
			Status |= *a - '0';
		}
		else if (*a >= 'a' && *a <= 'f')
		{
			Status <<= 4;
			Status |= *a - 'a' + 10;
		}
		else if (*a >= 'A' && *a <= 'F')
		{
			Status <<= 4;
			Status |= *a - 'A' + 10;
		}
		else break;
	}

	return Status;
}

int64 htoi64(char *a)
{
	int64 Status = 0;

	for (; a && *a; a++)
	{
		if (*a >= '0' && *a <= '9')
		{
			Status <<= 4;
			Status |= *a - '0';
		}
		else if (*a >= 'a' && *a <= 'f')
		{
			Status <<= 4;
			Status |= *a - 'a' + 10;
		}
		else if (*a >= 'A' && *a <= 'F')
		{
			Status <<= 4;
			Status |= *a - 'A' + 10;
		}
		else break;
	}

	return Status;
}

//////////////////////////////////////////////////////////////////////////
// UTF-16
#define CompareDefault	(a && b ? *a - *b : -1)

char16 *StrchrW(char16 *s, char16 c)
{
	if (s)
	{
		while (*s)
		{
			if (*s == c) return s;
			s++;
		}
	}

	return 0;
}

char16 *StrnchrW(char16 *s, char16 c, int Len)
{
	if (s)
	{
		while (*s && Len > 0)
		{
			if (*s == c)
				return s;
			s++;
			Len--;
		}
	}

	return 0;
}

char16 *StrrchrW(char16 *s, char16 c)
{
	char16 *Last = 0;

	while (s && *s)
	{
		if (*s == c)
			Last = s;
		s++;
	}

	return Last;
}

char16 *StrstrW(char16 *a, char16 *b)
{
	if (a && b)
	{
		int Len = StrlenW(b);
		for (char16 *s=a; *s; s++)
		{
			if (*s == *b)
			{
				// check match
				if (StrncmpW(s+1, b+1, Len-1) == 0)
					return s;
			}
		}
	}

	return 0;
}

char16 *StristrW(char16 *a, char16 *b)
{
	if (a && b)
	{
		int Len = StrlenW(b);
		for (char16 *s=a; *s; s++)
		{
			if (tolower(*s) == tolower(*b))
			{
				// check match
				if (StrnicmpW(s+1, b+1, Len-1) == 0)
					return s;
			}
		}
	}

	return 0;
}

char16 *StrnstrW(char16 *a, char16 *b, int n)
{
	if (a && b)
	{
		int Len = StrlenW(b);
		for (char16 *s=a; n >= Len && *s; s++, n--)
		{
			if (*s == *b)
			{
				// check match
				if (StrncmpW(s+1, b+1, Len-1) == 0)
					return s;
			}
		}
	}

	return 0;
}

char16 *StrnistrW(char16 *a, char16 *b, int n)
{
	if (a && b)
	{
		int Len = StrlenW(b);
		for (char16 *s=a; n >= Len && *s; s++, n--)
		{
			if (*s == *b)
			{
				// check match
				if (StrnicmpW(s+1, b+1, Len-1) == 0)
					return s;
			}
		}
	}

	return 0;
}

int StrcmpW(char16 *a, char16 *b)
{
	if (a && b)
	{
		while (true)
		{
			if (!*a || !*b || *a != *b)
				return *a - *b;

			a++;
			b++;
		}

		return 0;
	}

	return -1;
}

int StricmpW(char16 *a, char16 *b)
{
	if (a && b)
	{
		while (true)
		{
			char16 A = tolower(*a);
			char16 B = tolower(*b);

			if (!A || !B || A != B)
				return A - B;

			a++;
			b++;
		}

		return 0;
	}

	return -1;
}

int StrncmpW(char16 *a, char16 *b, int n)
{
	if (a && b)
	{
		while (n > 0)
		{
			if (!*a || !*b || *a != *b)
				return *a - *b;

			a++;
			b++;
			n--;
		}

		return 0;
	}

	return -1;
}

int StrnicmpW(char16 *a, char16 *b, int n)
{
	if (a && b)
	{
		while (n > 0)
		{
			char16 A = tolower(*a);
			char16 B = tolower(*b);

			if (!A || !B || A != B)
				return A - B;

			a++;
			b++;
			n--;
		}

		return 0;
	}

	return -1;
}

char16 *StrcpyW(char16 *a, char16 *b)
{
	if (a && b)
	{
		do
		{
			*a++ = *b++;
		}
		while (*b);

		*a++ = 0;
	}

	return a;
}

char16 *StrncpyW(char16 *a, char16 *b, int n)
{
	if (a && b && n > 0)
	{
		while (*b && --n > 0)
		{
			*a++ = *b++;
		}

		*a++ = 0; // always NULL terminate
	}

	return a;
}

void StrcatW(char16 *a, char16 *b)
{
	if (a && b)
	{
		// Seek to end of string
		while (*a)
		{
			*a++;
		}

		// Append 'b'
		while (*b)
		{
			*a++ = *b++;
		}

		*a++ = 0;
	}
}

int StrlenW(char16 *a)
{
	if (!a)
		return 0;

	int i = 0;
	while (*a++)
	{
		i++;
	}

	return i;
}

int HtoiW(char16 *a)
{
	int i = 0;
	if (a)
	{
		while (*a)
		{
			if (*a >= '0' && *a <= '9')
			{
				i <<= 4;
				i |= *a - '0';
			}
			else if (*a >= 'a' && *a <= 'f')
			{
				i <<= 4;
				i |= *a - 'a' + 10;
			}
			else if (*a >= 'A' && *a <= 'F')
			{
				i <<= 4;
				i |= *a - 'A' + 10;
			}
			else break;
		}
	}
	return i;
}

char16 *TrimStrW(char16 *s, char16 *Delim)
{
	if (!Delim)
	{
		static char16 Def[] = {' ', '\r', '\n', '\t', 0};
		Delim = Def;
	}

	if (s)
	{
		// Leading delim
		while (StrchrW(Delim, *s))
		{
			s++;
		}

		// Trailing delim
		int i = StrlenW(s);
		while (i > 0 && StrchrW(Delim, s[i-1]))
		{
			i--;
		}

		return NewStrW(s, i);
	}

	return 0;
}

bool MatchStrW(char16 *a, char16 *b)
{
	return 0;
}

char16 *NewStrW(char16 *Str, int l)
{
	char16 *s = 0;
	if (Str)
	{
		if (l < 0) l = StrlenW(Str);
		s = new char16[l+1];
		if (s)
		{
			memcpy(s, Str, l * sizeof(char16));
			s[l] = 0;
		}
	}
	return s;
}

bool ValidStrW(char16 *s)
{
	if (s)
	{
		for (char16 *c=s; *c; c++)
		{
			if (*c != ' ' && *c != '\t') return true;
		}
	}

	return false;
}

char *LgiDecodeUri(char *uri, int len)
{
	GStringPipe p;
	if (uri)
	{
		char *end = len >= 0 ? uri + len : 0;
		for (char *s=uri; s && *s; )
		{
			int Len;
			char *e = s;
			for (Len = 0; *e && *e != '%' && (!end || e < end); e++)
				Len++;
			
			p.Push(s, Len);
			if ((!end || e < end) && *e)
			{
				e++;
				if (e[0] && e[1])
				{
					char h[3] = { e[0], e[1], 0 };
					char c = htoi(h);
					p.Push(&c, 1);
					e += 2;
					s = e;
				}
				else break;
			}
			else
			{
				break;
			}
		}
	}
	return p.NewStr();
}

char *LgiEncodeUri(char *uri, int len)
{
	GStringPipe p;
	if (uri)
	{
		char *end = len >= 0 ? uri + len : 0;
		for (char *s=uri; s && *s; )
		{
			int Len;
			char *e = s;
			for
			(
				Len = 0;
				*e
				&&
				(!end || e < end)
				&&
				*e > ' '
				&&
				(uchar)*e < 0x7f
				&&
				strchr("$&+,/:;=?@\'\"<>#%{}|\\^~[]`", *e) == 0;
				e++
			)
			{
				Len++;
			}
			
			p.Push(s, Len);
			if ((!end || e < end) && *e)
			{
				char h[4];
				sprintf(h, "%%%02.2X", (uchar)*e);
				p.Push(h, 3);
				s = ++e;
			}
			else
			{
				break;
			}
		}
	}
	return p.NewStr();
}

