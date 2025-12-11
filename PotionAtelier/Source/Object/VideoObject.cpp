#include "VideoObject.h"
#include <mfmediaengine.h>
#include <mfapi.h>
#include <mfobjects.h>
#include <mferror.h>
#include <mfidl.h>
#include <windows.h>
#include <iostream>
#include <filesystem>
#include <dcomp.h>
#include <Object\BackGroundMusicObject.h>

#pragma comment ( lib, "mfplat.lib")
#pragma comment ( lib, "Mf.lib")
#pragma comment ( lib, "dcomp.lib")

std::atomic<bool> load_complete = false;
std::atomic<bool> Video_complete = false;
class MediaEngineNotify : public IMFMediaEngineNotify
{
private:
	std::atomic<ULONG> refCount; // 참조 카운트를 위한 변수

public:
	MediaEngineNotify() : refCount(1) {} // 초기화

	// IMFMediaEngineNotify을(를) 통해 상속됨
	HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override
	{
		if (__uuidof(IMFMediaEngineNotify) == riid)
		{
			*ppvObject = static_cast<IMFMediaEngineNotify*>(this);
			AddRef(); // 참조 카운트 증가
			return S_OK;
		}

		*ppvObject = nullptr;
		return E_NOINTERFACE;
	}

	ULONG __stdcall AddRef(void) override
	{
		return refCount.fetch_add(1) + 1; // 참조 카운트를 증가시키고 반환
	}

	ULONG __stdcall Release(void) override
	{
		ULONG newCount = refCount.fetch_sub(1) - 1; // 참조 카운트를 감소시키고 반환
		if (newCount == 0)
		{
			delete this; // 참조 카운트가 0이 되면 객체를 삭제
		}
		return newCount;
	}

	HRESULT __stdcall EventNotify(DWORD event, DWORD_PTR param1, DWORD param2) override
	{
		// 이벤트 처리 로직을 여기에 추가
		switch (event)
		{
		case MF_MEDIA_ENGINE_EVENT_LOADEDMETADATA:
			break;
		case MF_MEDIA_ENGINE_EVENT_ERROR:
			// 오류 발생 시 처리
			break;
			// 추가 이벤트 케이스를 여기에 추가할 수 있습니다.
		default:
			break;
		}
		return S_OK; // 성공적으로 이벤트를 처리한 경우
	}
};
class VideoComponent : public Component
{
public:
    virtual ~VideoComponent()
    {
		HRESULT result;

		if (attributes)
		{
			attributes->DeleteAllItems();
			result = mediaEngine->Shutdown();
			attributes.ReleaseAndGetAddressOf();
			mediaEngine.ReleaseAndGetAddressOf();


			isCreditEnd = true;
			result = MFShutdown();
		}

		th.join();
    }
	virtual void Awake() {}

	ComPtr<IDCompositionDevice> dcompDevice;
	ComPtr<IDCompositionTarget> dCompositionTarget;
	ComPtr<IDCompositionVisual> dCompositionVisual;
	ComPtr<IMFAttributes> attributes;
	ComPtr<IMFMediaEngine> mediaEngine;
	virtual void Start() 
	{

		HRESULT result = MFStartup(MF_VERSION);

		ComPtr<IMFMediaEngineClassFactory> mediaEngineFactory;
		ComPtr<IMFMediaEngineNotify > unknown = new MediaEngineNotify();
		ComPtr<IDXGIDevice> dxgiDevice;
		RendererUtility::GetDevice()->QueryInterface(IID_PPV_ARGS(dxgiDevice.GetAddressOf()));
		result = DCompositionCreateDevice(dxgiDevice.Get(), __uuidof(IDCompositionDevice), (void**)&dcompDevice);
		ComPtr<IDCompositionMatrixTransform> sacle;
		dcompDevice->CreateMatrixTransform(&sacle);
		result = dcompDevice->CreateTargetForHwnd(D3D11_GameApp::GetHWND(), false, &dCompositionTarget);
		result = dcompDevice->CreateVisual(&dCompositionVisual);
		result = dCompositionTarget->SetRoot(dCompositionVisual.Get());
		sacle->SetMatrix(D2D1::Matrix3x2F::Scale(1920.0f / 640.0f, 1080.0f / 480.0f));
		dCompositionVisual->SetTransform(sacle.Get());
		//CreateTargetForHwnd로 만든 dCompositionTarget의 사이즈가 640.0f, 480.0f....
	//	result = dCompositionVisual->SetTransform(D2D1::Matrix3x2F::Scale(1920.0f / 640.0f, 1080.0f / 480.0f));




		result = MFCreateAttributes(&attributes, 0);
		result = attributes->SetUnknown(MF_MEDIA_ENGINE_PLAYBACK_VISUAL, dCompositionVisual.Get());
		//result = attributes->SetUINT64(MF_MEDIA_ENGINE_PLAYBACK_HWND, (UINT64)DG::Core::GetInstance().GetWindowHandle());
		////result = attributes->SetUINT64(MF_MEDIA_ENGINE_OPM_HWND, (UINT64)DG::Core::GetInstance().GetWindowHandle());
		////result = attributes->SetUINT32(MF_MEDIA_ENGINE_VIDEO_OUTPUT_FORMAT, DXGI_FORMAT_B8G8R8A8_UNORM);
		result = attributes->SetUnknown(MF_MEDIA_ENGINE_CALLBACK, unknown.Get());


		result = CoCreateInstance(CLSID_MFMediaEngineClassFactory, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&mediaEngineFactory));
		result = mediaEngineFactory->CreateInstance(MF_MEDIA_ENGINE_REAL_TIME_MODE | MF_MEDIA_ENGINE_DISABLE_LOCAL_PLUGINS, attributes.Get(), &mediaEngine);

