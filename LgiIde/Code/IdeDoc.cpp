#include <stdio.h>
#include <ctype.h>

#include "Lgi.h"
#include "LgiIde.h"
#include "GToken.h"
#include "GLexCpp.h"
#include "INet.h"

char *Untitled = "[untitled]";
static char *White = " \r\t\n";

#define IDC_EDIT		100
#define isword(s)		(s && (isdigit(s) || isalpha(s) || (s) == '_') )
#define iswhite(s)		(s && strchr(White, s) != 0)
#define skipws(s)		while (iswhite(*s)) s++;

GAutoPtr<GDocFindReplaceParams> GlobalFindReplace;

int FileNameSorter(char **a, char **b)
{
	char *A = strrchr(*a, DIR_CHAR);
	char *B = strrchr(*b, DIR_CHAR);
	return stricmp(A?A:*a, B?B:*b);
}

class EditTray : public GLayout
{
	GRect HeaderBtn;
	GRect FuncBtn;
	GRect SymBtn;
	GTextView3 *Ctrl;
	IdeDoc *Doc;

public:
	int Line, Col;

	EditTray(GTextView3 *ctrl, IdeDoc *doc)
	{
		Ctrl = ctrl;
		Doc = doc;
		Line = Col = 0;
		FuncBtn.ZOff(-1, -1);
		SymBtn.ZOff(-1, -1);
	}
	
	~EditTray()
	{
	}

	void OnPaint(GSurface *pDC)
	{
		GRect c = GetClient();
		SysFont->Colour(LC_TEXT, LC_MED);
		
		int BtnHt = c.Y()-5;
		HeaderBtn.ZOff(20, BtnHt);
		HeaderBtn.Offset(2, 2);

		FuncBtn.ZOff(20, BtnHt);
		FuncBtn.Offset(HeaderBtn.x2 + 3, 2);

		SymBtn.ZOff(20, BtnHt);
		SymBtn.Offset(FuncBtn.x2 + 3, 2);

		char s[256];
		sprintf(s, "Cursor: %i,%i", Col, Line + 1);
		SysFont->Transparent(false);
		{
			GDisplayString ds(SysFont, s);
			ds.Draw(pDC, SymBtn.x2 + 4, 4, &c);
		}

		GRect f = HeaderBtn;
		LgiThinBorder(pDC, f, RAISED);
		SysFont->Transparent(true);
		{
			GDisplayString ds(SysFont, "h");
			ds.Draw(pDC, f.x1 + 6, f.y1);
		}

		f = FuncBtn;
		LgiThinBorder(pDC, f, RAISED);
		SysFont->Transparent(true);
		{
			GDisplayString ds(SysFont, "{ }");
			ds.Draw(pDC, f.x1 + 3, f.y1);
		}

		f = SymBtn;
		LgiThinBorder(pDC, f, RAISED);
		SysFont->Transparent(true);
		{
			GDisplayString ds(SysFont, "s");
			ds.Draw(pDC, f.x1 + 6, f.y1);
		}
	}

	bool Pour(GRegion &r)
	{
		GRect *c = FindLargest(r);
		if (c)
		{
			GRect n = *c;
			n.x1 += 2;
			n.x2 -= 2;
			n.y2 -= 2;
			SetPos(n);		
			return true;
		}
		return false;
	}
	
	void OnMouseClick(GMouse &m);
};

