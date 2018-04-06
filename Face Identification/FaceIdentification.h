#pragma once

#include "faceidentification_define.h"
#include "FaceComparison.h"
#include "FaceDatabase.h"

class FaceIdentification
{
public:
	FaceIdentification();
	~FaceIdentification();

	FI_RESULT Identify(Mat &Image);
	Face_RECT FaceDetection(Mat &Image);
	void InitializeLivingDetection();
	void SetLivingDetection(bool IsEnable);
	FI_RESULT RegisterNewFace(Mat &Image, PInfo &Info);
	string QRScanner(Mat &Image);
	
private:
	const char *APP_ID = "H38Mv4gHhdR1dwKq8r3sjZSDKRpna4JWVRY7un6uhQFC";
	const char *FT_SDK_KEY = "6VMPpuSm7EMfQ8oeTFHWb7zzck11GcdVXxyNNmoYjCbU";
	const char *FD_SDK_KEY = "6VMPpuSm7EMfQ8oeTFHWb817n9G9yJgDCX6R5DnmnJ4J";

	bool IsEngineStart = false;									//引擎是否启动
	MByte *Engine_WorkMemory_FT, *Engine_WorkMemory_FD;			//引擎工作缓存空间
	unsigned char *LivingDetectMemory;							//活体检测引擎缓存空间
	MHandle Engine_FT, Engine_FD;								//引擎句柄
	MInt32 FaceScale = 16;										//最小人脸尺寸，有效范围[2,16]，建议16
	MInt32 MaxFaceNumber = 1;									//最多检测出人脸数量，有效范围[1,20]
	ASVLOFFSCREEN InputImage = { 0 };							//输入图像
	Face_RECT *FaceRect = nullptr;								//检测出的人脸区域
	FaceDatabase *DatabaseOperate = nullptr;
	FaceComparison *FaceSearch;									//面部对比类指针
	int FrameCount = 0;											//帧数记录
	int PreviousFrameCount = -1;								//标记是否为同一个人
	struct
	{
		bool IsEnable = true;
		bool LivingDetectionAccept = false;
		double LeftEyeRatio = 0, RightEyeRatio = 0;
		int FrameCount = 0;
	}LivingDetectionInfo;										//活体检测
	FI_RESULT IdentificationResult;								//检测结果
};

