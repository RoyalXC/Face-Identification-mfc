#include"stdafx.h"
#include "FaceIdentification.h"
#include <zbar.h>

//构造函数
FaceIdentification::FaceIdentification()
{
	Engine_FT = nullptr;
	Engine_FD = nullptr;
	Engine_WorkMemory_FT = (MByte *)malloc(WORKBUF_SIZE);
	Engine_WorkMemory_FD = (MByte *)malloc(WORKBUF_SIZE);
	LivingDetectMemory = (unsigned char *)malloc(KEY_POINT_DETECT_BUFFER_SIZE);
	InputImage.u32PixelArrayFormat = IMAGE_TYPE;
	InputImage.ppu8Plane[0] = nullptr;

	FaceSearch = new FaceComparison();
	DatabaseOperate = new FaceDatabase();

	if (AFT_FSDK_InitialFaceEngine((char*)APP_ID, (char*)FT_SDK_KEY, Engine_WorkMemory_FT, WORKBUF_SIZE, &Engine_FT, AFT_FSDK_OPF_0_HIGHER_EXT, FaceScale, MaxFaceNumber) != MOK)
		IsEngineStart = false;
	else if (AFD_FSDK_InitialFaceEngine((char*)APP_ID, (char*)FD_SDK_KEY, Engine_WorkMemory_FD, WORKBUF_SIZE, &Engine_FD, AFD_FSDK_OPF_0_HIGHER_EXT, FaceScale, 1) != MOK)
		IsEngineStart = false;
	else
		IsEngineStart = true;
}

//析构函数
FaceIdentification::~FaceIdentification()
{
	AFT_FSDK_UninitialFaceEngine(Engine_FT);
	free(Engine_WorkMemory_FT);
	AFD_FSDK_UninitialFaceEngine(Engine_FD);
	free(Engine_WorkMemory_FD);
	free(LivingDetectMemory);
	//free(InputImage.ppu8Plane[0]);
	delete FaceSearch;
	delete DatabaseOperate;
	if (FaceRect != NULL)
		delete[] FaceRect;
}

