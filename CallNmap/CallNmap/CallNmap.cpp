#include <iostream>
#include <string>
#include <Windows.h>

using namespace std;

void main()
{
	char c[100];
	scanf("%s", c);	//不能正确存储命令，空格认为结束符
	printf(c);
	printf("\n");
	system(c);
	system("pause");
}