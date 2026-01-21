# DirectXGame

Direct3D 12 の学習を目的とした個人開発プロジェクトです。  
D3D12 の基礎から応用まで、実践的なレンダリングエンジンを自作しながら学んでいます。

## 特徴

- **3層アーキテクチャ**: App（Win32）/ Engine（D3D12）/ Game（ロジック）の責務分離
- **RAII / ComPtr**: スマートポインタによる安全なリソース管理
- **PSO 事前生成**: Pipeline State Object は初期化時に作成し、実行時は切替のみ
- **Root Signature 1.1**: 静的データ宣言によるドライバ最適化
- **PBR マテリアル**: 物理ベースレンダリング対応（開発中）

## 実装機能

### グラフィックス

- D3D12 デバイス・スワップチェイン・コマンドリスト初期化
- ダブルバッファリング（2フレームバッファ）
- レンダーターゲット・深度バッファ管理
- ディスクリプタプール（CBV/SRV/UAV、RTV、DSV、Sampler）

### パイプライン

- PSO（Pipeline State Object）ビルダーパターン
- Root Signature 1.1 ビルダーパターン
- 頂点フォーマット（Position, Normal, Tangent, TexCoord, Color）

### アセット管理

- GLB/glTF インポート（Assimp）
- テクスチャ読み込み（DirectXTex）
- マテリアルシステム（BaseColor, Metallic, Roughness, Normal, Emissive, Occlusion）
- デフォルトテクスチャ生成（White, NormalFlat, RMA）

### 入力システム

- キーボード: 押下中 / 押された瞬間 / 離された瞬間
- マウス: ボタン状態 / 移動差分

### カメラ

- View / Projection 行列計算
- 位置・回転（オイラー角）・注視点設定
- キーボード / マウスによるカメラ操作

## プロジェクト構成

| モジュール  | 役割                                                   |
| ----------- | ------------------------------------------------------ |
| **App/**    | Win32 エントリーポイント、メインループ、ウィンドウ管理 |
| **Engine/** | D3D12 初期化、リソース管理、描画システム               |
| **Game/**   | ゲームロジック、カメラコントローラー                   |

```
DirectXGame/
├── App/                    # アプリケーション層
├── Engine/                 # エンジン層（D3D12）
├── Game/                   # ゲームロジック層
├── include/                # ヘッダファイル
├── src/                    # ソースファイル
└── assets/                 # アセット（モデル、シェーダー）
```

## 依存ライブラリ

| ライブラリ                                              | 用途                      |
| ------------------------------------------------------- | ------------------------- |
| [DirectXTK12](https://github.com/microsoft/DirectXTK12) | DirectX 12 ユーティリティ |
| [DirectXTex](https://github.com/microsoft/DirectXTex)   | テクスチャ読み込み・処理  |
| [Assimp](https://github.com/assimp/assimp)              | 3D モデルインポート       |

## 開発環境

- Windows 10/11
- Visual Studio 2022
- vcpkg

## ライセンス

本プロジェクトは [MIT License](LICENSE) の下で公開されています。

## Third-Party Licenses

本プロジェクトは以下のオープンソースライブラリを使用しています：

- [DirectXTK12](https://github.com/microsoft/DirectXTK12) - MIT License
- [DirectXTex](https://github.com/microsoft/DirectXTex) - MIT License
- [Assimp](https://github.com/assimp/assimp) - BSD 3-Clause License

各ライブラリのライセンス全文は、それぞれのリポジトリをご確認ください。
