// RegisterNewUser.cpp: 实现文件
//

#include "stdafx.h"
#include "Face Identification.h"
#include "RegisterNewUser.h"
#include "afxdialogex.h"
#include <string>
using namespace std;
// RegisterNewUser 对话框

IMPLEMENT_DYNAMIC(RegisterNewUser, CDialogEx)

RegisterNewUser::RegisterNewUser(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_REG, pParent)
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
	int UserType = -1;
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
}


void RegisterNewUser::OnBnClickedRegCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	EndDialog(0);
}