void EditTray::OnMouseClick(GMouse &m)
{
	if (m.Left() && m.Down())
	{
		if (HeaderBtn.Overlap(m.x, m.y))
		{
			// Header list button
			GArray<char*> Paths;
			if (Doc->BuildIncludePaths(Paths))
			{
				GArray<char*> Headers;
				if (Doc->BuildHeaderList(Ctrl->NameW(), Headers, Paths))
				{
					// Sort them..
					Headers.Sort(FileNameSorter);
					
					GSubMenu *s = new GSubMenu;
					if (s)
					{
						// Construct the menu
						int n=1;
						for (int i=0; i<Headers.Length(); i++)
						{
							char *h = Headers[i];
							char *f = strrchr(h, DIR_CHAR);
							s->AppendItem(f ? f + 1 : h, n++, true);
						}
						if (!Headers.Length())
						{
							s->AppendItem("(none)", 0, false);
						}
						
						// Show the menu
						GdcPt2 p(m.x, m.y);
						PointToScreen(p);
						int Goto = s->Float(this, p.x, p.y, true);
						if (Goto)
						{
							char *File = Headers[Goto-1];
							if (File)
							{
								// Open the selected file
								Doc->GetProject()->GetApp()->OpenFile(File);
							}
						}
						
						DeleteObj(s);
					}
				}
				
				// Clean up memory
				Paths.DeleteArrays();
				Headers.DeleteArrays();
			}
			else
			{
				printf("%s:%i - No include paths set.\n", __FILE__, __LINE__);
			}
		}
		else if (FuncBtn.Overlap(m.x, m.y))
		{
			// Function list button
			List<DefnInfo> Funcs;
			if (Doc->BuildDefnList(Doc->GetFileName(), Ctrl->NameW(), Funcs, DefnFunc))
			{
				GSubMenu *s = new GSubMenu;
				if (s)
				{
					int n=1;
					for (DefnInfo *i=Funcs.First(); i; i=Funcs.Next(), n++)
					{
						char Buf[256], *o = Buf;
						
						for (char *k = i->Name; *k; k++)
						{
							if (*k == '&')
							{
								*o++ = '&';
								*o++ = '&';
							}
							else if (*k == '\t')
							{
								*o++ = ' ';
							}
							else
							{
								*o++ = *k;
							}
						}
						*o++ = 0;
						
						s->AppendItem(Buf, n, true);
					}
					
					GdcPt2 p(m.x, m.y);
					PointToScreen(p);
					int Goto = s->Float(this, p.x, p.y, true);
					if (Goto)
					{
						DefnInfo *Info = Funcs[Goto-1];
						if (Info)
						{
							Ctrl->GotoLine(Info->Line);
						}
					}
					
					DeleteObj(s);
				}
				
				Funcs.DeleteObjects();
			}
			else
			{
				printf("%s:%i - No functions in input.\n", __FILE__, __LINE__);
			}
		}
		else if (SymBtn.Overlap(m.x, m.y))
		{
			GAutoString s(Ctrl->GetSelection());
			if (s)
			{
				GAutoWString sw(LgiNewUtf8To16(s));
				if (sw)
				{
					/*
					List<DefnInfo> Matches;
					Doc->FindDefn(sw, Ctrl->NameW(), Matches);
					*/
					GArray<FindSymResult> Matches;
					Doc->GetApp()->FindSymbol(s, Matches);

					GSubMenu *s = new GSubMenu;
					if (s)
					{
						// Construct the menu
						int n=1;
						// for (DefnInfo *Def = Matches.First(); Def; Def = Matches.Next())
						for (int i=0; i<Matches.Length(); i++)
						{
							FindSymResult &Res = Matches[i];
							char m[512];
							char *d = strrchr(Res.File, DIR_CHAR);
							sprintf(m, "%s (%s:%i)", Res.Symbol.Get(), d ? d + 1 : Res.File.Get(), Res.Line);
							s->AppendItem(m, n++, true);
						}
						if (!Matches.Length())
						{
							s->AppendItem("(none)", 0, false);
						}
						
						// Show the menu
						GdcPt2 p(m.x, m.y);
						PointToScreen(p);
						int Goto = s->Float(this, p.x, p.y, true);
						if (Goto)
						{
							FindSymResult &Def = Matches[Goto-1];
							{
								// Open the selected symbol
								if (Doc->GetProject() &&
									Doc->GetProject()->GetApp())
								{
									AppWnd *App = Doc->GetProject()->GetApp();
									IdeDoc *Doc = App->OpenFile(Def.File);

									if (Doc)
									{
										Doc->GetEdit()->GotoLine(Def.Line);
									}
									else
									{
										printf("%s:%i - Couldn't open doc '%s'\n", _FL, Def.File.Get());
									}
								}
								else
								{
									printf("%s:%i - No project / app ptr.\n", _FL);
								}
							}
						}
						
						DeleteObj(s);
					}
				}
			}
			else
			{
				GSubMenu *s = new GSubMenu;
				if (s)
				{
					s->AppendItem("(No symbol currently selected)", 0, false);
					GdcPt2 p(m.x, m.y);
					PointToScreen(p);
					s->Float(this, p.x, p.y, true);
					DeleteObj(s);
				}
			}
		}
	}
}

