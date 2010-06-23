/*hdr
**      FILE:           GFindRepalce.cpp
**      AUTHOR:         Matthew Allen
**      DATE:           14/11/2001
**      DESCRIPTION:    Common find and replace windows
**
**      Copyright (C) 2001, Matthew Allen
**              fret@memecode.com
*/

#include <stdio.h>
#include "Lgi.h"
#include "GTextView3.h"
#include "GTextLabel.h"
#include "GEdit.h"
#include "GCheckBox.h"
#include "GButton.h"

////////////////////////////////////////////////////////////////////////////
GFindReplaceCommon::GFindReplaceCommon()
{
	Find = 0;
	MatchWord = false;
	MatchCase = false;
	SelectionOnly = false;
}

GFindReplaceCommon::~GFindReplaceCommon()
{
	DeleteArray(Find);
}

////////////////////////////////////////////////////////////////////////////
// Find Window
#define IDS_16						1000
#define IDC_TEXT					1001
#define IDC_MATCH_WORD				1004
#define IDC_MATCH_CASE				1005
#define IDC_PREV_SEARCH				1006
#define IDC_SELECTION_ONLY			1007

class GFindDlgPrivate
{
public:
	GEdit *Edit;
	GFrCallback Callback;
	void *CallbackData;
};

GFindDlg::GFindDlg(GView *Parent, char *Init, GFrCallback Callback, void *UserData)
{
	d = new GFindDlgPrivate;
	Find = NewStr(Init);
	d->Callback = Callback;
	d->CallbackData = UserData;

	SetParent(Parent);
	Name(LgiLoadString(L_FR_FIND, "Find"));
	GRect r(0, 0, 380, 130);
	SetPos(r);
	MoveToCenter();

	Children.Insert(new GText(IDS_16, 14, 14, -1, -1, LgiLoadString(L_FR_FIND_WHAT, "Find what:")));
	
	Children.Insert(d->Edit = new GEdit(IDC_TEXT, 91, 7, 168, 21, ""));
	// Children.Insert(new GCombo(IDC_PREV_SEARCH, 259, 7, 21, 21, ""));
	
	Children.Insert(new GCheckBox(IDC_MATCH_WORD, 14, 42, -1, -1, LgiLoadString(L_FR_MATCH_WORD, "Match whole word only")));
	Children.Insert(new GCheckBox(IDC_MATCH_CASE, 14, 63, -1, -1, LgiLoadString(L_FR_MATCH_CASE, "Match case")));
	Children.Insert(new GCheckBox(IDC_SELECTION_ONLY, 14, 84, -1, -1, LgiLoadString(L_FR_SELECTION_ONLY, "Selection only")));
	
	Children.Insert(new GButton(IDOK, 294, 7, 70, 21, LgiLoadString(L_FR_FIND_NEXT, "Find Next")));
	Children.Insert(new GButton(IDCANCEL, 294, 35, 70, 21, LgiLoadString(L_BTN_CANCEL, "Cancel")));
	
	if (d->Edit) d->Edit->Focus(true);
}

GFindDlg::~GFindDlg()
{
	DeleteObj(d);
}

void GFindDlg::OnCreate()
{
	// Load controls
	if (Find) SetCtrlName(IDC_TEXT, Find);
	SetCtrlValue(IDC_MATCH_WORD, MatchWord);
	SetCtrlValue(IDC_MATCH_CASE, MatchCase);
	
	if (d->Edit)
	{
		d->Edit->Select(0);
		d->Edit->Focus(true);
	}
}

