#include"stdafx.h"
#include "FaceComparison.h"


FaceComparison::FaceComparison()
{
	Engine_FR = nullptr;
	Engine_AE = nullptr;
	Engine_GE = nullptr;
	FusionAlpha = 0.5;
	Engine_WrokMemory_FR = (MByte *)malloc(WORKBUF_SIZE);
	Engine_WrokMemory_GE = (MByte *)malloc(WORKBUF_SIZE);
	Engine_WrokMemory_AE = (MByte *)malloc(WORKBUF_SIZE);
	FaceKeyPointEngineMemory[0] = (unsigned char *)malloc(KEY_POINT_DETECT_BUFFER_SIZE);
	FaceKeyPointEngineMemory[1] = (unsigned char *)malloc(KEY_POINT_DETECT_BUFFER_SIZE);

	InputImage.u32PixelArrayFormat = IMAGE_TYPE;
	InputImage.ppu8Plane[0] = nullptr;

	SameFaceThreshold_Direct = 0.2;
	SameFaceThreshold_Limit = 0.5;
	SameFaceThreshold_Filter = 0.6;

	DatabaseOperate = new FaceDatabase();

	if (AFR_FSDK_InitialEngine((char*)APP_ID, (char*)FR_SDK_KEY, Engine_WrokMemory_FR, WORKBUF_SIZE, &Engine_FR) != MOK)
		IsEngineStart = false;
	else if (ASGE_FSDK_InitGenderEngine((char*)APP_ID, (char*)GE_SDK_KEY, Engine_WrokMemory_GE, WORKBUF_SIZE, &Engine_GE) != MOK)
		IsEngineStart = false;
	else if (ASAE_FSDK_InitAgeEngine((char*)APP_ID, (char*)AE_SDK_KEY, Engine_WrokMemory_AE, WORKBUF_SIZE, &Engine_AE) != MOK)
		IsEngineStart = false;
	else
		IsEngineStart = true;
}


FaceComparison::~FaceComparison()
{
	AFR_FSDK_UninitialEngine(Engine_FR);
	free(Engine_WrokMemory_FR);
	ASGE_FSDK_UninitGenderEngine(Engine_GE);
	free(Engine_WrokMemory_GE);
	ASAE_FSDK_UninitAgeEngine(Engine_AE);
	free(Engine_WrokMemory_AE);
	free(FaceKeyPointEngineMemory[0]);
	free(FaceKeyPointEngineMemory[1]);

	//	if(InputImage.ppu8Plane[0] != nullptr)
	//		free(InputImage.ppu8Plane[0]);
}

