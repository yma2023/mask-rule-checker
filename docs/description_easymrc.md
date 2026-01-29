## 0. このファイルは何をしている？

この `easymrc.hpp` は「EasyMRCの入口」です。

* 外部の人は基本ここだけ見れば使える（＝API）
* 内部では他のhpp/cppにある関数を呼び出して、チェック全体を進める
* つまり「司令塔」「パイプライン」

 
## 1. 先頭：`#pragma once`

```cpp
#pragma once
```

* これは「このヘッダファイルを **同じコンパイル単位で2回読み込まない**」ための命令。
* C++では `#include` が重なると同じ定義が二重になってエラーになることがある。
* `#pragma once` はそれを防ぐ “ガード”。


## 2. `#include` 群（他ファイルの読み込み）

```cpp
#include "types.hpp"
#include "format_conversion.hpp"
#include "candidate_pairs.hpp"
#include "sampling.hpp"
#include "type_a_violations.hpp"
#include "type_b_violations.hpp"
#include "width_check.hpp"
#include "parallel.hpp"
```

* 2-1 `#include` って何？
  * 「このファイルの内容を、ここに貼り付ける」イメージ。
  * 実際には前処理で展開される。

* 2-2. `"..."` と `<...>` の違い（初歩ポイント）
  * `"types.hpp"` みたいな `" "` は **自分のプロジェクト内**のファイルを探す。　#include "types.hpp"
  * `<iostream>` みたいな `< >` は **標準ライブラリ**を探す。　#include <iostream>

* 2-3. 何を読み込んでるか（ざっくり）
  * `types.hpp`：`Polygon` や `Violation` みたいな型定義があるはず
  * `format_conversion.hpp`：画像/ファイル → Polygon配列へ変換
  * `candidate_pairs.hpp`：ポリゴンの「検査候補ペア」を作る
  * `sampling.hpp`：代表点・代表エッジのサンプリング
  * `type_a_violations.hpp`：Type A違反検出
  * `type_b_violations.hpp`：Type B違反検出
  * `width_check.hpp`：幅違反検出
  * `parallel.hpp`：並列化処理（スレッド）関連

このファイル自体は「中身を実装する」より「呼び出す」役。


## 3. `namespace easymrc {`

```cpp
namespace easymrc {
```

* namespace って何？
  * 名前の衝突を防ぐ「名前のグルーピング」。
  * 例えば `EasyMRC` というクラス名が他のライブラリにもあったら衝突する。
  * だから `easymrc::EasyMRC` という “フルネーム” にする。

この `{` の中は全部 `easymrc::` が付く世界。

 
## 4. クラス定義：`class EasyMRC`

```cpp
// Complete MRC checking pipeline
class EasyMRC {
```

* `class` は「型（設計図）」を定義する。
* `EasyMRC` はクラス名。
* `{` から `};` までがクラスの中身。


## 5. `public:`（外から使える部分）

```cpp
 public:
```

* public / private の初歩
  * `public:` 以降は外部からアクセスできる（使える）
  * `private:` 以降はクラス内部だけで使える（隠す）

EasyMRCは「外に見せたいもの」と「内部実装」を分けている。

 
### 5.1 `struct Config`（設定をまとめる型）

```cpp
  struct Config {
```

* `struct` も「型（設計図）」を作る。
* `class` と似てるが、初歩としては「structはデータ入れ物」と思ってOK。
* `Config` は EasyMRC の中にある型なので、正式には `easymrc::EasyMRC::Config`。


#### Configのメンバ変数（設定項目）

```cpp
    double rule_distance_R;             // Space/width rule distance
    double sampling_radius_multiplier;  // Usually 4.0
    int num_threads;                    // 0 = auto-detect
    bool enable_space_check;
    bool enable_width_check;
    bool enable_parallel;
```

初歩の型説明つきで読む：

* `double`：小数（例：50.0）
* `int`：整数（例：0, 10）
* `bool`：真偽（true/false）

それぞれの意味：

* `rule_distance_R`：ルール距離R（スペース/幅チェックの基準距離）
* `sampling_radius_multiplier`：サンプリング半径を調整する倍率（だいたい4.0）
* `num_threads`：スレッド数（0なら自動検出）
* `enable_space_check`：空間ルールチェックを有効にするか
* `enable_width_check`：幅チェックを有効にするか
* `enable_parallel`：並列化を有効にするか


#### `Config()`（コンストラクタ：初期値を入れる）

```cpp
    Config()
        : rule_distance_R(50.0),
          sampling_radius_multiplier(4.0),
          num_threads(0),
          enable_space_check(true),
          enable_width_check(true),
          enable_parallel(true) {}
```

ここが初歩で一番引っかかる。

#### これは何をしてる？

