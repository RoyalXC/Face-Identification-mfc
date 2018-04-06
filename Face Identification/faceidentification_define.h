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

#define WORKBUF_SIZE    (40*1024*1024)			//���湤������ռ��С
#define KEY_POINT_DETECT_BUFFER_SIZE 0x20000	//�沿�ؼ��������滺��ռ��С
#define FACE_KEY_POINT_NUMBER	68				//�沿�ؼ�������
#define IMAGE_TYPE		ASVL_PAF_RGB24_B8G8R8	//ͼƬ����

#define SUCCESS				1					//�ɹ�����
#define NO_FACE				2					//δ��⵽����
#define LIVING_DETECT_FAIL	3					//���ڽ��л�����
#define NO_INFO				4					//�޴�����Ϣ
#define INSTABLE			5					//�������ȶ�
#define ENGINE_FAILED		6					//����δ����
#define TRACKING_FAILED		7					//��������ʧ��
#define REGISTER_SUCCESS	8					//�ɹ�¼��
#define REGISTERING			9					//����¼��
#define USER_EXISTS			10					//¼��ʧ�ܣ��ù����Ѵ���
#define IMPROPER			11					//�沿���򲻺��ʣ��뽫��������Ļ��������
#define QR_ACCEPT			12					//��ά����֤�ɹ�

#define MALE				0					//����
#define FEMALE				1					//Ů��
#define UNKNOW				-1					//�Ա�δ֪

#define AGERANGE_0_19		100					//���䷶Χ0-19
#define AGERANGE_20_39		101					//���䷶Χ20-39
#define AGERANGE_39_59		102					//���䷶Χ39-59
#define AGERANGE_60Plus		103					//���䷶Χ60+

//RECT�ṹ�� ���ڱ궨��������
typedef struct _FI_Rect
{
	int left, right, top, bottom;
}Face_RECT;

//��Ա��Ϣ�ṹ��
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

//�������
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

//���ӿڷ�����Ϣ�ṹ��
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

//���ݿ�����Ա����
struct PersonalData
{
	int DatabaseID;
	PInfo Info;
	list<FaceModel*> HistoricalFaceModel;
	FaceModel FusionModel;
	Mat FusionImage;
	//...

	//��д�Ⱥ�
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



//��Mat�������ݸ�ֵ��ASVLOFFSCREEN����
static void MatToASVLOFFSCREEN(Mat &Source, ASVLOFFSCREEN &Destination)
{
	Destination.i32Width = Source.cols;
	Destination.i32Height = Source.rows;
	Destination.ppu8Plane[0] = Source.data;
	Destination.pi32Pitch[0] = Destination.i32Width * Source.channels();
}