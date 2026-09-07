<!-- 配置先: assets/CREDITS.md -->

# アセットのクレジット

本リポジトリには 3D モデル・HDRI・IES プロファイルの実体を含めていません．
以下は，デモの実行に使用しているアセットの出典とライセンスです．

## HDRI

| ファイル | 出典 | ライセンス | 取得日 |
|---|---|---|---|
| `HDRI/venice_sunset_4k.hdr` | [Poly Haven — Venice Sunset](https://polyhaven.com/a/venice_sunset) | CC0 | 2026-09-07 |

## IES プロファイル

| ファイル | 出典 | 作者 | ライセンス | 取得日 |
|---|---|---|---|---|
| `ies/Light_161_200525.ies` | [3DTexel — Light IES 113](https://3dtexel.com/product/light-ies-113/) | Lucas Anton Gomez | CC0 | 2026-09-07 |
| `ies/Light_115_200525.ies` | [3DTexel — Light IES 83](https://3dtexel.com/product/light-ies-83/) | Lucas Anton Gomez | CC0 | 2026-09-07 |

3DTexel の無料アセットは CC0 Public Domain として配布されています
（有料アセットは 3DTexel License という別の条件です）．
帰属表示は義務ではありませんが，配布元が credit appreciated と表明しているため記載しています．

### `ies/test/` について

パーサーとサンプリングの動作確認用に，**生成 AI（Claude）で作成した合成プロファイル**です．
実在の照明器具を測定したデータではないため，描画結果の見本には使用していません．

一様配光（`01_uniform_1000cd`）やランバート分布（`04_lambert_disk`）など，
期待値が解析的に分かる配光を用意してあり，読み込みと角度方向の補間が正しいかの確認に使っています．

## 3D モデル

`model/` 以下の GLB はすべて自作です（Blender で作成）．
