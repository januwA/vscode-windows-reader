#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <sapi.h>
#include <iostream>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "uuid.lib")

int main(int argc, char* argv[]) {
    // 1. 基础检查：至少需要 argv[1] (待读文本)
    if (argc < 2) {
        std::cerr << "用法: speech.exe \"文本\" [语速] [音量]" << std::endl;
        return 1;
    }

    // 2. 解析参数：使用 atol 处理窄字符 (char*)
    long rate = 0;      // 默认语速: -10 到 10
    long volume = 100;  // 默认音量: 0 到 100

    if (argc >= 3) {
        rate = atol(argv[2]);
    }
    if (argc >= 4) {
        volume = atol(argv[3]);
    }

    // 3. 初始化 COM 环境
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        if (hr == RPC_E_CHANGED_MODE) {
            std::cerr << "CoInitializeEx: 线程已用不同并发模型初始化 (RPC_E_CHANGED_MODE)" << std::endl;
        } else {
            std::cerr << "CoInitializeEx 失败: 0x" << std::hex << hr << std::endl;
        }
        return 1;
    }
    bool comInitialized = true;

    IMMDeviceEnumerator* pEnumerator = NULL;
    IMMDevice* pDevice = NULL;
    IAudioClient2* pAudioClient = NULL;

    // 4. 获取音频通信设备并激活 IAudioClient2
    hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    if (FAILED(hr) || pEnumerator == nullptr) {
        std::cerr << "CoCreateInstance(MMDeviceEnumerator) 失败: 0x"
                  << std::hex << hr << std::endl;
        if (pEnumerator) { pEnumerator->Release(); pEnumerator = nullptr; }
        CoUninitialize();
        return 1;
    }

    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eCommunications, &pDevice);
    if (FAILED(hr) || pDevice == nullptr) {
        std::cerr << "GetDefaultAudioEndpoint 失败: 0x" << std::hex << hr << std::endl;
        if (pDevice) { pDevice->Release(); pDevice = nullptr; }
        pEnumerator->Release();
        CoUninitialize();
        return 1;
    }

    hr = pDevice->Activate(__uuidof(IAudioClient2), CLSCTX_ALL, NULL, (void**)&pAudioClient);
    if (FAILED(hr) || pAudioClient == nullptr) {
        std::cerr << "Activate(IAudioClient2) 失败: 0x" << std::hex << hr << std::endl;
        if (pAudioClient) { pAudioClient->Release(); pAudioClient = nullptr; }
        pDevice->Release();
        pEnumerator->Release();
        CoUninitialize();
        return 1;
    }

    if (pAudioClient) {
        // 5. 核心：开启 Ducking (音频衰减) 机制
        AudioClientProperties prop = { 0 };
        prop.cbSize = sizeof(AudioClientProperties);
        prop.eCategory = AudioCategory_Communications;
        pAudioClient->SetClientProperties(&prop);

        // 启动静默流以保持会话处于 Active 状态
        WAVEFORMATEX* pwfx = NULL;
        pAudioClient->GetMixFormat(&pwfx);
        pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, 1000000, 0, pwfx, NULL);
        pAudioClient->Start();

        // 6. 执行 SAPI 朗读
        ISpVoice* pVoice = NULL;
        if (SUCCEEDED(CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void**)&pVoice))) {
            // 设置语速和音量
            pVoice->SetRate(rate);
            pVoice->SetVolume((USHORT)volume);

            // UTF8 编码转为宽字符 (Unicode)
            int len = MultiByteToWideChar(CP_UTF8, 0, argv[1], -1, NULL, 0);
            wchar_t* wtext = new wchar_t[len];
            MultiByteToWideChar(CP_UTF8, 0, argv[1], -1, wtext, len);

            std::cout << "TTS 发声中 (Rate: " << rate << ", Vol: " << volume << ")..." << std::endl;

            // 同步朗读：读完才会执行后面的 Stop
            pVoice->Speak(wtext, SPF_DEFAULT, NULL);

            delete[] wtext;
            pVoice->Release();
        }

        // 7. 停止音频流，背景音会自动恢复
        pAudioClient->Stop();
        CoTaskMemFree(pwfx);
        pAudioClient->Release();
    }

    // 8. 资源清理
    if (pDevice) pDevice->Release();
    if (pEnumerator) pEnumerator->Release();
    CoUninitialize();

    return 0;
}