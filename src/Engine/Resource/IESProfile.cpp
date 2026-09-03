#include "Engine/Resource/IESProfile.h"

#include <DirectXMath.h>

#include <algorithm>
#include <fstream>

#include "Engine/Core/DescriptorPool.h"
#include "Engine/Core/DxDebug.h"
#include "Engine/Core/GraphicsDevice.h"

namespace
{

//-----------------------------------------------
// Constants
//-----------------------------------------------
constexpr int TypeC = 1; // C-Plane

/// @brief IESプロファイルの読み込み
bool LoadIESProfile(const std::filesystem::path& path, IESProfileData& outProfileData)
{
    // ファイルのオープン
    std::ifstream stream(path);
    if (!stream)
    {
        OutputDebugStringW(L"Failed to open IES file.\n");
        return false;
    }

    std::string token;
    stream >> token;

    // フォーマット確認
    if (token != "IESNA:LM-63-2002" && token != "IESNA:LM-63-1995")
    {
        OutputDebugStringW(L"Unsupported IES format.\n");
        return false;
    }

    bool foundTILTNone = false;

    // チルト角情報を探す
    while (stream >> token)
    {
        // TILT=NONE
        if (token == "TILT=NONE")
        {
            foundTILTNone = true;
            break;
        }

        // TILT=NONEでない場合は非対応
        if (token == "TILT=")
        {
            OutputDebugStringW(L"Unsupported IES file with TILT data.\n");
            return false;
        }
    }

    // TILT=NONEが見つからなかった場合はエラー
    if (!foundTILTNone)
    {
        OutputDebugStringW(L"TILT=NONE not found in IES file.\n");
        return false;
    }

    int angleCountV = 0;
    int angleCountH = 0;
    int futureUse   = 0;

    // 光源情報の読み込み
    stream >> outProfileData.lampCount;         // ランプ数
    stream >> outProfileData.lumensPerLamp;     // ランプあたりの光束
    stream >> outProfileData.candelaMultiplier; // 乗算係数
    stream >> angleCountV;                      // 垂直角数
    stream >> angleCountH;                      // 水平角数
    stream >> outProfileData.photometricType;   // 測定座標系
    stream >> outProfileData.unitType;          // 単位
    stream >> outProfileData.shapeWidth;        // 形状横幅
    stream >> outProfileData.shapeLength;       // 形状奥行
    stream >> outProfileData.shapeHeight;       // 形状高さ
    stream >> outProfileData.ballastFactor;     // 安定器光出力係数
    stream >> futureUse;                        // 予約領域
    stream >> outProfileData.inputWattage;      // 入力ワット数

    // 複数光源は未対応
    if (outProfileData.lampCount > 1)
    {
        OutputDebugStringW(L"Multiple lamps are not supported.\n");
        return false;
    }

    // TypeC (C-Plane) のみ対応
    if (outProfileData.photometricType != TypeC)
    {
        OutputDebugStringW(L"Only Type C photometric data is supported.\n");
        return false;
    }

    // 垂直角の読み込み
    outProfileData.anglesV.resize(angleCountV);
    for (int i = 0; i < angleCountV; i++)
    {
        stream >> outProfileData.anglesV[i];
    }

    // 水平角の読み込み
    outProfileData.anglesH.resize(angleCountH);
    for (int i = 0; i < angleCountH; i++)
    {
        stream >> outProfileData.anglesH[i];
    }

    outProfileData.maxCandela = 0.0f;

    // 光度値の読み込み
    outProfileData.candela.resize(angleCountV * angleCountH);
    for (int h = 0; h < angleCountH; h++)
    {
        for (int v = 0; v < angleCountV; v++)
        {
            float value = 0.0f;
            stream >> value;
            auto candela                                = value * outProfileData.candelaMultiplier;
            outProfileData.candela[h * angleCountV + v] = candela;
            outProfileData.maxCandela                   = DirectX::XMMax(outProfileData.maxCandela, candela);
        }
    }

    stream.close();

    return true;
}

/// @brief 角度から浮動小数点インデックスを計算
float GetPos(float value, const std::vector<float>& container)
{
    // containerのサイズが1の場合
    if (container.size() == 1)
    {
        return 0.0f;
    }

    // 範囲チェック
    if (value < container.front() || value > container.back())
    {
        return -1.0f;
    }

    // 二分探索でvalueがcontainerのどこに位置するかを探す
    size_t left  = 0;
    size_t right = container.size() - 1;
    while (left < right)
    {
        int mid      = (left + right + 1) / 2;
        float midVal = container[mid];

        if (value >= midVal)
        {
            left = mid;
        }
        else
        {
            right = mid - 1;
        }
    }

    // leftとrightの間のどこにvalueが位置するかを計算する
    float t = 0.0f;
    if (left + 1 < container.size())
    {
        float leftVal  = container[left];
        float rightVal = container[left + 1];
        float delta    = rightVal - leftVal;

        if (delta > 1e-5f)
        {
            t = (value - leftVal) / delta;
        }
    }

    return static_cast<float>(left + t);
}

/// @brief カンデラ値の取得
/// @param x 垂直角のインデックス
/// @param y 水平角のインデックス
float GetCandela(int x, int y, const IESProfileData& profileData)
{
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
float BilinearSample(float x, float y, const IESProfileData& profileData)
{
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
float Interpolate(float angleV, float angleH, const IESProfileData& profileData)
{
    // 最大範囲でチェック
    assert(0 <= angleV && angleV <= 180.0f);
    assert(0 <= angleH && angleH <= 360.0f);

    // angleVとangleHが配列のどの位置にあるか（インデックス）を求める
    auto s = GetPos(angleV, profileData.anglesV);
    auto t = GetPos(angleH, profileData.anglesH);

    // インデックスが範囲外の場合は0を返す
    if (s < 0.0f || t < 0.0f)
    {
        return 0.0f;
    }

    // バイリニアサンプリングで補間
    return BilinearSample(s, t, profileData);
}

/// @brief IESProfileのカンデラ値からテクセルへの変換
std::vector<float> BuildPixels(const IESProfileData& profileData, int w, int h, float& outMeanCandela)
{
    // カンデラ値を格納する配列の作成
    std::vector<float> pixels(w * h, 0.0f); // テクセル

    // カンデラ値の補間と正規化に使うための値の計算
    auto invW = 1.0f / float(w);
    auto invH = 1.0f / float(h);

    // profileDataに格納された水平角の最大値
    auto lastH = profileData.anglesH.back();

    // カンデラ値の補間と正規化
    for (auto j = 0; j < h; j++)
    {
        auto angleH = 0.0f;

        // テクスチャの縦方向jを0~360度の水平角に対応させる
        // profileDataには90度までや180度までの水平角しかない場合があるため、360度に対応させるための処理を行う
        // 配光が対象であることを前提とした処理であることに注意
        if (lastH > 0.0f)
        {
            angleH = (j + 0.5f) * invH * 360.0f;
            angleH = fmod(angleH, 2.0f * lastH);
            if (angleH > lastH)
            {
                angleH = lastH * 2.0f - angleH;
            }
        }

        // 補間関数を使い，テクセル配列をカンデラ値で埋める
        for (auto i = 0; i < w; i++)
        {
            // テクスチャの横方向iを0~180度の垂直角に対応させる=垂直角のコサイン
            // iを-1~1の範囲に変換し，acosで角度に戻している
            // 1テクセルが担う立体角を一定にするため，テクスチャの横方向を垂直角のコサインに線形にする
            // また，各テクセルが左端でなく中央で角度をサンプリングするように0.5を足している
            //  （サンプラーはテクセルの中心をサンプリングするため）
            auto cosTheta = (i + 0.5f) * invW * 2.0f - 1.0f;
            auto angleV   = DirectX::XMConvertToDegrees(acos(cosTheta));

            // 補間関数を呼び出してカンデラ値を求める
            auto cd = Interpolate(angleV, angleH, profileData);

            // データ格納
            pixels[j * w + i] = cd;
        }
    }

    // 全球平均光度の計算
    // 横軸がcosθ，縦軸がφに線形なので，このグリッドは
    // 立体角 dΩ = d(cosθ)dφ について等間隔である．
    // よって単純平均がそのまま (1/4π)∫I dΩ = Φ/(4π) になる
    double sum = 0.0f;
    for (int j = 0; j < h; j++)
    {
        for (int i = 0; i < w; i++)
        {
            sum += pixels[j * w + i];
        }
    }
    outMeanCandela = static_cast<float>(sum / (w * h));

    // 平均光度で正規化
    assert(outMeanCandela > 0.0f && "Average candela is zero, cannot normalize.");
    float invAve = 1.0f / (std::max)(outMeanCandela, 1e-6f);
    for (int j = 0; j < h; j++)
    {
        for (int i = 0; i < w; i++)
        {
            pixels[j * w + i] *= invAve;
        }
    }

    return pixels;
}

} // namespace

//------------------------------------------------
// IESProfile class
//------------------------------------------------

IESProfile::~IESProfile()
{
    Term();
}

/// @brief 初期化処理
bool IESProfile::Init(GraphicsDevice& graphicsDevice)
{
    // 二重呼び出し時のリソース開放
    Term();

    m_pPoolSRV = graphicsDevice.CbvSrvUavPool();
    m_pDevice  = graphicsDevice.GetDevice();

    // Texture2DArrayの作成
    // リソースの生成
    if (!m_textureArray.InitAsTexture2DArray(m_pDevice, kWidth, kHeight, DXGI_FORMAT_R32_FLOAT, kMaxIESProfiles, 1,
            D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE))
    {
        return false;
    }

    // SRVインデックスの確保
    m_srv = m_pPoolSRV->Allocate();
    if (!m_srv.IsValid())
    {
        OutputDebugStringW(L"Failed to allocate SRV descriptor for IESProfile.\n");
        return false;
    }

    // SRVディスクリプタの設定
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc    = {};
    srvDesc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
    srvDesc.Format                             = DXGI_FORMAT_R32_FLOAT;
    srvDesc.Shader4ComponentMapping            = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2DArray.MipLevels           = 1;
    srvDesc.Texture2DArray.MostDetailedMip     = 0;
    srvDesc.Texture2DArray.FirstArraySlice     = 0;
    srvDesc.Texture2DArray.ArraySize           = kMaxIESProfiles;
    srvDesc.Texture2DArray.PlaneSlice          = 0;
    srvDesc.Texture2DArray.ResourceMinLODClamp = 0;

    m_pDevice->CreateShaderResourceView(m_textureArray.GetResource(), &srvDesc, m_srv.GetCPUHandle());

    return true;
}

void IESProfile::Term()
{
    m_pPoolSRV = nullptr;
    m_pDevice  = nullptr;
    m_count    = 0;
    m_srv      = {};
    m_textureArray.Term();
}

std::optional<uint32_t> IESProfile::CreateIESTexture(
    const std::filesystem::path& path, DirectX::ResourceUploadBatch& batch)
{
    // テクスチャの最大数を超えた場合は何もしない
    if (m_count >= kMaxIESProfiles)
    {
        OutputDebugStringW(L"Maximum number of IES profiles reached.\n");
        return std::nullopt;
    }

    // IESプロファイルの読み込み
    IESProfileData profileData;
    if (!LoadIESProfile(path, profileData))
    {
        OutputDebugStringW(L"Failed to load IES profile data.\n");
        return std::nullopt;
    }

    // 角度サンプル数がテクスチャサイズを超えた場合
    if (profileData.anglesV.size() > kWidth || profileData.anglesH.size() > kHeight)
    {
        OutputDebugStringW(L"IES profile data exceeds texture size.\n");
        return std::nullopt;
    }

    // テクセルを格納する配列の作成
    float meanCandela       = 0.0f;
    auto pixels             = BuildPixels(profileData, kWidth, kHeight, meanCandela);
    profileData.meanCandela = meanCandela;

    D3D12_SUBRESOURCE_DATA subRes = {};
    subRes.RowPitch               = kWidth * sizeof(float);
    subRes.SlicePitch             = kHeight * subRes.RowPitch;
    subRes.pData                  = pixels.data();

    // 一時的にCOPY_DESTに遷移してアップロードし，PIXEL_SHADER_RESOURCEに戻す
    auto* pRes = m_textureArray.GetResource();
    batch.Transition(pRes, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
    batch.Upload(pRes, m_count, &subRes, 1);
    batch.Transition(pRes, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    return m_count++; // 作成したテクスチャのインデックスを返す
}

D3D12_GPU_DESCRIPTOR_HANDLE IESProfile::GetSrvGpuHandle() const
{
    if (m_srv.IsValid() && m_pPoolSRV)
    {
        return m_srv.GetGPUHandle();
    }

    return {};
}
