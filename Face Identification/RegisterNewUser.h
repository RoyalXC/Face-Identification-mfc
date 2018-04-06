#pragma once


// RegisterNewUser 对话框

class RegisterNewUser : public CDialogEx
{
	DECLARE_DYNAMIC(RegisterNewUser)

public:
	RegisterNewUser(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~RegisterNewUser();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_REG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
//	int radio_gender;
	afx_msg void OnBnClickedRadioMale();
	afx_msg void OnBnClickedRegOk();
	afx_msg void OnBnClickedRegCancel();
};
