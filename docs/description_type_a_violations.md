# src/easymrc/type_a_violations.hpp 解説（超詳細）

このファイルは **Type (a) 空間違反** を検出する処理です。
Type (a) は「ポリゴン同士の **点と点** がルール距離 R より近い」ケースを検出します。

---

## 1. 基本アイデア
- 代表点を使って候補点を絞り込む（sampling.hpp の結果）。
- スイープラインで「近くにある代表点ペア」を効率よく見つける。
- 見つかった代表点同士の **shielded_vertices** を詳しく調べ、
  本当に距離 R 未満なら違反と判定。

---

## 2. SegmentTree（実際は set）
```cpp
class SegmentTree {
  std::set<RepresentativePoint, CompareByY> tree_;
};
```
- 名前は SegmentTree ですが、実際は **y座標順に並んだ set**。
- `CompareByY` により y -> x の順で並びます。

### 主な操作
- `insert(p)`：点を追加
- `erase_by_x(x_threshold)`：x が小さい点を削除
- `range_query(y_min, y_max)`：y範囲内の点を列挙

**ポイント**: スイープラインで「x方向に近い点のみ残す」ために、
`erase_by_x` を使って古い点を削除します。

---

## 3. TypeAViolationDetector
### コンストラクタ
```cpp
TypeAViolationDetector(points_p1, points_p2, R, r)
```
- `points_p1`, `points_p2` は2ポリゴンの代表点。
- `R` はルール距離。
- `r` はサンプリング半径。
- 内部で `R_prime = R + 2r` を計算。
  - 代表点で検索する範囲を広げるため。

---

### detect() の流れ
#### Step 1: 点をまとめてソート
- p1 と p2 の代表点をまとめる。
- それぞれ「どのポリゴン由来か」を記録する。
- x座標でソート。

#### Step 2: スイープライン
- x を左から右へ進めるイメージ。
- 現在の点 `current` に対して次を行う:

1) 古い点を削除
```cpp
tree_p1.erase_by_x(x - R_prime_);
tree_p2.erase_by_x(x - R_prime_);
```
- xが遠すぎる点は、距離R以内に入らないので削除。

2) y範囲検索
```cpp
found_points = tree_p2.range_query(y - R', y + R')
```
- 近い可能性のある点だけ取り出す。

3) 詳細チェック
```cpp
check_violation(current.point, found, violations);
```
- 代表点レベルで「近そう」と判断されたペアを
  **shielded_vertices 同士**で最終確認。

4) 現在の点をツリーに追加
- p1なら tree_p1、p2なら tree_p2。

---

### check_violation の詳細
```cpp
for (point_v in v.shielded_vertices)
  for (point_q in q.shielded_vertices)
    if (distance < R) -> violation
```
- 代表点だけではなく、代表点が持つ「近い頂点」を総当たりで確認。
- ここで正確性を担保している。

---

## 4. 補助関数
### detect_type_a_violations
- クラスをラップして簡単に呼べる関数。

### check_space_violations_type_a
- 2ポリゴンを直接受け取り、
  代表点サンプリング → Type A 判定まで一気に実行。

---

## まとめ
- Type (a) は **点 vs 点の距離違反**。
- 代表点 + スイープラインで候補を絞り、
  shielded_vertices を使って厳密に判定。
- `R' = R + 2r` を使うのが重要ポイント。
