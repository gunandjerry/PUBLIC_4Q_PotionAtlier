#include "WinUtility.h"
#include <Core/WinGameApp.h>
#include <tchar.h>
#include <cassert>
#include <cstring>

std::wstring WinUtility::GetOpenFilePath(const wchar_t* fileType, const wchar_t* defaultFileName)
{
	HWND hWnd = WinGameApp::GetHWND();
	if (hWnd)
	{
		OPENFILENAME ofn{};
		wchar_t filter[30];
		const int len = wcslen(fileType);
		wcscpy_s(filter, _ARRAYSIZE(filter), fileType);

		//필터 설정
		wchar_t temp[20] = L"\0*.";
		for (int i = 0; i < len + 1; i++)
		{
			temp[i + 3] = fileType[i]; //널문자를 포함하는 문자열 이여서 전부 순회해야함
		}
		for (int i = 0; i < _ARRAYSIZE(temp) - 1; i++)
		{
			if (unsigned long long(i + len) == _ARRAYSIZE(filter))
				break;
			filter[i + len] = temp[i]; //널문자를 포함하는 문자열 이여서 전부 순회해야함
		}

		wchar_t szFile[256]{}; // 선택된 파일 경로를 저장할 배열
		if (defaultFileName)
		{
			wcscpy_s(szFile, defaultFileName);
		}

		// 파일 경로 선택 창 초기화
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = hWnd;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;// 필터링할 파일 타입 지정
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		ofn.lpstrTitle = L"파일 열기";

		// 파일 경로 선택 창 띄우기
		if (GetOpenFileName(&ofn) == TRUE)
		{
			return ofn.lpstrFile; // 선택한 파일 경로 반환
		}
		else
		{
			return L""; // 사용자가 취소한 경우 빈 문자열 반환
		}
	}

	assert(!"hWnd가 존재하지 않습니다.");
	return L""; 
}

std::wstring WinUtility::GetSaveAsFilePath(const wchar_t* fileType, const wchar_t* defaultFileName)
{
	HWND hWnd = WinGameApp::GetHWND();
	if (hWnd)
	{
		OPENFILENAME ofn{};
		wchar_t filter[30];
		const int len = wcslen(fileType);
		wcscpy_s(filter, _ARRAYSIZE(filter), fileType);

		//필터 설정
		wchar_t temp[20] = L"\0*.";
		for (int i = 0; i < len + 1; i++)
		{
			temp[i + 3] = fileType[i]; //널문자를 포함하는 문자열 이여서 전부 순회해야함
		}
		for (int i = 0; i < _ARRAYSIZE(temp) - 1; i++)
		{
			if (unsigned long long(i + len) == _ARRAYSIZE(filter))
				break;
			filter[i + len] = temp[i]; //널문자를 포함하는 문자열 이여서 전부 순회해야함
		}

		wchar_t szFile[256]{}; // 선택된 파일 경로를 저장할 배열
		if (defaultFileName)
		{
			wcscpy_s(szFile, defaultFileName);
		}

		// 파일 경로 선택 창 초기화
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = hWnd;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;// 필터링할 파일 타입 지정
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.lpstrTitle = L"다른 이름으로 저장";
		ofn.lpstrDefExt = fileType;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

		// 파일 경로 선택 창 띄우기
		if (GetSaveFileName(&ofn) == TRUE)
		{
			return ofn.lpstrFile; // 선택한 파일 경로 반환
		}
		else
		{
			return L""; // 사용자가 취소한 경우 빈 문자열 반환
		}
	}

	assert(!"hWnd가 존재하지 않습니다.");
	return L"";
}

bool WinUtility::ShowConfirmationDialog(const wchar_t* title, const wchar_t* text)
{
	// MessageBox 함수 호출
	int result = MessageBox
	(
		WinGameApp::GetHWND(),
		text,    
		title,   
		MB_YESNO | MB_ICONQUESTION
	);

	if (result == IDYES)
	{
		return true;
	}
	else 
	{
		return false;
	}
}