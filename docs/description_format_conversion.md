# src/easymrc/format_conversion.hpp 解説（超詳細）

このファイルは **画像データやGDSIIファイルからポリゴンを作る**処理をまとめています。
EasyMRCの入口に近い部分で、
「画像 → ポリゴン → 解析」の最初の段階です。

---

## 1. Image 構造体
```cpp
struct Image {
  int width;
  int height;
  std::vector<unsigned char> data;
  ...
};
```
- OpenCVを使わずに自前で簡易的な画像クラスを定義。
- `data` は 1次元配列だが、`at(x,y)` で2次元的にアクセスできる。

### is_mask_pixel
```cpp
bool is_mask_pixel(int x, int y) const {
  if (x < 0 || x >= width || y < 0 || y >= height) return false;
  return at(x, y) == 255;
}
```
- マスク領域（白=255）かどうかを判定。
- 境界外は false。

---

## 2. PGM 読み込み
### read_pgm
- PGM形式（P2 ASCII / P5 Binary）を読み込む。
- 画像は**下から上へ**（bottom-up）読み込む。
  - これにより座標系が「左下が原点」に近くなる。

#### ASCII (P2) と Binary (P5)
- P2はテキスト、P5はバイナリ。
- `magic` で判別して処理を分けている。

---

## 3. GDSII 書き込み・読み込み
GDSII は半導体レイアウトの標準バイナリ形式。

### gdsii 名前空間
- レコードタイプやデータ型の定数を定義。
- `write_record_header`, `write_int16` など、
  低レベルの読み書き関数を提供。

### write_gdsii
- ポリゴン配列をGDSII形式で書き出す。
- 各ポリゴンを1構造として保存。
- GDSIIでは**閉じたポリゴン**が必要なので、
  最初と最後の点が一致しなければ追加する。

### read_gdsii
- GDSIIファイルを読み込んでポリゴン化。
- `BOUNDARY` → `XY` → `ENDEL` の流れで頂点を集める。
- 最後の点が最初と同じなら削除して閉じ情報を除去。

---

## 4. FormatConverter クラス
### visited_ 配列
- 画像の各ピクセルを「訪問済みか」を管理する2次元配列。

### convert()
```cpp
std::vector<Polygon> convert(const std::string& gdsii_filename = "")
```
- 画像全体をスキャンし、
  未訪問のマスクピクセルがあれば `trace_polygon` を開始。
- `gdsii_filename` が指定されていれば
  PGM → GDSII → 再読込 という流れを通す。
  - これは GDSII変換の動作確認にもなる。

---

### trace_polygon()
- 境界追跡でポリゴンを作る中核。
- 右方向から開始し、**時計回り**に境界をたどる。

#### 流れ
1. 開始ピクセルを訪問済みにする。
2. 現在方向に進めるなら進む。
3. 右に曲がれるなら曲がる。
4. 曲がったら角を記録。
5. 進めない場合は左回転（方向を変える）。
6. 最初の角に戻ったら終了。

### get_next_corner()
- 方向に応じて「次の角の座標」を計算する。

---

## 5. 変換の入口関数
### format_conversion
- 画像ファイルを読み込み、ポリゴンに変換。
- `gdsii_filename` を指定すれば GDSII変換を挟む。

### format_conversion_from_data
- 生のピクセル配列（2次元配列）から変換したい場合に使う。

---

## まとめ
- 画像 → ポリゴン変換を担当。
- PGM読み込みとGDSII読み書きを両方サポート。
- 境界追跡（trace_polygon）が中心ロジック。
- 変換したポリゴンが後続の候補生成や違反検出の入力になる。
