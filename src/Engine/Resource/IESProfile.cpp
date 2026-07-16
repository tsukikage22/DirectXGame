#include "Engine/Resource/IESProfile.h"

#include <DirectXMath.h>

#include <fstream>

#include "Engine/Core/DescriptorPool.h"
#include "Engine/Core/DxDebug.h"

namespace {

//-----------------------------------------------
// Constants
//-----------------------------------------------
constexpr int TypeC = 1;  // C-Plane

/// @brief IESプロファイルの読み込み
bool LoadIESProfile(
    const std::filesystem::path& path, IESProfileData& outProfileData) {
    // ファイルのオープン
    std::ifstream stream(path);
    if (!stream) {
        OutputDebugStringW(L"Failed to open IES file.\n");
        return false;
    }

    std::string token;
    stream >> token;

    // フォーマット確認
    if (token != "IESNA:LM-63-2002" && token != "IESNA:LM-63-1995") {
        OutputDebugStringW(L"Unsupported IES format.\n");
        return false;
    }

    bool foundTILTNone = false;

    // チルト角情報を探す
    while (stream >> token) {
        // TILT=NONE
        if (token == "TILT=NONE") {
            foundTILTNone = true;
            break;
        }

        // TILT=NONEでない場合は非対応
        if (token == "TILT=") {
            OutputDebugStringW(L"Unsupported IES file with TILT data.\n");
            return false;
        }
    }

    // TILT=NONEが見つからなかった場合はエラー
    if (!foundTILTNone) {
        OutputDebugStringW(L"TILT=NONE not found in IES file.\n");
        return false;
    }

    int angleCountV = 0;
    int angleCountH = 0;
    int futureUse   = 0;

    // 光源情報の読み込み
    stream >> outProfileData.lampCount;          // ランプ数
    stream >> outProfileData.lumensPerLamp;      // ランプあたりの光束
    stream >> outProfileData.candelaMultiplier;  // 乗算係数
    stream >> angleCountV;                       // 垂直角数
    stream >> angleCountH;                       // 水平角数
    stream >> outProfileData.photometricType;    // 測定座標系
    stream >> outProfileData.unitType;           // 単位
    stream >> outProfileData.shapeWidth;         // 形状横幅
    stream >> outProfileData.shapeLength;        // 形状奥行
    stream >> outProfileData.shapeHeight;        // 形状高さ
    stream >> outProfileData.ballastFactor;      // 安定器光出力係数
    stream >> futureUse;                         // 予約領域
    stream >> outProfileData.inputWattage;       // 入力ワット数

    // 複数光源は未対応
    if (outProfileData.lampCount > 1) {
        OutputDebugStringW(L"Multiple lamps are not supported.\n");
        return false;
    }

    // TypeC (C-Plane) のみ対応
    if (outProfileData.photometricType != TypeC) {
        OutputDebugStringW(L"Only Type C photometric data is supported.\n");
        return false;
    }

    // 垂直角の読み込み
    outProfileData.anglesV.resize(angleCountV);
    for (int i = 0; i < angleCountV; i++) {
        stream >> outProfileData.anglesV[i];
    }

    // 水平角の読み込み
    outProfileData.anglesH.resize(angleCountH);
    for (int i = 0; i < angleCountH; i++) {
        stream >> outProfileData.anglesH[i];
    }

    outProfileData.maxCandela = 0.0f;
    float candelaSum          = 0.0f;

    // 光度値の読み込み
    outProfileData.candela.resize(angleCountV * angleCountH);
    for (int h = 0; h < angleCountH; h++) {
        for (int v = 0; v < angleCountV; v++) {
            float value = 0.0f;
            stream >> value;
            auto candela = value * outProfileData.candelaMultiplier;
            outProfileData.candela[h * angleCountV + v] = candela;
            outProfileData.maxCandela =
                DirectX::XMMax(outProfileData.maxCandela, candela);
            candelaSum += candela;
        }
    }
    outProfileData.aveCandela = candelaSum / (angleCountV * angleCountH);

    stream.close();

    return true;
}

/// @brief 角度から浮動小数点インデックスを計算
float GetPos(float value, const std::vector<float>& container) {
    // containerのサイズが1の場合
    if (container.size() == 1) {
        return 0.0f;
    }

    // 範囲チェック
    if (value < container.front() || value > container.back()) {
        return -1.0f;
    }

    // 二分探索でvalueがcontainerのどこに位置するかを探す
    size_t left  = 0;
    size_t right = container.size() - 1;
    while (left < right) {
        int mid      = (left + right + 1) / 2;
        float midVal = container[mid];

        if (value >= midVal) {
            left = mid;
        } else {
            right = mid - 1;
        }
    }

    // leftとrightの間のどこにvalueが位置するかを計算する
    float t = 0.0f;
    if (left + 1 < container.size()) {
        float leftVal  = container[left];
        float rightVal = container[left + 1];
        float delta    = rightVal - leftVal;

        if (delta > 1e-5f) {
            t = (value - leftVal) / delta;
        }
    }

    return static_cast<float>(left + t);
}

/// @brief カンデラ値の取得
/// @param x 垂直角のインデックス
/// @param y 水平角のインデックス
float GetCandela(int x, int y, const IESProfileData& profileData) {
    int v = int(profileData.anglesV.size());
    int h = int(profileData.anglesH.size());

    // インデックスが範囲外に出たときに先頭に戻す処理
    x %= v;
    y %= h;

    int index = y * v + x;
    assert(index < profileData.candela.size());

    return profileData.candela[index];
}

/// @brief 周囲4点の値から補完するバイリニアサンプリング
float BilinearSample(float x, float y, const IESProfileData& profileData) {
    // 補間に使う4点を作成
    int x0 = int(floor(x));
    int y0 = int(floor(y));
    int x1 = x0 + 1;
    int y1 = y0 + 1;

    // x, yの周囲4点との位置関係
    float tx = x - x0;
    float ty = y - y0;

    // 周囲4点のカンデラ値
    float c00 = GetCandela(x0, y0, profileData);
    float c01 = GetCandela(x0, y1, profileData);
    float c10 = GetCandela(x1, y0, profileData);
    float c11 = GetCandela(x1, y1, profileData);

    // y方向の補間
    float c0 = c00 * (1.0f - ty) + c01 * ty;
    float c1 = c10 * (1.0f - ty) + c11 * ty;

    return c0 * (1.0f - tx) + c1 * tx;
}

/// @brief カンデラ値の補間
float Interpolate(
    float angleV, float angleH, const IESProfileData& profileData) {
    // 最大範囲でチェック
    assert(0 <= angleV && angleV <= 180.0f);
    assert(0 <= angleH && angleH <= 360.0f);

    // angleVとangleHが配列のどの位置にあるか（インデックス）を求める
    auto s = GetPos(angleV, profileData.anglesV);
    auto t = GetPos(angleH, profileData.anglesH);

    // インデックスが範囲外の場合は0を返す
    if (s < 0.0f || t < 0.0f) {
        return 0.0f;
    }

    // バイリニアサンプリングで補間
    return BilinearSample(s, t, profileData);
}

/// @brief テクスチャサイズの計算
int ComputeTextureSize(const IESProfileData& profileData) {
    // 128, anglesVの数, anglesHの数の中で最大の値を選び，
    // それを超える最小の2のべき乗をテクスチャサイズとする
    auto size = 128;
    size      = DirectX::XMMax(size, int(profileData.anglesV.size()));
    size      = DirectX::XMMax(size, int(profileData.anglesH.size()));

    // 128pxから2次元テクスチャの最大値までの範囲で、最も近い2のべき乗を求める
    bool find = false;
    for (int i = 7; i <= D3D12_MAX_TEXTURE_DIMENSION_2_TO_EXP; i++) {
        auto lhs = exp2(i - 1);
        auto rhs = exp2(i);
        if (lhs < size && size <= rhs) {
            size = int(rhs);
            find = true;
            break;
        }
    }

    if (!find) {
        OutputDebugStringW(
            L"Failed to determine texture size for IES profile.\n");
        return -1;
    }

    return size;
}

/// @brief IESProfileのカンデラ値からテクセルへの変換
std::vector<float> BuildPixels(
    const IESProfileData& profileData, int w, int h) {
    // カンデラ値を格納する配列の作成
    std::vector<float> pixels(w * h, 0.0f);  // テクセル

    // カンデラ値の補間と正規化に使うための値の計算
    auto invW   = 1.0f / float(w);
    auto invH   = 1.0f / float(h);
    auto invAve = 1.0f / profileData.aveCandela;

    // profileDataに格納された水平角の最大値
    auto lastH = profileData.anglesH.back();

    // カンデラ値の補間と正規化
    for (auto j = 0; j < h; j++) {
        auto angleH = 0.0f;

        // テクスチャの縦方向jを0~360度の水平角に対応させる
        // profileDataには90度までや180度までの水平角しかない場合があるため、360度に対応させるための処理を行う
        // 配光が対象であることを前提とした処理であることに注意
        if (lastH > 0.0f) {
            angleH = j * invH * 360.0f;
            angleH = fmod(angleH, 2.0f * lastH);
            if (angleH > lastH) {
                angleH = lastH * 2.0f - angleH;
            }
        }

        for (auto i = 0; i < w; i++) {
            // テクスチャの横方向iを0~180度の垂直角に対応させる
            // iを-1~1の範囲に変換し，acosで角度に戻している
            auto rad    = i * invW * 2.0f - 1.0f;
            auto angleV = DirectX::XMConvertToDegrees(acos(rad));

            // 補間関数を呼び出してカンデラ値を求める
            // 平均値を使った正規化
            auto cd = invAve * Interpolate(angleV, angleH, profileData);

            // データ格納
            pixels[j * w + i] = cd;
        }
    }

    return pixels;
}

}  // namespace

