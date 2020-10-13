#ifndef __BLENCODE_H__
#define __BLENCODE_H__
#include<string>
#include<vector>
#include"mysql.h"

class BlEncode
{
public:
	BlEncode();
	void BlockEncode(std::string compressedDataPath);			//��ѹ�����ݴ���dat�ļ���
	void checkData(std::string compressedDataPath,std::string decompressedDataPath);			//��ѹ�����ݽ�ѹ��������txt�ļ���
private:
	struct CardNetInfo {
		std::string cardnet;
		int count;
	};
	
	bool connectMysql();			//����mysql���ݿ�
	unsigned char compressData(std::string str, bool* isCorrect);	//ѹ������
	std::string decompressData(int num);
	std::string getCardnetInfo();		//�õ�cardnet�����Լ���ֵ����������
	std::string _path;
	std::vector<unsigned char> _loadedData;
	std::vector<CardNetInfo> _cardNetInfos;
	MYSQL _mysql;
	std::string _tableName;
};

#endif // !__BLENCODE_H__

