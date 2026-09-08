# アセットのクレジット

本リポジトリには 3D モデル・HDRI・IES プロファイルの実体を含めていません．
以下は，本プロジェクトで使用しているアセットの出典とライセンスです．

## HDRI

| ファイル                         | 出典                                                                          | 作者             | ライセンス | 取得日     |
| -------------------------------- | ----------------------------------------------------------------------------- | ---------------- | ---------- | ---------- |
| `HDRI/abandoned_workshop_4k.hdr` | [Poly Haven — Abandoned workshop](https://polyhaven.com/a/abandoned_workshop) | Sergej Majboroda | CC0        | 2026-09-08 |
| `HDRI/venice_sunset_4k.hdr`      | [Poly Haven — Venice Sunset](https://polyhaven.com/a/venice_sunset)           | Greg Zaal        | CC0        | 2026-09-07 |

## 3D モデル

### Poly Haven（CC0）

`model/demo_scene.glb` は，以下の4モデルと自作の床を Blender で1つにまとめたものです．

| モデル          | 出典                                                                   | 作者                  | 取得日     |
| --------------- | ---------------------------------------------------------------------- | --------------------- | ---------- |
| Camera 01       | [Poly Haven — Camera 01](https://polyhaven.com/a/Camera_01)            | Rajil Jose Macatangay | 2026-09-08 |
| Lantern 01      | [Poly Haven — Lantern 01](https://polyhaven.com/a/Lantern_01)          | Rajil Jose Macatangay | 2026-09-08 |
| Wooden Table 02 | [Poly Haven — Wooden Table 02](https://polyhaven.com/a/WoodenTable_02) | Fran Calvente         | 2026-09-08 |
| CheeseBox 01    | [Poly Haven — CheeseBox 01](https://polyhaven.com/a/CheeseBox_01)      | Gabriel Radić         | 2026-09-08 |

### テクスチャ（CC0）

| テクスチャ      | 出典                                                                    | 作者                                                  | 用途                  | 取得日     |
| --------------- | ----------------------------------------------------------------------- | ----------------------------------------------------- | --------------------- | ---------- |
| Wooden Floor 01 | [Poly Haven — Wooden Floor 01](https://polyhaven.com/a/wooden_floor_01) | Charlotte Baglioni（スキャン）／Rico Cilliers（加工） | `demo_scene.glb` の床 | 2026-09-08 |

### 自作

以下は Blender で作成したものです．

- `model/Katana.glb`
- `model/Plane.glb`
- `model/white_furnace_sphere.glb`
- `model/lowpoly_apple.glb`
- `model/demo_scene.glb` に含まれる床の形状（テクスチャは上記の Wooden Floor 01）

## IES プロファイル

| ファイル                   | 出典                                                                  | 作者              | ライセンス | 取得日     |
| -------------------------- | --------------------------------------------------------------------- | ----------------- | ---------- | ---------- |
| `ies/Light_161_200525.ies` | [3DTexel — Light IES 113](https://3dtexel.com/product/light-ies-113/) | Lucas Anton Gomez | CC0        | 2026-09-07 |
| `ies/Light_115_200525.ies` | [3DTexel — Light IES 83](https://3dtexel.com/product/light-ies-83/)   | Lucas Anton Gomez | CC0        | 2026-09-07 |

### `ies/test/` について

パーサーとサンプリングの動作確認用に，**生成 AI（Claude）で作成した合成プロファイル**です．
実在の照明器具を測定したデータではないため，描画結果の見本には使用していません．

一様配光（`01_uniform_1000cd`）やランバート分布（`04_lambert_disk`）など，
期待値が解析的に分かる配光を用意してあり，読み込みと角度方向の補間が正しいかの確認に使っています．

## ライセンスについて

**Poly Haven** のアセットは，モデル・HDRI・テクスチャのすべてが CC0 Public Domain として配布されています．

**3DTexel** は無料アセットを CC0 Public Domain として配布しています
（有料アセットは 3DTexel License という別の条件です）．

いずれも帰属表示は義務ではありませんが，配布元が credit appreciated と表明しているため作者名を記載しています．