//------------------------------------------------
// IESProfile class
//------------------------------------------------
IESProfile::IESProfile() : m_pPoolSRV(nullptr) {}

IESProfile::~IESProfile() { Term(); }

/// @brief 初期化処理
bool IESProfile::Init(ID3D12Device* pDevice, DescriptorPool* pPool,
    std::filesystem::path path, DirectX::ResourceUploadBatch& batch) {
    // 引数チェック
    if (!pDevice || !pPool || path.empty()) {
        OutputDebugStringW(L"Invalid arguments to IESProfile::Init.\n");
        return false;
    }

    // 二重呼び出し時のリソース開放
    Term();

    m_pPoolSRV = pPool;

    // IESプロファイルの読み込み
    IESProfileData profileData;
    if (!LoadIESProfile(path, profileData)) {
        OutputDebugStringW(L"Failed to load IES profile data.\n");
        return false;
    }

    // テクスチャのサイズの決定
    int size = ComputeTextureSize(profileData);
    if (size < 0) {
        return false;
    }

    // テクセルを格納する配列の作成
    auto w      = size;
    auto h      = size;
    auto pixels = BuildPixels(profileData, w, h);

    // pixelsからテクスチャを作成
    // リソースの生成
    if (!m_texture.InitAsTexture2D(pDevice, w, h, DXGI_FORMAT_R32_FLOAT, 1,
            D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST)) {
        return false;
    }

    // SRVインデックスの確保
    m_srv = m_pPoolSRV->Allocate();

    // SRVディスクリプタの設定
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension                   = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Format                          = DXGI_FORMAT_R32_FLOAT;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels     = 1;
    srvDesc.Texture2D.MostDetailedMip     = 0;
    srvDesc.Texture2D.PlaneSlice          = 0;
    srvDesc.Texture2D.ResourceMinLODClamp = 0;

    // SRVの生成
    pDevice->CreateShaderResourceView(
        m_texture.GetResource(), &srvDesc, m_srv.GetCPUHandle());

    // ResourceUploadBatchでアップロード
    D3D12_SUBRESOURCE_DATA subRes = {};
    subRes.RowPitch               = w * sizeof(float);
    subRes.SlicePitch             = h * subRes.RowPitch;
    subRes.pData                  = pixels.data();

    batch.Upload(m_texture.GetResource(), 0, &subRes, 1);
    batch.Transition(m_texture.GetResource(), D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    return true;
}

void IESProfile::Term() { m_pPoolSRV = nullptr; }

D3D12_GPU_DESCRIPTOR_HANDLE IESProfile::GetSrvGpuHandle() const {
    if (m_srv.IsValid() && m_pPoolSRV) {
        return m_srv.GetGPUHandle();
    }

    return {};
}