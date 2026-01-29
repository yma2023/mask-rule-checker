# src/easymrc/width_check.hpp 解説（超詳細）

このファイルは **幅（Width）違反** を検出する処理です。
対象は「同じポリゴン内で、向かい合うエッジ間の距離が小さすぎる」ケース。

---

## 1. 方向判定ユーティリティ
### is_edge_upward / downward / rightward / leftward
- エッジの向きを判定します。
- たとえば `is_edge_upward` は `start.y < end.y` なら true。

### are_opposite_vertical / are_opposite_horizontal
- 2本の縦（または横）エッジが「逆向き」かどうか判定します。
- 逆向きとは、片方が上向き・もう片方が下向き（または右向きと左向き）。

### are_opposite
- 縦同士 or 横同士で、方向が逆なら true。
- 垂直と水平の組み合わせは false。

**なぜ必要？**
- 幅違反は「向かい合っているエッジ」だけが対象だから。
- 同じ向きのエッジ同士は幅にはならない。

---

## 2. segment_to_segment_distance
```cpp
inline double segment_to_segment_distance(...)
```
- 2本の線分間の最短距離を求める関数。
- 実装はシンプルで、＠
  - 片方の端点をもう片方の線分に落とした距離を計算
  - これを両方向で試す
- 厳密解ではないが、
  「代表エッジの距離チェック」には十分という設計。

**補足**: 本来は線分同士の最短距離を厳密に計算する式があります。
ここでは実装を簡略化している点に注意。

---

## 3. WidthChecker クラス
### コンストラクタ
```cpp
WidthChecker(const Polygon& poly, double R, double r)
```
- `poly` 対象ポリゴン
- `R` ルール距離
- `r` サンプリング半径

### check() の流れ
1. 代表点と代表エッジをサンプリング
2. 代表エッジの組み合わせを総当たり
3. 逆向きエッジだけを対象
4. 距離を計算して R 未満なら違反

#### 重要ポイント
- 「すべてのエッジ」ではなく「代表エッジ」のみを使う。
- これで計算量を減らしている。

---

## 4. 補助関数
### check_width_violations
- `WidthChecker` のラッパー。

### check_all_width_violations
- 複数ポリゴンに対してまとめて実行。
- 各ポリゴンで `calculate_sampling_radius` を使い r を決める。

---

## まとめ
- 幅違反は「同じポリゴン内」の逆向きエッジ間距離を見る。
- 方向判定が鍵。
- 距離計算は簡略版で、代表エッジのペアを総当たり。
