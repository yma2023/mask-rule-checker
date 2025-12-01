# EasyMRC - Efficient Mask Rule Checking

EasyMRCは、論文「EasyMRC: Efficient Mask Rule Checking with Precomputing and Parallel Computing」に基づいて実装された効率的なマスクルールチェッカーです。

## 概要

OPC（Optical Proximity Correction）後のマスクデータに対して、DRC（Design Rule Check）よりも効率的にMRC（Mask Rule Check）を実行します。

### 主な特徴

1. **Format Conversion**: PNG画像からGDSII形式（セグメントベース）への変換
2. **Candidate Pair Generation**: バウンディングボックススイープラインによる効率的な候補ペア生成
3. **Representative Edge Sampling**: 代表的な頂点・エッジのみを選択して処理量を削減（20-30%に削減）
4. **Type (a) & (b) Violations**: スイープラインアルゴリズムによる高速な違反検出
5. **Width Checking**: 反対向きエッジの最小幅チェック
6. **Multithread Parallelization**: マルチスレッドによる並列実行（8スレッドで約4.7倍高速化）

## アルゴリズムの詳細

### 1. Format Conversion（形式変換）

**入力**: PNG形式のバイナリマスク画像（前景=255, 背景=0）

**処理**:
- 左下隅から右上隅へスキャン
- 未訪問のマスクピクセルを見つけたら、新規ポリゴン作成開始
- 時計回りでポリゴン境界を辿る（右→下→左→上）
- コーナーごとに座標を記録してセグメントを生成

**出力**: 水平・垂直セグメントで構成される閉じたポリゴン

**計算量**: O(m) （m = ピクセル数）

### 2. Candidate Pair Generation（候補ペア生成）

**入力**: ポリゴンリスト、スペースルール距離 R

**処理**:
1. 各ポリゴンのバウンディングボックスを計算
2. バウンディングボックスをR距離だけ膨張
3. 膨張ボックスが重なるペアを候補として抽出（スイープラインアルゴリズム）

**出力**: 候補ペアリスト

**計算量**: O(p log p) （p = ポリゴン数）

### 3. Representative Edge Sampling（代表エッジサンプリング）

**入力**: ポリゴン、サンプリング半径 r = 4 × 平均エッジ長

**処理**:
1. 貪欲法で代表点を選択（距離r以内の頂点をカバー）
2. 各代表点の遮蔽情報（距離r以内の頂点・エッジ）を記録
3. 長さがr以上のエッジを代表エッジとして選択

**出力**: 代表点リスト、代表エッジリスト

**削減率**: 20-30%（論文実験値）

**計算量**: O(N) （N = 頂点数）

### 4. Type (a) Violations（代表点間の違反検出）

**入力**: 2つのポリゴンの代表点リスト、R' = R + 2r

**処理**:
1. 全代表点をx座標でソート
2. スイープラインで左から右へ走査
3. 各代表点で、セグメント木を使ってy範囲内の反対ポリゴンの代表点を検索
4. 見つかった代表点の遮蔽情報同士の距離をチェック

**出力**: 違反リスト

**計算量**: O(N log N)

### 5. Type (b) Violations（代表点とエッジ間の違反検出）

**入力**: 代表点リスト、代表エッジリスト、R' = R + r

**処理**:
1. 垂直エッジと水平エッジのイベントを生成
2. 垂直エッジ：左端と右端で2つのイベント
3. 水平エッジ：右端で1つのイベント
4. スイープラインでイベント処理
5. 代表点の遮蔽情報とエッジの遮蔽情報間の距離をチェック

**出力**: 違反リスト

**計算量**: O((p+n) log(p+n))

### 6. Width Checking（ウィドスチェック）

**入力**: 単一ポリゴン、ウィドスルール距離 R

**処理**:
1. サンプリング実行（Type a/bと同じ）
2. スイープライン実行
3. **フィルタリング**: 反対向きのエッジペアのみを報告
   - 垂直ペア：片方が上向き、片方が下向き
   - 水平ペア：片方が右向き、片方が左向き

**出力**: ウィドス違反リスト

**計算量**: O(N log N)

### 7. Multithread Parallelization（マルチスレッド並列化）

