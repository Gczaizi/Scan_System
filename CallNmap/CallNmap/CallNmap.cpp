#include <iostream>
#include <string>
#include <Windows.h>

using namespace std;

void main()
{
	char c[100];
	scanf("%s", c);	//������ȷ�洢����ո���Ϊ������
	printf(c);
	printf("\n");
	system(c);
	system("pause");
}