# main.cpp 詳細解説（C++初心者向け）

このドキュメントは `src/main.cpp` を **「行ごとに何をしているか」レベルで** かなり細かく説明したものです。C++初心者向けに、基本的な構文や典型的な流れも補足しています。

## 1. まず全体像（ざっくり）

この `main.cpp` は、**マスクルールチェック（MRC）** を実行するためのコマンドラインプログラムの「入口（main関数）」です。大まかな流れは次の通りです。

1. ルール設定ファイルを読み込む
2. 入力画像を読み込み、ポリゴンに変換する
3. EasyMRC を実行して違反を検出する
4. 結果を JSON 形式で保存する

 

## 2. インクルード（#include）の意味

```cpp
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <algorithm>

#include "easymrc/easymrc.hpp"
```

### 標準ライブラリ（<...>）
- `<iostream>`: `std::cout`/`std::cerr` などの入出力
- `<fstream>`: ファイルの読み書き（`std::ifstream`, `std::ofstream`）
- `<string>`: 文字列（`std::string`）
- `<chrono>`: 時間計測（実行時間の計測）


### プロジェクト固有（"...")
- `"easymrc/easymrc.hpp"`
  - EasyMRC というライブラリの本体
  - `EasyMRC` クラスや `Polygon` などの型がここに定義されている

 

## 3. `using namespace easymrc;`

```cpp
using namespace easymrc;
```

- `easymrc::EasyMRC` のように書く代わりに、`EasyMRC` と直接書けるようにする宣言
- 便利だが、名前衝突の可能性があるので大規模なコードでは注意
- このファイルは小さいので、わかりやすさ重視で使っている

 

## 4. 文字列の前後の空白を削る関数 `trim`

C++の関数の作り方
```cpp
返り値の型 関数名(引数の型 引数の名前, ...) {
  // 処理
  return 返す値;
}
```

```cpp
std::string trim(const std::string& str) {
  size_t start = str.find_first_not_of(" \t\r\n");
  if (start == std::string::npos) return "";
  size_t end = str.find_last_not_of(" \t\r\n");
  return str.substr(start, end - start + 1);
}
```

#### 関数の目的：
　設定ファイルの1行を解析するとき、前後の空白を無視したい。
  例：
  ```
  rule_distance: 50.0
  ```
  この「`rule_distance`」「`50.0`」を正しく取り出すために使う。

#### 行ごとの説明
- `std::string  　　　　　　　返り値の型(C++標準の文字列型)
- trim　　　　　　　　　　　　　関数名
- (const std::string& str)  引数の型　引数の名前
  - 入力文字列 `str` を受け取って、空白を削って
  - 変更不可(const)の入力文字列の参照で返す

- *C++標準のメンバ関数を使用*
- `.find_first_not_of(" \t\r\n");`
  - 先頭から見て「空白でない最初の位置」を探す
  - 空白として `space, tab, CR, LF` を対象にしている

- `if (start == std::string::npos) return "";`
  - `npos` は「見つからなかった」ことを意味する特別な値
  - つまり、文字列が全部空白なら空文字を返す

- `.find_last_not_of(" \t\r\n");`
  - 後ろから見て「空白でない最後の位置」を探す

- `.substr(start, end - start + 1);`
  - `start` から `end` までを切り出して返す

 

## 5. ルール設定ファイルの読み込み `load_rule_file`

```cpp
EasyMRC::Config load_rule_file(const std::string& filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open rule file: " + filename);
  }

  EasyMRC::Config config;
  std::string line;
  int line_number = 0;

  while (std::getline(file, line)) {
    line_number++;
    ...
  }

  file.close();
  return config;
}
```

### 目的
ルール設定ファイル（例: `rules.txt`）を読み込み、`EasyMRC::Config` に設定を入れて返す。

- ルールファイルは `key: value` 形式
- `#` 以降はコメントとして無視
- 空行は無視

