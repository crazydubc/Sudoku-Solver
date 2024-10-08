#pragma once
#include <afxwin.h>


class CPuzzleEdit : public CEdit {
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};