* `Config()` は「Configオブジェクトを作るときに自動で呼ばれる初期化関数」
* `:` から始まる部分は **初期化リスト**（メンバ変数に初期値を入れる場所）

#### つまり

Configを何も指定せず作ると：

* R = 50.0
* multiplier = 4.0
* threads = 0（自動）
* space/width/parallel = true（全部ON）

最後の `{}` は「このコンストラクタの中身は空」という意味。


### 5.2 `struct Results`（結果をまとめる型）

```cpp
  struct Results {
```

* `Results` も EasyMRC 内の型（`EasyMRC::Results`）

```cpp
    std::vector<Violation> space_violations_type_a;
    std::vector<ViolationTypeB> space_violations_type_b;
    std::vector<WidthViolation> width_violations;
```

#### 初歩：`std::vector<T>` は「可変長配列」
* 結果を入れる配列（vector）
* Pythonの `list` に近い
* `<Violation>` みたいに中身の型を指定する

#### 分解すると

* `std::` → 標準ライブラリの名前空間
* `vector` → 可変長配列（Pythonの `list`）
* `<Violation>` → 中に入れる型(自作)を指定
* `space_violations_type_a` → 変数名


<!-- ```cpp
struct Violation {
  Point point1, point2;
  double distance;
  int polygon_id_1, polygon_id_2;

  Violation() : distance(0), polygon_id_1(-1), polygon_id_2(-1) {}

  Violation(const Point& p1, const Point& p2, double dist, int pid1, int pid2)
      : point1(p1), point2(p2), distance(dist),
        polygon_id_1(pid1), polygon_id_2(pid2) {}
};
``` -->

<!-- ### Pythonで書くと

```python
space_violations_type_a = []
```
Pythonは、

　　 xs = []
　　 xs.append(1)
　　 xs.append("abc")

といったように型を混ぜることができるが、
C++はできなためリストの型まで定義する。 -->


#### その場で集計する関数（メンバ関数）

```cpp
    int total_space_violations() const {
      return space_violations_type_a.size() +
             space_violations_type_b.size();
    }
```

* `int total_space_violations()`：戻り値がintの関数
* `const`：この関数は **Resultsの中身を変更しない** という約束
* `return ...;`：値を返す

`.size()` って何？
* vectorのメンバ関数
* 要素数（何個入ってるか）を返す（型は `size_t` だがここでは int に足してる）

そして

* TypeAの件数 + TypeBの件数 を返す


#### 全違反数を返す関数

```cpp
    int total_violations() const {
      return total_space_violations() + width_violations.size();
    }
```

* `total_space_violations()` の結果 + 幅違反の数
* ここでも `const` なので中身を書き換えない


### 5.3 EasyMRCのコンストラクタ

```cpp
  EasyMRC(const Config& config = Config()) : config_(config) {}

  コンストラクタ(引数) : メンバ1(初期値), メンバ2(初期値) { 本体 }
　→コンストラクタ初期化の構文
　　メンバは変数のこと。つまり構造体の中に複数の変数があればそれら全てに初期値を設定できる。
```

これは「EasyMRCを作るとき」の初期化

大きく分けると3つ
* コンストラクタの宣言部分
  EasyMRC(const Config& config = Config())
* メンバ初期化リスト
  : config_(config)
* コンストラクタ本体
  {}

---

EasyMRC(const Config& config = Config())

* EasyMRC      このクラスのコンストラクタ
* `Config`     型の引数を受け取る
* `&`          参照（コピーせずに渡す）
* `const`      「この引数をこの中で書き換えない」
* config       引数名
* `= Config()` 引数が渡されなかった場合はデフォルトConfig()
  
つまり：
> Configをコピーせず受け取る。中身は変更しない。
> 引数を省略したら `Config()` を自動で使う。

```cpp
EasyMRC checker;
```

でも作れるし、内部的には

```cpp
EasyMRC checker(Config());
```

と同じになる。

---

`: config_(config)`

初期化リスト

* EasyMRC型        → このクラスのメンバ変数
* Config型         → メンバ変数 config_
* (初期化値)       → 引数 config で初期化する

EasyMRCのメンバ変数 `config_` に、引数 `config` をコピーして入れる。

---

{}
コンストラクタを作った後に何か処理が必要なときはここに書く。


### 5.4 `run()`：ポリゴン配列からチェック

```cpp
  // Run complete MRC check
  Results run(const std::vector<Polygon>& polygons) {
```

#### 関数の形

* 戻り値：`Results`
* 関数名：`run`
* 引数：`const std::vector<Polygon>& polygons`

#### 引数の読み方（初歩）

* `std::vector<Polygon>`：Polygonの配列
* `&`：参照（大きい配列をコピーしない）
* `const`：この関数内で polygons を変更しない


#### 結果箱を作る

```cpp
    Results results;
```

* Results型のオブジェクト results を生成
* これが最終的に返される


