// RegisterNewUser.cpp: 实现文件
//

#include "stdafx.h"
#include "Face Identification.h"
#include "RegisterNewUser.h"


using namespace std;
using namespace cv;
// RegisterNewUser 对话框

IMPLEMENT_DYNAMIC(RegisterNewUser, CDialogEx)

RegisterNewUser::RegisterNewUser(CWnd* pParent, VideoCapture* Camera)
	: CDialogEx(IDD_DIALOG_REG, pParent), parent(pParent), Camera(Camera)
{

}

RegisterNewUser::~RegisterNewUser()
{
}

void RegisterNewUser::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(RegisterNewUser, CDialogEx)
	ON_BN_CLICKED(IDC_RADIO_MALE, &RegisterNewUser::OnBnClickedRadioMale)
	ON_BN_CLICKED(IDC_REG_OK, &RegisterNewUser::OnBnClickedRegOk)
	ON_BN_CLICKED(IDC_REG_CANCEL, &RegisterNewUser::OnBnClickedRegCancel)
	ON_WM_CLOSE()
	ON_WM_PAINT()
END_MESSAGE_MAP()


// RegisterNewUser 消息处理程序


BOOL RegisterNewUser::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	SetWindowText(_T("注册新用户"));
	((CButton*)GetDlgItem(IDC_RADIO_MALE))->SetCheck(TRUE);
	SetDlgItemTextW(IDC_COMBO_USERTYPE, _T("普通用户"));
	SetDlgItemTextW(IDC_COMBO_ADDRESS1, _T("A区"));
	SetDlgItemTextW(IDC_COMBO_ADDRESS2, _T("1栋"));

	initCamera = 0;
	InfoReady = 0;
	if (initCamera == 0) {
		namedWindow("view1", WINDOW_AUTOSIZE);
		HWND hWnd = (HWND)cvGetWindowHandle("view1");
		HWND hParent = ::GetParent(hWnd);
		::SetParent(hWnd, GetDlgItem(IDC_STATIC_USERFACE)->m_hWnd);
		::ShowWindow(hParent, SW_HIDE);
		Demo.SetLivingDetection(true);
		initCamera = 1;
		Camera->set(CV_CAP_PROP_FRAME_WIDTH, 640);
		Camera->set(CV_CAP_PROP_FRAME_HEIGHT, 360);
	}
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

void RegisterNewUser::OnBnClickedRadioMale()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(true);
}

void RegisterNewUser::OnBnClickedRegOk()
{

	// TODO: 在此添加控件通知处理程序代码
	CString Name;
	int Age = -1;
	int Gender = -1;
	int UserType = 9;
	CString Address1;
	CString Address2;
	CString Phone;
	CString Remarks;
	CString tmp;
	int check = 1;

	GetDlgItem(IDC_EDIT_NAME)->GetWindowTextW(tmp);
	Name = tmp;

	GetDlgItem(IDC_EDIT_AGE)->GetWindowTextW(tmp);
	Age = _ttoi(tmp);

	check = ((CButton *)GetDlgItem(IDC_RADIO_MALE))->GetCheck();
	if (check == 1)
	{
		Gender = 0;
	}
	GetDlgItem(IDC_COMBO_USERTYPE)->GetWindowTextW(tmp);
	if (tmp == "普通用户")
	{
		UserType = 0;
	}
	else if (tmp == "管理员")
	{
		UserType = 1;
	}
	else if (tmp == "游客")
	{
		UserType = 2;
	}
	if (Name.IsEmpty() || Age == -1 || Gender == -1 || UserType == -1)
	{
		MessageBox(_T("请填写完整信息！"));
	}
	else
	{
		initCamera = 1;
		InfoReady = 1;
		while (InfoReady)
		{
			*Camera >> UserFace;

			RECT rec;
			GetDlgItem(IDC_STATIC_USERFACE)->GetClientRect(&rec);
			double scal = (double)UserFace.cols / (rec.right - rec.left);
			resize(UserFace, UserFace, Size((double)UserFace.cols / scal, (double)UserFace.rows / scal));
			GetDlgItem(IDC_STATIC_USERFACE)->SetWindowPos(NULL, 0, 0, UserFace.cols, UserFace.rows, SWP_NOZORDER | SWP_NOMOVE);

			PInfo info;
			info.Name = CStringA(Name);;
			info.PersonType = UserType;
			FI_RESULT Result = Demo.RegisterNewFace(UserFace, info);

			if (Result.IdentifyStatus == IMPROPER)
			{
				string cs = "请将面部置于屏幕中央区域\n";
				putTextZH(UserFace, cs.c_str(), Point(0, 0), Scalar(0, 0, 255), 30, "Arial");
			}
			else if (Result.IdentifyStatus == REGISTER_SUCCESS)
			{
				string cs = "Success!";
				putTextZH(UserFace, cs.c_str(), Point(0, 0), Scalar(0, 0, 255), 30, "Arial");
				initCamera = 0;
				InfoReady = 0;
				CDialog::OnOK();
				MessageBox(_T("OK！"));

			}
			else if (Result.IdentifyStatus == NO_FACE)
			{
				string cs = "未检测到人脸!";
				putTextZH(UserFace, cs.c_str(), Point(0, 0), Scalar(0, 0, 255), 30, "Arial");
			}
			imshow("view1", UserFace);
			waitKey(24);
		}
	}
}

