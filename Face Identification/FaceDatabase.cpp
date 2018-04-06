#include"stdafx.h"
#include "FaceDatabase.h"

//Ĭ�Ϲ��캯�����ڴ˳�ʼ��DB
FaceDatabase::FaceDatabase()
{
	char *zErrMsg = 0;
	int rc;

	rc = sqlite3_open(DBPath.c_str(), &db);

	if (rc) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		exit(0);
	}
	else {
		fprintf(stderr, "Opened database successfully\n");
	}

}

FaceDatabase::~FaceDatabase()
{
	sqlite3_close(db);
}

void FaceDatabase::SerachFace(int gender, int age, DataIterator &Iterator) {

	//ͨ�����䡢�Ա����Ϣ����
	int high_age = age + 20;
	int low_age = age - 20;
	sqlite3_stmt* stmt;
	sqlite3_prepare(db, "select * from Base where Gender = ? and Age < ? and Age > ? order by VisitTime desc", -1, &stmt, 0);

	sqlite3_bind_int(stmt, 1, gender);
	sqlite3_bind_int(stmt, 2, high_age);
	sqlite3_bind_int(stmt, 3, low_age);

	while (sqlite3_step(stmt) == SQLITE_ROW)                                  //��ѯ�ɹ����ص���SQLITE_ROW
	{
		PersonalData* p = new PersonalData();

		p->DatabaseID = sqlite3_column_int(stmt, 0);

		p->Info.Name = (char*)sqlite3_column_text(stmt, 1);
		p->Info.Gender = sqlite3_column_int(stmt, 2);
		p->Info.Age = sqlite3_column_int(stmt, 3);

		int channels = sqlite3_column_int(stmt, 7);
		p->FusionImage.rows = sqlite3_column_int(stmt, 8);
		p->FusionImage.cols = sqlite3_column_int(stmt, 9);

		unsigned char* charBuf = (unsigned char*)sqlite3_column_blob(stmt, 5);//0��ʾ��ѯ���еĵ�һ������
		std::vector<unsigned char> vectordata(charBuf, charBuf + channels * p->FusionImage.rows *p->FusionImage.cols);
		cv::Mat picture(p->FusionImage.rows, p->FusionImage.cols, CV_8UC3, vectordata.data());
		p->FusionImage = picture.clone();

		p->Info.VisitTime = sqlite3_column_int(stmt, 10);

		p->FusionModel.Model.lFeatureSize = sqlite3_column_bytes(stmt, 6);
		p->FusionModel.Model.pbFeature = (MByte*)malloc(p->FusionModel.Model.lFeatureSize);
		memcpy(p->FusionModel.Model.pbFeature, sqlite3_column_blob(stmt, 6), p->FusionModel.Model.lFeatureSize);


		Iterator.List.push_back(p);
	}

	sqlite3_finalize(stmt);
}

void FaceDatabase::SearchPersonInfoByID(int ID, PInfo& data)
{
	sqlite3_stmt* stmt;
	sqlite3_prepare(db, "select * from Base where DataBaseID = ?", -1, &stmt, 0);
	sqlite3_bind_int(stmt, 1, ID);
	int result = sqlite3_step(stmt);
	if (result == SQLITE_ROW)                                     //��ѯ�ɹ����ص���SQLITE_ROW
	{
		data.Name = (char*)sqlite3_column_text(stmt, 1);
		data.Gender = sqlite3_column_int(stmt, 2);
		data.Age = sqlite3_column_int(stmt, 3);
		data.PersonType = sqlite3_column_int(stmt, 4);

	}

	sqlite3_finalize(stmt);
}



//�����Ա��Ϣ��DataBase��
void FaceDatabase::AddNewUser(PInfo& Info, FaceModel& Model, Mat& FaceImage)
{
	sqlite3_stmt* stmt;

	const char* sql = "insert into Base(Name,Gender,Age,PersonType,FusionImage,FusionModel,ImageChannels,ImageRows,ImageCols) values(?,?,?,?,?,?,?,?,?)";
	sqlite3_prepare(db, sql, strlen(sql), &stmt, 0);

	sqlite3_bind_text(stmt, 1, Info.Name.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 2, Info.Gender);
	sqlite3_bind_int(stmt, 3, Info.Age);
	sqlite3_bind_int(stmt, 4, Info.PersonType);

	char* buffer = new char[FaceImage.channels() * FaceImage.rows * FaceImage.cols];

	memcpy(buffer, FaceImage.data, FaceImage.channels() * FaceImage.rows * FaceImage.cols);

	sqlite3_bind_blob(stmt, 5, buffer, FaceImage.channels()*FaceImage.rows * FaceImage.cols, NULL);

	sqlite3_bind_blob(stmt, 6, Model.Model.pbFeature, Model.Model.lFeatureSize, NULL);

	sqlite3_bind_int(stmt, 7, FaceImage.channels());
	sqlite3_bind_int(stmt, 8, FaceImage.rows);
	sqlite3_bind_int(stmt, 9, FaceImage.cols);
	sqlite3_step(stmt);

	sqlite3_finalize(stmt);//�ͷ�stmt

}

