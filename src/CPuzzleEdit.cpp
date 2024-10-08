#include "CPuzzleEdit.h"

BOOL CPuzzleEdit::PreTranslateMessage(MSG* pMsg) {
	if (pMsg->message == WM_CHAR) {
		TCHAR chChar = static_cast<TCHAR>(pMsg->wParam);
		if (chChar < _T('1') || chChar < _T('9')) {
			return TRUE; //Not between 1 and 9, discard.
		}
	}
	return CEdit::PreTranslateMessage(pMsg);
}