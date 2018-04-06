#pragma once

#include <opencv\cv.h>
#include <opencv2\opencv.hpp>
#include <string>
#include <thread>
#include <mutex>
#include <list>
#include <vector>
#include <list>
#include <fstream>
#include <sqlite3.h>
#include <omp.h>
#include "merror.h"
#include "arcsoft_fsdk_face_tracking.h"
#include "arcsoft_fsdk_face_detection.h"
#include "arcsoft_fsdk_face_recognition.h"
#include "arcsoft_fsdk_gender_estimation.h"
#include "arcsoft_fsdk_age_estimation.h"
#include "facedetect-dll.h"

using namespace cv;
using namespace std;

typedef int IDENTIFY_STATUS;

#define WORKBUF_SIZE    (40*1024*1024)			//引擎工作缓存空间大小
#define KEY_POINT_DETECT_BUFFER_SIZE 0x20000	//面部关键点检测引擎缓冲空间大小
#define FACE_KEY_POINT_NUMBER	68				//面部关键点数量
#define IMAGE_TYPE		ASVL_PAF_RGB24_B8G8R8	//图片类型

#define SUCCESS				1					//成功鉴别
#define NO_FACE				2					//未检测到人脸
#define LIVING_DETECT_FAIL	3					//正在进行活体检测
#define NO_INFO				4					//无此人信息
#define INSTABLE			5					//人脸不稳定
#define ENGINE_FAILED		6					//引擎未启动
#define TRACKING_FAILED		7					//人脸跟踪失败
#define REGISTER_SUCCESS	8					//成功录入
#define REGISTERING			9					//正在录入
#define USER_EXISTS			10					//录入失败，该工号已存在
#define IMPROPER			11					//面部区域不合适，请将脸置于屏幕中央区域
#define QR_ACCEPT			12					//二维码验证成功

#define MALE				0					//男性
#define FEMALE				1					//女性
#define UNKNOW				-1					//性别未知

#define AGERANGE_0_19		100					//年龄范围0-19
#define AGERANGE_20_39		101					//年龄范围20-39
#define AGERANGE_39_59		102					//年龄范围39-59
#define AGERANGE_60Plus		103					//年龄范围60+

//RECT结构体 用于标定矩形区域
typedef struct _FI_Rect
{
	int left, right, top, bottom;
}Face_RECT;

//人员信息结构体
typedef struct _Personal_Information
{
	int DataBaseID;
	string Name;
	int Gender;
	int PersonType;
	int Age;
	int VisitTime;
	_Personal_Information() {
		this->DataBaseID = 0;
		this->VisitTime = 0;
	}
	_Personal_Information& operator=(_Personal_Information& data)
	{
		DataBaseID = data.DataBaseID;
		Name = data.Name;
		//Name = string(data.Name.c_str());
		PersonType = data.PersonType;
		Gender = data.Gender;
		Age = data.Age;
		VisitTime = data.VisitTime;
		return *this;
	}

}PInfo;

//检索结果
struct RetrievalResult
{
	bool IsSuccess = false;
	_Personal_Information PersonInfo;
	float SimilarScore = 0;
	RetrievalResult& operator=(RetrievalResult& Another)
	{
		IsSuccess = Another.IsSuccess;
		PersonInfo = Another.PersonInfo;
		SimilarScore = Another.SimilarScore;
		return *this;
	}
};

//主接口返回信息结构体
struct FI_RESULT
{
	Face_RECT *FaceRect;
	int NumberOfFace;
	IDENTIFY_STATUS IdentifyStatus;
	RetrievalResult PersonInfo;
};

struct FaceModel
{
	AFR_FSDK_FACEMODEL Model = { 0 };
	~FaceModel()
	{
		free(Model.pbFeature);
	}
};

//数据库中人员数据
struct PersonalData
{
	int DatabaseID;
	PInfo Info;
	list<FaceModel*> HistoricalFaceModel;
	FaceModel FusionModel;
	Mat FusionImage;
	//...

	//重写等号
	PersonalData& operator=(PersonalData& data) {
		DatabaseID = data.DatabaseID;
		Info = data.Info;
		FusionImage = data.FusionImage;
		FusionModel.Model.lFeatureSize = data.FusionModel.Model.lFeatureSize;
		FusionModel.Model.pbFeature = data.FusionModel.Model.pbFeature;
	}
	~PersonalData()
	{
		list<FaceModel*>::iterator iter;
		for (iter = HistoricalFaceModel.begin(); iter != HistoricalFaceModel.end(); ++iter)
		{
			iter = HistoricalFaceModel.erase(iter);
		}
	}
};



//将Mat类型数据赋值给ASVLOFFSCREEN类型
static void MatToASVLOFFSCREEN(Mat &Source, ASVLOFFSCREEN &Destination)
{
	Destination.i32Width = Source.cols;
	Destination.i32Height = Source.rows;
	Destination.ppu8Plane[0] = Source.data;
	Destination.pi32Pitch[0] = Destination.i32Width * Source.channels();
}