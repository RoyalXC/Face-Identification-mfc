#pragma once

#include "faceidentification_define.h"

class DataIterator
{
public:
	DataIterator();
	~DataIterator();
	PersonalData* Next();
	PersonalData * Pointer = nullptr;
	list<PersonalData*>::iterator Iterator;
	list<PersonalData*> List;
	void CleanList();
};

class FaceModelIterator {
public:
	FaceModelIterator();
	~FaceModelIterator();
	PersonalData* Next();
	PersonalData * Pointer = nullptr;
	list<PersonalData*>::iterator Iterator;
	list<PersonalData*> List;
	void CleanList();
};

class FaceDatabase
{
	sqlite3* db;
public:
	FaceDatabase();
	//Ĭ����������
	~FaceDatabase();
	//���ݴ������Ĳ������Ա����䣩�����������ݣ����������仮�ֵ�����ָ��
	void SerachFace(int Gender, int GenderRange, DataIterator& Iterator);
	void SearchPersonInfoByID(int ID, PInfo& data);
	void AddNewUser(PInfo & Info, FaceModel & Model, Mat & FaceImage);
	void DeletUserFromBase(int DataBaseID);
	void DeletUserFromFaceModel(int DataBaseID);
	void UpdateUserFaceInfo(int DataBaseID, int Age, int Gender, FaceModel &FusionModel, Mat & FusionImage, FaceModel &NewFaceModel, Mat & NewFaceImage);
	void SearchFaceModel(int DataBaseID, FaceModelIterator& Iterator);
	void GetUserFusionImageByID(int DatabaseID, Mat &FusionImage);
	const string DBPath = "FaceDataBase.db";
};