// RegisterNewUser.cpp: 实现文件
//

#include "stdafx.h"
#include "Face Identification.h"
#include "RegisterNewUser.h"
#include "afxdialogex.h"


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
END_MESSAGE_MAP()


// RegisterNewUser 消息处理程序