void FaceComparison::Retrieve(Mat &FaceImage, RetrievalResult &Result)
{
	FaceModel InputFaceModel, ContrastFaceModel;

	MatToASVLOFFSCREEN(FaceImage, InputImage);

	thread Thread_GetFaceModel_Input([&] {
		GetFaceModel(InputImage, InputFaceModel);
	});

	MInt32 Gender, Age;

	thread Thread_GenderEstimation([&] {
		Gender = GenderEstimation(InputImage);
		printf("***Debug*** Gender = %d\n", Gender);

	});
	thread Thread_AgeEstimation([&] {
		Age = AgeEstimation(InputImage);
		printf("***Debug*** Age = %d\n", Age);
	});


	Thread_GenderEstimation.join();
	Thread_AgeEstimation.join();

	DataIterator FaceIterator;
	DatabaseOperate->SerachFace(Gender, Age, FaceIterator);

	struct CandidateFace
	{
		int FaceID;
		int Score;
	};
	vector<CandidateFace> Candidate;

	Thread_GetFaceModel_Input.join();

	while (FaceIterator.Next() != nullptr)
	{
		PersonalData *Person = FaceIterator.Pointer;

		MFloat Score;
		FaceMatching(InputFaceModel, Person->FusionModel, Score);

		if (Score >= SameFaceThreshold_Direct)
		{
			Result.PersonInfo = Person->Info;
			Result.IsSuccess = true;
			Result.SimilarScore = Score;
			UpdateUserInfo(Person->DatabaseID, FaceImage, InputFaceModel);
			return;
			//break;
		}
		else if (Score >= SameFaceThreshold_Limit)
		{
			CandidateFace Temp;
			Temp.FaceID = Person->DatabaseID;
			Temp.Score = Score;
			Candidate.push_back(Temp);
		}
	}

	if (Candidate.size() != 0)
	{
		sort(Candidate.begin(), Candidate.end(), [](CandidateFace &a, CandidateFace &b) { return a.Score > b.Score; });
		for (int i = 0; i < Candidate.size(); ++i)
		{
			FaceModelIterator DetailFaceModelIterator;
			DatabaseOperate->SearchFaceModel(Candidate[i].FaceID, DetailFaceModelIterator);
			while (DetailFaceModelIterator.Next() != nullptr)
			{
				MFloat Score;
				FaceMatching(InputFaceModel, DetailFaceModelIterator.Pointer->FusionModel, Score);

				if (Score >= SameFaceThreshold_Filter)
				{
					Result.PersonInfo = DetailFaceModelIterator.Pointer->Info;
					Result.IsSuccess = true;
					Result.SimilarScore = Score;
					UpdateUserInfo(DetailFaceModelIterator.Pointer->DatabaseID, FaceImage, InputFaceModel);
					return;
				}
			}
		}
	}

	Result.IsSuccess = false;
	Result.SimilarScore = 0;
	return;
}

//获取面部特征信息（传入图片为面部区域）
void FaceComparison::GetFaceModel(ASVLOFFSCREEN &FaceAreaImage, FaceModel &Model)
{
	AFR_FSDK_FACEINPUT FaceInfo;
	FaceInfo.rcFace.left = 0;
	FaceInfo.rcFace.top = 0;
	FaceInfo.rcFace.right = FaceAreaImage.i32Width - 1;
	FaceInfo.rcFace.bottom = FaceAreaImage.i32Height - 1;
	FaceInfo.lOrient = AFR_FSDK_FOC_0;

	AFR_FSDK_FACEMODEL TempModel{ 0 };
	AFR_FSDK_ExtractFRFeature(Engine_FR, &FaceAreaImage, &FaceInfo, &TempModel);

	Model.Model.lFeatureSize = TempModel.lFeatureSize;
	Model.Model.pbFeature = (MByte*)malloc(TempModel.lFeatureSize);
	memcpy(Model.Model.pbFeature, TempModel.pbFeature, TempModel.lFeatureSize);
}

//面部特征匹配
void FaceComparison::FaceMatching(FaceModel &FaceA, FaceModel &FaceB, MFloat &Score)
{
	AFR_FSDK_FacePairMatching(Engine_FR, &FaceA.Model, &FaceB.Model, &Score);
}

//性别检测
MInt32 FaceComparison::GenderEstimation(ASVLOFFSCREEN &FaceAreaImage)
{
	ASGE_FSDK_GENDERFACEINPUT GenderFaceInfo;
	GenderFaceInfo.pFaceOrientArray = new MInt32[1];
	GenderFaceInfo.pFaceRectArray = new MRECT[1];
	GenderFaceInfo.lFaceNumber = 1;
	GenderFaceInfo.pFaceOrientArray[0] = ASGE_FSDK_FOC_Gender_0;
	GenderFaceInfo.pFaceRectArray[0].left = 0;
	GenderFaceInfo.pFaceRectArray[0].top = 0;
	GenderFaceInfo.pFaceRectArray[0].right = FaceAreaImage.i32Width - 1;
	GenderFaceInfo.pFaceRectArray[0].bottom = FaceAreaImage.i32Height - 1;

	ASGE_FSDK_GENDERRESULT Result;
	ASGE_FSDK_GenderEstimation_StaticImage(Engine_GE, &FaceAreaImage, &GenderFaceInfo, &Result);
	delete[] GenderFaceInfo.pFaceOrientArray;
	delete[] GenderFaceInfo.pFaceRectArray;

	MInt32 GenderResult = Result.pGenderResultArray[0];

	return GenderResult;
}