#### 行ごとの説明
#### ブロック１
- EasyMRC::Config 　　 返り値の型(C++標準の文字列型)
- load_rule_file　　 　関数名
- const std::string&  引数の型
- filename            引数の名前

- std::ifstream file(filename);　　
  
  ```cpp
  入力ファイルストリーム型  変数名(ファイル名);
  ```

  Pythonで書くと。。。
  ```python
  file = open(filename, "r")
  ```

- file.is_open()
  file        ：ifstream型のオブジェクト
  is_open() ：「開いているか？」を返す関数（ifstream型のメンバ関数）
　返り値：
　　　開いている → true
　　　開いていない → false

- throw std::runtime_error("Cannot open rule file: " + filename);
  Cは、thorow catchがないから全て戻り値 return -1 , -2 などでエラーを区分していた
　しかし、戻り値だと使う側が毎回確認しないといけない
　```c
  int ret = read_file(name);
  if (ret != 0) {
  ```
  これは、A → B → C → D　と、呼び出し側が深くなってくると管理が大変
  それに戻り値の確認ミスなども発生する。
   そこでthorw!! 異常なら通常の処理フローから切り離す！みたいにしておけば管理が楽
   exit にしちゃえって意見もあるが、ファイルやメモリのロックなど
   C++でどんなときでもここまではやりたいみたいのがあればそこまでしちゃいたい。よってthorow
   throwは通常、一番近いcatch処理に飛ぶ。


#### ブロック2

```c++
EasyMRC::Config config;
std::string line;

 型 変数名;
```
- EasyMRC  namespace
- Config   構造体名称(中身は `easymrc/easymrc.hpp` で定義されている)
- config   変数

```C++
int line_number = 0;

 型 変数名 = 初期値;
```



#### ブロック3
```cpp
while (std::getline(file, line)) {
  ...
}

while ( 条件 ) {
   処理
}

条件式がFalseになるまでループがまわる
```

- 条件式：　
  `std::getline`　　fileを一行ずつ読み込む標準関数
   一行未満になれば勝手ループを出る

- 処理：
line_number++; 　　　　　　　　　　　　 　 | コメント用にループの数を数えているだけ
size_t comment_pos = line.find('#');　 | size_t 型  :　「位置」や「サイズ」を表すための型
    　　　　　　　　　　　　　　　　　　     |　.find     :  文字 # を 最初に見つけた位置を返す
std::string::npos                      |「見つかりませんでした」を表す特別な値
line = line.substr(0, comment_pos);  　|　line.substr(0, comment_pos)  :0文字目から comment_pos 文字分を取り出す

    まとめると、#の行をlineの中に見つけたら、
    先頭行から#までをlineにいれるよってことをしている。

line = trim(line);            |  空白を削除
if (line.empty()) continue;   |  空行は無視



#### ブロック4

```cpp
// key: value形式を解析
size_t colon_pos = line.find(':');
if (colon_pos == std::string::npos) {
  std::cerr << "Warning: Invalid format at line " 
            << line_number
            << " (expected 'key: value'): " 
            << line << std::endl;
  continue;
}
```
size_t 型        :　「位置」や「サイズ」を表すための型
line.find(':')  :   ':' の位置を探すメンバ関数

もしも std::string::npos　なら、ログ出力して次の行

```cpp
std::cerr << A << B << C;
```
左から順に、文字列や数字を連結して出力
例)　Warning: Invalid format at line 12 (expected 'key: value'): rule_distance 10

C++は勝手に変数を文字列に変換してくれないから、文字列と変数を同じ<<の中に入れると
期待通りにならない。だから<<で、文字列と変数を分けている



#### プロック5

```cpp
std::string key = trim(line.substr(0, colon_pos));
std::string value = trim(line.substr(colon_pos + 1));
```
key : valude のkeyの部分とvalueの部分をそれぞれ変数に格納



