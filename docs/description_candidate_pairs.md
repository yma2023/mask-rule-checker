# src/easymrc/candidate_pairs.hpp 解説（超詳細）

このファイルは「ポリゴン同士の距離チェック」を効率化するために、
**実際に調べるべき候補ペアだけを抽出する**ロジックです。
全組み合わせを調べると `N*(N-1)/2` で爆発するので、
**バウンディングボックス + スイープライン**で候補を減らします。

---

## 1. 基本アイデア
- 各ポリゴンを「最小の長方形（BoundingBox）」で囲む。
- その長方形を **ルール距離 R だけ拡張**する。
  - これで「R より離れているポリゴンは絶対に違反しない」と判断できる。
- 拡張された長方形同士が重なるペアだけを候補とする。

---

## 2. スイープライン用の構造体
### EventType
```cpp
enum EventType {
  LEFT_EVENT = 0,
  RIGHT_EVENT = 1
};
```
- スイープラインで「矩形が始まる x」と「終わる x」をイベント化。
- 左端が `LEFT_EVENT`、右端が `RIGHT_EVENT`。

### Event
```cpp
struct Event {
  double x;
  EventType type;
  int polygon_id;
  double y_min, y_max;
  ...
};
```
- x座標・種類・ポリゴンID・y範囲を持ちます。
- `operator<` があるので `std::sort` で並び替えできます。
  - x が同じなら LEFT/RIGHT の順序で安定化。

### Interval
```cpp
struct Interval {
  double y_min, y_max;
  int polygon_id;
  ...
};
```
- スイープライン中に「アクティブな矩形の y 範囲」を管理します。
- `overlaps()` で y 範囲が重なるか判定。

---

## 3. CandidatePairGenerator クラス
### コンストラクタ
```cpp
CandidatePairGenerator(const std::vector<Polygon>& polygons, double R)
```
- ポリゴン配列とルール距離 R を受け取る。
- 内部で参照として保存するため、コピーはしない。

### generate() の流れ
#### Step 1: バウンディングボックス計算＋拡張
```cpp
BoundingBox bbox = compute_bounding_box(poly);
bbox.expand(rule_distance_);
```
- `compute_bounding_box` は `types.hpp` に定義。
- `expand` で上下左右に R を足し、候補範囲を広げる。

#### Step 2: イベント生成
```cpp
events.emplace_back(bbox.min_x, LEFT_EVENT, ...);
events.emplace_back(bbox.max_x, RIGHT_EVENT, ...);
```
- 左端と右端でイベントを作る。
- これを使ってスイープラインを行う。

#### Step 3: イベントのソート
```cpp
std::sort(events.begin(), events.end());
```
- x座標の昇順で処理するためにソート。

#### Step 4: スイープライン
```cpp
std::set<Interval> active_intervals;
```
- 現在のスイープラインに重なっている矩形の y 範囲を保持。
- **LEFT_EVENT のとき**：
  - 既存の `active_intervals` と重なり判定。
  - 重なったら候補ペアに追加。
  - そのあと自分の interval を追加。
- **RIGHT_EVENT のとき**：
  - その矩形の interval を削除。

#### 候補ペアの重複防止
```cpp
std::set<std::pair<int, int>> candidate_pairs_set;
```
- `set` を使うので同じペアは1回だけ。
- `i < j` になるように swap して順番を統一。

---

## 4. 統計情報
### Statistics 構造体
```cpp
struct Statistics {
  int total_polygons;
  int candidate_pairs;
  double reduction_ratio;
};
```
- `total_possible_pairs` と比べてどのくらい削減できたかを計算。

### reduction_ratio
```cpp
1.0 - (double)num_pairs / total_possible_pairs
```
- 1.0 に近いほど削減率が高い。
- 例えば 0.9 なら 90% 削減できたことになる。

---

## 5. 便利なラッパー関数
### candidate_pair_generation
```cpp
inline std::vector<std::pair<int, int>> candidate_pair_generation(...)
```
- クラスを作らずに1行で候補ペアを取得するための関数。

### get_candidate_pair_statistics
```cpp
inline auto get_candidate_pair_statistics(...)
```
- 統計情報をまとめて返す。

---

## まとめ
- ここでやっていることは「候補を減らすための前処理」。
- バウンディングボックスを拡張して、重なるものだけ候補にする。
- スイープライン + set を使うことで効率よく抽出。
- 本体の距離計算は後続の `type_a_violations` / `type_b_violations` が担当。
