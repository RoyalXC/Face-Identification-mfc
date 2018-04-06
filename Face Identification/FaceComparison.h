#pragma once
#include "faceidentification_define.h"
#include "FaceDatabase.h"

class FaceComparison
{
public:
	FaceComparison();
	~FaceComparison();

	void Retrieve(Mat &FaceImage, RetrievalResult &Result);
	void GetFaceModel(ASVLOFFSCREEN &FaceAreaImage, FaceModel &Model);
	void UpdateUserInfo(int DatabaseID, Mat &FaceImage, FaceModel &InputFaceModel);
	void FaceFusion(Mat &FusionImage, Mat &NewFaceImage);
	MInt32 GenderEstimation(ASVLOFFSCREEN &FaceAreaImage);
	MInt32 AgeEstimation(ASVLOFFSCREEN &FaceAreaImage);
private:
	void FaceMatching(FaceModel &FaceA, FaceModel &FaceB, MFloat &Score);
	
private:
	const char *APP_ID = "H38Mv4gHhdR1dwKq8r3sjZSDKRpna4JWVRY7un6uhQFC";
	const char *FR_SDK_KEY = "6VMPpuSm7EMfQ8oeTFHWb81EwYXNguit6mLK7ztBQSLh";
	const char *GE_SDK_KEY = "6VMPpuSm7EMfQ8oeTFHWb81yux6R2v8RsUUHcEwmKRox";
	const char *AE_SDK_KEY = "6VMPpuSm7EMfQ8oeTFHWb81rkYqCUxFwmYRqcUai6pe3";

	double FusionAlpha;																		//�����ں�ʱ��������ռȨ��
	bool IsEngineStart = false;																//�����Ƿ�����
	MByte *Engine_WrokMemory_FR, *Engine_WrokMemory_GE, *Engine_WrokMemory_AE;				//���湤������ռ�
	unsigned char *FaceKeyPointEngineMemory[2];												//���������滺��ռ�
	MHandle Engine_FR, Engine_GE, Engine_AE;												//������
	FaceDatabase *DatabaseOperate = nullptr;
	ASVLOFFSCREEN InputImage = { 0 };														//����ͼ��
	double SameFaceThreshold_Direct, SameFaceThreshold_Limit, SameFaceThreshold_Filter;		//�沿�Ա���ֵ
};

