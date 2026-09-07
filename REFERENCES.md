# 参考文献

本リポジトリのレンダリング実装が参照した資料の一覧です．
参照の性質によって3つに分けています．

- **実装の土台** — コードの構成そのものを参考にしたもの
- **理論の理解に用いた資料** — 概念を学ぶために参照したもの．実装は自分で書いている
- **式・手法の出典** — 実装した式やアルゴリズムの由来

<!-- 書式
  論文: 著者, "タイトル", 掲載媒体, 巻(号), pp. X–Y, 年.
  Web : 著者, 「タイトル」, 媒体・組織, 公開年. URL（参照日: YYYY-MM-DD）
  URL は引用の直後に独立した行で置く．
-->

---

## 実装の土台

### [Pocol 2021]

Pocol, 『Direct3D 12 ゲームグラフィックス実践ガイド』, 技術評論社, 2021. ISBN 978-4-297-12365-9.
https://gihyo.jp/book/2021/978-4-297-12365-9

本リポジトリのレンダラは本書の構成をベースに実装している．
PBR パイプライン，マテリアル定義，HDR 出力の基本設計は本書に依拠する．

---

## 理論の理解に用いた資料

このセクションの資料は，概念を理解するために参照したもので，
コードの構成や実装そのものを写したものではない．

### [Filament]

Google, 「Physically Based Rendering in Filament」, Google.
https://google.github.io/filament/Filament.md.html
（参照日: 2026-09-07）

PBR と IBL の理論を理解するために参照している．
BRDF の理論と式の導出，および IBL における distant light probe の扱いが主な対象．

### [emadurandal 2019]

emadurandal, 「物理ベースレンダリングを柔らかく説明してみる（1）〜（6）」, Qiita, 2019–2025.
https://qiita.com/emadurandal/items/3a8db7bc61438245654d
（参照日: 2026-09-07）

PBRの全体像や，BRDFの各項の役割，IBLの概要を理解する補助として参照した．

---

## 式・手法の出典

以下は，実装した式・手法の由来を示す一覧である．
多くは上記の書籍と解説資料を通じて理解し，
原典では該当する式と実装例を確認する形で参照した．

### [Schlick 1994]

C. Schlick, "An Inexpensive BRDF Model for Physically-Based Rendering", _Computer Graphics Forum_, 13(3), pp. 233–246, 1994.

フレネル項の多項式近似 `F ≈ F0 + (1 - F0)(1 - cosθ)^5`．

- 使用箇所: `assets/shader/BRDF.hlsli` (`SchlickFresnel`)

### [Walter 2007]

B. Walter, S. R. Marschner, H. Li, K. E. Torrance, "Microfacet Models for Refraction through Rough Surfaces", _Eurographics Symposium on Rendering (EGSR)_, pp. 195–206, 2007.
https://www.graphics.cornell.edu/~bjw/microfacetbsdf.pdf

GGX 法線分布関数の定義．
分布そのものの原典は Trowbridge & Reitz, _JOSA_ 65(5), 1975 で，本論文により GGX としてグラフィックス分野に導入された．

- 使用箇所: `assets/shader/BRDF.hlsli` (`D_GGX`)

### [Heitz 2014]

E. Heitz, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs", _Journal of Computer Graphics Techniques (JCGT)_, 3(2), pp. 48–107, 2014.
https://jcgt.org/published/0003/02/03/

Height-Correlated Smith 幾何項の導出．マスキングとシャドウイングの相関を扱う理論的基礎．

- 使用箇所: `assets/shader/BRDF.hlsli` (`G2_SmithCorrelated`) — 理論部分

### [Burley 2012]

B. Burley, "Physically-Based Shading at Disney", _SIGGRAPH 2012 Course: Practical Physically Based Shading in Film and Game Production_, 2012.
https://blog.selfshadow.com/publications/s2012-shading-course/

perceptual roughness の導入．アーティストが操作する roughness を二乗して α とする慣習の出典．

- 使用箇所: `assets/shader/BRDF.hlsli` (`EvaluateBRDF` 内の `float a = roughness * roughness`)

### [Karis 2013]

B. Karis, "Real Shading in Unreal Engine 4", _SIGGRAPH 2013 Course: Physically Based Shading in Theory and Practice_, 2013.
https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_slides.pdf
（コース全体: https://blog.selfshadow.com/publications/s2013-shading-course/ ）

IBL の実装の土台となっている資料．参照した内容は以下のとおり．