void FaceDatabase::DeletUserFromBase(int DataBaseID)
{
	sqlite3_stmt* stmt;
	const char* sql = "delete from Base where DataBaseID = ?";
	sqlite3_prepare(db, sql, strlen(sql), &stmt, 0);
	sqlite3_bind_int(stmt, 1, DataBaseID);
	sqlite3_step(stmt);
	sqlite3_finalize(stmt);//�ͷ�stmt
}

void FaceDatabase::DeletUserFromFaceModel(int DataBaseID)
{
	sqlite3_stmt* stmt;
	const char* sql = "delete from FaceModel where DataBaseID = ?";
	sqlite3_prepare(db, sql, strlen(sql), &stmt, 0);
	sqlite3_bind_int(stmt, 1, DataBaseID);
	sqlite3_step(stmt);
	sqlite3_finalize(stmt);//�ͷ�stmt
}

void FaceDatabase::UpdateUserFaceInfo(int DataBaseID, int Age, int Gender, FaceModel &FusionModel, Mat &FusionImage, FaceModel &NewFaceModel, Mat &NewFaceImage)
{
	sqlite3_stmt* stmt;

	//����һ��FaceModel
	const char* sql = "insert into FaceModel(DataBaseID,Gender,Age,FaceImage,FaceModel,ImageChannels,ImageRows,ImageCols,Time) values(?,?,?,?,?,?,?,?,?)";
	sqlite3_prepare(db, sql, strlen(sql), &stmt, 0);

	sqlite3_bind_int(stmt, 1, DataBaseID);
	sqlite3_bind_int(stmt, 2, Gender);
	sqlite3_bind_int(stmt, 3, Age);


	char* buffer = new char[NewFaceImage.channels() * NewFaceImage.rows * NewFaceImage.cols];

	memcpy(buffer, NewFaceImage.data, NewFaceImage.channels() * NewFaceImage.rows * NewFaceImage.cols);

	sqlite3_bind_blob(stmt, 4, buffer, NewFaceImage.channels()*NewFaceImage.rows * NewFaceImage.cols, NULL);

	sqlite3_bind_blob(stmt, 5, NewFaceModel.Model.pbFeature, NewFaceModel.Model.lFeatureSize, NULL);

	sqlite3_bind_int(stmt, 6, NewFaceImage.channels());
	sqlite3_bind_int(stmt, 7, NewFaceImage.rows);
	sqlite3_bind_int(stmt, 8, NewFaceImage.cols);

	time_t timep;
	time(&timep);
	char tmp[64];
	strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&timep));

	sqlite3_bind_text(stmt, 9, tmp, -1, SQLITE_STATIC);
	sqlite3_step(stmt);

	//���FaceModel��������ͬDataBaseID��������5��ɾ�����ϵ�һ��
	const char* sql2 = "select count(*) from FaceModel where DataBaseID = ?";
	sqlite3_prepare(db, sql2, strlen(sql2), &stmt, 0);
	sqlite3_bind_int(stmt, 1, DataBaseID);
	int result = sqlite3_step(stmt);
	int num = 0;
	if (result == SQLITE_ROW)                                     //��ѯ�ɹ����ص���SQLITE_ROW
	{
		num = sqlite3_column_int(stmt, 0);
	}
	if (num > 5)
	{
		//����ɾ��
		const char* sql3 = "delete from FaceModel where Time in (select Time from FaceModel where DataBaseID = ? order by time limit 0, ? - 5)";
		sqlite3_prepare(db, sql3, strlen(sql3), &stmt, 0);
		sqlite3_bind_int(stmt, 1, DataBaseID);
		sqlite3_bind_int(stmt, 2, num);
		sqlite3_step(stmt);
	}
	//numӦ�ô���0
	if (num > 0)
	{
		//����Base���е����䣬ȡFaceModel���е�ֵƽ��
		const char* sql4 = "update Base set Age = (select avg(Age) from FaceModel where DataBaseID = ?),Gender = (select sum(Gender)/(count(Gender) / 2 + 1) from FaceModel where DataBaseID = ?),FusionImage=?,FusionModel=?,ImageChannels = ?,ImageRows=?,ImageCols=?,VisitTime = VisitTime + 1  where DataBaseID = ?";
		sqlite3_prepare(db, sql4, strlen(sql4), &stmt, 0);
		sqlite3_bind_int(stmt, 1, DataBaseID);
		sqlite3_bind_int(stmt, 2, DataBaseID);

		char* buffer = new char[FusionImage.channels() * FusionImage.rows * FusionImage.cols];

		memcpy(buffer, FusionImage.data, FusionImage.channels() * FusionImage.rows * FusionImage.cols);

		sqlite3_bind_blob(stmt, 3, buffer, FusionImage.channels()*FusionImage.rows * FusionImage.cols, NULL);

		sqlite3_bind_blob(stmt, 4, FusionModel.Model.pbFeature, FusionModel.Model.lFeatureSize, NULL);

		sqlite3_bind_int(stmt, 5, FusionImage.channels());
		sqlite3_bind_int(stmt, 6, FusionImage.rows);
		sqlite3_bind_int(stmt, 7, FusionImage.cols);

		sqlite3_bind_int(stmt, 8, DataBaseID);

		sqlite3_step(stmt);
	}

	sqlite3_finalize(stmt);//�ͷ�stmt
}

