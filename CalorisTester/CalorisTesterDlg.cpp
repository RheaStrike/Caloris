
// CalorisTesterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CalorisTester.h"
#include "CalorisTesterDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CCalorisTesterDlg dialog




CCalorisTesterDlg::CCalorisTesterDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CCalorisTesterDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCalorisTesterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CCalorisTesterDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CONNECT, &CCalorisTesterDlg::OnBnClickedConnect)
	ON_BN_CLICKED(IDC_CLOSE, &CCalorisTesterDlg::OnBnClickedClose)
	ON_BN_CLICKED(IDC_SEND, &CCalorisTesterDlg::OnBnClickedSend)
	ON_BN_CLICKED(IDC_BUTTON_LOGIN, &CCalorisTesterDlg::OnBnClickedButtonLogin)
END_MESSAGE_MAP()


// CCalorisTesterDlg message handlers

BOOL CCalorisTesterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_pCaloris = NULL;

	GetDlgItem(IDC_ADDRESS)->SetWindowText(_T("127.0.0.1"));
	GetDlgItem(IDC_PORT)->SetWindowText(_T("8080"));

	GetDlgItem(IDC_STATIC_CON)->SetWindowText(_T("NOT CONNECT"));
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CCalorisTesterDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CCalorisTesterDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCalorisTesterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CCalorisTesterDlg::OnBnClickedConnect()
{	
	if(NULL != m_pCaloris)
		return;

	m_pCaloris = new CCaloris(&this->m_hWnd);

	CString strAddress, strPort;
	UINT uPort;

	GetDlgItem(IDC_ADDRESS)->GetWindowText(strAddress);
	GetDlgItem(IDC_PORT)->GetWindowText(strPort);
	uPort = _ttoi(strPort.GetBuffer(strPort.GetLength()));
	
	CStringA strAdressA( strAddress );
	int iRet = m_pCaloris->ConnectToNodeJS(strAdressA.GetBuffer(strAdressA.GetLength()), uPort);
}


void CCalorisTesterDlg::OnBnClickedClose()
{
	SetConnInfo(FALSE);
	if(m_pCaloris)
	{
		delete m_pCaloris;
		m_pCaloris = NULL;
	}
}



void CCalorisTesterDlg::OnBnClickedButtonLogin()
{
	CString strEvent;

	GetDlgItem(IDC_EDIT_ID)->GetWindowText(strEvent);

	char szEvent[256] = {0,};
	::WideCharToMultiByte( CP_ACP, 0, strEvent, -1, szEvent, 256, NULL, NULL );

	if(m_pCaloris && m_pCaloris->IsConnected())
	{
		Json::Value sendMsg;

		sendMsg["action"] = "loginReq";
		sendMsg["username"] = szEvent;

		m_pCaloris->sendMessage(sendMsg);
	}
}


void CCalorisTesterDlg::OnBnClickedSend()
{
	CString strMessage;
	GetDlgItem(IDC_SENDDATA)->GetWindowText(strMessage);

	char szMessage[1024] = {0,};
	::WideCharToMultiByte( CP_ACP, 0, strMessage, -1, szMessage, 1024, NULL, NULL );

	if(m_pCaloris && m_pCaloris->IsConnected())
	{
		Json::Value sendMsg;

		sendMsg["action"] = "chatReq";
		sendMsg["chat"] = szMessage;

		m_pCaloris->sendMessage(sendMsg);
	}
}


LRESULT CCalorisTesterDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_PRINT_RECV :
		{	
			CListBox* pList = (CListBox*)GetDlgItem(IDC_RECV_LIST);
			CString strTemp(m_pCaloris->GetRecvData().c_str());
			pList->AddString(strTemp);			
			pList->SetCurSel(pList->GetCount()-1);
		}
		break;

		case WM_PRINT_CON :
		{
			BOOL bIsConnect = (BOOL)wParam;
			SetConnInfo(bIsConnect);				
		}
		break;

		case  WM_RECV_MSG:
		{
			CListBox* pList = (CListBox*)GetDlgItem(IDC_RECV_LIST);
			CString strTemp(m_pCaloris->GetRecvData().c_str());

			pList->AddString(strTemp);			
			pList->SetCurSel(pList->GetCount()-1);
			}
		break;
	}

	return CDialogEx::DefWindowProc(message, wParam, lParam);
}


void CCalorisTesterDlg::SetConnInfo(BOOL bIsConn)
{
	if(FALSE == bIsConn)
	{
		GetDlgItem(IDC_STATIC_CON)->SetWindowText(_T("NOT CONNECT"));
		
		CString t = CTime::GetCurrentTime().Format(_T("Disconnect - %H:%M:%S")); 
		CListBox* pList = (CListBox*)GetDlgItem(IDC_RECV_LIST);		
		pList->AddString(t);
		pList->SetCurSel(pList->GetCount()-1);

		if(m_pCaloris)
		{
			delete m_pCaloris;
			m_pCaloris = NULL;
		}
	}
	else
		GetDlgItem(IDC_STATIC_CON)->SetWindowText(_T("CONNECTED"));
}