class DocEdit : public GTextView3, public GDocumentEnv
{
	IdeDoc *Doc;

public:
	DocEdit(IdeDoc *d, GFontType *f) : GTextView3(IDC_EDIT, 0, 0, 100, 100, f)
	{
		Doc = d;
		if (!GlobalFindReplace)
		{
			GlobalFindReplace.Reset(CreateFindReplaceParams());
		}
		SetFindReplaceParams(GlobalFindReplace);
		
		CanScrollX = true;
		
		if (!f)
		{
			GFontType Type;
			if (Type.GetSystemFont("Fixed"))
			{
				GFont *f = Type.Create();
				if (f)
				{
					#if defined LINUX
					f->PointSize(9);
					#elif defined WIN32
					f->PointSize(8);
					#endif
					SetFont(f);
				}
			}
		}
		
		SetWrapType(TEXTED_WRAP_NONE);
		SetEnv(this);
	}
	
	#define IDM_FILE_COMMENT			100
	#define IDM_FUNC_COMMENT			101
	bool AppendItems(GSubMenu *Menu, int Base)
	{
		GSubMenu *Insert = Menu->AppendSub("Insert...");
		if (Insert)
		{
			Insert->AppendItem("File Comment", IDM_FILE_COMMENT, Doc->GetProject() != 0);
			Insert->AppendItem("Function Comment", IDM_FUNC_COMMENT, Doc->GetProject() != 0);
		}

		return true;
	}
	
	~DocEdit()
	{
		SetEnv(0);
	}
	
	bool OnMenu(GDocView *View, int Id);
	
	char *TemplateMerge(const char *Template, char *Name, List<char> *Params)
	{
		// Parse template and insert into doc
		GStringPipe T;
		for (const char *t = Template; *t; )
		{
			char *e = strstr((char*) t, "<%");
			if (e)
			{
				// Push text before tag
				T.Push(t, e-t);
				char *TagStart = e;
				e += 2;
				skipws(e);
				char *Start = e;
				while (*e && isalpha(*e)) e++;
				
				// Get tag
				char *Tag = NewStr(Start, e-Start);
				if (Tag)
				{
					// Process tag
					if (Name && stricmp(Tag, "name") == 0)
					{
						T.Push(Name);
					}
					else if (Params && stricmp(Tag, "params") == 0)
					{
						char *Line = TagStart;
						while (Line > Template && Line[-1] != '\n') Line--;
						
						int i = 0;
						for (char *p=Params->First(); p; p=Params->Next(), i++)
						{
							if (i) T.Push(Line, TagStart-Line);
							T.Push(p);
							if (i < Params->Length()-1) T.Push("\n");
						}
					}
					
					DeleteArray(Tag);
				}
				
				e = strstr(e, "%>");
				if (e)
				{
					t = e + 2;
				}
				else break;
			}
			else
			{
				T.Push(t);
				break;
			}
		}
		T.Push("\n");
		return T.NewStr();
	}

	void PourText(int Start, int Len)
	{
		GTextView3::PourText(Start, Len);
		
		bool Lut[128];
		ZeroObj(Lut);
		Lut[' '] = true;
		Lut['\r'] = true;
		Lut['\t'] = true;
		
		bool LongComment = false;
		COLOUR CommentColour = Rgb32(0, 0x80, 0);
		char16 Eoc[] = { '*', '/', 0 };
		for (GTextLine *l=GTextView3::Line.First(); l; l=GTextView3::Line.Next())
		{
			char16 *s = Text + l->Start;			
			if (LongComment)
			{
				l->c.c32(CommentColour);
				if (StrnstrW(s, Eoc, l->Len))
				{
					LongComment = false;
				}
			}
			else
			{
				while (*s <= 256 && Lut[*s]) s++;
				if (*s == '#')
				{
					l->c.Rgb(0, 0, 222);
				}
				else if (s[0] == '/')
				{
					if (s[1] == '/')
					{
						l->c.c32(CommentColour);
					}
					else if (s[1] == '*')
					{
						l->c.c32(CommentColour);
						LongComment = StrnstrW(s, Eoc, l->Len) == 0;
					}
				}
			}
		}
	}
	
	void PourStyle(int Start, int Length)
	{
	}

	bool Pour(GRegion &r)
	{
		GRect c = r.Bound();

		c.Size(2, 2);
		c.y2 -= 20;
		SetPos(c);
		
		return true;
	}
};

