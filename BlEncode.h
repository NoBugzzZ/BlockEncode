#ifndef __BLENCODE_H__
#define __BLENCODE_H__
#include<string>
#include<vector>
#include"mysql.h"

class BlEncode
{
public:
	BlEncode();
	void BlockEncode(std::string databaseName, std::string tableName, int flag ,std::string compressedDataPath, std::string compressedDataFileName);			//将压缩数据存入dat文件中
	void checkData(std::string compressedDataPath,std::string compressedDataFileName ,int flag ,std::string decompressedDataPath);			//将压缩数据解压缩并存入txt文件中
private:
	struct CardNetInfo {
		std::string cardnet;
		int count;
	};
	
	bool connectMysql(std::string databaseName);			//链接mysql数据库
	unsigned char compressData(std::string str, bool* isCorrect);	//压缩数据
	std::string decompressData(int num);
	bool getCardnetInfo(std::string tableName);		//得到cardnet类数以及其值，建立索引
	std::vector<unsigned char> _loadedData;
	std::vector<CardNetInfo> _cardNetInfos;
	MYSQL _mysql;
};

#endif // !__BLENCODE_H__

