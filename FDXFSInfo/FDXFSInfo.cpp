// XFSInfo.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>
#include "FDXFSHelper.h"

int wmain(int argc, const wchar_t* argv[])
{
    setlocale(LC_ALL, "");

    LPCTSTR filename = L"G:\\Evidences\\Dumps\\XFSDump\\xfs.raw";
    if (argc > 1)
        filename = argv[1];

    if (GetXFSHelper()->Initialize()) {
        
        if (GetXFSHelper()->Mount(NULL, "xfs_mp"))
        {
            GetXFSHelper()->Open(filename);
            //GetXFSHelper()->PrintVolumeInfo();
            //GetXFSHelper()->Dir();
            GetXFSHelper()->PrintHierarchy();
            GetXFSHelper()->Close();
        }
    }
}