bool DocEdit::OnMenu(GDocView *View, int Id)
{
	if (View)
	{
		switch (Id)
		{
			case IDM_FILE_COMMENT:
			{
				const char *Template = Doc->GetProject()->GetFileComment();
				if (Template)
				{
					char *File = strrchr(Doc->GetFileName(), DIR_CHAR);
					if (File)
					{
						char *Comment = TemplateMerge(Template, File + 1, 0);
						if (Comment)
						{
							char16 *C16 = LgiNewUtf8To16(Comment);
							DeleteArray(Comment);
							if (C16)
							{
								Insert(Cursor, C16, StrlenW(C16));
								DeleteArray(C16);
								Invalidate();
							}
						}
					}
				}
				break;
			}
			case IDM_FUNC_COMMENT:
			{
				const char *Template = Doc->GetProject()->GetFunctionComment();
				if (ValidStr(Template))
				{
					char16 *n = NameW();
					if (n)
					{
						List<char16> Tokens;
						char16 *s;
						char16 *p = n + GetCursor();
						char16 OpenBrac[] = { '(', 0 };
						char16 CloseBrac[] = { ')', 0 };
						int OpenBracketIndex = -1;							
						
						// Parse from cursor to the end of the function defn
						while (s = LexCpp(p))
						{
							if (StricmpW(s, OpenBrac) == 0)
							{
								OpenBracketIndex = Tokens.Length();
							}

							Tokens.Insert(s);

							if (StricmpW(s, CloseBrac) == 0)
							{
								break;
							}
						}
						
						if (OpenBracketIndex > 0)
						{
							char *FuncName = LgiNewUtf16To8(Tokens[OpenBracketIndex-1]);
							if (FuncName)
							{
								// Get a list of parameter names
								List<char> Params;
								for (int i = OpenBracketIndex+1; p = Tokens[i]; i++)
								{
									char16 Comma[] = { ',', 0 };
									if (StricmpW(p, Comma) == 0 ||
										StricmpW(p, CloseBrac) == 0)
									{
										char16 *Param = Tokens[i-1];
										if (Param)
										{
											Params.Insert(LgiNewUtf16To8(Param));
										}
									}
								}
								
								// Do insertion
								char *Comment = TemplateMerge(Template, FuncName, &Params);
								if (Comment)
								{
									char16 *C16 = LgiNewUtf8To16(Comment);
									DeleteArray(Comment);
									if (C16)
									{
										Insert(Cursor, C16, StrlenW(C16));
										DeleteArray(C16);
										Invalidate();
									}
								}
								
								// Clean up
								DeleteArray(FuncName);
								Params.DeleteArrays();
							}
							else
							{
								printf("%s:%i - No function name.\n", __FILE__, __LINE__);
							}
						}
						else
						{
							printf("%s:%i - OpenBracketIndex not found.\n", __FILE__, __LINE__);
						}
						
						Tokens.DeleteArrays();
					}
					else
					{
						printf("%s:%i - No input text.\n", __FILE__, __LINE__);
					}
				}
				else
				{
					printf("%s:%i - No template.\n", __FILE__, __LINE__);
				}
				break;
			}
		}
	}
	
	return true;
}

class IdeDocPrivate : public NodeView
{
	GAutoString FileName;

public:
	IdeDoc *Doc;
	AppWnd *App;
	IdeProject *Project;
	bool IsDirty;
	DocEdit *Edit;
	EditTray *Tray;
	
	IdeDocPrivate(IdeDoc *d, AppWnd *a, NodeSource *src, char *file) : NodeView(src)
	{
		IsDirty = false;
		App = a;
		Doc = d;
		Project = 0;
		FileName.Reset(NewStr(file));
		
		GFontType Font, *Use = 0;
		if (Font.Serialize(App->GetOptions(), OPT_EditorFont, false))
		{
			Use = &Font;
		}		
		
		Doc->AddView(Edit = new DocEdit(Doc, Use));
		Doc->AddView(Tray = new EditTray(Edit, Doc));

		if (src || file)
			Load();
	}

	void OnDelete()
	{
		IdeDoc *Temp = Doc;
		DeleteObj(Temp);
	}

	void UpdateName()
	{
		char n[MAX_PATH+30];
		
		char *Dsp = GetDisplayName();
		strsafecpy(n, Dsp ? Dsp : Untitled, sizeof(n));
		if (Edit->IsDirty())
		{
			strcat(n, " (changed)");
		}	
		Doc->Name(n);
		DeleteArray(Dsp);
	}

	char *GetDisplayName()
	{
		if (nSrc)
		{
			char *Fn = nSrc->GetFileName();
			if (Fn)
			{
				if (stristr(Fn, "://"))
				{
					GUri u(nSrc->GetFileName());
					if (u.Pass)
					{
						DeleteArray(u.Pass);
						u.Pass = NewStr("******");
					}
					
					return u.GetUri().Release();
				}
				else if (*Fn == '.')
				{
					GAutoString a = nSrc->GetFullPath();
					return a.Release();
				}
			}

			return NewStr(Fn);
		}

		return NewStr(FileName);
	}

