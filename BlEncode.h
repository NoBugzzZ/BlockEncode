#ifndef __BLENCODE_H__
#define __BLENCODE_H__
#include<string>
#include<vector>
#include"mysql.h"

class BlEncode
{
public:
	BlEncode();
	void BlockEncode(std::string compressedDataPath);			//将压缩数据存入dat文件中
	void checkData(std::string compressedDataPath,std::string decompressedDataPath);			//将压缩数据解压缩并存入txt文件中
private:
	struct CardNetInfo {
		std::string cardnet;
		int count;
	};
	
	bool connectMysql();			//链接mysql数据库
	unsigned char compressData(std::string str, bool* isCorrect);	//压缩数据
	std::string decompressData(int num);
	std::string getCardnetInfo();		//得到cardnet类数以及其值，建立索引
	std::string _path;
	std::vector<unsigned char> _loadedData;
	std::vector<CardNetInfo> _cardNetInfos;
	MYSQL _mysql;
	std::string _tableName;
};

#endif // !__BLENCODE_H__

