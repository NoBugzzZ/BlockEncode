#include"BlEncode.h"
#include<iostream>
#include<string>
#include<fstream>
#include<ctime>
#include"spdlog/spdlog.h"
using namespace std;

BlEncode::BlEncode()
{}

bool BlEncode::connectMysql(string databaseName)
{
	char* p = _strdup(databaseName.c_str());
	mysql_init(&_mysql);
	mysql_options(&_mysql, MYSQL_SET_CHARSET_NAME, "gbk");
	//mysql_real_connect参数：2.本地地址  3.你的mysql用户名  4.你的mysql密码   5.数据库名字  6.端口号
	if (mysql_real_connect(&_mysql, "localhost", "root", "123456", p, 3306, NULL, 0) == NULL)
	{
		return false;
	}
	else
	{
		return true;
	}
}

unsigned char BlEncode::compressData(string str, bool* isCorrect)
{
	int num = 0;
	int temp = 0;
	for (int i = 0; i < 2; i++)
	{
		if (((str[i] - '0') >= 0) && ((str[i] - '0') <= 9))
		{
			temp = str[i] - '0';
			num = (num << 4) | (temp & 0xf);
		}
		else if (((str[i] - 'a') >= 0) && ((str[i] - 'a') <= 5))
		{
			temp = 10 + (str[i] - 'a');
			num = (num << 4) | (temp & 0xf);
		}
		else if (((str[i] - 'A') >= 0) && ((str[i] - 'A') <= 5))
		{
			temp = 10 + (str[i] - 'A');
			num = (num << 4) | (temp & 0xf);
		}
		else
		{
			*isCorrect = false;
		}
	}
	unsigned char tempByte = (unsigned char)(num & 0xff);
	return tempByte;
}

string BlEncode::decompressData(int num)
{
	string str = "";
	int temp[2];
	temp[0] = (num >> 4) & 0xf;
	temp[1] = num & 0xf;
	for (int i = 0; i < 2; i++)
	{
		if (temp[i] >= 0 && temp[i] <= 9)
		{
			str = str + to_string(temp[i]);
		}
		else if (temp[i] >= 10 && temp[i] <= 15)
		{
			char t = 'A';
			t = t + (temp[i] - 10);
			str = str + t;
		}
	}
	return str;
}

bool BlEncode::getCardnetInfo(string tableName)
{
	string sql = "SELECT DISTINCT cardnet FROM " + tableName + " ORDER BY CardNet ASC";
	char* p = _strdup(sql.c_str());
	mysql_query(&_mysql, p);
	MYSQL_RES* res = mysql_store_result(&_mysql);
	string str = "";
	CardNetInfo cardNetInfo;
	MYSQL_ROW row;
	while (row = mysql_fetch_row(res))
	{
		str = row[0];
		cardNetInfo.cardnet = str;
		cardNetInfo.count = 0;
		_cardNetInfos.push_back(cardNetInfo);
	}
	int category = _cardNetInfos.size();
	for (int i = 0; i < category; i++)
	{
		sql = "select COUNT(*) from " + tableName + " where CardNet = " + _cardNetInfos[i].cardnet;
		p = _strdup(sql.c_str());
		mysql_query(&_mysql, p);
		res = mysql_store_result(&_mysql);
		while (row = mysql_fetch_row(res))
		{
			str = row[0];
			_cardNetInfos[i].count = stoi(str);
		}
	}
	mysql_free_result(res);
	if (_cardNetInfos.size() != 0)
	{
		return true;
	}
	return false;
}