		std::wstring wstr = std::wstring(videoPath.begin(), videoPath.end());
		result = mediaEngine->SetSource(wstr.data());
		result = mediaEngine->Load();
		result = mediaEngine->Play();
		isCreditEnd = false;
		th = std::thread([this]()
						 {
							 while (!isCreditEnd)
							 {
								 if (dcompDevice)
								 {
									 dcompDevice->Commit();
								 }
							 }
						 });
		if (BackGroundMusicObject* bgmObject = GameObject::Find<BackGroundMusicObject>(L"BackGroundMusic"))
		{
			BGM = bgmObject->component;
		}
		if (BGM)
		{
			BGM->Stop();
		}
	}
	std::thread th;
	bool isCreditEnd;
	BackGroundMusicComponent* BGM = nullptr;
protected:
	virtual void FixedUpdate() {}
	virtual void Update() 
	{

#ifdef _EDITOR
		if (!Scene::EditorSetting.IsPlay()) return;
#endif 

		isCreditEnd = mediaEngine->GetCurrentTime() >= mediaEngine->GetDuration();

		auto& input = inputManager.input;
		;

		if (input.IsKeyDown(KeyboardKeys::Escape) || isCreditEnd)
		{
			isCreditEnd = true;
			if (nextScenenPath.size())
			{
				std::wstring wstr = std::wstring(nextScenenPath.begin(), nextScenenPath.end());
				TimeSystem::Time.DelayedInvok([wstr]()
											  {
												  sceneManager.LoadScene(wstr.c_str());
											  }, 0.f);			
			}
			else
			{
				if (BGM)
				{
					BGM->Play();
				}
				GameObject::Destroy(gameObject);
			}
		}

	}
	virtual void LateUpdate() {}
	virtual void Render() {}
	virtual void InspectorImguiDraw() 
	{
		ImGui::InputText("Next Scene Path", (char*)nextScenenPath.c_str(), nextScenenPath.size(), ImGuiInputTextFlags_CallbackResize,
						 [](ImGuiInputTextCallbackData* data)
						 {
							 if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
							 {
								 std::string* str = (std::string*)data->UserData;
								 IM_ASSERT(data->Buf == str->c_str());
								 str->resize(data->BufTextLen);
								 data->Buf = str->data();
							 }
							 return 0;
						 }, &nextScenenPath);

		ImGui::InputText("Video Path", (char*)videoPath.c_str(), videoPath.size(), ImGuiInputTextFlags_CallbackResize,
						 [](ImGuiInputTextCallbackData* data)
						 {
							 if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
							 {
								 std::string* str = (std::string*)data->UserData;
								 IM_ASSERT(data->Buf == str->c_str());
								 str->resize(data->BufTextLen);
								 data->Buf = str->data();
							 }
							 return 0;
						 }, & videoPath);




	}

	virtual void Serialized(std::ofstream& ofs) 
	{
		Binary::Write::string(ofs, nextScenenPath);
		Binary::Write::string(ofs, videoPath);
	}
	
	virtual void Deserialized(std::ifstream& ifs) 
	{
		nextScenenPath = Binary::Read::string(ifs);
		videoPath = Binary::Read::string(ifs);

	}
	std::string nextScenenPath;
	std::string videoPath;
};


VideoObject::VideoObject()
{
	AddComponent<VideoComponent>();
	AddComponent<UIRenderComponenet>();
}

void VideoObject::Awake()
{
}