//年龄评估
MInt32 FaceComparison::AgeEstimation(ASVLOFFSCREEN &FaceAreaImage)
{
	ASAE_FSDK_AGEFACEINPUT AgeFaceInfo;
	AgeFaceInfo.pFaceOrientArray = new MInt32[1];
	AgeFaceInfo.pFaceRectArray = new MRECT[1];
	AgeFaceInfo.lFaceNumber = 1;
	AgeFaceInfo.pFaceOrientArray[0] = ASGE_FSDK_FOC_Gender_0;
	AgeFaceInfo.pFaceRectArray[0].left = 0;
	AgeFaceInfo.pFaceRectArray[0].top = 0;
	AgeFaceInfo.pFaceRectArray[0].right = FaceAreaImage.i32Width - 1;
	AgeFaceInfo.pFaceRectArray[0].bottom = FaceAreaImage.i32Height - 1;

	ASAE_FSDK_AGERESULT Result;
	ASAE_FSDK_AgeEstimation_StaticImage(Engine_AE, &FaceAreaImage, &AgeFaceInfo, &Result);
	delete[] AgeFaceInfo.pFaceOrientArray;
	delete[] AgeFaceInfo.pFaceRectArray;

	MInt32 AgeResult = Result.pAgeResultArray[0];

	return AgeResult;
}

//更新人脸学习数据
void FaceComparison::UpdateUserInfo(int DatabaseID, Mat &FaceImage, FaceModel &InputFaceModel)
{
	Mat FusionImage;
	DatabaseOperate->GetUserFusionImageByID(DatabaseID, FusionImage);

	FaceFusion(FusionImage, FaceImage);
	//waitKey();
	/*imshow("rh", FusionImage);
	imshow("new", FaceImage);
	waitKey(0);*/
	FaceModel FusionFaceModel;
	MatToASVLOFFSCREEN(FusionImage, InputImage);

	thread Thread_GetFaceModel_Input([&] {
		GetFaceModel(InputImage, FusionFaceModel);
	});

	MInt32 Age, Gender;
	thread Thread_GenderEstimation([&] {
		Gender = GenderEstimation(InputImage);
		if (Gender == UNKNOW)
			Gender = MALE;
	});
	thread Thread_AgeEstimation([&] {
		Age = AgeEstimation(InputImage);
		if (Age == 0)
			Age = 20;
	});

	Thread_GenderEstimation.join();
	Thread_AgeEstimation.join();
	Thread_GetFaceModel_Input.join();

	if (FusionFaceModel.Model.lFeatureSize != 0 && InputFaceModel.Model.lFeatureSize != 0)
		DatabaseOperate->UpdateUserFaceInfo(DatabaseID, Age, Gender, FusionFaceModel, FusionImage, InputFaceModel, FaceImage);
	else if (InputFaceModel.Model.lFeatureSize != 0)
		DatabaseOperate->UpdateUserFaceInfo(DatabaseID, Age, Gender, InputFaceModel, FaceImage, InputFaceModel, FaceImage);
}

