#include "StringResource.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <regex>
#include "../D3D11_Engine/Source/Utility/utfConvert.h"

StringResource::StringResource()
{
    std::ifstream file(resource_path, std::ios_base::binary);
    if (!file)
    {
        printf("Failed to open string resource file.\n");
        return;
    }
    
    file.seekg(0, std::ios::end);
    std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string buffer;
    buffer.resize(file_size);;
    file.read(reinterpret_cast<char*>(buffer.data()), file_size);

    std::wstring _ws;
    _ws = utfConvert::utf8_to_wstring(buffer);

    file.close();

    std::wregex pattern(L"([^|]+)\\|[^|]+\\|([^|]+)\r?\n");
    std::wsmatch match;

    std::wstring::const_iterator searchStart(_ws.cbegin());

    // 정규식 검색 반복
    while (std::regex_search(searchStart, _ws.cend(), match, pattern))
    {
        std::wstring first = match[1].str();  // A
        std::wstring third = match[2].str().substr(0, match[2].str().size() - 1);  // C

        size_t pos = 0;
        while ((pos = third.find(L"\\n", pos)) != std::wstring::npos)
        {
            third.replace(pos, 2, L"\n");
            pos += 1;
        }
        strings[first] = third;

        // 검색 시작 위치를 매칭된 끝으로 이동
        searchStart = match.suffix().first;
    }

    int a = 10;
}
