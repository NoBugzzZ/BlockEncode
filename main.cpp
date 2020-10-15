#include"BlEncode.h"
#include<iostream>
using namespace std;
int main()
{
	BlEncode blEncode;
	blEncode.BlockEncode("mydb", "test", 0, "C:/Users/tz/source/repos/", "BasicData");
	blEncode.checkData("C:/Users/tz/source/repos/", "BasicData", 0, "C:/Users/tz/source/repos/");
	blEncode.BlockEncode("mydb", "test_copy", 1, "C:/Users/tz/source/repos/", "ExtraData");
	blEncode.checkData("C:/Users/tz/source/repos/", "ExtraData", 1, "C:/Users/tz/source/repos/");
	return 0;
}
