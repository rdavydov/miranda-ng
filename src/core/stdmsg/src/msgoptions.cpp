/*

Copyright 2000-12 Miranda IM, 2012-17 Miranda NG project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

#include "m_fontservice.h"

int ChatOptionsInitialize(WPARAM);

struct FontOptionsList
{
	const wchar_t* szDescr;
	COLORREF     defColour;
	const wchar_t* szDefFace;
	BYTE         defStyle;
	char         defSize;
}
static const fontOptionsList[] =
{
	{ LPGENW("Outgoing messages"), RGB(106, 106, 106), L"Arial",    0, -12},
	{ LPGENW("Incoming messages"), RGB(0, 0, 0),       L"Arial",    0, -12},
	{ LPGENW("Outgoing name"),     RGB(89, 89, 89),    L"Arial",    DBFONTF_BOLD, -12},
	{ LPGENW("Outgoing time"),     RGB(0, 0, 0),       L"Terminal", DBFONTF_BOLD, -9},
	{ LPGENW("Outgoing colon"),    RGB(89, 89, 89),    L"Arial",    0, -11},
	{ LPGENW("Incoming name"),     RGB(215, 0, 0),     L"Arial",    DBFONTF_BOLD, -12},
	{ LPGENW("Incoming time"),     RGB(0, 0, 0),       L"Terminal", DBFONTF_BOLD, -9},
	{ LPGENW("Incoming colon"),    RGB(215, 0, 0),     L"Arial",    0, -11},
	{ LPGENW("Message area"),      RGB(0, 0, 0),       L"Arial",    0, -12},
	{ LPGENW("Other events"),      RGB(90, 90, 160),   L"Arial",    0, -12},
};

static BYTE MsgDlgGetFontDefaultCharset(const wchar_t*)
{
  return DEFAULT_CHARSET;
}

bool LoadMsgDlgFont(int i, LOGFONT* lf, COLORREF * colour)
{
	if (i >= _countof(fontOptionsList))
		return false;

	char str[32];

	if (colour) {
		mir_snprintf(str, "SRMFont%dCol", i);
		*colour = db_get_dw(0, SRMMMOD, str, fontOptionsList[i].defColour);
	}
	if (lf) {
		mir_snprintf(str, "SRMFont%dSize", i);
		lf->lfHeight = (char)db_get_b(0, SRMMMOD, str, fontOptionsList[i].defSize);
		lf->lfWidth = 0;
		lf->lfEscapement = 0;
		lf->lfOrientation = 0;
		mir_snprintf(str, "SRMFont%dSty", i);
		int style = db_get_b(0, SRMMMOD, str, fontOptionsList[i].defStyle);
		lf->lfWeight = style & DBFONTF_BOLD ? FW_BOLD : FW_NORMAL;
		lf->lfItalic = style & DBFONTF_ITALIC ? 1 : 0;
		lf->lfUnderline = 0;
		lf->lfStrikeOut = 0;
		lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
		lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf->lfQuality = DEFAULT_QUALITY;
		lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		mir_snprintf(str, "SRMFont%d", i);

		ptrW wszFontFace(db_get_wsa(0, SRMMMOD, str));
		if (wszFontFace == nullptr)
			wcsncpy_s(lf->lfFaceName, fontOptionsList[i].szDefFace, _TRUNCATE);
		else
			mir_wstrncpy(lf->lfFaceName, wszFontFace, _countof(lf->lfFaceName));

		mir_snprintf(str, "SRMFont%dSet", i);
		lf->lfCharSet = db_get_b(0, SRMMMOD, str, MsgDlgGetFontDefaultCharset(lf->lfFaceName));
	}
	return true;
}

void RegisterSRMMFonts(void)
{
	char idstr[10];

	FontIDW fontid = { sizeof(fontid) };
	fontid.flags = FIDF_ALLOWREREGISTER | FIDF_DEFAULTVALID;
	for (int i = 0; i < _countof(fontOptionsList); i++) {
		strncpy_s(fontid.dbSettingsGroup, SRMMMOD, _TRUNCATE);
		wcsncpy_s(fontid.group, LPGENW("Message log"), _TRUNCATE);
		wcsncpy_s(fontid.name, fontOptionsList[i].szDescr, _TRUNCATE);
		mir_snprintf(idstr, "SRMFont%d", i);
		strncpy_s(fontid.prefix, idstr, _TRUNCATE);
		fontid.order = i;

		fontid.flags &= ~FIDF_CLASSMASK;
		fontid.flags |= (fontOptionsList[i].defStyle == DBFONTF_BOLD) ? FIDF_CLASSHEADER : FIDF_CLASSGENERAL;

		fontid.deffontsettings.colour = fontOptionsList[i].defColour;
		fontid.deffontsettings.size = fontOptionsList[i].defSize;
		fontid.deffontsettings.style = fontOptionsList[i].defStyle;
		wcsncpy_s(fontid.deffontsettings.szFace, fontOptionsList[i].szDefFace, _TRUNCATE);
		fontid.deffontsettings.charset = MsgDlgGetFontDefaultCharset(fontOptionsList[i].szDefFace);
		wcsncpy_s(fontid.backgroundGroup, LPGENW("Message log"), _TRUNCATE);
		wcsncpy_s(fontid.backgroundName, LPGENW("Background"), _TRUNCATE);
		Font_RegisterW(&fontid);
	}

	ColourIDW colourid = { sizeof(colourid) };
	strncpy_s(colourid.dbSettingsGroup, SRMMMOD, _TRUNCATE);
	strncpy_s(colourid.setting, SRMSGSET_BKGCOLOUR, _TRUNCATE);
	colourid.defcolour = SRMSGDEFSET_BKGCOLOUR;
	wcsncpy_s(colourid.name, LPGENW("Background"), _TRUNCATE);
	wcsncpy_s(colourid.group, LPGENW("Message log"), _TRUNCATE);
	Colour_RegisterW(&colourid);
}

/////////////////////////////////////////////////////////////////////////////////////////

struct CheckBoxValues_t
{
	DWORD  style;
	wchar_t* szDescr;
}
statusValues[] =
{
	{ MODEF_OFFLINE, LPGENW("Offline") },
	{ PF2_ONLINE, LPGENW("Online") },
	{ PF2_SHORTAWAY, LPGENW("Away") },
	{ PF2_LONGAWAY, LPGENW("Not available") },
	{ PF2_LIGHTDND, LPGENW("Occupied") },
	{ PF2_HEAVYDND, LPGENW("Do not disturb") },
	{ PF2_FREECHAT, LPGENW("Free for chat") },
	{ PF2_INVISIBLE, LPGENW("Invisible") },
	{ PF2_OUTTOLUNCH, LPGENW("Out to lunch") },
	{ PF2_ONTHEPHONE, LPGENW("On the phone") }
};

static void FillCheckBoxTree(HWND hwndTree, const struct CheckBoxValues_t *values, int nValues, DWORD style)
{
	TVINSERTSTRUCT tvis;
	tvis.hParent = nullptr;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_STATE;
	for (int i = 0; i < nValues; i++) {
		tvis.item.lParam = values[i].style;
		tvis.item.pszText = TranslateW(values[i].szDescr);
		tvis.item.stateMask = TVIS_STATEIMAGEMASK;
		tvis.item.state = INDEXTOSTATEIMAGEMASK((style & tvis.item.lParam) != 0 ? 2 : 1);
		TreeView_InsertItem(hwndTree, &tvis);
	}
}

static DWORD MakeCheckBoxTreeFlags(HWND hwndTree)
{
	DWORD flags = 0;

	TVITEM tvi;
	tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_STATE;
	tvi.hItem = TreeView_GetRoot(hwndTree);
	while (tvi.hItem) {
		TreeView_GetItem(hwndTree, &tvi);
		if (((tvi.state & TVIS_STATEIMAGEMASK) >> 12 == 2))
			flags |= tvi.lParam;
		tvi.hItem = TreeView_GetNextSibling(hwndTree, tvi.hItem);
	}
	return flags;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR CALLBACK DlgProcOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_POPLIST), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_POPLIST), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);
		FillCheckBoxTree(GetDlgItem(hwndDlg, IDC_POPLIST), statusValues, _countof(statusValues), db_get_dw(0, SRMMMOD, SRMSGSET_POPFLAGS, SRMSGDEFSET_POPFLAGS));

		CheckDlgButton(hwndDlg, IDC_DELTEMP, g_dat.bDeleteTempCont);
		CheckDlgButton(hwndDlg, IDC_AUTOMIN, g_dat.bAutoMin);
		CheckDlgButton(hwndDlg, IDC_CASCADE, g_dat.bCascade);
		CheckDlgButton(hwndDlg, IDC_AUTOCLOSE, g_dat.bAutoClose);
		CheckDlgButton(hwndDlg, IDC_CHARCOUNT, g_dat.bShowReadChar);
		CheckDlgButton(hwndDlg, IDC_STATUSWIN, g_dat.bUseStatusWinIcon);
		CheckDlgButton(hwndDlg, IDC_CTRLSUPPORT, g_dat.bCtrlSupport);
		CheckDlgButton(hwndDlg, IDC_SHOWSENDBTN, g_dat.bSendButton);
		CheckDlgButton(hwndDlg, IDC_SENDONENTER, g_dat.bSendOnEnter);
		CheckDlgButton(hwndDlg, IDC_LIMITAVATARH, db_get_b(0, SRMMMOD, SRMSGSET_LIMITAVHEIGHT, SRMSGDEFSET_LIMITAVHEIGHT));
		CheckDlgButton(hwndDlg, IDC_SENDONDBLENTER, g_dat.bSendOnDblEnter);
		CheckDlgButton(hwndDlg, IDC_AVATARSUPPORT, g_dat.bShowAvatar);
		CheckDlgButton(hwndDlg, IDC_SHOWBUTTONLINE, g_dat.bShowButtons);
		CheckDlgButton(hwndDlg, IDC_SAVEPERCONTACT, g_dat.bSavePerContact);
		CheckDlgButton(hwndDlg, IDC_DONOTSTEALFOCUS, g_dat.bDoNotStealFocus);
		{
			DWORD avatarHeight = db_get_dw(0, SRMMMOD, SRMSGSET_AVHEIGHT, SRMSGDEFSET_AVHEIGHT);
			SetDlgItemInt(hwndDlg, IDC_NFLASHES, db_get_b(0, SRMMMOD, SRMSGSET_FLASHCOUNT, SRMSGDEFSET_FLASHCOUNT), FALSE);
			SetDlgItemInt(hwndDlg, IDC_AVATARHEIGHT, avatarHeight, FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LIMITAVATARH), IsDlgButtonChecked(hwndDlg, IDC_AVATARSUPPORT));
			if (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_AVATARSUPPORT))
				EnableWindow(GetDlgItem(hwndDlg, IDC_AVATARHEIGHT), FALSE);
			else
				EnableWindow(GetDlgItem(hwndDlg, IDC_AVATARHEIGHT), IsDlgButtonChecked(hwndDlg, IDC_LIMITAVATARH));
			
			DWORD msgTimeout = db_get_dw(0, SRMMMOD, SRMSGSET_MSGTIMEOUT, SRMSGDEFSET_MSGTIMEOUT);
			SetDlgItemInt(hwndDlg, IDC_SECONDS, msgTimeout >= SRMSGSET_MSGTIMEOUT_MIN ? msgTimeout / 1000 : SRMSGDEFSET_MSGTIMEOUT / 1000, FALSE);
		}
		EnableWindow(GetDlgItem(hwndDlg, IDC_CASCADE), !g_dat.bSavePerContact);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CTRLSUPPORT), !g_dat.bAutoClose);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_AUTOMIN:
			CheckDlgButton(hwndDlg, IDC_AUTOCLOSE, BST_UNCHECKED);
			break;
		case IDC_AUTOCLOSE:
			CheckDlgButton(hwndDlg, IDC_AUTOMIN, BST_UNCHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CTRLSUPPORT), BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_AUTOCLOSE));
			break;
		case IDC_SENDONENTER:
			CheckDlgButton(hwndDlg, IDC_SENDONDBLENTER, BST_UNCHECKED);
			break;
		case IDC_SENDONDBLENTER:
			CheckDlgButton(hwndDlg, IDC_SENDONENTER, BST_UNCHECKED);
			break;
		case IDC_SAVEPERCONTACT:
			EnableWindow(GetDlgItem(hwndDlg, IDC_CASCADE), BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_SAVEPERCONTACT));
			break;
		case IDC_SECONDS:
			if (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())
				return 0;
			break;
		case IDC_AVATARSUPPORT:
			EnableWindow(GetDlgItem(hwndDlg, IDC_LIMITAVATARH), IsDlgButtonChecked(hwndDlg, IDC_AVATARSUPPORT));
			if (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_AVATARSUPPORT))
				EnableWindow(GetDlgItem(hwndDlg, IDC_AVATARHEIGHT), FALSE);
			else EnableWindow(GetDlgItem(hwndDlg, IDC_AVATARHEIGHT), IsDlgButtonChecked(hwndDlg, IDC_LIMITAVATARH));
			break;
		case IDC_LIMITAVATARH:
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATARHEIGHT), IsDlgButtonChecked(hwndDlg, IDC_LIMITAVATARH));
			break;
		case IDC_AVATARHEIGHT:
			if (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())
				return 0;
			break;
		}
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;
	
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case IDC_POPLIST:
			if (((LPNMHDR)lParam)->code == NM_CLICK) {
				TVHITTESTINFO hti;
				hti.pt.x = (short)LOWORD(GetMessagePos());
				hti.pt.y = (short)HIWORD(GetMessagePos());
				ScreenToClient(((LPNMHDR)lParam)->hwndFrom, &hti.pt);
				if (TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom, &hti))
					if (hti.flags & TVHT_ONITEMSTATEICON) {
						TVITEM tvi;
						tvi.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
						tvi.hItem = hti.hItem;
						TreeView_GetItem(((LPNMHDR)lParam)->hwndFrom, &tvi);
						tvi.iImage = tvi.iSelectedImage = tvi.iImage == 1 ? 2 : 1;
						TreeView_SetItem(((LPNMHDR)lParam)->hwndFrom, &tvi);
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}
			}
			break;
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				db_set_dw(0, SRMMMOD, SRMSGSET_POPFLAGS, MakeCheckBoxTreeFlags(GetDlgItem(hwndDlg, IDC_POPLIST)));
				db_set_b(0, SRMMMOD, SRMSGSET_DONOTSTEALFOCUS, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_DONOTSTEALFOCUS));
				db_set_b(0, SRMMMOD, SRMSGSET_SHOWBUTTONLINE, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOWBUTTONLINE));
				db_set_b(0, SRMMMOD, SRMSGSET_AUTOMIN, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_AUTOMIN));
				db_set_b(0, SRMMMOD, SRMSGSET_AUTOCLOSE, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_AUTOCLOSE));
				db_set_b(0, SRMMMOD, SRMSGSET_SAVEPERCONTACT, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SAVEPERCONTACT));
				db_set_b(0, SRMMMOD, SRMSGSET_CASCADE, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_CASCADE));
				db_set_b(0, SRMMMOD, SRMSGSET_SENDONENTER, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SENDONENTER));
				db_set_b(0, SRMMMOD, SRMSGSET_SENDONDBLENTER, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SENDONDBLENTER));
				db_set_b(0, SRMMMOD, SRMSGSET_STATUSICON, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_STATUSWIN));

				db_set_b(0, SRMMMOD, SRMSGSET_AVATARENABLE, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_AVATARSUPPORT));
				db_set_b(0, SRMMMOD, SRMSGSET_LIMITAVHEIGHT, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_LIMITAVATARH));
				
				DWORD avatarHeight = GetDlgItemInt(hwndDlg, IDC_AVATARHEIGHT, nullptr, TRUE);
				db_set_dw(0, SRMMMOD, SRMSGSET_AVHEIGHT, avatarHeight <= 0 ? SRMSGDEFSET_AVHEIGHT : avatarHeight);

				db_set_b(0, SRMMMOD, SRMSGSET_SENDBUTTON, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOWSENDBTN));
				db_set_b(0, SRMMMOD, SRMSGSET_CHARCOUNT, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_CHARCOUNT));
				db_set_b(0, SRMMMOD, SRMSGSET_CTRLSUPPORT, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_CTRLSUPPORT));
				db_set_b(0, SRMMMOD, SRMSGSET_DELTEMP, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_DELTEMP));
				db_set_b(0, SRMMMOD, SRMSGSET_FLASHCOUNT, (BYTE)GetDlgItemInt(hwndDlg, IDC_NFLASHES, nullptr, TRUE));

				DWORD msgTimeout = GetDlgItemInt(hwndDlg, IDC_SECONDS, nullptr, TRUE) * 1000;
				if (msgTimeout < SRMSGSET_MSGTIMEOUT_MIN) msgTimeout = SRMSGDEFSET_MSGTIMEOUT;
				db_set_dw(0, SRMMMOD, SRMSGSET_MSGTIMEOUT, msgTimeout);

				ReloadGlobals();
				Srmm_Broadcast(DM_OPTIONSAPPLIED, TRUE, 0);
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR CALLBACK DlgProcLogOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HBRUSH hBkgColourBrush;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		switch (db_get_b(0, SRMMMOD, SRMSGSET_LOADHISTORY, SRMSGDEFSET_LOADHISTORY)) {
		case LOADHISTORY_UNREAD:
			CheckDlgButton(hwndDlg, IDC_LOADUNREAD, BST_CHECKED);
			break;
		case LOADHISTORY_COUNT:
			CheckDlgButton(hwndDlg, IDC_LOADCOUNT, BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOADCOUNTN), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOADCOUNTSPIN), TRUE);
			break;
		case LOADHISTORY_TIME:
			CheckDlgButton(hwndDlg, IDC_LOADTIME, BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOADTIMEN), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOADTIMESPIN), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STMINSOLD), TRUE);
			break;
		}
		SendDlgItemMessage(hwndDlg, IDC_LOADCOUNTSPIN, UDM_SETRANGE, 0, MAKELONG(100, 0));
		SendDlgItemMessage(hwndDlg, IDC_LOADCOUNTSPIN, UDM_SETPOS, 0, db_get_w(0, SRMMMOD, SRMSGSET_LOADCOUNT, SRMSGDEFSET_LOADCOUNT));
		SendDlgItemMessage(hwndDlg, IDC_LOADTIMESPIN, UDM_SETRANGE, 0, MAKELONG(12 * 60, 0));
		SendDlgItemMessage(hwndDlg, IDC_LOADTIMESPIN, UDM_SETPOS, 0, db_get_w(0, SRMMMOD, SRMSGSET_LOADTIME, SRMSGDEFSET_LOADTIME));

		CheckDlgButton(hwndDlg, IDC_SHOWLOGICONS, db_get_b(0, SRMMMOD, SRMSGSET_SHOWLOGICONS, SRMSGDEFSET_SHOWLOGICONS));
		CheckDlgButton(hwndDlg, IDC_SHOWNAMES, !db_get_b(0, SRMMMOD, SRMSGSET_HIDENAMES, SRMSGDEFSET_HIDENAMES));
		CheckDlgButton(hwndDlg, IDC_SHOWTIMES, db_get_b(0, SRMMMOD, SRMSGSET_SHOWTIME, SRMSGDEFSET_SHOWTIME));
		EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWSECS), IsDlgButtonChecked(hwndDlg, IDC_SHOWTIMES));
		CheckDlgButton(hwndDlg, IDC_SHOWSECS, db_get_b(0, SRMMMOD, SRMSGSET_SHOWSECS, SRMSGDEFSET_SHOWSECS));
		EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWDATES), IsDlgButtonChecked(hwndDlg, IDC_SHOWTIMES));
		CheckDlgButton(hwndDlg, IDC_SHOWDATES, db_get_b(0, SRMMMOD, SRMSGSET_SHOWDATE, SRMSGDEFSET_SHOWDATE));
		CheckDlgButton(hwndDlg, IDC_SHOWFORMATTING, db_get_b(0, SRMMMOD, SRMSGSET_SHOWFORMAT, SRMSGDEFSET_SHOWFORMAT));
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_LOADUNREAD:
		case IDC_LOADCOUNT:
		case IDC_LOADTIME:
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOADCOUNTN), IsDlgButtonChecked(hwndDlg, IDC_LOADCOUNT));
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOADCOUNTSPIN), IsDlgButtonChecked(hwndDlg, IDC_LOADCOUNT));
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOADTIMEN), IsDlgButtonChecked(hwndDlg, IDC_LOADTIME));
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOADTIMESPIN), IsDlgButtonChecked(hwndDlg, IDC_LOADTIME));
			EnableWindow(GetDlgItem(hwndDlg, IDC_STMINSOLD), IsDlgButtonChecked(hwndDlg, IDC_LOADTIME));
			break;

		case IDC_SHOWTIMES:
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWSECS), IsDlgButtonChecked(hwndDlg, IDC_SHOWTIMES));
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWDATES), IsDlgButtonChecked(hwndDlg, IDC_SHOWTIMES));
			break;

		case IDC_LOADCOUNTN:
		case IDC_LOADTIMEN:
			if (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())
				return TRUE;
			break;
		}
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				if (IsDlgButtonChecked(hwndDlg, IDC_LOADCOUNT))
					db_set_b(0, SRMMMOD, SRMSGSET_LOADHISTORY, LOADHISTORY_COUNT);
				else if (IsDlgButtonChecked(hwndDlg, IDC_LOADTIME))
					db_set_b(0, SRMMMOD, SRMSGSET_LOADHISTORY, LOADHISTORY_TIME);
				else
					db_set_b(0, SRMMMOD, SRMSGSET_LOADHISTORY, LOADHISTORY_UNREAD);
				db_set_w(0, SRMMMOD, SRMSGSET_LOADCOUNT, (WORD)SendDlgItemMessage(hwndDlg, IDC_LOADCOUNTSPIN, UDM_GETPOS, 0, 0));
				db_set_w(0, SRMMMOD, SRMSGSET_LOADTIME, (WORD)SendDlgItemMessage(hwndDlg, IDC_LOADTIMESPIN, UDM_GETPOS, 0, 0));
				db_set_b(0, SRMMMOD, SRMSGSET_SHOWLOGICONS, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOWLOGICONS));
				db_set_b(0, SRMMMOD, SRMSGSET_HIDENAMES, (BYTE)BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_SHOWNAMES));
				db_set_b(0, SRMMMOD, SRMSGSET_SHOWTIME, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOWTIMES));
				db_set_b(0, SRMMMOD, SRMSGSET_SHOWSECS, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOWSECS));
				db_set_b(0, SRMMMOD, SRMSGSET_SHOWDATE, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOWDATES));
				db_set_b(0, SRMMMOD, SRMSGSET_SHOWFORMAT, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOWFORMATTING));

				FreeMsgLogIcons();
				LoadMsgLogIcons();
				ReloadGlobals();
				Srmm_Broadcast(DM_OPTIONSAPPLIED, TRUE, 0);
				return TRUE;
			}
			break;
		}
		break;
	case WM_DESTROY:
		DeleteObject(hBkgColourBrush);
		break;
	}
	return FALSE;
}

static void ResetCList(HWND hwndDlg)
{
	SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETUSEGROUPS, db_get_b(0, "CList", "UseGroups", SETTING_USEGROUPS_DEFAULT), 0);
	SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETHIDEEMPTYGROUPS, 1, 0);
}

static void RebuildList(HWND hwndDlg, HANDLE hItemNew, HANDLE hItemUnknown)
{
	BYTE defType = db_get_b(0, SRMMMOD, SRMSGSET_TYPINGNEW, SRMSGDEFSET_TYPINGNEW);
	if (hItemNew && defType)
		SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETCHECKMARK, (WPARAM)hItemNew, 1);

	if (hItemUnknown && db_get_b(0, SRMMMOD, SRMSGSET_TYPINGUNKNOWN, SRMSGDEFSET_TYPINGUNKNOWN))
		SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETCHECKMARK, (WPARAM)hItemUnknown, 1);

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		HANDLE hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_FINDCONTACT, hContact, 0);
		if (hItem && db_get_b(hContact, SRMMMOD, SRMSGSET_TYPING, defType))
			SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETCHECKMARK, (WPARAM)hItem, 1);
	}
}

static void SaveList(HWND hwndDlg, HANDLE hItemNew, HANDLE hItemUnknown)
{
	if (hItemNew)
		db_set_b(0, SRMMMOD, SRMSGSET_TYPINGNEW, (BYTE)(SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_GETCHECKMARK, (WPARAM)hItemNew, 0) ? 1 : 0));

	if (hItemUnknown)
		db_set_b(0, SRMMMOD, SRMSGSET_TYPINGUNKNOWN, (BYTE)(SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_GETCHECKMARK, (WPARAM)hItemUnknown, 0) ? 1 : 0));

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		HANDLE hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_FINDCONTACT, hContact, 0);
		if (hItem)
			db_set_b(hContact, SRMMMOD, SRMSGSET_TYPING, (BYTE)(SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_GETCHECKMARK, (WPARAM)hItem, 0) ? 1 : 0));
	}
}

static INT_PTR CALLBACK DlgProcTypeOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HANDLE hItemNew, hItemUnknown;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			CLCINFOITEM cii = { sizeof(cii) };
			cii.flags = CLCIIF_GROUPFONT | CLCIIF_CHECKBOX;
			cii.pszText = TranslateT("** New contacts **");
			hItemNew = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_ADDINFOITEM, 0, (LPARAM)&cii);
			cii.pszText = TranslateT("** Unknown contacts **");
			hItemUnknown = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_ADDINFOITEM, 0, (LPARAM)&cii);
		}
		SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_CLIST), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_CLIST), GWL_STYLE) | (CLS_SHOWHIDDEN) | (CLS_NOHIDEOFFLINE));
		ResetCList(hwndDlg);
		CheckDlgButton(hwndDlg, IDC_SHOWNOTIFY, db_get_b(0, SRMMMOD, SRMSGSET_SHOWTYPING, SRMSGDEFSET_SHOWTYPING));
		CheckDlgButton(hwndDlg, IDC_TYPEWIN, db_get_b(0, SRMMMOD, SRMSGSET_SHOWTYPINGWIN, SRMSGDEFSET_SHOWTYPINGWIN));
		CheckDlgButton(hwndDlg, IDC_TYPETRAY, db_get_b(0, SRMMMOD, SRMSGSET_SHOWTYPINGNOWIN, SRMSGDEFSET_SHOWTYPINGNOWIN));
		CheckDlgButton(hwndDlg, IDC_NOTIFYTRAY, db_get_b(0, SRMMMOD, SRMSGSET_SHOWTYPINGCLIST, SRMSGDEFSET_SHOWTYPINGCLIST));
		CheckDlgButton(hwndDlg, IDC_NOTIFYBALLOON, !db_get_b(0, SRMMMOD, SRMSGSET_SHOWTYPINGCLIST, SRMSGDEFSET_SHOWTYPINGCLIST));
		EnableWindow(GetDlgItem(hwndDlg, IDC_TYPEWIN), IsDlgButtonChecked(hwndDlg, IDC_SHOWNOTIFY));
		EnableWindow(GetDlgItem(hwndDlg, IDC_TYPETRAY), IsDlgButtonChecked(hwndDlg, IDC_SHOWNOTIFY));
		EnableWindow(GetDlgItem(hwndDlg, IDC_NOTIFYTRAY), IsDlgButtonChecked(hwndDlg, IDC_TYPETRAY));
		EnableWindow(GetDlgItem(hwndDlg, IDC_NOTIFYBALLOON), IsDlgButtonChecked(hwndDlg, IDC_TYPETRAY));
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_TYPETRAY:
			if (IsDlgButtonChecked(hwndDlg, IDC_TYPETRAY)) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_NOTIFYTRAY), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_NOTIFYBALLOON), TRUE);
			}
			else {
				EnableWindow(GetDlgItem(hwndDlg, IDC_NOTIFYTRAY), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_NOTIFYBALLOON), FALSE);
			}
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		case IDC_SHOWNOTIFY:
			EnableWindow(GetDlgItem(hwndDlg, IDC_TYPEWIN), IsDlgButtonChecked(hwndDlg, IDC_SHOWNOTIFY));
			EnableWindow(GetDlgItem(hwndDlg, IDC_TYPETRAY), IsDlgButtonChecked(hwndDlg, IDC_SHOWNOTIFY));
			EnableWindow(GetDlgItem(hwndDlg, IDC_NOTIFYTRAY), IsDlgButtonChecked(hwndDlg, IDC_SHOWNOTIFY));
			EnableWindow(GetDlgItem(hwndDlg, IDC_NOTIFYBALLOON), IsDlgButtonChecked(hwndDlg, IDC_SHOWNOTIFY));
			//fall-thru
		case IDC_TYPEWIN:
		case IDC_NOTIFYTRAY:
		case IDC_NOTIFYBALLOON:
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		}
		break;

	case WM_NOTIFY:
		switch (((NMHDR *)lParam)->idFrom) {
		case IDC_CLIST:
			switch (((NMHDR *)lParam)->code) {
			case CLN_OPTIONSCHANGED:
				ResetCList(hwndDlg);
				break;
			case CLN_CHECKCHANGED:
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case CLN_LISTREBUILT:
				RebuildList(hwndDlg, hItemNew, hItemUnknown);
				break;
			}
			break;
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				SaveList(hwndDlg, hItemNew, hItemUnknown);
				db_set_b(0, SRMMMOD, SRMSGSET_SHOWTYPING, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOWNOTIFY));
				db_set_b(0, SRMMMOD, SRMSGSET_SHOWTYPINGWIN, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_TYPEWIN));
				db_set_b(0, SRMMMOD, SRMSGSET_SHOWTYPINGNOWIN, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_TYPETRAY));
				db_set_b(0, SRMMMOD, SRMSGSET_SHOWTYPINGCLIST, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_NOTIFYTRAY));
				ReloadGlobals();
				Srmm_Broadcast(DM_OPTIONSAPPLIED, TRUE, 0);
			}
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

class COptionsTabDlg : public CDlgBase
{
	CCtrlCheck m_chkTabs, m_chkTabsBottom, m_chkTabsClose, m_chkTabsRestore;

public:
	COptionsTabDlg() :
		CDlgBase(g_hInst, IDD_OPT_TABS),
		m_chkTabs(this, IDC_USETABS),
		m_chkTabsBottom(this, IDC_TABSBOTTOM),
		m_chkTabsClose(this, IDC_CLOSETABS),
		m_chkTabsRestore(this, IDC_RESTORETABS)
	{
		m_chkTabs.OnChange = Callback(this, &COptionsTabDlg::onChange_Tabs);
	}

	virtual void OnInitDialog() override
	{
		m_chkTabs.SetState(db_get_b(0, CHAT_MODULE, "Tabs", 1));
		m_chkTabsBottom.SetState(db_get_b(0, CHAT_MODULE, "TabBottom", 1));
		m_chkTabsClose.SetState(db_get_b(0, CHAT_MODULE, "TabCloseOnDblClick", 1));
		m_chkTabsRestore.SetState(db_get_b(0, CHAT_MODULE, "TabRestore", 1));
		onChange_Tabs(&m_chkTabs);
	}

	virtual void OnApply() override
	{
		BYTE bOldValue = db_get_b(0, CHAT_MODULE, "Tabs", 1);

		db_set_b(0, CHAT_MODULE, "Tabs", m_chkTabs.GetState());
		db_set_b(0, CHAT_MODULE, "TabBottom", m_chkTabsBottom.GetState());
		db_set_b(0, CHAT_MODULE, "TabCloseOnDblClick", m_chkTabsClose.GetState());
		db_set_b(0, CHAT_MODULE, "TabRestore", m_chkTabsRestore.GetState());

		pci->ReloadSettings();

		if (bOldValue != db_get_b(0, CHAT_MODULE, "Tabs", 1)) {
			pci->SM_BroadcastMessage(nullptr, WM_CLOSE, 0, 1, FALSE);
			g_Settings.bTabsEnable = db_get_b(0, CHAT_MODULE, "Tabs", 1) != 0;
		}
		else Chat_UpdateOptions();
	}

	void onChange_Tabs(CCtrlCheck *pCheck)
	{
		bool bEnabled = pCheck->GetState() != 0;
		m_chkTabsBottom.Enable(bEnabled);
		m_chkTabsClose.Enable(bEnabled);
		m_chkTabsRestore.Enable(bEnabled);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

static int OptInitialise(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 910000000;
	odp.hInstance = g_hInst;
	odp.szTab.a = LPGEN("Messaging");
	odp.flags = ODPF_BOLDGROUPS;

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_MSGDLG);
	odp.szTitle.a = LPGEN("Message sessions");
	odp.pfnDlgProc = DlgProcOptions;
	Options_AddPage(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_MSGLOG);
	odp.szTab.a = LPGEN("Messaging log");
	odp.pfnDlgProc = DlgProcLogOptions;
	Options_AddPage(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_MSGTYPE);
	odp.szTab.a = LPGEN("Typing notify");
	odp.pfnDlgProc = DlgProcTypeOptions;
	Options_AddPage(wParam, &odp);

	odp.pszTemplate = nullptr;
	odp.pfnDlgProc = nullptr;
	odp.szTab.a = LPGEN("Tabs");
	odp.pDialog = new COptionsTabDlg();
	Options_AddPage(wParam, &odp);

	ChatOptionsInitialize(wParam);
	return 0;
}

void InitOptions(void)
{
	HookEvent(ME_OPT_INITIALISE, OptInitialise);
}