	bool IsFile(char *File)
	{
		if (nSrc)
		{
			GAutoString f = nSrc->GetFullPath();
			return f ? stricmp(File, f) == 0 : false;
		}
		else if (FileName)
		{
			return stricmp(File, FileName) == 0;
		}

		return false;
	}
	
	char *GetLocalFile()
	{
		if (nSrc)
		{
			if (nSrc->IsWeb())
				return nSrc->GetLocalCache();
			
			return nSrc->GetFileName();
		}
		
		return FileName;
	}

	void SetFileName(char *f)
	{
		if (nSrc)
		{
		}
		else
		{
			FileName.Reset(NewStr(f));
		}
	}

	bool Load()
	{
		if (nSrc)
		{
			return nSrc->Load(Edit, this);
		}
		else if (FileName)
		{
			return Edit->Open(FileName);
		}

		return false;
	}

	bool Save()
	{
		bool Status = false;
		
		if (nSrc)
		{
			Status = nSrc->Save(Edit, this);
		}
		else if (FileName)
		{
			Status = Edit->Save(FileName);
			OnSaveComplete(Status);
		}
		
		return Status;
	}

	void OnSaveComplete(bool Status)
	{
		IsDirty = false;
		UpdateName();
	}
};

IdeDoc::IdeDoc(AppWnd *a, NodeSource *src, char *file)
{
	d = new IdeDocPrivate(this, a, src, file);
	d->UpdateName();
}

IdeDoc::~IdeDoc()
{
	d->App->OnDocDestroy(this);
	DeleteObj(d);
}

AppWnd *IdeDoc::GetApp()
{
	return d->App;
}

bool IdeDoc::IsFile(char *File)
{
	return File ? d->IsFile(File) : false;
}

IdeProject *IdeDoc::GetProject()
{
	return d->Project;
}

void IdeDoc::SetProject(IdeProject *p)
{
	d->Project = p;
}

char *IdeDoc::GetFileName()
{
	return d->GetLocalFile();
}

void IdeDoc::SetFileName(char *f, bool Write)
{
	if (Write)
	{
		d->SetFileName(f);
		d->Edit->Save(d->GetLocalFile());
	}
}

void IdeDoc::SetFocus()
{
	d->Edit->Focus(true);
}

int IdeDoc::OnNotify(GViewI *v, int f)
{
	switch (v->GetId())
	{
		case IDC_EDIT:
		{
			switch (f)
			{
				case GTVN_DOC_CHANGED:
				{
					if (!d->IsDirty)
					{
						d->IsDirty = true;
						d->UpdateName();
					}
					break;
				}
				case GTVN_CURSOR_CHANGED:
				{
					if (d->Tray)
					{
						d->Edit->PositionAt(d->Tray->Col, d->Tray->Line, d->Edit->GetCursor());
						d->Tray->Invalidate();
					}
					break;
				}
			}
			break;
		}
	}
	
	return 0;
}

void IdeDoc::SetDirty()
{
	d->IsDirty = true;
	d->Edit->IsDirty(true);
	d->UpdateName();
}

bool IdeDoc::SetClean()
{
	static bool Processing = false;
	bool Status = false;

	if (!Processing)
	{
		Status = Processing = true;
		
		if (d->Edit->IsDirty())
		{
			d->Save();
		}
		
		Processing = false;
	}
	
	return Status;
}

void IdeDoc::OnPaint(GSurface *pDC)
{
	GMdiChild::OnPaint(pDC);
	
	GRect c = GetClient();
	LgiWideBorder(pDC, c, SUNKEN);
}

void IdeDoc::OnPosChange()
{
	GMdiChild::OnPosChange();
}

bool IdeDoc::OnRequestClose(bool OsShuttingDown)
{
	if (d->Edit->IsDirty())
	{
		char *Dsp = d->GetDisplayName();
		int a = LgiMsg(this, "Save '%s'?", AppName, MB_YESNOCANCEL, Dsp ? Dsp : Untitled);
		DeleteArray(Dsp);
		switch (a)
		{
			case IDYES:
			{
				if (!SetClean())
				{				
					return false;
				}
				break;
			}
			case IDNO:
			{
				break;
			}
			default:
			case IDCANCEL:
			{
				return false;
			}
		}
	}
	
	return true;
}

GTextView3 *IdeDoc::GetEdit()
{
	return d->Edit;
}