int GFindDlg::OnNotify(GViewI *Ctrl, int Flags)
{
	switch (Ctrl->GetId())
	{
		case IDOK:
		{
			// Save controls
			DeleteArray(Find);
			Find = NewStr(GetCtrlName(IDC_TEXT));
			MatchWord = GetCtrlValue(IDC_MATCH_WORD);
			MatchCase = GetCtrlValue(IDC_MATCH_CASE);

			if (d->Callback)
			{
				d->Callback(this, false, d->CallbackData);
				break;
			}
			// else fall thru
		}
		case IDCANCEL:
		{
			EndModal(Ctrl->GetId());
			break;
		}
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////
// Replace Window

#define IDS_33						1009
#define IDC_REPLACE_WITH			1010
#define IDC_PREV_REPLACE			1011

class GReplaceDlgPrivate
{
public:
	GFrCallback Callback;
	void *CallbackData;
};

GReplaceDlg::GReplaceDlg(GView *Parent, char *InitFind, char *InitReplace, GFrCallback Callback, void *UserData)
{
	d = new GReplaceDlgPrivate;
	d->Callback = Callback;
	d->CallbackData = UserData;
	Find = NewStr(InitFind);
	Replace = NewStr(InitReplace);
	MatchWord = false;
	MatchCase = false;

	SetParent(Parent);
	Name(LgiLoadString(L_FR_REPLACE, "Replace"));
	GRect r(0, 0, 385, 160);
	SetPos(r);
	MoveToCenter();

	Children.Insert(new GText(IDS_16, 14, 14, -1, -1, LgiLoadString(L_FR_FIND_WHAT, "Find what:")));
	Children.Insert(new GText(IDS_33, 14, 42, -1, -1, LgiLoadString(L_FR_REPLACE_WITH, "Replace with:")));
	
	int EditX = 100;
	int ComboX = EditX + 170;
	GView *f;
	Children.Insert(f = new GEdit(IDC_TEXT, EditX, 7, 168, 21, ""));
	//Children.Insert(new GCombo(IDC_PREV_SEARCH, ComboX, 7, 17, 21, ""));
	
	Children.Insert(new GEdit(IDC_REPLACE_WITH, EditX, 35, 168, 21, ""));
	//Children.Insert(new GCombo(IDC_PREV_REPLACE, ComboX, 35, 17, 21, ""));

	Children.Insert(new GCheckBox(IDC_MATCH_WORD, 14, 70, -1, -1, LgiLoadString(L_FR_MATCH_WORD, "Match whole word only")));
	Children.Insert(new GCheckBox(IDC_MATCH_CASE, 14, 91, -1, -1, LgiLoadString(L_FR_MATCH_CASE, "Match case")));
	Children.Insert(new GCheckBox(IDC_SELECTION_ONLY, 14, 112, -1, -1, LgiLoadString(L_FR_SELECTION_ONLY, "Selection only")));
	
	int BtnX = 80;
	Children.Insert(new GButton(IDC_FR_FIND, 294, 7, BtnX, 21, LgiLoadString(L_FR_FIND_NEXT, "Find Next")));
	Children.Insert(new GButton(IDC_FR_REPLACE, 294, 35, BtnX, 21, LgiLoadString(L_FR_REPLACE, "Replace")));
	Children.Insert(new GButton(IDOK, 294, 63, BtnX, 21, LgiLoadString(L_FR_REPLACE_ALL, "Replace All")));
	Children.Insert(new GButton(IDCANCEL, 294, 91, BtnX, 21, LgiLoadString(L_BTN_CANCEL, "Cancel")));
	
	if (f) f->Focus(true);
}

GReplaceDlg::~GReplaceDlg()
{
	DeleteArray(Find);
	DeleteArray(Replace);
	DeleteObj(d);
}

void GReplaceDlg::OnCreate()
{
	if (Find) SetCtrlName(IDC_TEXT, Find);
	if (Replace) SetCtrlName(IDC_REPLACE_WITH, Replace);
	SetCtrlValue(IDC_MATCH_WORD, MatchWord);
	SetCtrlValue(IDC_MATCH_CASE, MatchCase);
}

int GReplaceDlg::OnNotify(GViewI *Ctrl, int Flags)
{
	switch (Ctrl->GetId())
	{
		case IDOK:
		case IDC_FR_FIND:
		case IDC_FR_REPLACE:
		{
			DeleteArray(Find);
			DeleteArray(Replace);

			Find = NewStr(GetCtrlName(IDC_TEXT));
			Replace = NewStr(GetCtrlName(IDC_REPLACE_WITH));
			MatchWord = GetCtrlValue(IDC_MATCH_WORD);
			MatchCase = GetCtrlValue(IDC_MATCH_CASE);
			SelectionOnly = GetCtrlValue(IDC_SELECTION_ONLY);

			if (d->Callback)
			{
				d->Callback(this, Ctrl->GetId() == IDC_FR_REPLACE, d->CallbackData);
				break;
			}
			// else fall thru
		}
		case IDCANCEL:
		{
			EndModal(Ctrl->GetId());
			break;
		}
	}

	return 0;
}