- **split-sum 近似** — 環境光の反射積分を，prefiltered environment map と environment BRDF の2つの積分の積で近似する手法．
  コースノートには `PrefilterEnvMap` と `IntegrateBRDF` の HLSL 実装例が含まれる
- 誘電体の F0 = 0.04 という既定値
- Hammersley 点列を用いた重点サンプリングの実装例（点列そのものの実装は [Dammertz 2012] を出典としている）

使用箇所:

- `assets/shader/PrefilteredEnvMapCS.hlsl`, `assets/shader/IntegrateBRDFCS.hlsl` — split-sum 近似
- `assets/shader/BRDF.hlsli` (`EvaluateBRDF` 内の `F0 = 0.04`)
- `assets/shader/IBLBake.hlsli` (`Hammersley`)

### [Fdez-Agüera 2019]

C. J. Fdez-Agüera, "A Multiple-Scattering Microfacet Model for Real-Time Image-based Lighting", _Journal of Computer Graphics Techniques (JCGT)_, 8(1), pp. 45-55, 2019.
https://jcgt.org/published/0008/01/03/

split-sum 近似では失われる多重散乱分のエネルギーを補償する手法．
単散乱項 `FssEss` に対して多重散乱項 `FmsEms` を加える形で定式化されている．

- 使用箇所: `assets/shader/IBL.hlsli` (`FssEss`, `FmsEms`, `Favg` を用いた補償の計算)

### [Frostbite 2014]

S. Lagarde, C. de Rousiers, "Moving Frostbite to Physically Based Rendering 3.0", _SIGGRAPH 2014 Course: Physically Based Shading in Theory and Practice_, 2014.
https://seblagarde.wordpress.com/2015/07/14/siggraph-2014-moving-frostbite-to-physically-based-rendering/
（コース全体: https://blog.selfshadow.com/publications/s2014-shading-course/ ）

本プロジェクトが最も広く依拠している資料．参照した内容は以下のとおり．

- 可視性関数 `V = G / (4·NL·NV)` に畳んだ Height-Correlated Smith の実装形
- 影響半径で滑らかに打ち切る窓関数付きの逆二乗距離減衰
- `angleScale` / `angleOffset` を CPU 側で前計算するスポットライトの角度減衰
- 測光単位（平行光源は lx，それ以外は cd）に基づくライト強度の設計
- IES プロファイルを用いたライトの扱い
- AO を間接光にのみ適用する扱い

使用箇所:

- `assets/shader/BRDF.hlsli` (`G2_SmithCorrelated`) — 実装形
- `assets/shader/Lighting.hlsli` (`Light`, `GetDistanceAttenuation`, `GetAngleAttenuation`, `GetIESProfileAttenuation`)
- `assets/shader/ScenePS.hlsl`（AO の適用箇所）
- CPU 側のライト強度・角度係数の事前計算

### [Uchimura 2017]

内村 創, 「HDR 理論と実践」, CEDEC 2017, ポリフォニー・デジタル, 2017.
https://www.slideshare.net/nikuque/hdr-theory-and-practicce-jp
（曲線の定義: https://www.desmos.com/calculator/gslcdxvipg ）

GT トーンマップ（Toe / Linear / Shoulder の三区間を重み合成する曲線）の定義とリファレンス実装．
リファレンス実装には `Copyright(c) 2017 by Hajime Uchimura @ Polyphony Digital Inc.` の著作権表示が付されている．

- 使用箇所: `assets/shader/Tonemap.hlsli` (`GT_Tonemap`)

### [Duff 2017]

T. Duff, J. Burgess, P. Christensen, C. Hery, A. Kensler, M. Liani, R. Villemin, "Building an Orthonormal Basis, Revisited", _Journal of Computer Graphics Techniques (JCGT)_, 6(1), pp. 1–8, 2017.
https://jcgt.org/published/0006/01/01/

分岐なしで法線から正規直交基底を構成する手法．実装は本論文 Listing 3（branchless 版）に対応する．

- 使用箇所: `assets/shader/IBLBake.hlsli` (`TangentSpace`)

### [Dammertz 2012]

H. Dammertz, 「Hammersley Points on the Hemisphere」, 2012.
https://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
（参照日: 2026-08-26）

ビット反転（radical inverse）による Hammersley 点列の実装．
正規化係数 `2.3283064365386963e-10`（= 1/2³²）もこの形．

- 使用箇所: `assets/shader/IBLBake.hlsli` (`Hammersley`)