bool IdeDoc::BuildIncludePaths(GArray<char*> &Paths)
{
	if (GetProject())
	{
		return GetProject()->BuildIncludePaths(Paths, true);
	}
	
	return false;
}

bool IdeDoc::BuildHeaderList(char16 *Cpp, GArray<char*> &Headers, GArray<char*> &IncPaths)
{
	bool Status = false;
	char *c8 = LgiNewUtf16To8(Cpp);
	if (c8)
	{
		Status = ::BuildHeaderList(c8, Headers, IncPaths, true);
		DeleteArray(c8);
	}
	return Status;
}

#define defnskipws(s)		while (iswhite(*s)) { if (*s == '\n') Line++; s++; }

bool IdeDoc::BuildDefnList(char *FileName, char16 *Cpp, List<DefnInfo> &Defns, DefnType LimitTo, bool Debug)
{
	if (Cpp)
	{
		static char16 StrClass[]		= {'c', 'l', 'a', 's', 's', 0};
		static char16 StrStruct[]		= {'s', 't', 'r', 'u', 'c', 't', 0};
		static char16 StrOpenBracket[]	= {'{', 0};
		static char16 StrColon[]		= {':', 0};
		static char16 StrSemiColon[]	= {';', 0};
		static char16 StrDefine[]		= {'d', 'e', 'f', 'i', 'n', 'e', 0};
		static char16 StrExtern[]		= {'e', 'x', 't', 'e', 'r', 'n', 0};
		static char16 StrTypedef[]		= {'t', 'y', 'p', 'e', 'd', 'e', 'f', 0};
		static char16 StrC[]			= {'\"', 'C', '\"', 0};
		
		char16 *s = Cpp;
		char16 *LastDecl = s;
		int Depth = 0;
		int Line = 0;
		int CaptureLevel = 0;
		int InClass = false;	// true if we're in a class definition			
		char16 *CurClassDecl = 0;
		bool FnEmit = false;	// don't emit functions between a f(n) and the next '{'
								// they are only parent class initializers

		while (s && *s)
		{
			// skip ws
			while (*s && strchr(" \t\r", *s)) s++;

			// tackle decl
			switch (*s)
			{
				case 0:
					break;
				case '\n':
				{
					Line ++;
					s++;
					break;
				}
				case '#':
				{
					char16 *Hash = s;
					
					s++;
					if
					(
						(
							LimitTo == DefnNone
							||
							LimitTo == DefnDefine
						)
						&&
						StrncmpW(StrDefine, s, 6) == 0
					)
					{
						s += 6;
						defnskipws(s);
						LexCpp(s, false);

						char16 r = *s;
						*s = 0;
						DefnInfo *Defn = new DefnInfo(DefnDefine, FileName, Hash, Line);
						*s = r;
						if (Defn)
						{
							Defns.Insert(Defn);
						}
					}
					
					while (*s)
					{
						if (*s == '\n')
						{
							// could be end of # command
							char Last = (s[-1] == '\r') ? s[-2] : s[-1];
							if (Last != '\\') break;
							Line++;
						}

						s++;
						LastDecl = s;
					}
					break;
				}
				case '\"':
				case '\'':
				{
					char16 Delim = *s;
					s++;
					while (*s)
					{
						if (*s == Delim) { s++; break; }
						if (*s == '\\') s++;
						if (*s == '\n') Line++;
						s++;
					}
					break;
				}
				case '{':
				{
					s++;
					Depth++;
					FnEmit = false;
					break;
				}
				case ';':
				{
					s++;
					LastDecl = s;
					
					if (Depth == 0)
					{
						if (InClass)
						{
							InClass = false;
							CaptureLevel = 0;
							DeleteArray(CurClassDecl);
						}
					}
					break;
				}
				case '}':
				{
					s++;
					if (Depth > 0) Depth--;
					LastDecl = s;
					break;
				}
				case '/':
				{
					s++;
					if (*s == '/')
					{
						// one line comment
						while (*s && *s != '\n') s++;
						LastDecl = s;
					}
					else if (*s == '*')
					{
						// multi line comment
						s++;
						while (*s)
						{
							if (s[0] == '*' && s[1] == '/')
							{
								s += 2;
								break;
							}

							if (*s == '\n') Line++;
							s++;
						}
						LastDecl = s;
					}
					break;
				}
				case '(':
				{
					s++;
					if (Depth == CaptureLevel && !FnEmit && LastDecl)
					{
						// function?
						
						// find start:
						char16 *Start = LastDecl;
						skipws(Start);
						
						// find end
						char16 *End = StrchrW(Start, ')');
						if (End)
						{
							End++;

							char16 *Buf = NewStrW(Start, End-Start);
							if (Buf)
							{
								// remove new-lines
								char16 *Out = Buf;
								// bool InArgs = false;
								for (char16 *In = Buf; *In; In++)
								{
									/*
									if (InArgs)
									{
										if (strchr(" \t\r\n", *In))
										{
											while (strchr(" \t\r\n", *In)) In++;
											*Out++ = ' ';
											In--;
											continue;
										}
									}
									else
									{
										if (*In == '(')
										{
											InArgs = true;
										}
									}
									*/

									if (*In == '\r' || *In == '\n' || *In == '\t' || *In == ' ')
									{
										*Out++ = ' ';
										skipws(In);
										In--;
									}
									else if (In[0] == '/' && In[1] == '/')
									{
										In = StrchrW(In, '\n');
										if (!In) break;
									}
									else
									{
										*Out++ = *In;
									}
								}
								*Out++ = 0;

								if (CurClassDecl)
								{
									char16 Str[1024];
									ZeroObj(Str);
									
									StrcpyW(Str, Buf);
									char16 *b = StrchrW(Str, '(');
									if (b)
									{
										// Skip whitespace between name and '('
										while
										(
											b > Str
											&&
											strchr(" \t\r\n", b[-1])																									
										)
										{
											b--;
										}

										// Skip over to the start of the name..
										while
										(
											b > Str
											&&
											b[-1] != '*'
											&&
											b[-1] != '&'
											&&
											!strchr(" \t\r\n", b[-1])																									
										)
										{
											b--;
										}
										
										int ClsLen = StrlenW(CurClassDecl);
										memmove(b + ClsLen + 2, b, sizeof(*b) * (StrlenW(b)+1));
										memcpy(b, CurClassDecl, sizeof(*b) * ClsLen);
										b += ClsLen;
										*b++ = ':';
										*b++ = ':';
										DeleteArray(Buf);
										Buf = NewStrW(Str);
									}
								}

								// cache f(n) def
								if (LimitTo == DefnNone || LimitTo == DefnFunc)
								{
									DefnInfo *Defn = new DefnInfo(DefnFunc, FileName, Buf, Line);
									if (Defn)
									{
										Defns.Insert(Defn);
									}
								}
								DeleteArray(Buf);
								
								skipws(End);
								FnEmit = *End != ';';
							}
						}
					}
					break;
				}
				default:
				{
					if (isalpha(*s) || isdigit(*s) || *s == '_')
					{
						char16 *Start = s;
						
						s++;
						while (	isalpha(*s) ||
								isdigit(*s) ||
								*s == '_' ||
								*s == ':' ||
								*s == '.' ||
								*s == '~')
						{
							s++;
						}
						
						int TokLen = s - Start;
						if (TokLen == 6 && StrncmpW(StrExtern, Start, 6) == 0)
						{
							// extern "C" block
							char16 *t = LexCpp(s); // "C"
							if (t && StrcmpW(t, StrC) == 0)
							{
								defnskipws(s);
								if (*s == '{')
								{
									Depth--;
								}
							}
							DeleteArray(t);
						}
						else if (TokLen == 7 && StrncmpW(StrTypedef, Start, 7) == 0)
						{
							// Typedef
							GStringPipe p;
							char16 *i;
							for (i = Start; i && *i;)
							{
								switch (*i)
								{
									case ' ':
									case '\t':
									{
										p.Push(Start, i - Start);
										
										defnskipws(i);

										char16 sp[] = {' ', 0};
										p.Push(sp);

										Start = i;
										break;
									}
									case '\n':
										Line++;
										// fall thru
									case '\r':
									{
										p.Push(Start, i - Start);
										i++;
										Start = i;
										break;
									}
									case '{':
									{
										p.Push(Start, i - Start);
										
										int Depth = 1;
										i++;
										while (*i && Depth > 0)
										{
											switch (*i)
											{
												case '{':
													Depth++;
													break;
												case '}':
													Depth--;
													break;
												case '\n':
													Line++;
													break;
											}
											i++;
										}
										Start = i;
										break;
									}
									case ';':
									{
										p.Push(Start, i - Start);
										s = i;
										i = 0;
										break;
									}
									default:
									{
										i++;
										break;
									}
								}
							}
							
							char16 *Typedef = p.NewStrW();
							if (Typedef)
							{
								if (LimitTo == DefnNone || LimitTo == DefnTypedef)
								{
									DefnInfo *Defn = new DefnInfo(DefnTypedef, FileName, Typedef, Line);
									if (Defn)
									{
										Defns.Insert(Defn);
									}
								}
								DeleteArray(Typedef);
							}
						}
						else if
						(
							(
								TokLen == 5
								&&
								StrncmpW(StrClass, Start, 5) == 0
							)
							||
							(
								TokLen == 6
								&&
								StrncmpW(StrStruct, Start, 6) == 0
							)
						)
						{
							// Class or Struct
							if (Depth == 0)
							{
								InClass = true;
								CaptureLevel = 1;
								
								char16 *n = Start + 5, *t;
								List<char16> Tok;
								
								while (n && *n)
								{
									char16 *Last = n;
									if (t = LexCpp(n))
									{
										if (StrcmpW(t, StrSemiColon) == 0)
										{
											break;
										}
										else if (StrcmpW(t, StrOpenBracket) == 0 ||
											StrcmpW(t, StrColon) == 0)
										{
											DeleteArray(CurClassDecl);
											CurClassDecl = Tok.Last();
											Tok.Delete(CurClassDecl);
											
											if (LimitTo == DefnNone || LimitTo == DefnClass)
											{
												char16 r = *Last;
												*Last = 0;
												DefnInfo *Defn = new DefnInfo(DefnClass, FileName, Start, Line);
												*Last = r;
												if (Defn)
												{
													Defns.Insert(Defn);
												}
											}
											break;
										}
										else
										{
											Tok.Insert(t);
										}
									}
									else
									{
										printf("%s:%i - LexCpp failed at %s:%i.\n", __FILE__, __LINE__, FileName, Line);
										break;
									}
								}
								Tok.DeleteArrays();
							}
						}
						
						if (s[-1] == ':')
						{
							LastDecl = s;
						}
					}
					else
					{
						s++;
					}

					break;
				}
			}
		}
		
		DeleteArray(CurClassDecl);
	}

	return Defns.First() != 0;
}