#### スペースチェックをするか？

```cpp
    if (config_.enable_space_check) {
      check_space_rules(polygons, results);
    }
```

* `if (条件) { ... }` は条件分岐
* `config_` は EasyMRC のメンバ変数
* `config_.enable_space_check` が true のときだけ実行

`check_space_rules(...)` は privateの関数（後で出てくる）
引数：

* polygons（入力）
* results（結果をここに追加していく）


#### 幅チェックをするか？

```cpp
    if (config_.enable_width_check) {
      check_width_rules(polygons, results);
    }
```

同様に enable_width_check が true のときだけ実行。



### 5.5 `run_from_image()`：ファイル入力からチェック

```cpp
  // Run MRC from image file
  Results run_from_image(const std::string& image_file) {
```

* 戻り値：Results
* 引数：`const std::string& image_file`

`std::string` は文字列型（Pythonのstr）
`&` は参照、`const` は変更しない。


#### 画像→ポリゴン変換

```cpp
    auto polygons = format_conversion(image_file);
```

ここは前に聞いてた構文。

* `format_conversion(image_file)` が `std::vector<Polygon>` を返す想定
* `auto` は「右辺の型をコンパイラが推論して決める」

つまりこれはほぼ：

```cpp
std::vector<Polygon> polygons = format_conversion(image_file);
```

と同じ。
 

最後に、run(polygons) を呼んで返す

return run(polygons);

* `run(polygons)` の結果をそのまま返している
* run_from_image は「前処理（変換）だけしてrunへ渡す」ラッパー

 

## 6. `private:`（内部だけの関数と変数）

```cpp
 private:
```

ここから下は外部から使えない。
EasyMRC内部専用。

 
### 6.1 設定を保持するメンバ変数

```cpp
  Config config_;
```

* `Config` 型のメンバ変数
* 名前が `config_` なのは「メンバ変数だよ」という命名流儀（末尾に _）

つまり EasyMRCは「設定を1個持つ」クラス。

 
### 6.2 `check_space_rules()`：スペース違反処理の本体

```cpp
void check_space_rules(const std::vector<Polygon>& polygons,
                      Results& results) {
```

- 戻り値が `void`:
 * 返す値はない
 * 結果は `results` に追加していくスタイル

- 引数:
  * `polygons`：入力（const参照）
  * `Results& results`：結果（参照で受け取り、書き換える）

ここで const が付いてないのが重要：

> results は中身を変更する（追加する）から const にできない


#### 候補ペア生成

```cpp
auto pairs = candidate_pair_generation(polygons, config_.rule_distance_R);
```

* 戻り値は `auto` で受ける（多分 `std::vector<std::pair<int,int>>` みたいな形）
* `candidate_pair_generation(...)` は別ファイルの関数　easymrc/candidate_pairs.hpp
* 引数：polygons型　`config_.rule_distance_R`


#### 並列にするか？

```cpp
if (config_.enable_parallel && pairs.size() > 10) {
```

* 条件：
  1. enable_parallel が true
  2. pairsの数が10より大きい
     このとき並列処理へ。

```cpp
parallel_space_check(polygons, pairs, config_.rule_distance_R,
                    results.space_violations_type_a,
                    results.space_violations_type_b,
                    config_.sampling_radius_multiplier,
                    config_.num_threads);
```

* `parallel_space_check(...)` を呼ぶ   easymrc/parallel.hpp
* 引数が多いので、設定や出力先を全部渡している
* `results.space_violations_type_a` などは Resultsのメンバ変数（vector）
* 並列処理の中でそれらに追加される想定

 
#### そうでなければ逐次実行

```cpp
} else {
  for (const auto& pair : pairs) {
```

* `for (A : B)` は「Bの要素を順にAに入れて回す」
* `const auto& pair`：
  * `auto`：型推論（pairの型）
  * `&`：参照（コピーしない）
  * `const`：pairは変えない

```cpp
const auto& poly1 = polygons[pair.first];
const auto& poly2 = polygons[pair.second];
```

* `polygons[...]` は配列アクセス
* `pair.first` と `pair.second` は pair型の要素（1個目と2個目）
* `const auto&` なのでコピーせず参照で受け取る

つまり：

> 「このペアが指す2つのPolygonを取り出した」


#### サンプリング半径を計算

```cpp
double r1 = calculate_sampling_radius(poly1,
                                     config_.sampling_radius_multiplier);
double r2 = calculate_sampling_radius(poly2,
                                     config_.sampling_radius_multiplier);
double r = std::max(r1, r2);
```

* `double r1`：poly1用半径
* `calculate_sampling_radius(...)`   ：    sampling.hpp
* multiplierを渡して半径を調整
* `std::max(r1, r2)`：大きい方を取る（標準ライブラリのmax）

 
#### 代表点・代表エッジの入れ物を用意