#### プロック6
```cpp
if (key == "rule_distance") {
  config.rule_distance_R = std::stod(value);
} else if (key == "sampling_multiplier") {
  config.sampling_radius_multiplier = std::stod(value);
} else if (key == "threads") {
  if (value == "auto" || value == "0") {
    config.num_threads = 0;
  } else {
    config.num_threads = std::stoi(value);
  }
} else if (key == "space_check") {
  config.enable_space_check = (value == "true" || value == "1");
} else if (key == "width_check") {
  config.enable_width_check = (value == "true" || value == "1");
} else if (key == "parallel") {
  config.enable_parallel = (value == "true" || value == "1");
} else {
  std::cerr << "Warning: Unknown parameter '" << key
            << "' at line " << line_number << std::endl;
}
```
keyは６パターン：
  rule_distance, sampling_multiplier, threads
  space_check, width_check, parallel

- rule_distance
  config.rule_distance_R = std::stod(value);

  > config　　　　　　　　　　 : line25の”EasyMRC::Config config;”で定義した構造体
  > config.rule_distance_R ： Configの中の"rule_distance_R"という変数
  > std::stod(value)        : std名前空間の"stod関数(string to double 変換する関数)"

  config.num_threads = std::stoi(value);

  > config.num_threads ： Configの中の"num_threads"という変数
  > std::stoi(value)   : std名前空間の"stoi関数(string to int 変換する関数)"
  > `threads` が `auto` や `0` の場合は「自動」として `num_threads=0`

  config.enable_space_check = (value == "true" || value == "1");

  > () は式を表す。つまりvalue が true または 1 だったら代入する 


#### プロック7

```cpp
file.close();
return config;
```

`ifstream` はスコープを抜けると自動で閉じられるので、`close()` は必須ではない

 



## 6. 使用方法の表示 `print_usage`

```cpp
void print_usage(const char* program_name) {
  std::cerr << "Usage: " << program_name << " <input_file> <output_file> <rule_file>\n";
  ...
}
```

`std::cerr` はエラー出力。
`program_name` は `argv[0]` から渡される（実行ファイル名）。

 

## 7. JSON出力の保存 `write_json_output`

```cpp
void write_json_output(const std::string& filename,
                      const EasyMRC::Results& results,
                      double execution_time_ms) {
  std::ofstream out(filename);
  if (!out.is_open()) {
    throw std::runtime_error("Cannot open output file: " + filename);
  }

  out << "{\n";
  ...
  out << "}\n";
}
```

### 目的
EasyMRC が検出した違反情報を JSON 形式で保存する。

### 行ごとの説明
#### ブロック１

```cpp
void write_json_output(const std::string& filename,
                      const EasyMRC::Results& results,
                      double execution_time_ms) {
```

```cpp
返り値の型 関数名(引数の型 引数の名前, ...) {
  // 処理
  return 返す値;
}
```
- 引数：
  const std::string& filename,　　  | file名称
  const EasyMRC::Results& results, | `EasyMRC::Results` は検出結果の構造体（詳細はライブラリ側）
  double execution_time_ms         | double型


#### ブロック２

```cpp
std::ofstream out(filename);

入力ファイルストリーム型  変数名(ファイル名);
```

Pythonで書くと。。。
```python
file = open(filename, "r")
```

```cpp
if (!out.is_open()) {
  throw std::runtime_error("Cannot open output file: " + filename);
}
```

- file.is_open()
  file        ：ifstream型のオブジェクト
  is_open() ：「開いているか？」を返す関数（ifstream型のメンバ関数）
　返り値：
　　　開いている → true
　　　開いていない → false


```cpp
out << "{\n";
out << "  \"execution_time_ms\": " << execution_time_ms << ",\n";
out << "  \"space_violations\": {\n";
out << "    \"type_a\": [\n";
```
outに書き込み


```cpp
for (size_t i = 0; i < results.space_violations_type_a.size(); ++i) {

for ( 初期化 ; 継続条件 ; 更新処理 )
```