void RegisterNewUser::OnBnClickedRegCancel()
{
	initCamera = 0;
	CDialog::OnOK();
}

void RegisterNewUser::OnInitCameraArea()
{
	while (initCamera)
	{
		*Camera >> UserFace;


		RECT rec;
		GetDlgItem(IDC_STATIC_USERFACE)->GetClientRect(&rec);
		double scal = (double)UserFace.cols / (rec.right - rec.left);
		resize(UserFace, UserFace, Size((double)UserFace.cols / scal, (double)UserFace.rows / scal));
		GetDlgItem(IDC_STATIC_USERFACE)->SetWindowPos(NULL, 0, 0, UserFace.cols, UserFace.rows, SWP_NOZORDER | SWP_NOMOVE);

		FI_RESULT Result = Demo.Identify(UserFace);

		for (int i = 0; i < Result.NumberOfFace; ++i)
			rectangle(UserFace, Rect(Result.FaceRect[i].left, Result.FaceRect[i].top, Result.FaceRect[i].right - Result.FaceRect[i].left, Result.FaceRect[i].bottom - Result.FaceRect[i].top), CV_RGB(255, 0, 0), 2);
		if (Result.IdentifyStatus == SUCCESS)
		{
			string cs = "用户 " + Result.PersonInfo.PersonInfo.Name + "已注册";
			putTextZH(UserFace, cs.c_str(), Point(0, 0), Scalar(0, 0, 255), 30, "Arial");

		}
		else if (Result.IdentifyStatus == NO_FACE)
		{
			string cs = "未检测到人脸！\n";
			putTextZH(UserFace, cs.c_str(), Point(50, 50), Scalar(0, 0, 255), 30, "Arial");

		}
		else if (Result.IdentifyStatus == LIVING_DETECT_FAIL)
		{
			string cs = "正在进行活体检测...！\n";
			putTextZH(UserFace, cs.c_str(), Point(50, 50), Scalar(0, 0, 255), 30, "Arial");

		}
		else if (Result.IdentifyStatus == QR_ACCEPT)
		{
			string cs = "二维码验证成功!\n" + Result.PersonInfo.PersonInfo.Name;
			putTextZH(UserFace, cs.c_str(), Point(50, 50), Scalar(0, 0, 255), 30, "Arial");

		}
		imshow("view1", UserFace);
		waitKey(24);
	}
}

void RegisterNewUser::OnClose()
{
	initCamera = 0;
	CDialogEx::OnClose();
}

void RegisterNewUser::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: 在此处添加消息处理程序代码
					   // 不为绘图消息调用 CDialogEx::OnPaint()
	OnInitCameraArea();
}
