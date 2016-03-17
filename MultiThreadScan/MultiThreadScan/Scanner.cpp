#include <Windows.h>
#include <iostream>
#include <winsock.h>

#pragma comment(lib,"wsock32.lib")

#define DEF_BUF_SIZE 1024
#define ICMP_HEADER_SIZE 12

using namespace std;

typedef struct _ICMP_HEADER
{
	BYTE bType;
	USHORT nCheckSum;
}ICMP_HEADER, *PICMP_HEADER;

USHORT GetCheckSum(LPBYTE lpBuff, DWORD dwSize)
{
	DWORD dwCheckSum = 0;
	USHORT *lpWord = (USHORT*)lpBuff;
	while (dwSize > 1)
	{
		dwCheckSum += *lpWord++;
		dwSize -= 2;
	}
	if (dwSize == 1)
		dwCheckSum += *((LPBYTE)lpBuff);
	return (USHORT)(~dwCheckSum);
}

DWORD WINAPI Thread(LPVOID lpParmeter);
HANDLE g_hMutex;

int main()
{
	static int u = 20, i = 0;
	HANDLE *hThread = new HANDLE[u];
	WSADATA wsaData;
	WORD wVersionRequested;
	wVersionRequested = MAKEWORD(1, 1);
	WSAStartup(wVersionRequested, &wsaData);

	for (; i < u; i++)
	{
		g_hMutex = CreateMutex(NULL, FALSE, TEXT("MutexToReceive"));
		hThread[i] = CreateThread(NULL, 0, Thread, &i, 0, NULL);
		Sleep(50);
	}

	SetEvent(g_hMutex);
	WaitForMultipleObjects(i, hThread, TRUE, INFINITE);
	for (; i < u; i++)
		CloseHandle(hThread[i]);
	return 0;
}

DWORD WINAPI Thread(LPVOID lpParmeter)
{
	int x = 80, nRet;
	int idx = *(int*)lpParmeter;
	SOCKET s = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

	if (s == INVALID_SOCKET)
	{
		cout << "Socket error code for the: " << s << endl;
		return 0;
	}

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(x);
	static unsigned long iSIP = inet_addr("192.168.17.1");
	static unsigned long iEIP = inet_addr("192.168.17.254");
	static unsigned long s1 = ntohl(iSIP);
	static unsigned long e1 = ntohl(iEIP);

	for (; s1 <= e1; s1++)
	{
		WaitForSingleObject(g_hMutex, INFINITE);
		if (s1 <= e1)
		{
			Sleep(3);
			addr.sin_addr.S_un.S_addr = ntohl(s1);
			char ICMPPack[ICMP_HEADER_SIZE] = { 0 };
			PICMP_HEADER pICMPHeader = (PICMP_HEADER)ICMPPack;
			pICMPHeader->bType = 8;
			int nTime = 1000;
			int ret = ::setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&nTime, sizeof(nTime));
			char szRcvBuff[DEF_BUF_SIZE];
			sockaddr_in SourceSockAddr;
			pICMPHeader->nCheckSum = GetCheckSum((LPBYTE)(ICMPPack), ICMP_HEADER_SIZE);
			sendto(s, ICMPPack, ICMP_HEADER_SIZE, 0, (sockaddr*)&addr, sizeof(addr));
			int nLen = sizeof(sockaddr_in);
			nRet = ::recvfrom(s, szRcvBuff, DEF_BUF_SIZE, 0, (sockaddr*)&SourceSockAddr, &nLen);
			if (nRet != SOCKET_ERROR)
			{
				SOCKET c = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				if (c == INVALID_SOCKET)
					return -1;
				DWORD dwset = 1;
				int ret = ioctlsocket(c, FIONBIO, (LPDWORD)&dwset);
				if (ret == SOCKET_ERROR)
					return -1;
				DWORD start = GetTickCount();
				connect(c, (sockaddr*)&addr, sizeof(addr));

				timeval timeout;
				fd_set r;
				FD_ZERO(&r);
				FD_SET(c, &r);
				timeout.tv_sec = 1;
				timeout.tv_usec = 0;

				ret = select(0, 0, &r, 0, &timeout);
				if (ret <= 0)
					cout << inet_ntoa(addr.sin_addr) << " " << x << "Close" << endl;
				else
					cout << inet_ntoa(addr.sin_addr) << " " << x << "Open" << endl;
				dwset = 0;
				ret = ioctlsocket(c, FIONBIO, (LPDWORD)&dwset);
				if (ret == SOCKET_ERROR)
					cout << "error " << WSAGetLastError() << endl;
			}
			else
				cout << inet_ntoa(addr.sin_addr) << " Not online" << endl;
		}
		else
		{
			ReleaseMutex(g_hMutex);
			break;
		}
	}
	return 0;
}