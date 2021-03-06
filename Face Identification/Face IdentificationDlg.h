
// Face IdentificationDlg.h: 头文件
//

#pragma once
#include<opencv2\opencv.hpp>
#include <opencv\cv.h>
#include "putText.h"
#include"RegisterNewUser.h"
#include "FaceIdentification.h"
#include "afxdialogex.h"

// CFaceIdentificationDlg 对话框
class CFaceIdentificationDlg : public CDialogEx
{
// 构造
public:
	CFaceIdentificationDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FACEIDENTIFICATION_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
public:
	HICON m_hIcon;
	int InitCameraOK;
	FaceIdentification Demo;
	VideoCapture *Camera;
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg
		void SetInitCameraOK();
	void OnBnClickedAddNewUser();
	afx_msg void OnBnClickedStart();
	afx_msg void OnClose();
};
