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

	double FusionAlpha;																		//人脸融合时新数据所占权重
	bool IsEngineStart = false;																//引擎是否启动
	MByte *Engine_WrokMemory_FR, *Engine_WrokMemory_GE, *Engine_WrokMemory_AE;				//引擎工作缓存空间
	unsigned char *FaceKeyPointEngineMemory[2];												//活体检测引擎缓存空间
	MHandle Engine_FR, Engine_GE, Engine_AE;												//引擎句柄
	FaceDatabase *DatabaseOperate = nullptr;
	ASVLOFFSCREEN InputImage = { 0 };														//输入图像
	double SameFaceThreshold_Direct, SameFaceThreshold_Limit, SameFaceThreshold_Filter;		//面部对比阈值
};

