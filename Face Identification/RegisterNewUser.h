#pragma once


// RegisterNewUser 对话框
#include "afxdialogex.h"
#include <string>
#include "Face IdentificationDlg.h"
#include<opencv2\opencv.hpp>
#include <opencv\cv.h>
#include "putText.h"
#include "FaceIdentification.h"

class RegisterNewUser : public CDialogEx
{
	DECLARE_DYNAMIC(RegisterNewUser)

public:
	RegisterNewUser(CWnd* pParent,VideoCapture* Camera);   // 标准构造函数
	virtual ~RegisterNewUser();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_REG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CWnd * parent;
	FaceIdentification Demo;
	VideoCapture *Camera;
	int initCamera;
	int InfoReady;
	Mat UserFace;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedRadioMale();
	afx_msg void OnBnClickedRegOk();
	afx_msg void OnBnClickedRegCancel();
	afx_msg void OnInitCameraArea();
	void OnClose();
	afx_msg void OnPaint();
};