//人脸鉴别 主接口
FI_RESULT FaceIdentification::Identify(Mat &Image)
{
	if (!IsEngineStart)
	{
		IdentificationResult.IdentifyStatus = ENGINE_FAILED;
		return IdentificationResult;
	}

	FrameCount = (++FrameCount) % 1000000;

	MatToASVLOFFSCREEN(Image, InputImage);
	LPAFT_FSDK_FACERES FT_IdentificationResult = nullptr;
	
	//人脸跟踪
	if (AFT_FSDK_FaceFeatureDetect(Engine_FT, &InputImage, &FT_IdentificationResult) != MOK)
	{
		IdentificationResult.IdentifyStatus = TRACKING_FAILED;
		return IdentificationResult;
	}

	IdentificationResult.NumberOfFace = FT_IdentificationResult->nFace;

	if (IdentificationResult.NumberOfFace != 0)
	{
		if (FaceRect != NULL)
			delete[] FaceRect;
		FaceRect = new Face_RECT[FT_IdentificationResult->nFace];

		int i;
		for (i = 0; i < FT_IdentificationResult->nFace; ++i)
		{
			FaceRect[i].left = FT_IdentificationResult->rcFace[i].left;
			FaceRect[i].right = FT_IdentificationResult->rcFace[i].right;
			FaceRect[i].top = FT_IdentificationResult->rcFace[i].top;
			FaceRect[i].bottom = FT_IdentificationResult->rcFace[i].bottom;
		}
		IdentificationResult.FaceRect = FaceRect;
	}
	else
	{
		if(IdentificationResult.IdentifyStatus != QR_ACCEPT)
			IdentificationResult.IdentifyStatus = NO_FACE;
	}

	if (IdentificationResult.IdentifyStatus == QR_ACCEPT && (FrameCount - PreviousFrameCount) > 5)
	{
		IdentificationResult.IdentifyStatus = NO_FACE;
		//return IdentificationResult;
	}

	if ((PreviousFrameCount != -1) && (FrameCount - PreviousFrameCount) <= 5)
	{
		if (IdentificationResult.IdentifyStatus == SUCCESS)
		{
			PreviousFrameCount = FrameCount;
			return IdentificationResult;
		}
		else if (IdentificationResult.IdentifyStatus == NO_INFO)
		{
			return IdentificationResult;
		}
		else if (IdentificationResult.IdentifyStatus == QR_ACCEPT)
		{
			return IdentificationResult;
		}
	}

	string QRResult;
	thread QRDetection([&QRResult, &Image, this] {
		QRResult = QRScanner(Image);
	});
	
	//画面中仅一张脸时
	if (IdentificationResult.NumberOfFace == 1)
	{
		//存在跟踪丢失 清零检测结果
		if (PreviousFrameCount == -1 || (FrameCount - PreviousFrameCount) > 2)
		{
			InitializeLivingDetection();
		}

		if (!LivingDetectionInfo.LivingDetectionAccept && LivingDetectionInfo.IsEnable)
		{
			IdentificationResult.IdentifyStatus = LIVING_DETECT_FAIL;
			Mat GrayFrame;
			cvtColor(Image, GrayFrame, CV_BGR2GRAY);
			int *FaceOutlineIdentificationResult = facedetect_multiview_reinforce(LivingDetectMemory, (unsigned char*)(GrayFrame.ptr(0)), GrayFrame.cols, GrayFrame.rows, (int)GrayFrame.step, 1.2f, 2, 48, 0, 1);

			if (*FaceOutlineIdentificationResult == 1)
			{
				++LivingDetectionInfo.FrameCount;

				short * p = ((short*)(FaceOutlineIdentificationResult + 1));
				int i;

				Face_RECT LeftEyeRect, RightEyeRect;
				LeftEyeRect.left = INT_MAX;
				LeftEyeRect.right = 0;
				LeftEyeRect.top = INT_MAX;
				LeftEyeRect.bottom = 0;
				RightEyeRect.left = INT_MAX;
				RightEyeRect.right = 0;
				RightEyeRect.top = INT_MAX;
				RightEyeRect.bottom = 0;

				int x, y;
				//左眼轮廓点
				for (i = 36; i <= 41; ++i)
				{
					x = (int)p[6 + 2 * i];
					y = (int)p[6 + 2 * i + 1];
					if (x < LeftEyeRect.left)
						LeftEyeRect.left = x;
					if (x > LeftEyeRect.right)
						LeftEyeRect.right = x;
					if (y < LeftEyeRect.top)
						LeftEyeRect.top = y;
					if (y > LeftEyeRect.bottom)
						LeftEyeRect.bottom = y;

				}

				//右眼轮廓点
				for (i = 42; i <= 47; ++i)
				{
					x = (int)p[6 + 2 * i];
					y = (int)p[6 + 2 * i + 1];
					if (x < RightEyeRect.left)
						RightEyeRect.left = x;
					if (x > RightEyeRect.right)
						RightEyeRect.right = x;
					if (y < RightEyeRect.top)
						RightEyeRect.top = y;
					if (y > RightEyeRect.bottom)
						RightEyeRect.bottom = y;
				}

				//提取眼睛区域
				Mat LeftEye, RightEye;
				Image(Rect(LeftEyeRect.left, LeftEyeRect.top, LeftEyeRect.right - LeftEyeRect.left, LeftEyeRect.bottom - LeftEyeRect.top)).copyTo(LeftEye);
				Image(Rect(RightEyeRect.left, RightEyeRect.top, RightEyeRect.right - RightEyeRect.left, RightEyeRect.bottom - RightEyeRect.top)).copyTo(RightEye);

				cvtColor(LeftEye, LeftEye, CV_BGR2GRAY);
				cvtColor(RightEye, RightEye, CV_BGR2GRAY);

				threshold(LeftEye, LeftEye, 0, 255, CV_THRESH_OTSU);
				threshold(RightEye, RightEye, 0, 255, CV_THRESH_OTSU);

				//眼睛区域膨胀处理
				Mat DilateKernel = getStructuringElement(MORPH_RECT, Size(1, 1));
				dilate(LeftEye, LeftEye, DilateKernel, Point(-1, -1), 2);
				dilate(RightEye, RightEye, DilateKernel, Point(-1, -1), 2);

				//反相
				bitwise_not(LeftEye, LeftEye);
				bitwise_not(RightEye, RightEye);

				//搜索最大连通域
				vector<vector<Point>> Contours;
				findContours(LeftEye, Contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
				double MaxArea = 0;
				vector<cv::Point> MaxContour;
				for (size_t i = 0; i < Contours.size(); i++)
				{
					double area = contourArea(Contours[i]);
					if (area > MaxArea)
					{
						MaxArea = area;
						MaxContour = Contours[i];
					}
				}
				Rect MaxRect = boundingRect(MaxContour);
				//rectangle(LeftEye, MaxRect, cv::Scalar(255));

				//判断是否眨眼
				LivingDetectionInfo.LeftEyeRatio += (double)MaxRect.width / MaxRect.height;
				printf("LeftEye: Width = %d\tHeight = %d\tPro = %.3lf\n", MaxRect.width, MaxRect.height, (double)MaxRect.width / MaxRect.height);

				if ((double)MaxRect.width / MaxRect.height > (double)(LivingDetectionInfo.LeftEyeRatio / LivingDetectionInfo.FrameCount) * 1.5)
				{
					LivingDetectionInfo.LivingDetectionAccept = true;
					printf("*****************************************************\n*****************************************************\n            L E F T W I N K \n*****************************************************\n*****************************************************\n");
				}

				//搜索最大连通域
				Contours.clear();
				findContours(RightEye, Contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
				MaxArea = 0;
				for (size_t i = 0; i < Contours.size(); i++)
				{
					double area = contourArea(Contours[i]);
					if (area > MaxArea)
					{
						MaxArea = area;
						MaxContour = Contours[i];
					}
				}
				MaxRect = boundingRect(MaxContour);
				//rectangle(RightEye, MaxRect, cv::Scalar(255));

				//判断是否眨眼
				LivingDetectionInfo.RightEyeRatio += (double)MaxRect.width / MaxRect.height;
				printf("Right: Width = %d\tHeight = %d\tPro = %.3lf\n\n", MaxRect.width, MaxRect.height, (double)MaxRect.width / MaxRect.height);
				if ((double)MaxRect.width / MaxRect.height > (double)(LivingDetectionInfo.RightEyeRatio / LivingDetectionInfo.FrameCount) * 1.5)
				{
					LivingDetectionInfo.LivingDetectionAccept = true;
					printf("*****************************************************\n*****************************************************\n            R I G H T W I N K \n*****************************************************\n*****************************************************\n");
				}

				//imshow("Left Eye 3", LeftEye);
				//imshow("Right Eye 3", RightEye);
			}

		}
		else
		{
			printf("-----------------------Accept-----------------------\n");

			Face_RECT AccurateFaceRect = FaceDetection(Image);
			if (AccurateFaceRect.left == -1)
			{
				IdentificationResult.IdentifyStatus = INSTABLE;
				return IdentificationResult;
			}
			Mat FaceImage;
			Image(Rect(AccurateFaceRect.left, AccurateFaceRect.top, AccurateFaceRect.right - AccurateFaceRect.left, AccurateFaceRect.bottom - AccurateFaceRect.top)).copyTo(FaceImage);
			RetrievalResult CompareIdentificationResult;
			FaceSearch->Retrieve(FaceImage, CompareIdentificationResult);
			if (CompareIdentificationResult.IsSuccess)
			{
				IdentificationResult.IdentifyStatus = SUCCESS;
				IdentificationResult.PersonInfo = CompareIdentificationResult;
			}
			else
			{
				IdentificationResult.IdentifyStatus = NO_INFO;
			}
			//imshow("Debug-Face", FaceImage);
		}

		PreviousFrameCount = FrameCount;
	}

	QRDetection.join();

	if (QRResult.length() != 0)
	{
		if (QRResult == "QRTest")
		{
			IdentificationResult.IdentifyStatus = QR_ACCEPT;
			IdentificationResult.PersonInfo.PersonInfo.Name = QRResult;
			PreviousFrameCount = FrameCount;
			return IdentificationResult;
		}
	}
	return IdentificationResult;
}

//人脸定位（给出更精确人脸区域用于鉴别）
Face_RECT FaceIdentification::FaceDetection(Mat &Image)
{
	Face_RECT Result;
	Result.left = -1;

	LPAFD_FSDK_FACERES	FD_Result = nullptr;
	if (AFD_FSDK_StillImageFaceDetection(Engine_FD, &InputImage, &FD_Result) != MOK)
	{
		return Result;
	}

	if (FD_Result->nFace == 1)
	{
		Result.left = FD_Result->rcFace[0].left;
		Result.right = FD_Result->rcFace[0].right;
		Result.top = FD_Result->rcFace[0].top;
		Result.bottom = FD_Result->rcFace[0].bottom;
	}

	return Result;
}

//初始化活体检测数据
void FaceIdentification::InitializeLivingDetection()
{
	LivingDetectionInfo.LivingDetectionAccept = false;
	LivingDetectionInfo.FrameCount = 0;
	LivingDetectionInfo.LeftEyeRatio = 0;
	LivingDetectionInfo.RightEyeRatio = 0;
}

//设置是否进行活体检测
void FaceIdentification::SetLivingDetection(bool IsEnable)
{
	LivingDetectionInfo.IsEnable = IsEnable;
}

//录入新用户
FI_RESULT FaceIdentification::RegisterNewFace(Mat &Image, PInfo &Info)
{
	FI_RESULT Result;

	if (!IsEngineStart)
	{
		Result.IdentifyStatus = ENGINE_FAILED;
		return Result;
	}

	FrameCount = (++FrameCount) % 1000000;

	MatToASVLOFFSCREEN(Image, InputImage);
	LPAFT_FSDK_FACERES FT_Result = nullptr;

	//人脸跟踪
	if (AFT_FSDK_FaceFeatureDetect(Engine_FT, &InputImage, &FT_Result) != MOK)
	{
		Result.IdentifyStatus = TRACKING_FAILED;
		return Result;
	}

	Result.NumberOfFace = FT_Result->nFace;

	if (Result.NumberOfFace != 0)
	{
		if (FaceRect != NULL)
			delete[] FaceRect;
		FaceRect = new Face_RECT[FT_Result->nFace];

		int i;
		for (i = 0; i < FT_Result->nFace; ++i)
		{
			FaceRect[i].left = FT_Result->rcFace[i].left;
			FaceRect[i].right = FT_Result->rcFace[i].right;
			FaceRect[i].top = FT_Result->rcFace[i].top;
			FaceRect[i].bottom = FT_Result->rcFace[i].bottom;
		}
		Result.FaceRect = FaceRect;
	}
	else
	{
		Result.IdentifyStatus = NO_FACE;
	}

	//画面中仅一张脸时
	if (Result.NumberOfFace == 1)
	{
		Point FaceCenter;
		FaceCenter.x = FT_Result->rcFace[0].left + (FT_Result->rcFace[0].right - FT_Result->rcFace[0].left) * 0.5;
		FaceCenter.y = FT_Result->rcFace[0].top + (FT_Result->rcFace[0].bottom - FT_Result->rcFace[0].top) * 0.5;
		if (FaceCenter.x < Image.cols * 0.3 || FaceCenter.x > Image.cols * 0.7 || FaceCenter.y < Image.rows * 0.3 || FaceCenter.y > Image.rows * 0.7)
		{
			//printf("%d %d\n", FaceCenter.x, FaceCenter.y);
			Result.IdentifyStatus = IMPROPER;
			return Result;
		}

		Face_RECT AccurateFaceRect = FaceDetection(Image);
		if (AccurateFaceRect.left == -1)
			return Result;
		Mat FaceImage;
		Image(Rect(AccurateFaceRect.left, AccurateFaceRect.top, AccurateFaceRect.right - AccurateFaceRect.left, AccurateFaceRect.bottom - AccurateFaceRect.top)).copyTo(FaceImage);

		FaceModel Model;
		MatToASVLOFFSCREEN(FaceImage, InputImage);

		thread Thread_GetFaceModel([&] {
			FaceSearch->GetFaceModel(InputImage, Model);
		});

		MInt32 Age, Gender;
		thread Thread_GenderEstimation([&] {
			Gender = FaceSearch->GenderEstimation(InputImage);
		});
		thread Thread_AgeEstimation([&] {
			Age = FaceSearch->AgeEstimation(InputImage);
		});

		Thread_GenderEstimation.join();
		Thread_AgeEstimation.join();
		Thread_GetFaceModel.join();

		Info.Age = Age;
		Info.Gender = Gender;

		DatabaseOperate->AddNewUser(Info, Model, FaceImage);

		Result.IdentifyStatus = REGISTER_SUCCESS;
	}

	return Result;
}

string FaceIdentification::QRScanner(Mat & Image)
{
	using namespace zbar;
	string Result;

	ImageScanner scanner;
	scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);
	Mat imageGray;
	cvtColor(Image, imageGray, CV_RGB2GRAY);
	int width = imageGray.cols;
	int height = imageGray.rows;
	uchar *raw = (uchar *)imageGray.data;
	zbar::Image imageZbar(width, height, "Y800", raw, width * height);
	scanner.scan(imageZbar); //扫描条码    
	Image::SymbolIterator symbol = imageZbar.symbol_begin();
	if (imageZbar.symbol_begin() == imageZbar.symbol_end())
	{
		Result = "";
		imageZbar.set_data(NULL, 0);
		return Result;
	}
	else
	{
		Result = symbol->get_data();
		imageZbar.set_data(NULL, 0);
		return Result;
	}
}