//ͨ��DataBaseID����FaceModel������ѯ�õ�����Ϣ����List��
void FaceDatabase::SearchFaceModel(int DataBaseID, FaceModelIterator & Iterator)
{

	sqlite3_stmt* stmt;
	sqlite3_prepare(db, "select * from FaceModel where DataBaseID = ? order by Time desc", -1, &stmt, 0);

	sqlite3_bind_int(stmt, 1, DataBaseID);

	while (sqlite3_step(stmt) == SQLITE_ROW)                                  //��ѯ�ɹ����ص���SQLITE_ROW
	{
		PersonalData* p = new PersonalData();

		p->DatabaseID = sqlite3_column_int(stmt, 0);

		p->Info.Gender = sqlite3_column_int(stmt, 1);
		p->Info.Age = sqlite3_column_int(stmt, 2);

		int channels = sqlite3_column_int(stmt, 5);
		p->FusionImage.rows = sqlite3_column_int(stmt, 6);
		p->FusionImage.cols = sqlite3_column_int(stmt, 7);

		unsigned char* charBuf = (unsigned char*)sqlite3_column_blob(stmt, 3);
		std::vector<unsigned char> vectordata(charBuf, charBuf + channels * p->FusionImage.rows *p->FusionImage.cols);
		cv::Mat picture(p->FusionImage.rows, p->FusionImage.cols, CV_8UC3, vectordata.data());
		p->FusionImage = picture.clone();

		p->FusionModel.Model.lFeatureSize = sqlite3_column_bytes(stmt, 4);
		p->FusionModel.Model.pbFeature = (MByte*)malloc(p->FusionModel.Model.lFeatureSize);
		memcpy(p->FusionModel.Model.pbFeature, sqlite3_column_blob(stmt, 4), p->FusionModel.Model.lFeatureSize);

		Iterator.List.push_back(p);
	}

	sqlite3_finalize(stmt);
}

//ͨ��DatabaseID����Base���е�FusionImage
void FaceDatabase::GetUserFusionImageByID(int DatabaseID, Mat &FusionImage)
{
	sqlite3_stmt* stmt;
	sqlite3_prepare(db, "select * from Base where DataBaseID = ?", -1, &stmt, 0);

	sqlite3_bind_int(stmt, 1, DatabaseID);

	if (sqlite3_step(stmt) == SQLITE_ROW)                                  //��ѯ�ɹ����ص���SQLITE_ROW
	{
		int channels = sqlite3_column_int(stmt, 7);
		int rows = sqlite3_column_int(stmt, 8);
		int cols = sqlite3_column_int(stmt, 9);

		unsigned char* charBuf = (unsigned char*)sqlite3_column_blob(stmt, 5);
		std::vector<unsigned char> vectordata(charBuf, charBuf + channels * rows * cols);
		cv::Mat picture(rows, cols, CV_8UC3, vectordata.data());
		FusionImage = picture.clone();
	}

	sqlite3_finalize(stmt);


}

//���ݵ�����Ĭ�Ϲ���
DataIterator::DataIterator()
{
}

//Ĭ���������ͷ�List��Դ
DataIterator::~DataIterator()
{
	this->CleanList();
}

//������++����
PersonalData* DataIterator::Next()
{
	if (List.empty())
		return nullptr;
	if (Pointer == nullptr)
	{
		Iterator = List.begin();
		Pointer = *(Iterator);
	}
	else
	{
		Iterator++;

		if (Iterator == List.end())
		{
			return nullptr;
		}
		Pointer = *Iterator;
	}
	return Pointer;
}

//�����ͷ�List��Դ
void DataIterator::CleanList()
{
	list<PersonalData*>::iterator iter;
	for (iter = List.begin(); iter != List.end();)
	{
		(*iter)->~PersonalData();
		iter = List.erase(iter);
	}
	List.clear();
}

FaceModelIterator::FaceModelIterator()
{
}

FaceModelIterator::~FaceModelIterator()
{
	CleanList();
}

PersonalData * FaceModelIterator::Next()
{
	if (List.empty())
		return nullptr;
	if (Pointer == nullptr)
	{
		Iterator = List.begin();
		Pointer = *(Iterator);
	}
	else
	{
		Iterator++;

		if (Iterator == List.end())
		{
			return nullptr;
		}
		Pointer = *Iterator;
	}
	return Pointer;
}

void FaceModelIterator::CleanList()
{
	list<PersonalData*>::iterator iter;
	for (iter = List.begin(); iter != List.end();)
	{
		iter = List.erase(iter);
	}
	List.clear();
}
