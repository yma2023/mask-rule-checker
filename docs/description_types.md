# src/easymrc/types.hpp 解説（超詳細）

このファイルは EasyMRC 全体で使う「基本データ型」と「距離計算などの基礎関数」をまとめた土台です。
C++の初心者向けに、1つずつ丁寧に説明します。

## 役割の全体像
- 図形（点・線分・ポリゴン）を表す構造体を定義する
- 解析結果（違反情報）を入れる構造体を定義する
- 距離計算などのよく使う関数を提供する

他のファイルは、この `types.hpp` にある型を前提に動いています。

---

## 1. 基本図形：Point
```cpp
struct Point {
  int x_coord;
  int y_coord;

  Point() : x_coord(0), y_coord(0) {}
  Point(int x, int y) : x_coord(x), y_coord(y) {}

  int x() const { return x_coord; }
  int y() const { return y_coord; }
};
```
- **Point** は2次元座標を表す構造体です。
- `x_coord`, `y_coord` は整数（ピクセル単位の座標を想定）。
- `x()` / `y()` は座標値を取り出すための関数です。
  - `const` なので中身を変更しません。

### 初心者向け補足
- `struct` は基本的に `class` と同じですが、デフォルトで public です。
- `Point()` のように同名の関数を用意すると**コンストラクタ**になり、初期値を入れられます。

---

## 2. 線分：Segment
```cpp
struct Segment {
  Point start;
  Point end;

  Segment() = default;
  Segment(const Point& s, const Point& e) : start(s), end(e) {}
  Segment(int x1, int y1, int x2, int y2)
      : start(Point(x1, y1)), end(Point(x2, y2)) {}
  ...
};
```
- **Segment** は線分。始点 `start` と終点 `end` を持ちます。
- 3つのコンストラクタがあり、用途に応じて作れます。
  - `Segment()` は何もしないデフォルト。
  - `Segment(Point, Point)` は点から作る。
  - `Segment(int, int, int, int)` は座標から直接作る。

### 方向判定
```cpp
bool is_vertical() const { return start.x() == end.x(); }
bool is_horizontal() const { return start.y() == end.y(); }
```
- 垂直か水平かを判定します。

### 長さ計算
```cpp
double length() const {
  double dx = end.x() - start.x();
  double dy = end.y() - start.y();
  return std::sqrt(dx * dx + dy * dy);
}
```
- 三平方の定理で長さを計算。
- `double` を使うのは、小数の距離が必要なためです。

### 最小/最大座標
```cpp
int min_x() const { return std::min(start.x(), end.x()); }
int max_x() const { return std::max(start.x(), end.x()); }
int min_y() const { return std::min(start.y(), end.y()); }
int max_y() const { return std::max(start.y(), end.y()); }
```
- バウンディングボックス計算に使います。

---

## 3. 多角形：Polygon
```cpp
struct Polygon {
  int id;
  std::vector<Point> vertices;
  std::vector<Segment> segments;
  ...
};
```
- **Polygon** はポリゴン（多角形）を表します。
- `vertices` は頂点リスト。
- `segments` は頂点をつないだ線分リスト。

### build_segments
```cpp
void build_segments() {
  segments.clear();
  if (vertices.size() < 2) return;

  for (size_t i = 0; i < vertices.size(); ++i) {
    size_t next = (i + 1) % vertices.size();
    segments.emplace_back(vertices[i], vertices[next]);
  }
}
```
- 頂点列から線分を作る関数です。
- `%` を使って最後の頂点を最初に繋ぐ → **閉じたポリゴン**が完成。

---

## 4. BoundingBox
```cpp
struct BoundingBox {
  double min_x, min_y, max_x, max_y;
  int polygon_id;
  ...
};
```
- ポリゴンを囲む最小の矩形です。
- `expand(R)` で四方向に距離 R だけ広げられます。
- `overlaps()` で他のボックスと重なるか判定します。

### compute_bounding_box
```cpp
inline BoundingBox compute_bounding_box(const Polygon& poly)
```
- ポリゴンの全線分の最小/最大を調べてバウンディングボックスを作ります。
- `polygon_id` にどのポリゴン由来かを記録します。

---

## 5. 代表点・代表エッジ
### RepresentativePoint
```cpp
struct RepresentativePoint {
  Point coordinates;
  std::vector<Point> shielded_vertices;
  std::vector<Segment> shielded_edges;
  int polygon_id;
};
```
- 代表点は「すべての頂点をそのまま使うと重いので、代表だけ使う」ためのもの。
- `shielded_vertices` は、この代表点の近く（距離 r 以内）の頂点リスト。
- `shielded_edges` は、この代表点の近くのエッジ。
- 代表点で見つかった候補を、本当に違反か詳しく調べるために使います。

### RepresentativeEdge
```cpp
struct RepresentativeEdge {
  Segment edge;
  std::vector<Point> shielded_vertices;
  int polygon_id;
};
```
- 代表エッジも同じ考え方で、長いエッジだけを抽出して処理負荷を減らします。
- `shielded_vertices` はそのエッジに近い頂点集合。

---

## 6. 違反情報の構造体
### Violation（Type A）
```cpp
struct Violation {
  Point point1, point2;
  double distance;
  int polygon_id_1, polygon_id_2;
};
```
- 点と点の距離がルールより近い場合の違反。

### ViolationTypeB（Type B）
```cpp
struct ViolationTypeB {
  Point point;
  Segment edge;
  double distance;
  int polygon_id_1, polygon_id_2;
};
```
- 点と線分が近すぎる場合の違反。

### WidthViolation
```cpp
struct WidthViolation {
  Segment edge1, edge2;
  double distance;
  Point closest_point_on_edge1;
  Point closest_point_on_edge2;
  int polygon_id;
};
```
- 同じポリゴン内で、向かい合うエッジ間の距離が小さすぎる場合の違反。

---

## 7. 基礎的な距離計算
### euclidean_distance
```cpp
inline double euclidean_distance(const Point& p1, const Point& p2)
```
- 2点間距離（いわゆるユークリッド距離）を返します。

### point_to_segment_distance
```cpp
inline double point_to_segment_distance(const Point& p, const Segment& seg)
```
- 点と線分の最短距離を計算します。
- 流れは次の通り：
  1. 線分をベクトルとして考える
  2. 点から線分への垂線の足を求める
  3. その足が線分の範囲外なら、端点までの距離を使う

### 初心者向け補足
- `t` は線分上の「割合」を表す値で、0なら始点、1なら終点です。
- `std::max` / `std::min` を使って `t` を [0,1] に収めています。

---

## まとめ
- `types.hpp` は EasyMRC 全体の「辞書」のような役割。
- Point/Segment/Polygon がベース。
- 代表点・代表エッジが高速化のカギ。
- 違反構造体が結果の器。
- 距離計算が全アルゴリズムの基礎。

このファイルを理解すると、他のファイルの構造がとても読みやすくなります。