bool MatchSymbol(DefnInfo *Def, char16 *Symbol)
{
	static char16 Dots[] = {':', ':', 0};

	GBase o;
	o.Name(Def->Name);
	char16 *Name = o.NameW();

	char16 *Sep = StristrW(Name, Dots);
	char16 *Start = Sep ? Sep : Name;
	char16 *End = StrchrW(Start, '(');
	int Len = StrlenW(Symbol);
	char16 *Match = StristrW(Start, Symbol);

	if (Match) // && Match + Len <= End)
	{
		if (Match > Start && isword(Match[-1]))
		{
			return false;
		}
		
		char16 *e = Match + Len;
		if (*e && isword(*e))
		{
			return false;
		}
		
		return true;
	}
	
	return false;
}

bool IdeDoc::FindDefn(char16 *Symbol, char16 *Source, List<DefnInfo> &Matches)
{
	if (Symbol && Source)
	{
		GArray<char*> Paths;
		GArray<char*> Headers;
		if (BuildIncludePaths(Paths) &&
			BuildHeaderList(Source, Headers, Paths))
		{
			List<DefnInfo> Defns;

			for (int i=0; i<Headers.Length(); i++)
			{
				char *h = Headers[i];
				char *c8 = ReadTextFile(h);
				if (c8)
				{
					char16 *c16 = LgiNewUtf8To16(c8);
					DeleteArray(c8);
					if (c16)
					{
						List<DefnInfo> Defns;
						if (BuildDefnList(h, c16, Defns, DefnNone, stristr(h, "/tgl.h") != 0 ))
						{
							for (DefnInfo *Def=Defns.First(); Def; )
							{
								if (MatchSymbol(Def, Symbol))
								{
									Matches.Insert(Def);
									Defns.Delete(Def);
									Def = Defns.Current();
								}
								else
								{
									Def = Defns.Next();
								}
							}
							
							Defns.DeleteObjects();
						}
						
						DeleteArray(c16);
					}
				}
			}

			if (BuildDefnList(GetFileName(), Source, Defns, DefnNone))
			{
				for (DefnInfo *Def=Defns.First(); Def; )
				{
					if (MatchSymbol(Def, Symbol))
					{
						Matches.Insert(Def);
						Defns.Delete(Def);
						Def = Defns.Current();
					}
					else
					{
						Def = Defns.Next();
					}
				}
				Defns.DeleteObjects();
			}
			
		}
		
		Paths.DeleteArrays();
		Headers.DeleteArrays();
	}
	
	return Matches.Length() > 0;
}