**スペースチェック**:
- 候補ペアをスレッド数で分割
- 各スレッドが独立してペアをチェック
- 結果を集約

**ウィドスチェック**:
- ポリゴンをスレッド数で分割
- 各スレッドが独立してポリゴンをチェック
- 結果を集約

**スレッドセーフティ**:
- 各スレッドは独立した結果配列に書き込み
- ロック不要（データ競合なし）

**性能**: 8スレッドで約4.7倍高速化（論文実験値）

## 使用方法

### ビルド

```bash
mkdir build
cd build
cmake ..
make easymrc_main
make easymrc_test
```

### 実行

```bash
# 画像ファイルからMRCチェック
./easymrc_main mask.pgm -r 50 -m 4 -t 8 -o violations.json

# オプション:
#   -r <distance>    ルール距離（デフォルト: 50.0）
#   -m <multiplier>  サンプリング半径倍率（デフォルト: 4.0）
#   -t <threads>     スレッド数（デフォルト: 自動検出）
#   -o <output>      出力ファイル（デフォルト: violations.json）
#   --no-space       スペースチェック無効
#   --no-width       ウィドスチェック無効
#   --no-parallel    並列実行無効
```

### テスト実行

```bash
./easymrc_test
```

## プログラムの構成

```
odrc/easymrc/
├── types.hpp                 # 共通型定義
├── format_conversion.hpp     # PNG→GDSII変換
├── candidate_pairs.hpp       # 候補ペア生成
├── sampling.hpp              # 代表エッジサンプリング
├── type_a_violations.hpp     # Type (a) 違反検出
├── type_b_violations.hpp     # Type (b) 違反検出
├── width_check.hpp           # ウィドスチェック
├── parallel.hpp              # マルチスレッド並列化
├── easymrc.hpp               # メイン統合ヘッダー
└── README.md                 # このファイル
```

## APIの使用例

```cpp
#include <odrc/easymrc/easymrc.hpp>

using namespace odrc::easymrc;

// 設定
EasyMRC::Config config;
config.rule_distance_R = 50.0;
config.sampling_radius_multiplier = 4.0;
config.num_threads = 8;

// チェッカー作成
EasyMRC checker(config);

// ポリゴンリストからチェック
auto results = checker.run(polygons);

// 画像ファイルから直接チェック
auto results = checker.run_from_image("mask.pgm");

// 結果の確認
std::cout << "Space violations: " << results.total_space_violations() << std::endl;
std::cout << "Width violations: " << results.width_violations.size() << std::endl;
```

## 性能

論文の実験結果より：

### スペースチェック

| テストケース | KLayout (ms) | EasyMRC (ms) | 高速化 |
|------------|--------------|--------------|--------|
| Mask1-10   | 154.8        | 2.1          | 74x    |
| Active/Metal/Poly/Via | 4665 | 17.1   | 273x   |
| CPU0/CPU1  | 26290        | 105.6        | 249x   |

### ウィドスチェック

| テストケース | 高速化 |
|------------|--------|
| 基本例     | 46x    |
| 大規模例   | 96x    |

### マルチスレッド効果

- 8スレッド: 平均 4.68倍高速化
- 16スレッド: 平均 5.2倍高速化

## 制約事項

1. **ポリゴン形式**: 水平・垂直エッジのみサポート（マンハッタン幾何）
2. **画像形式**: 現在はPGM形式のみサポート（PNG対応は将来追加予定）
3. **GDSII読み込み**: 既存のGDSIIリーダーとの統合が必要

## 理論的保証

**Theorem 2（完全性保証）**:
拡張ルール距離 R' = R + 2r を使用することで、すべての元の違反が検出されることが数学的に保証されています。

**削減効果**:
サンプリング半径 r = 4l を使用すると、処理対象が20-30%に削減されながら、全違反を検出できます。

## 参考文献

EasyMRC論文：
- タイトル: "EasyMRC: Efficient Mask Rule Checking with Precomputing and Parallel Computing"
- アルゴリズム: バウンディングボックススイープライン + 代表エッジサンプリング
- 性能: KLayoutと比較して100-400倍高速化

## ライセンス

プロジェクトのライセンスに従います。