//人脸图像融合
void FaceComparison::FaceFusion(Mat &FusionImage, Mat &NewFaceImage)
{
	clock_t  clockBegin, clockEnd;
	clockBegin = clock();

	Point2f FusionImagePoint[FACE_KEY_POINT_NUMBER + 8], NewFaceImagePoint[FACE_KEY_POINT_NUMBER + 8], MedianPoint[FACE_KEY_POINT_NUMBER + 8];

	bool FaceAError = false, FaceBError = false;

	thread FindKeyPoint_Thread1([&FusionImagePoint, &FusionImage, &FaceAError, this] {
		Mat GrayFusionImage;
		cvtColor(FusionImage, GrayFusionImage, CV_BGR2GRAY);
		int *KeyPointResult;
		short *p;
		KeyPointResult = facedetect_frontal_surveillance(FaceKeyPointEngineMemory[0], (unsigned char*)(GrayFusionImage.ptr(0)), GrayFusionImage.cols, GrayFusionImage.rows, (int)GrayFusionImage.step, 1.2f, 2, 48, 0, 1);
		if (*KeyPointResult != 1)
		{
			FaceAError = true;
			return;
		}
		p = ((short*)(KeyPointResult + 1));
		for (int i = 0; i < FACE_KEY_POINT_NUMBER; i++)
		{
			FusionImagePoint[i].x = abs((int)p[6 + 2 * i]);
			FusionImagePoint[i].y = abs((int)p[6 + 2 * i + 1]);
		}

		FusionImagePoint[FACE_KEY_POINT_NUMBER].x = 0;
		FusionImagePoint[FACE_KEY_POINT_NUMBER].y = 0;
		FusionImagePoint[FACE_KEY_POINT_NUMBER + 1].x = FusionImage.cols - 1;
		FusionImagePoint[FACE_KEY_POINT_NUMBER + 1].y = 0;
		FusionImagePoint[FACE_KEY_POINT_NUMBER + 2].x = 0;
		FusionImagePoint[FACE_KEY_POINT_NUMBER + 2].y = FusionImage.rows - 1;
		FusionImagePoint[FACE_KEY_POINT_NUMBER + 3].x = FusionImage.cols - 1;
		FusionImagePoint[FACE_KEY_POINT_NUMBER + 3].y = FusionImage.rows - 1;
		FusionImagePoint[FACE_KEY_POINT_NUMBER + 4].x = (FusionImage.cols - 1) * 0.5;
		FusionImagePoint[FACE_KEY_POINT_NUMBER + 4].y = 0;
		FusionImagePoint[FACE_KEY_POINT_NUMBER + 5].x = (FusionImage.cols - 1) * 0.5;
		FusionImagePoint[FACE_KEY_POINT_NUMBER + 5].y = FusionImage.rows - 1;
		FusionImagePoint[FACE_KEY_POINT_NUMBER + 6].x = 0;
		FusionImagePoint[FACE_KEY_POINT_NUMBER + 6].y = (FusionImage.rows - 1) * 0.5;
		FusionImagePoint[FACE_KEY_POINT_NUMBER + 7].x = FusionImage.cols - 1;
		FusionImagePoint[FACE_KEY_POINT_NUMBER + 7].y = (FusionImage.rows - 1) * 0.5;

		FusionImage.convertTo(FusionImage, CV_32F);
	});

	thread FindKeyPoint_Thread2([&NewFaceImagePoint, &NewFaceImage, &FaceBError, this] {
		Mat GrayNewFaceImage;
		cvtColor(NewFaceImage, GrayNewFaceImage, CV_BGR2GRAY);
		int *KeyPointResult;
		short *p;
		KeyPointResult = facedetect_frontal_surveillance(FaceKeyPointEngineMemory[1], (unsigned char*)(GrayNewFaceImage.ptr(0)), GrayNewFaceImage.cols, GrayNewFaceImage.rows, (int)GrayNewFaceImage.step, 1.2f, 2, 48, 0, 1);
		if (*KeyPointResult != 1)
		{
			FaceBError = true;
			return;
		}
		p = ((short*)(KeyPointResult + 1));
		for (int i = 0; i < FACE_KEY_POINT_NUMBER; i++)
		{
			NewFaceImagePoint[i].x = abs((int)p[6 + 2 * i]);
			NewFaceImagePoint[i].y = abs((int)p[6 + 2 * i + 1]);
		}

		NewFaceImagePoint[FACE_KEY_POINT_NUMBER].x = 0;
		NewFaceImagePoint[FACE_KEY_POINT_NUMBER].y = 0;
		NewFaceImagePoint[FACE_KEY_POINT_NUMBER + 1].x = NewFaceImage.cols - 1;
		NewFaceImagePoint[FACE_KEY_POINT_NUMBER + 1].y = 0;
		NewFaceImagePoint[FACE_KEY_POINT_NUMBER + 2].x = 0;
		NewFaceImagePoint[FACE_KEY_POINT_NUMBER + 2].y = NewFaceImage.rows - 1;
		NewFaceImagePoint[FACE_KEY_POINT_NUMBER + 3].x = NewFaceImage.cols - 1;
		NewFaceImagePoint[FACE_KEY_POINT_NUMBER + 3].y = NewFaceImage.rows - 1;
		NewFaceImagePoint[FACE_KEY_POINT_NUMBER + 4].x = (NewFaceImage.cols - 1) * 0.5;
		NewFaceImagePoint[FACE_KEY_POINT_NUMBER + 4].y = 0;
		NewFaceImagePoint[FACE_KEY_POINT_NUMBER + 5].x = (NewFaceImage.cols - 1) * 0.5;
		NewFaceImagePoint[FACE_KEY_POINT_NUMBER + 5].y = NewFaceImage.rows - 1;
		NewFaceImagePoint[FACE_KEY_POINT_NUMBER + 6].x = 0;
		NewFaceImagePoint[FACE_KEY_POINT_NUMBER + 6].y = (NewFaceImage.rows - 1) * 0.5;
		NewFaceImagePoint[FACE_KEY_POINT_NUMBER + 7].x = NewFaceImage.cols - 1;
		NewFaceImagePoint[FACE_KEY_POINT_NUMBER + 7].y = (NewFaceImage.rows - 1) * 0.5;

		NewFaceImage.convertTo(NewFaceImage, CV_32F);
	});

	FindKeyPoint_Thread1.join();
	FindKeyPoint_Thread2.join();

	if (FaceAError || FaceBError)
	{
		FusionImage.convertTo(FusionImage, CV_8U);
		NewFaceImage.convertTo(NewFaceImage, CV_8U);
		return;
	}

	int ResultImageWidth = 0, ResultImageHeight = 0;
	for (int i = 0; i < FACE_KEY_POINT_NUMBER + 8; i++)
	{
		MedianPoint[i].x = (int)((1.0 - FusionAlpha) * FusionImagePoint[i].x + FusionAlpha * NewFaceImagePoint[i].x);
		MedianPoint[i].y = (int)((1.0 - FusionAlpha) * FusionImagePoint[i].y + FusionAlpha * NewFaceImagePoint[i].y);
		if (MedianPoint[i].x > ResultImageWidth)
			ResultImageWidth = MedianPoint[i].x;
		if (MedianPoint[i].y > ResultImageHeight)
			ResultImageHeight = MedianPoint[i].y;
	}

	if (ResultImageHeight != ResultImageWidth)
		return;
	//++ResultImageWidth;

	Rect rect(0, 0, ++ResultImageWidth, ++ResultImageHeight);
	Mat FusionResult = Mat::zeros(Size(ResultImageWidth, ResultImageHeight), CV_32FC3);
	Subdiv2D subdiv(rect);
	for (int i = 0; i < FACE_KEY_POINT_NUMBER + 8; ++i)
		subdiv.insert(MedianPoint[i]);
	std::vector<Vec6f> triangleList;
	subdiv.getTriangleList(triangleList);

	struct CorrespondingPoint
	{
		vector<int> Number;
	};
	vector<CorrespondingPoint> DelaunayTriangle;

	Point2f pt[3];
	for (int i = 0; i < triangleList.size(); ++i)
	{
		Vec6f t = triangleList[i];
		pt[0] = Point2f(t[0], t[1]);
		pt[1] = Point2f(t[2], t[3]);
		pt[2] = Point2f(t[4], t[5]);

		CorrespondingPoint Cooresponding;
		if (rect.contains(pt[0]) && rect.contains(pt[1]) && rect.contains(pt[2]))
		{
			int count = 0;
			for (int j = 0; j < 3; ++j)
			{
				for (int k = 0; k < FACE_KEY_POINT_NUMBER + 8; ++k)
				{
					if (abs(pt[j].x - MedianPoint[k].x) < 1.0   &&  abs(pt[j].y - MedianPoint[k].y) < 1.0)
					{
						Cooresponding.Number.push_back(k);
						++count;
						break;
					}
				}
			}
			if (count == 3)
				DelaunayTriangle.push_back(Cooresponding);
			//line(FusionResult, pt[0], pt[1], Scalar(255, 0, 0), 1);
			//line(FusionResult, pt[1], pt[2], Scalar(255, 0, 0), 1);
			//line(FusionResult, pt[0], pt[2], Scalar(255, 0, 0), 1);
		}
	}
	//imshow("sjx", FusionResult);
	//waitKey();
	//#pragma omp parallel for
	for (int i = 0; i < DelaunayTriangle.size(); ++i)
	{
		int x, y, z;

		CorrespondingPoint CorPoint = DelaunayTriangle[i];
		x = CorPoint.Number[0];
		y = CorPoint.Number[1];
		z = CorPoint.Number[2];

		vector<Point2f> TriangleFusion, TriangleNew, TriangleResult;
		TriangleFusion.push_back(FusionImagePoint[x]);
		TriangleFusion.push_back(FusionImagePoint[y]);
		TriangleFusion.push_back(FusionImagePoint[z]);

		TriangleNew.push_back(NewFaceImagePoint[x]);
		TriangleNew.push_back(NewFaceImagePoint[y]);
		TriangleNew.push_back(NewFaceImagePoint[z]);

		TriangleResult.push_back(MedianPoint[x]);
		TriangleResult.push_back(MedianPoint[y]);
		TriangleResult.push_back(MedianPoint[z]);

		Rect FusionTriangleRect, NewFaceTriangleRect, ResultTriangleRect;
		FusionTriangleRect = boundingRect(TriangleFusion);
		NewFaceTriangleRect = boundingRect(TriangleNew);
		ResultTriangleRect = boundingRect(TriangleResult);

		vector<Point2f> FusionArea, NewFaceArea, ResultArea;
		vector<Point> ResultArea_Int;
		for (int j = 0; j < 3; ++j)
		{
			ResultArea.push_back(Point2f(TriangleResult[j].x - ResultTriangleRect.x, TriangleResult[j].y - ResultTriangleRect.y));
			ResultArea_Int.push_back(Point(TriangleResult[j].x - ResultTriangleRect.x, TriangleResult[j].y - ResultTriangleRect.y));
			FusionArea.push_back(Point2f(TriangleFusion[j].x - FusionTriangleRect.x, TriangleFusion[j].y - FusionTriangleRect.y));
			NewFaceArea.push_back(Point2f(TriangleNew[j].x - NewFaceTriangleRect.x, TriangleNew[j].y - NewFaceTriangleRect.y));
		}
		Mat Mask;
		Mask = Mask.zeros(ResultTriangleRect.height, ResultTriangleRect.width, CV_32FC3);
		fillConvexPoly(Mask, ResultArea_Int, Scalar(1.0, 1.0, 1.0), 16, 0);

		if (FusionTriangleRect.x + FusionTriangleRect.width >= FusionImage.cols)
			FusionTriangleRect.width = FusionImage.cols - FusionTriangleRect.x;
		if (FusionTriangleRect.y + FusionTriangleRect.height >= FusionImage.rows)
			FusionTriangleRect.height = FusionImage.rows - FusionTriangleRect.y;

		if (NewFaceTriangleRect.x + NewFaceTriangleRect.width >= NewFaceImage.cols)
			NewFaceTriangleRect.width = NewFaceImage.cols - NewFaceTriangleRect.x;
		if (NewFaceTriangleRect.y + NewFaceTriangleRect.height >= NewFaceImage.rows)
			NewFaceTriangleRect.height = NewFaceImage.rows - NewFaceTriangleRect.y;

		/*printf("%d %d - %d %d Size: %d %d\n", FusionTriangleRect.x, FusionTriangleRect.y, FusionTriangleRect.x + FusionTriangleRect.width, FusionTriangleRect.y + FusionTriangleRect.height, FusionImage.cols, FusionImage.rows);
		printf("%d %d - %d %d (%d %d )Size: %d %d\n", NewFaceTriangleRect.x, NewFaceTriangleRect.y, NewFaceTriangleRect.x + NewFaceTriangleRect.width, NewFaceTriangleRect.y + NewFaceTriangleRect.height, NewFaceTriangleRect.width, NewFaceTriangleRect.height, NewFaceImage.cols, NewFaceImage.rows);
		for (int test = 0; test < TriangleNew.size(); ++test)
			printf("%.2lf %.2lf  ", TriangleNew[test].x, TriangleNew[test].y);*/

		Mat FusionImageRect, NewFaceImageRect;
		NewFaceImage(NewFaceTriangleRect).copyTo(NewFaceImageRect);
		FusionImage(FusionTriangleRect).copyTo(FusionImageRect);

		Mat FussionWrap, NewFaceWarp;
		FussionWrap = FussionWrap.zeros(ResultTriangleRect.height, ResultTriangleRect.width, FusionImageRect.type());
		NewFaceWarp = NewFaceWarp.zeros(ResultTriangleRect.height, ResultTriangleRect.width, NewFaceImageRect.type());

		Mat WarpMatA = getAffineTransform(FusionArea, ResultArea);
		warpAffine(FusionImageRect, FussionWrap, WarpMatA, FussionWrap.size(), INTER_LINEAR, BORDER_REFLECT_101);

		Mat WarpMatB = getAffineTransform(NewFaceArea, ResultArea);
		/*printf("TriangleRect: %d %d %d %d\n", NewFaceTriangleRect.x, NewFaceTriangleRect.y, NewFaceTriangleRect.x + NewFaceTriangleRect.width, NewFaceTriangleRect.y + NewFaceTriangleRect.height);
		printf("%d %d---%d %d---%d %d\n\n", NewFaceImageRect.cols, NewFaceImageRect.rows, WarpMatB.cols, WarpMatB.rows, NewFaceWarp.size().width, NewFaceWarp.size().height);*/
		warpAffine(NewFaceImageRect, NewFaceWarp, WarpMatB, NewFaceWarp.size(), INTER_LINEAR, BORDER_REFLECT_101);
		//	continue;

		Mat ResultImageRect = (1.0 - FusionAlpha) * FussionWrap + FusionAlpha * NewFaceWarp;

		multiply(ResultImageRect, Mask, ResultImageRect);
		multiply(FusionResult(ResultTriangleRect), Scalar(1.0, 1.0, 1.0) - Mask, FusionResult(ResultTriangleRect));
		//	printf("%d %d %d %d i = %d all = %d size: %d %d\n", ResultTriangleRect.x, ResultTriangleRect.y, ResultTriangleRect.width, ResultTriangleRect.height, i, DelaunayTriangle.size(), FusionResult.cols, FusionResult.rows);

		/*	printf("Multiply success\n");

			printf("FusionResult %d %d	TriangleRect %d %d %d %d\n", FusionResult.cols, FusionResult.rows, ResultTriangleRect.x, ResultTriangleRect.y, ResultTriangleRect.x + ResultTriangleRect.width, ResultTriangleRect.y + ResultTriangleRect.height);
			printf("ResultImageRect %d %d\n", ResultImageRect.cols, ResultImageRect.rows);
	*/
		FusionResult(ResultTriangleRect) = FusionResult(ResultTriangleRect) + ResultImageRect;

		//printf("End---------------\n");

	}

	FusionResult.convertTo(FusionImage, CV_8U);
}