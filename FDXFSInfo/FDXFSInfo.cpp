// XFSInfo.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>
#include "FDXFSHelper.h"

int main()
{
    std::cout << "Hello World!\n";

    if (GetXFSHelper()->Initialize()) {
        GetXFSHelper()->Open(L"G:\\Evidences\\Dumps\\XFSDump\\xfs_ubuntu.raw");
        GetXFSHelper()->TestVolume();
        GetXFSHelper()->DirTest();
        GetXFSHelper()->Close();
    }
}
