# src/easymrc/type_b_violations.hpp 解説（超詳細）

このファイルは **Type (b) 空間違反** を検出します。
Type (b) は「点とエッジ（線分）が近すぎる」ケースを見つけます。

---

## 1. 基本アイデア
- 代表点と代表エッジを使って処理量を減らす。
- スイープラインで「近くにありそうな点とエッジ」を候補にする。
- 代表点の shielded_vertices と、代表エッジの shielded_vertices を比較し、
  本当に距離 R 未満なら違反とする。

---

## 2. イベント構造体
### EdgeEventType
```cpp
enum EdgeEventType {
  VERTICAL_LEFT,
  VERTICAL_RIGHT,
  HORIZONTAL,
  POINT_EVENT
};
```
- スイープライン用のイベント種別。
- 縦エッジは「左端」「右端」に2つのイベントを作る。
- 横エッジは右端で1つだけ。
- 代表点は POINT_EVENT。

### EdgeEvent
- x座標、イベント種別、対象ID、y情報を持つ。
- `operator<` で x→種別順にソート。

---

## 3. TypeBViolationDetector
### コンストラクタ
```cpp
TypeBViolationDetector(points_p1, points_p2, edges_p1, edges_p2, R, r)
```
- 2ポリゴンの代表点と代表エッジ。
- `R_prime = R + r` を使う。
  - Type (a) より広がりが小さいのは計算設計の違い。

---

### detect() の流れ
1. イベント生成
2. イベントを x でソート
3. スイープライン開始

#### スイープラインの基本
- `point_tree` に代表点を入れる。
- エッジイベントが来たとき、
  近くにある点を `range_query` で取得して詳細チェック。

---

### generate_events()
- 代表点を POINT_EVENT にする。
- 代表エッジは `add_edge_events` でイベントを作る。

### add_edge_events()
- 縦エッジ：
  - x = x0 で VERTICAL_LEFT
  - x = x0 + R' で VERTICAL_RIGHT
  - つまり「この範囲をスイープラインが通過している間」点を調べる。
- 横エッジ：
  - 右端 + r の位置でイベント1回だけ。

---

### handle_vertical_edge_event()
- y範囲 `[y_min - r, y_max + r]` で点を検索。
- その点とエッジの組を詳細チェックする。

### handle_horizontal_edge_event()
- y範囲 `[y0 - R', y0 + R']` で点を検索。
- その点とエッジをチェック。

---

### check_point_edge_violation()
```cpp
for (point_v in point.shielded_vertices)
  for (point_e in edge.shielded_vertices)
    if (distance < R) -> violation
```
- 代表点の近くの頂点と、
  代表エッジの近くの頂点を総当たりで調べる。
- 距離がルール未満なら違反。

---

## 4. 補助関数
### detect_type_b_violations
- クラスのラッパー。

### check_space_violations_complete
- Type (a) と Type (b) をまとめて計算。
- ここで `detect_type_a_violations` も呼ぶ。

---

## まとめ
- Type (b) は **点 vs エッジ** の違反。
- スイープライン + イベントで候補を絞る。
- shielded_vertices 同士の比較で正確性を確保。
- `R_prime = R + r` が探索範囲に使われる。
