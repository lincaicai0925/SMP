
#ifndef HL_VERSION_INFO_PRINT_HPP_
#define HL_VERSION_INFO_PRINT_HPP_
#include "labez_version_info.hh"
#include "labez_build_info.hh"
#include <stdio.h>
#include <assert.h>

static char * LabEZ_VersionInfoToString(int utf_8)
{
    static char buffer[512] = { 0 };
    if (!buffer[0])
    {
        if(utf_8)
        {
            sprintf(buffer, /*{u8"%s\n%s\n文件版本:%d.%d.%d.%d\n产品版本:%d.%d.%d.%d\n源码版本:%s\n构建时间:%s\n公司主页:%s\n联系我们:%s"}:*/"\x25\x73\x5c\x6e\x25\x73\x5c\x6e\xe6\x96\x87\xe4\xbb\xb6\xe7\x89\x88\xe6\x9c\xac\x3a\x25\x64\x2e\x25\x64\x2e\x25\x64\x2e\x25\x64\x5c\x6e\xe4\xba\xa7\xe5\x93\x81\xe7\x89\x88\xe6\x9c\xac\x3a\x25\x64\x2e\x25\x64\x2e\x25\x64\x2e\x25\x64\x5c\x6e\xe6\xba\x90\xe7\xa0\x81\xe7\x89\x88\xe6\x9c\xac\x3a\x25\x73\x5c\x6e\xe6\x9e\x84\xe5\xbb\xba\xe6\x97\xb6\xe9\x97\xb4\x3a\x25\x73\x5c\x6e\xe5\x85\xac\xe5\x8f\xb8\xe4\xb8\xbb\xe9\xa1\xb5\x3a\x25\x73\x5c\x6e\xe8\x81\x94\xe7\xb3\xbb\xe6\x88\x91\xe4\xbb\xac\x3a\x25\x73",
                HL_Module_Name_CHS, HL_Copyright_CHS,
                HL_Ver_Major, HL_Ver_Minor, HL_Ver_Release, HL_Ver_Build,
                HL_Product_Ver_Field1, HL_Product_Ver_Field2, HL_Product_Ver_Field3, HL_Product_Ver_Field4,
                HL_Src_Version, HL_Build_Time, HL_Web, HL_Email
            );
        }
        else
        {
            sprintf(buffer, "%s\n%s\n文件版本:%d.%d.%d.%d\n产品版本:%d.%d.%d.%d\n源码版本:%s\n构建时间:%s\n公司主页:%s\n联系我们:%s",
                HL_Module_Name_CHS, HL_Copyright_CHS,
                HL_Ver_Major, HL_Ver_Minor, HL_Ver_Release, HL_Ver_Build,
                HL_Product_Ver_Field1, HL_Product_Ver_Field2, HL_Product_Ver_Field3, HL_Product_Ver_Field4,
                HL_Src_Version,HL_Build_Time, HL_Web,HL_Email
            );
        }
        assert(strlen(buffer) < 500);
    }

    return buffer;
}
#endif
