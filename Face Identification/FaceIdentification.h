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

	bool IsEngineStart = false;									//�����Ƿ�����
	MByte *Engine_WorkMemory_FT, *Engine_WorkMemory_FD;			//���湤������ռ�
	unsigned char *LivingDetectMemory;							//���������滺��ռ�
	MHandle Engine_FT, Engine_FD;								//������
	MInt32 FaceScale = 16;										//��С�����ߴ磬��Ч��Χ[2,16]������16
	MInt32 MaxFaceNumber = 1;									//������������������Ч��Χ[1,20]
	ASVLOFFSCREEN InputImage = { 0 };							//����ͼ��
	Face_RECT *FaceRect = nullptr;								//��������������
	FaceDatabase *DatabaseOperate = nullptr;
	FaceComparison *FaceSearch;									//�沿�Ա���ָ��
	int FrameCount = 0;											//֡����¼
	int PreviousFrameCount = -1;								//����Ƿ�Ϊͬһ����
	struct
	{
		bool IsEnable = true;
		bool LivingDetectionAccept = false;
		double LeftEyeRatio = 0, RightEyeRatio = 0;
		int FrameCount = 0;
	}LivingDetectionInfo;										//������
	FI_RESULT IdentificationResult;								//�����
};