void BlEncode::BlockEncode(string databaseName, string tableName, int flag ,string compressedDataPath, string compressedDataFileName)
{
	if (connectMysql(databaseName) == false)
	{
		cout << (mysql_error(&_mysql));
		return;
	}
	if (!getCardnetInfo(tableName))
	{
		spdlog::error("获取索引失败");
		return;
	}
	
	string sql = "";
	char* p;
	bool* isCorrect = new bool;
	*isCorrect = true;
	string str;
	MYSQL_ROW row;
	MYSQL_RES* res = NULL;
	int category = _cardNetInfos.size();
	unsigned char cardID[8];
	_loadedData.clear();
	for (int i = 0; i < category; i++)
	{
		spdlog::info("{}", _cardNetInfos[i].cardnet);
		sql = "SELECT * FROM " + tableName + " WHERE CardNet = "
			+ _cardNetInfos[i].cardnet + " ORDER BY CardID ASC";
		p = _strdup(sql.c_str());
		mysql_query(&_mysql, p);
		res = mysql_store_result(&_mysql);
		while (row = mysql_fetch_row(res))
		{
			str = row[1];
			int temp = str.size();
			if (temp < 16)
			{
				for (int i = temp; i < 16; i++)
				{
					str = str + "0";
				}
			}
			else if (temp > 16)
			{
				spdlog::error("数据错误: {}", str);
				_cardNetInfos[i].count--;
				continue;
			}
			
			for (int i = 0; i < 15; i = i + 2)
			{
				cardID[(i / 2)] = compressData(str.substr(i, 2), isCorrect);
			}
			if (*isCorrect == false)
			{
				spdlog::error("数据错误: {}", str);
				_cardNetInfos[i].count--;
				*isCorrect = true;
				continue;
			}
			for (int i = 0; i < 8; i++)
			{
				//outfile.write((char*)&cardID[i], sizeof(cardID[i]));
				_loadedData.push_back(cardID[i]);
			}
			str = row[2];
			temp = stoi(str);
			cardID[0] = (unsigned char)(temp & 0xff);
			_loadedData.push_back(cardID[0]);
			if (flag == 1)
			{
				str = row[3];
				temp = stoi(str);
				cardID[0] = (unsigned char)(temp & 0xff);
				_loadedData.push_back(cardID[0]);
			}
		}

	}
	ofstream outfile(compressedDataPath + compressedDataFileName +".dat", ios::app | ios::binary);
	string index = "";
	for (int i = 0; i < category; i++)
	{
		index = index + _cardNetInfos[i].cardnet + ":" + to_string(_cardNetInfos[i].count) + "#";
	}
	spdlog::info("index: {}", index);
	outfile << index << endl;
	for (int i = 0; i < _loadedData.size(); i++)
	{
		outfile.write((char*)&_loadedData[i], sizeof(_loadedData[i]));
	}
	mysql_free_result(res);
	mysql_close(&_mysql);
	outfile.close();
	delete isCorrect;
	for (int i = 0; i < _cardNetInfos.size(); i++)
	{
		spdlog::info("{}:{}", _cardNetInfos[i].cardnet, _cardNetInfos[i].count);
	}
}

void BlEncode::checkData(string compressedDataPath, string compressedDataFileName,int flag ,string decompressedDataPath)
{
	_loadedData.clear();
	_cardNetInfos.clear();
	ifstream inFile(compressedDataPath + compressedDataFileName + ".dat", ios::binary);
	char index;
	string str = "";
	CardNetInfo cardNetInfo;
	int sequence = 0;
	//读取dat文件中索引
	while (inFile.read((char*)&index, sizeof(char)))
	{
		if (index == ':')
		{
			cardNetInfo.cardnet = str;
			cardNetInfo.count = 0;
			_cardNetInfos.push_back(cardNetInfo);
			str = "";
			continue;
		}
		else if (index == '#')
		{
			_cardNetInfos[sequence].count = stoi(str);
			sequence++;
			str = "";
			continue;
		}
		else if (index == '\n')
		{
			break;
		}
		str = str + index;
	}
	inFile.close();
	int category = _cardNetInfos.size();

	//读取压缩数据
	ifstream inFile2(compressedDataPath + compressedDataFileName + ".dat", ios::binary);
	unsigned char dataStream;
	bool isBinary = false;
	while (inFile2.read((char*)&dataStream, sizeof(char)))
	{
		//cout << tempB << endl;
		if (isBinary == false)
		{
			if (dataStream == '\n')
			{
				isBinary = true;
				continue;
			}
			continue;
		}
		_loadedData.push_back(dataStream);
	}
	//cout << _loadedData.size() << endl;
	inFile.close();
	int loadDataLength = _loadedData.size();
	string strs = "";
	int num = 0;
	string cardNet = "";
	string cardID = "";
	int bListType = 0;
	int status = 0;
	int count = 0;
	sequence = 0;
	ofstream outfile(decompressedDataPath + compressedDataFileName + "OriginalData.txt", ios::app);
	int dataLength = loadDataLength;
	if (flag == 0)
	{
		dataLength = 9;
	}
	else if (flag == 1)
	{
		dataLength = 10;
	}
	for (int i = 0; i < loadDataLength; i = i + dataLength)
	{
		strs = "";
		for (int j = 0; j < 8; j++)
		{
			num = _loadedData[(j + i)] & 0xff;
			strs += decompressData(num);
		}
		cardID = strs;
		bListType = _loadedData[(i + 8)] & 0xff;
		
		if (flag == 1)
		{
			status = _loadedData[(i + 9)] & 0xff;
		}
		cardNet = _cardNetInfos[sequence].cardnet;
		count++;
		if (count >= _cardNetInfos[sequence].count)
		{
			sequence++;
			count = 0;
		}
		if (flag == 0)
		{
			spdlog::info("{} {} {}", cardNet, cardID, bListType);
			outfile << cardNet << " " << cardID << " " << bListType << endl;
		}
		else if (flag == 1)
		{
			spdlog::info("{} {} {} {}", cardNet, cardID, bListType, status);
			outfile << cardNet << " " << cardID << " " << bListType << " " << status << endl;
		}
		
	}

	outfile.close();
}