初期値 　　　　size_t i = 0
条件　　　　　　i < results.space_violations_type_a.size(
更新処理      ++i


resultにメンバ関数　space_violation_type_a　をしている
result の型は　EasyMRC::Results& 　だから　EasyMRCのネームスペースのどっかにResultsがあるはず

```cpp
for (size_t i = 0; i < results.space_violations_type_b.size(); ++i) {
for (size_t i = 0; i < results.width_violations.size(); ++i) {
```


出力例：
```json
{
  "execution_time_ms": 123,
  "space_violations": {
    "type_a": [ ... ],
    "type_b": [ ... ]
  },
  "width_violations": [ ... ],
  "summary": {
    "total_space_violations": 10,
    "total_width_violations": 5,
    "total_violations": 15
  }
}
```

## 8. `main` 関数（プログラムの入口）

1. 引数チェック
2. 設定ファイル読み込み
3. 入力ファイルの拡張子チェック
4. 画像を読み込みポリゴンへ変換
5. MRC実行
6. JSONファイル出力

```cpp
int main(int argc, char* argv[]) {
  ...
}
```

`argc` は「引数の数（実行ファイル名含む）」
`argv` は「引数の値」


#### ブロック1　引数チェック

```cpp
if (argc != 4) {
  std::cerr << "Error: Expected exactly 3 arguments, got " << (argc - 1) << std::endl;
  print_usage(argv[0]);
  return 1;
}
```

- `argc` は「引数の数（実行ファイル名含む）」
- 期待するのは `3` 個（入力・出力・ルール） + 実行ファイル名で **4**
- 間違いなら使い方を表示して終了


```cpp
std::string input_file = argv[1];
std::string output_file = argv[2];
std::string rule_file = argv[3];
```


#### ブロック2　設定ファイル読み込み

```cpp
EasyMRC::Config config;
try {
  config = load_rule_file(rule_file);
} catch (const std::exception& e) {
  std::cerr << "Error loading rule file: " << e.what() << std::endl;
  return 1;
}
```

- `load_rule_file` が例外を投げる可能性があるので `try/catch` で捕まえる
- エラーならメッセージを出して終了



#### ブロック3　入力ファイルの拡張子チェック

```cpp
std::string ext = input_file.substr(input_file.find_last_of(".") + 1);
if (ext != "pgm" && ext != "png" && ext != "ppm") {
  std::cerr << "Error: Unsupported file type '" << ext << "'. "
            << "Supported formats: pgm, png, ppm\n";
  return 1;
}
```

- `input_file` の最後の `.` 以降を拡張子として取り出す
- 対応は `pgm`, `png`, `ppm` のみ
- それ以外なら終了

 
#### ブロック4　　画像を読み込みポリゴンへ変換


ヘッダ表示と設定の出力
```cpp
std::cout << "========================================\n";
std::cout << "EasyMRC - Efficient Mask Rule Checking\n";
std::cout << "========================================\n\n";
```

```cpp
std::cout << "Configuration:\n";
std::cout << "  Input file: " << input_file << "\n";
std::cout << "  Rule distance: " << config.rule_distance_R << "\n";
std::cout << "  Sampling multiplier: "
          << config.sampling_radius_multiplier << "\n";
```

```cpp
std::cout << "  Threads: ";
if (config.num_threads == 0) {
  std::cout << "auto (" << std::thread::hardware_concurrency() << ")\n";
} else {
  std::cout << config.num_threads << "\n";
}
```

- `num_threads == 0` は「自動」
- `std::thread::hardware_concurrency()` でCPUスレッド数を表示

```cpp
std::cout << "  Space check: "
          << (config.enable_space_check ? "enabled" : "disabled") << "\n";
std::cout << "  Width check: "
          << (config.enable_width_check ? "enabled" : "disabled") << "\n";
std::cout << "  Parallel: "
          << (config.enable_parallel ? "enabled" : "disabled") << "\n\n";
```

- `?:` は「三項演算子」
- 条件が true なら前、false なら後ろの文字列


画像読み込み → ポリゴン抽出

```cpp
std::cout << "Loading image file...\n";
std::vector<Polygon> polygons = format_conversion(input_file);
std::cout << "  Polygons extracted: " << polygons.size() << "\n\n";
```

- `format_conversion` は画像ファイルからポリゴンを生成する関数
- 定義は `easymrc` 側にある
- 結果は `std::vector<Polygon>`（ポリゴンの配列）
  
よくあるC言語と同じ形です：
```
std::vector<Polygon> polygons = format_conversion(input_file);

int x = f();
```
  

#### ブロック5  EasyMRC を実行

```cpp
EasyMRC checker(config);
```

EasyMRC    型　
checker    変数名
(config)   引数

例： ファイルの読み込み   std::ifstream file(filename);

- 型　変数名          std::ifstream file
  fileという変数を定義している

- 型 変数名(引数);    std::ifstream file(filename);
  入力ファイルストリーム型 変数 file を作り
  同時に filename のファイルを開く
 　 →　C++ではコンストラクタの初期化って言う
　「この型の変数を、入力を使って作れ」って意味

std::ifstream file = filename;  // ← 実はこれもOK　同じ意味だよ！コンストラクタの初期化




```cpp
auto start = std::chrono::high_resolution_clock::now();
auto results = checker.run(polygons);
auto end = std::chrono::high_resolution_clock::now();
```

- `EasyMRC checker(config);` で設定を渡して MRC 実行クラスを作成
- `checker.run(polygons)` で実際のチェックを実行
- 実行時間を `chrono` で測定

オブジェクト生成。これをやればメンバ関数が使用できる。
```
クラス名 変数名(引数);
```
※struct や class で作った「型」を使って変数を作ると、
その変数（オブジェクト）に対してメンバ関数が使えるようになる。


#### ブロック7
実行時間の計算

```cpp
auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
    end - start);
```
- `end - start` は時間差
- ミリ秒単位に変換して `duration` に入れる
- `duration.count()` で数値（整数）として取得

 
#### ブロック8
結果表示

```cpp
std::cout << "\nResults:\n";
std::cout << "  Execution time: " << duration.count() << " ms\n";
std::cout << "  Space violations (type a): "
          << results.space_violations_type_a.size() << "\n";
std::cout << "  Space violations (type b): "
          << results.space_violations_type_b.size() << "\n";
std::cout << "  Width violations: "
          << results.width_violations.size() << "\n";
std::cout << "  Total violations: " << results.total_violations() << "\n\n";
```

- `results` に入っている違反数を表示
- `total_violations()` は EasyMRC 側で合計を返す関数


#### ブロック9
JSONファイルへ出力

```cpp
std::cout << "Writing violations to: " << output_file << "\n";
write_json_output(output_file, results, duration.count());
```

- `write_json_output` を呼んで JSON ファイルを作成

 
#### ブロック10
正常終了メッセージ

```cpp
std::cout << "\n========================================\n";
std::cout << "EasyMRC completed successfully!\n";
std::cout << "========================================\n";

return 0;
```

- `return 0` は「正常終了」

 
#### ブロック11
例外処理

```cpp
} catch (const std::exception& e) {
  std::cerr << "\nError: " << e.what() << std::endl;
  return 1;
}
```

- `try` ブロック内で例外が起きたらここに来る
- `e.what()` はエラー内容の文字列
- `return 1` は「異常終了」

 

## 9. まとめ（このmain.cppがしていること）

1. 引数チェック
2. 設定ファイル読み込み
3. 画像を読み込みポリゴンへ変換
4. MRC実行
6. JSONファイル出力


- ルール設定ファイルを解析し、`EasyMRC::Config` に詰める
- 入力画像（pgm/png/ppm）を読み込んでポリゴンに変換
- EasyMRC でスペース違反・幅違反をチェック
- 結果を JSON で保存
- 実行ログを標準出力に出す