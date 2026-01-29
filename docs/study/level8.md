> ## 優先順位（これだけやれば読める順）
> 
> 1. **`std::string`** 
> 2. 参照 / const    
> 3. 例外処理 
> 4. `ifstream` / `ofstream` + `getline`  
> 5. `vector` + `size_t` + `auto&`
> 6. クラス / namespace（EasyMRC周り） 👈今回はここ！！
> 7. `argc` / `argv`
> 8. `chrono`
> 9. `thread`（おまけ）<br>
><br>
>
<br>
<br>

了解。
では **`argc / argv` → `chrono` → `thread`** を、
これまでと同じ思想で **「HelloWorldから自然に積み上げる」＋「気持ち悪さを潰す」** 形で**まとめて**解説する。
（`---` は使わない）

   

# 第2弾-⑦ CLI・時間・CPU の最小セット

ここは共通して
「**プログラムの外側とやり取りする**」ための機能。

* `argc / argv` → 起動時の入力
* `chrono` → 実行中の時間
* `thread` → CPU資源の情報

   

## 第1章：`argc / argv`（コマンドライン引数）

### 1-1. まず形を見る（HelloWorld拡張）

```cpp
#include <iostream>

int main(int argc, char* argv[]){
    std::cout << argc << "\n";
    return 0;
}
```

### 何が起きている？

* `argc` = 引数の個数
* **必ず 1 以上**（実行ファイル名が入る）

   

### 1-2. `argv` は「文字列の配列」

```cpp
int main(int argc, char* argv[]){
    std::cout << argv[0] << "\n";
}
```

* `argv[0]` → 実行ファイル名
* `argv[1]` → 1番目の引数

Cの配列感覚そのまま。

   

### 1-3. 実行例

```bash
./app input.txt output.txt
```

中身はこう：

* `argc` = 3
* `argv[0]` = "./app"
* `argv[1]` = "input.txt"
* `argv[2]` = "output.txt"

   

### 1-4. C++では string に変換する

```cpp
std::string input = argv[1];
```

👉 **char* → std::string**
（ここで初めて argv の役目が終わる）

   

### 1-5. あなたのコードでの役割

```cpp
if (argc != 4) {
    print_usage(argv[0]);
}
```

意味：

> 「引数が足りなかったら使い方を表示して終了」

   

## 第2章：`chrono`（処理時間を測る）

### 2-1. C的な時間測定の問題

* `clock()` は粗い
* 単位が分かりづらい

   

### 2-2. `chrono` の基本形

```cpp
#include <chrono>

auto start = std::chrono::high_resolution_clock::now();
// 処理
auto end = std::chrono::high_resolution_clock::now();
```

👉 **時刻を2回取るだけ**

   

### 2-3. 差を計算する

```cpp
auto diff = end - start;
```

でもこのままでは単位が曖昧。

   

### 2-4. 単位を指定する

```cpp
auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(diff);
std::cout << ms.count() << "\n";
```

意味：

* `duration_cast` → 単位変換
* `count()` → 数値だけ取り出す

   

### 2-5. あなたのコードでの対応

```cpp
auto duration =
    std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
```

👉 **MRC の性能評価**

   

## 第3章：`thread`（おまけ：CPU情報）

### 3-1. まず事実

あなたのコードでは **スレッドを作っていない**。

使っているのはこれだけ：

```cpp
std::thread::hardware_concurrency()
```

   

### 3-2. これは何？

```cpp
unsigned int n = std::thread::hardware_concurrency();
```

意味：

> 「このマシン、だいたい何スレッド並列できる？」

* CPUコア数 or 論理コア数
* あくまで「目安」

   

### 3-3. HelloWorldで試す

```cpp
#include <iostream>
#include <thread>

int main(){
    std::cout << std::thread::hardware_concurrency() << "\n";
}
```

👉 自分の PC の数字が出る。

   

### 3-4. あなたのコードでの意味

```cpp
if (config.num_threads == 0) {
    std::cout << "auto (" << std::thread::hardware_concurrency() << ")\n";
}
```

意味：

> 「スレッド数を指定しなければ
> マシン性能に任せる」

   

## 第4章：3つを一文でつなぐと

あなたの EasyMRC ツールは：

* `argc / argv`
  → 入力ファイル・出力ファイル・ルールを受け取る
* `chrono`
  → 処理がどれくらい速いか測る
* `thread`
  → 並列実行の目安を出す

👉 **完全に「実務CLIツール」**

   

## 第5章：最小統合サンプル（全部入り）

```cpp
#include <iostream>
#include <string>
#include <chrono>
#include <thread>

int main(int argc, char* argv[]){
    if(argc < 2){
        std::cout << "usage: app <name>\n";
        return 1;
    }

    std::string name = argv[1];

    auto start = std::chrono::high_resolution_clock::now();

    std::cout << "Hello " << name << "\n";
    std::cout << "CPU threads: "
              << std::thread::hardware_concurrency() << "\n";

    auto end = std::chrono::high_resolution_clock::now();
    auto ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "time: " << ms.count() << " ms\n";
}
```

   

## 最終到達点

ここまで来たあなたは：

* C構文 → C++実務構文
* 文字列 / ファイル / 例外 / vector
* CLI / 時間 / CPU

を **全部「気持ち悪くならずに」読める状態**。

👉 EasyMRC の main.cpp は
**もう“未知の呪文”ではなく、ただ長いだけのコード**になっているはず。

   

次にやるとしたらおすすめは：

* EasyMRC 全体の「処理フロー図」を作る
* `load_rule_file()` を仕様書レベルに落とす
* 結果構造 `Results` を自分で定義してみる

どこまで行く？