```cpp
std::vector<RepresentativePoint> rep_points_1, rep_points_2;
std::vector<RepresentativeEdge> rep_edges_1, rep_edges_2;
```

* 型<>  は配列みたいなもの　配列型<配列名>　= [rep_points_1, rep_points_2]　　みたいな感じかな
* 4つのvectorを作っている
* `RepresentativePoint` と `RepresentativeEdge` は types.hpp で定義されてる型

 

#### サンプリング実行

```cpp
sample_representatives(poly1, r, rep_points_1, rep_edges_1);
sample_representatives(poly2, r, rep_points_2, rep_edges_2);
```

* `sample_representatives(...)` に渡すと、　sampling.hpp
* 代表点vector
* 代表エッジvector
  が埋まる（参照渡しのはず）



#### Type A / Type B の違反検出

```cpp
auto vio_a = detect_type_a_violations(rep_points_1, rep_points_2,
                                      config_.rule_distance_R, r);
auto vio_b = detect_type_b_violations(rep_points_1, rep_points_2,
                                      rep_edges_1, rep_edges_2,
                                      config_.rule_distance_R, r);
```

* `detect_type_a_violations(...)` の戻り値を `vio_a` に
* `detect_type_b_violations(...)` の戻り値を `vio_b` に
* 戻り値は多分 `std::vector<Violation>` 系なので `auto` を使ってる

 

#### 結果を results に追加（insert）

```cpp
results.space_violations_type_a.insert(
    results.space_violations_type_a.end(),
    vio_a.begin(), vio_a.end());
```

初歩分解：

* `results.space_violations_type_a` は vector
* `insert(pos, first, last)` は「範囲をまとめて挿入」
* `end()` は「末尾」
* `vio_a.begin()`〜`vio_a.end()` は vio_a の全要素

つまり：

> vio_a の中身を results の末尾に全部追加

 

同様にTypeBも追加：

```cpp
results.space_violations_type_b.insert(
    results.space_violations_type_b.end(),
    vio_b.begin(), vio_b.end());
```


## 7. `check_width_rules()`：幅違反処理の本体

```cpp
void check_width_rules(const std::vector<Polygon>& polygons,
                      Results& results) {
```

構造は check_space_rules と同じ：

* polygonsは読み取り専用
* resultsに追記する
 

### 7.1 並列条件

```cpp
if (config_.enable_parallel && polygons.size() > 10) {
```

* enable_parallel が true
* polygonsが10個より多い
  なら並列へ

 
#### 並列幅チェック

```cpp
results.width_violations = parallel_width_check(
    polygons, config_.rule_distance_R,
    config_.sampling_radius_multiplier,
    config_.num_threads);
```

* `parallel_width_check(...)` の戻り値を   parallel
* `results.width_violations` に丸ごと代入

ここは space の方と違って「戻り値で返す」設計になってる。



### 7.2 逐次幅チェック

```cpp
} else {
  for (const auto& poly : polygons) {
```

* polygonsの各polyを順に処理

 
#### polyごとにサンプリング半径

```cpp
double r = calculate_sampling_radius(poly,
                                     config_.sampling_radius_multiplier);
```

* polyごとに r を計算


#### 幅違反検出

```cpp
auto violations = check_width_violations(poly,
                                        config_.rule_distance_R, r);
```

* `check_width_violations(...)` が violations（多分vector）を返す


#### 結果を追加

```cpp
results.width_violations.insert(results.width_violations.end(),
                               violations.begin(), violations.end());
```

* insertでまとめて末尾に追加
* for終了
* else終了
* check_width_rules終了


## 8. このhppの使い方（初歩の視点で）

EasyMRCのクラスで定義されていること
- Config   設定ファイルの構造
- Results  結果の構造

- Config を受け取って config_ 関数の定義
  check_space_rules
    　1. 候補ペア生成：　　candidate_pair_generation　(candidate_pairs.hpp)
    　2. 並列処理：　　　　parallel_space_check       (parallel.hpp)
    　3. 逐次処理
  check_width_rules
      1. 並列処理　　　　　parallel_width_check　　　　(parallel.hpp)
      2. 逐次処理　　　　　calculate_sampling_radius　(sampling.hpp)
   　　　　　　　　　　　　　check_width_violations

<!--  
外部からはこう使う想定：

```cpp
#include "easymrc.hpp"

int main() {
  easymrc::EasyMRC::Config cfg;
  cfg.rule_distance_R = 60.0;
  cfg.enable_parallel = true;

  easymrc::EasyMRC checker(cfg);

  std::vector<Polygon> polygons = format_conversion("input.pgm");
  auto results = checker.run(polygons);

  std::cout << results.total_violations() << "\n";
}
```

（ここまでの説明が入っていれば、全部読めるはず） -->
