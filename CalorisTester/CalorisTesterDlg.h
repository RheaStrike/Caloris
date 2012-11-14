
// CalorisTesterDlg.h : header file
//

#pragma once

#include "../Caloris/Caloris.h"

// CCalorisTesterDlg dialog
class CCalorisTesterDlg : public CDialogEx
{
// Construction
public:
	CCalorisTesterDlg(CWnd* pParent = NULL);	// standard constructor

	CCaloris* m_pCaloris;
// Dialog Data
	enum { IDD = IDD_CALORISTESTER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedConnect();
	afx_msg void OnBnClickedClose();
	afx_msg void OnBnClickedSend();
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	void SetConnInfo(BOOL bIsConn);
	afx_msg void OnBnClickedButtonLogin();
};
