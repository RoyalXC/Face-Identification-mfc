
// Face IdentificationDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "Face Identification.h"
#include "Face IdentificationDlg.h"

using namespace cv;


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	//	virtual BOOL DestroyWindow();
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()


// CFaceIdentificationDlg 对话框



CFaceIdentificationDlg::CFaceIdentificationDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_FACEIDENTIFICATION_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFaceIdentificationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CFaceIdentificationDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(IDC_ADD, &CFaceIdentificationDlg::OnBnClickedAddNewUser)
	ON_COMMAND(IDC_START, &CFaceIdentificationDlg::OnBnClickedStart)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CFaceIdentificationDlg 消息处理程序

BOOL CFaceIdentificationDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	SetWindowText(_T("人脸识别智能门禁"));

	InitCameraOK = 0;

	if (InitCameraOK == 0) {
		namedWindow("view", WINDOW_AUTOSIZE);
		HWND hWnd = (HWND)cvGetWindowHandle("view");
		HWND hParent = ::GetParent(hWnd);
		::SetParent(hWnd, GetDlgItem(IDC_STATIC)->m_hWnd);
		::ShowWindow(hParent, SW_HIDE);

		InitCameraOK = 1;

		Demo.SetLivingDetection(true);

		Camera = new VideoCapture("E:\\1\\1.mp4");
		Camera->set(CV_CAP_PROP_FRAME_WIDTH, 640);
		Camera->set(CV_CAP_PROP_FRAME_HEIGHT, 360);
	}
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CFaceIdentificationDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CFaceIdentificationDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}

	OnBnClickedStart();
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CFaceIdentificationDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CAboutDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CMenu menu;
	menu.LoadMenuW(IDR_MENU);
	CMenu   *pContextMenu = menu.GetSubMenu(0); //获取第一个弹出菜单，所以第一个菜单必须有子菜单 
	CPoint point1;//定义一个用于确定光标位置的位置  
	GetCursorPos(&point1);//获取当前光标的位置，以便使得菜单可以跟随光标  
	pContextMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point1.x, point1.y, AfxGetMainWnd());

	CDialogEx::OnRButtonDown(nFlags, point);
}

void CFaceIdentificationDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	CMenu menu;
	menu.LoadMenuW(IDR_MENU);
	CMenu   *pContextMenu = menu.GetSubMenu(0); //获取第一个弹出菜单，所以第一个菜单必须有子菜单 
	CPoint point1;//定义一个用于确定光标位置的位置  
	GetCursorPos(&point1);//获取当前光标的位置，以便使得菜单可以跟随光标  
	pContextMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point1.x, point1.y, AfxGetMainWnd());

	CDialogEx::OnRButtonDown(nFlags, point);
}

void CFaceIdentificationDlg::SetInitCameraOK() {
	InitCameraOK = 1;
}

void CFaceIdentificationDlg::OnBnClickedAddNewUser() {
	RegisterNewUser* reg;
	reg = new RegisterNewUser(this, Camera);
	reg->DoModal();

}

void CFaceIdentificationDlg::OnBnClickedStart()
{
	// TODO: 在此添加控件通知处理程序代码
	Mat Frame;
	while (InitCameraOK)
	{
		*Camera >> Frame;
		flip(Frame, Frame, 1);

		RECT rec;
		GetDlgItem(IDC_STATIC)->GetClientRect(&rec);
		double scal = (double)Frame.cols / (rec.right - rec.left);
		resize(Frame, Frame, Size((double)Frame.cols / scal, (double)Frame.rows / scal));
		GetDlgItem(IDC_STATIC)->SetWindowPos(NULL, 0, 0, Frame.cols, Frame.rows, SWP_NOZORDER | SWP_NOMOVE);


		Mat Blank(Frame.size(), CV_8UC3, Scalar(220, 220, 220));

		FI_RESULT Result = Demo.Identify(Frame);

		for (int i = 0; i < Result.NumberOfFace; ++i)
			rectangle(Frame, Rect(Result.FaceRect[i].left, Result.FaceRect[i].top, Result.FaceRect[i].right - Result.FaceRect[i].left, Result.FaceRect[i].bottom - Result.FaceRect[i].top), CV_RGB(255, 0, 0), 2);

		if (Result.IdentifyStatus == SUCCESS)
		{
			string cs = "鉴别成功\n姓名: " + Result.PersonInfo.PersonInfo.Name;
			putTextZH(Frame, cs.c_str(), Point(50, 50), Scalar(0, 0, 255), 30, "Arial");
		}
		else if (Result.IdentifyStatus == NO_INFO)
		{
			string cs = "无此人信息！\n";
			putTextZH(Frame, cs.c_str(), Point(50, 50), Scalar(0, 0, 255), 30, "Arial");
		}
		else if (Result.IdentifyStatus == NO_FACE)
		{
			string cs = "未检测到人脸！\n";
			putTextZH(Frame, cs.c_str(), Point(50, 50), Scalar(0, 0, 255), 30, "Arial");

		}
		else if (Result.IdentifyStatus == LIVING_DETECT_FAIL)
		{
			string cs = "正在进行活体检测...！\n";
			putTextZH(Frame, cs.c_str(), Point(50, 50), Scalar(0, 0, 255), 30, "Arial");

		}
		else if (Result.IdentifyStatus == QR_ACCEPT)
		{
			string cs = "二维码验证成功!\n" + Result.PersonInfo.PersonInfo.Name;
			putTextZH(Frame, cs.c_str(), Point(50, 50), Scalar(0, 0, 255), 30, "Arial");

		}
		imshow("view", Frame);
		waitKey(24);
	}
}

void CFaceIdentificationDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	InitCameraOK = 0;
	CDialogEx::OnClose();
}
