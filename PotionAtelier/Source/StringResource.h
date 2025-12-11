#pragma once
#include <map>
#include <string>
#include "Core/TSingleton.h"
#include <fstream>
#include <filesystem>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
// Memo
// 

class StringResource : public TSingleton<StringResource>
{
	const std::wstring resource_path{ L"./Resource/Strings.txt" };
	std::map<std::wstring, std::wstring> strings;

public:
	StringResource();

public:
	static std::wstring GetTutorialText(std::wstring key)
	{
		auto& instance = GetInstance();
		if (instance.strings.find(key) != instance.strings.end())
		{
			return instance.strings[key];
		}
		else
		{
			return L"";
		}
	}
};


struct CSVFildData
{
	int offset;
	std::function<void(std::string_view value, void* buffer)> parser;
};
#include <cstddef>  // offsetof가 정의된 헤더
/**
 * @brief 
 * 
 *  사용 예시
 *  셀에 , "" 있는거 처리는 없음....
 *  
 * 
struct Test 
{
	int ID;
	std::string Name;
};
	CSVReader reader;
	reader.header["ID"] = { offsetof(Test, ID), [](std::string_view value, void* buffer) { *static_cast<int*>(buffer) = std::stoi(value.data()); } };
	reader.header["Name"] = { offsetof(Test, Name), [](std::string_view value, void* buffer) { *static_cast<std::string*>(buffer) = value.data(); } };

	std::vector<Test> tests;
	std::stringstream sstream;
	sstream <<
R"R(ID,Name
1,t
2,tt
44,asd)R";
	reader.Read(sstream, tests);
 */

struct CSVReader 
{
	std::map<std::string, CSVFildData> header;


    template<class T>
    void Read(std::istream& sstream, std::vector<T>& container)
    {
        std::string line;
        // 1. 첫 번째 행(헤더 행) 읽기
        if (!std::getline(sstream, line))
        {
            std::cerr << "File is empty or header cannot be read.\n";
            return;
        }

        // CSV의 헤더 토큰 분리 (단순 ',' 구분자 기준)
        std::vector<std::string> headerTokens;
        {
            std::istringstream iss(line);
            std::string token;
            while (std::getline(iss, token, ','))
            {
                headerTokens.push_back(token);
            }
        }

        // 헤더 행에 나온 컬럼명과 CSVReader::header에 등록된 필드명을 매칭하여,
        // 각 필드의 CSV 파일 내 인덱스를 기록.
        // (여기서는 별도의 임시 맵을 사용)
        std::unordered_map<std::string, int> fieldColumnIndices;
        for (int i = 0; i < static_cast<int>(headerTokens.size()); ++i)
        {
            // headerTokens[i]가 우리가 처리할 필드에 등록되어 있는지 확인.
            auto it = header.find(headerTokens[i]);
            if (it != header.end())
            {
                fieldColumnIndices[headerTokens[i]] = i;
            }
        }

        // 2. 데이터 행 처리: 각 행마다 T 객체 생성 후, 각 등록 필드에 값을 할당.
        while (std::getline(sstream, line))
        {
            // CSV 파일의 한 행을 ','로 구분하여 토큰화.
            std::vector<std::string> tokens;
            {
                std::istringstream iss(line);
                std::string token;
                while (std::getline(iss, token, ','))
                {
                    tokens.push_back(token);
                }
            }

            T row{}; // 새로운 T 객체 (기본 생성)

            // CSVReader::header에 등록된 각 필드에 대해 값을 대입.
            // fieldColumnIndices에는 CSV 파일의 각 필드명이 몇 번째 컬럼에 위치하는지가 저장되어 있음.
            for (const auto& [fieldName, colIndex] : fieldColumnIndices)
            {
                // CSVFildData 참조: T 객체 내 어느 오프셋에 값을 기록할지, 그리고 파싱 함수는 무엇인지.
                const CSVFildData& fieldData = header[fieldName];

                // 데이터 행에서 해당 컬럼의 값이 존재하는지 체크.
                if (colIndex < static_cast<int>(tokens.size()))
                {
                    // T 객체의 base 주소에, 오프셋을 더해 해당 멤버의 포인터 계산.
                    void* fieldPtr = reinterpret_cast<void*>(reinterpret_cast<char*>(&row) + fieldData.offset);
                    // 파서 함수 호출: 문자열 값을 적절한 타입으로 변환 후 T 객체의 해당 멤버에 대입.
                    fieldData.parser(tokens[colIndex], fieldPtr);
                }
                else
                {
                    std::cerr << "Column index " << colIndex << " out of range in line: " << line << "\n";
                }
            }

            // 완성된 행(row)을 컨테이너에 추가.
            container.push_back(std::move(row));
        }

    }
    template<class T>
    void Read(const std::filesystem::path& path, std::vector<T>& container)
    {
        std::ifstream ifs(path);
        if (ifs.is_open())
        {
            Read(ifs, container);
            ifs.close();
        }
    }
};