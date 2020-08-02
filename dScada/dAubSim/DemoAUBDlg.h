// DemoAUBDlg.h : header file
//

#include "afxwin.h"
#if !defined(AFX_DemoAUBDLG_H__B7C56399_1361_41A2_B842_AC5FB14AB8FA__INCLUDED_)
#define AFX_DemoAUBDLG_H__B7C56399_1361_41A2_B842_AC5FB14AB8FA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CDemoAUBDlg dialog

class CDemoAUBDlg : public CDialog
{
// Construction
public:
	CDemoAUBDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CDemoAUBDlg)
	enum { IDD = IDD_DemoAUB_DIALOG };
	CStatic	m_txt5;
	CStatic	m_txt4;
	CStatic	m_txt3;
	CStatic	m_txt2;
	CStatic	m_txt1;
	CStatic	m_dbg2;
	CStatic	m_dbg1;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDemoAUBDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CDemoAUBDlg)
	virtual BOOL OnInitDialog();

    void StartScadaTasks();

    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	//virtual void OnCancel();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnClose();
  CListBox m_lista;
  CFont m_font;
  afx_msg void OnBnClickedClrevents();
  afx_msg void OnBnClickedTndSwap();

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.



#endif // !defined(AFX_DemoAUBDLG_H__B7C56399_1361_41A2_B842_AC5FB14AB8FA__INCLUDED_)
