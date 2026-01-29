# src/easymrc/sampling.hpp 解説（超詳細）

このファイルは「代表点・代表エッジのサンプリング」を実装しています。
目的は**ポリゴン全頂点を使わずに、少数の代表点だけで違反判定を効率化**することです。

---

## 1. なぜサンプリングが必要？
- ポリゴンの頂点が多いと、距離計算が爆発的に増える。
- 代表点だけを使い、近くの頂点情報（shielded）を持たせることで、
  正確さを保ちながら計算量を減らす。

---

## 2. RepresentativeSampler クラス
### コンストラクタ
```cpp
RepresentativeSampler(const Polygon& poly, double sampling_radius)
```
- `poly` は対象ポリゴン。
- `sampling_radius (r)` は代表点の「影響範囲」。

---

### sample() の流れ
#### ステップ1: 代表点の選択
```cpp
std::vector<int> rep_indices = select_representative_points();
```
- 頂点番号を返す関数で「代表となる頂点」を選ぶ。
- 詳しくは後述（貪欲法）。

#### ステップ2: 代表点の shielded 情報構築
```cpp
for (int idx : rep_indices) {
  RepresentativePoint rep_point(...);
  // 近い頂点を shielded_vertices に入れる
  // 近いエッジを shielded_edges に入れる
}
```
- 代表点自身の周辺にある頂点やエッジを集める。
- これにより「代表点だけで全体を代表」できる。

#### ステップ3: 代表エッジの選択
```cpp
if (seg.length() > r_) {
  RepresentativeEdge rep_edge(...);
  // 近い頂点を shielded_vertices に入れる
}
```
- 長さが r より長いエッジだけを代表にする。
- 短いエッジは近距離の代表点が十分カバーできるため。

---

## 3. 統計情報
```cpp
struct SamplingStats {
  int original_vertices;
  int representative_points;
  int representative_edges;
  double reduction_ratio;
};
```
- 代表点数が元の頂点数よりどのくらい減ったかを可視化。

---

## 4. 代表点選択アルゴリズム
### calculate_average_edge_length
- 平均エッジ長を計算。
- ただし `calculate_sampling_radius` の方でも使っている。

### distance_along_boundary
```cpp
// 境界に沿った距離を計算
```
- 直線距離ではなく、**ポリゴンの輪郭をたどった距離**。
- `start_idx` から `end_idx` までエッジを順に足していく。

### find_next_representative
- 現在の頂点から `r` 以内で最も遠い頂点を選ぶ。
- これにより「間隔がなるべく大きい」代表点になる。

### select_representative_points（貪欲法）
```cpp
// 未カバーの頂点がなくなるまで繰り返す
```
流れ：
1. 最初の頂点を代表に選ぶ。
2. その頂点から距離 r 以内の頂点を「カバー済み」にする。
3. 次の未カバー頂点を見つける。
4. `find_next_representative` で次の代表点を決める。
5. すべてカバーされるまで繰り返す。

**結果**: 少数の代表点で全体をカバー。

---

## 5. ユーティリティ関数
### sample_representatives
```cpp
inline void sample_representatives(...)
```
- 代表点・代表エッジを作るための簡易関数。

### calculate_sampling_radius
```cpp
inline double calculate_sampling_radius(...)
```
- `r = multiplier * 平均エッジ長` というルールで決まる。
- デフォルトでは `multiplier = 4.0`。

### sample_all_polygons
- すべてのポリゴンに対して代表点・代表エッジを作る補助関数。

---

## まとめ
- `sampling.hpp` は**計算量削減の要**。
- 代表点に近い頂点・エッジをまとめて保持することで精度を維持。
- 貪欲法で代表点を選び、全頂点をカバーするようにしている。
