# C++初心者向け

## 関数の呼び出し方が複数ある理由 ― 構造で理解する資料

## 全体像（まず地図）

C++で出てくる「呼び出し方の違い」は、**実は4種類だけ**です。

| 見た目                 | 正体                     |
| ------------------- | ---------------------- |
| `func()`            | 普通の関数                  |
| `namespace::func()` | 名前空間付き関数               |
| `object.func()`     | 型（クラス）のメンバ関数           |
| `Type::func()`      | 型に属する関数（static / 関連付け） |

👉 **見た目が違う＝所属が違う**だけ

   

## `config = load_rule_file(rule_file);`

`std::vector<Polygon> polygons = format_conversion(input_file);`

 => int x = f();

### これは何？

👉 **ただの関数呼び出し**

### 正体

```cpp
load_rule_file(...)   // 関数
```

* Pythonでいう：

  ```python
  config = load_rule_file(rule_file)
  ```
* Bashでいう：

  ```bash
  config=$(load_rule_file rule_file)
  ```

### なぜ `std::` が無い？

* この関数は

  * **同じファイル**
  * もしくは **同じ名前空間**
    にあるから

👉 **「今いる場所から見えている関数」はそのまま呼べる**

   

## ② `std::vector<Polygon> polygons = format_conversion(input_file);`

### 疑問ポイント

> ファイル内に無い関数なのに、なぜ直接呼べる？

### 答え

**「宣言が見えている」から**

   

### C++の超重要ルール

> 関数は
> **定義が別ファイルでも、宣言が見えていれば呼べる**

つまり：

```cpp
// どこかのヘッダに
std::vector<Polygon> format_conversion(const std::string&);
```

これがあれば、

```cpp
format_conversion(input_file);
```

と書ける。

👉 **ファイル内かどうかは関係ない**

   

## ③ `std::cout << ...`

### これは何？

👉 **名前空間 `std` に入っているオブジェクト**

```cpp
std::cout
```

* `std` = 標準ライブラリの箱
* `cout` = その中にある変数（オブジェクト）

### なぜ `.` じゃない？

```cpp
std::cout.size()  // ← これは cout のメンバ
```

ここで初めて **`.` が出る**

   

## ④ `polygons.size()`

### これは何？

👉 **型にくっついた関数（メンバ関数）**

```cpp
polygons.size()
```

* `polygons` は `std::vector<Polygon>`
* `size()` は vector型の機能

Pythonで完全に同じ👇

```python
len(polygons)
polygons.append(x)
```

👉 **データ自身が機能を持っている**

   

## ⑤ `using namespace easymrc;`

### これは何をしている？

```cpp
using namespace easymrc;
```

=

> **「easymrc:: を毎回書くのを省略する」**

### これがあると

```cpp
easymrc::EasyMRC checker(config);
```

↓ 省略できる

```cpp
EasyMRC checker(config);
```

👉 **名前空間の省略宣言**

Pythonでいう：

```python
from easymrc import *
```

にかなり近い。

   

## ⑥ `EasyMRC checker(config);`

### これは関数？変数？

👉 **変数定義**

   

### 分解すると

```cpp
EasyMRC checker(config);
```

| 部分         | 意味          |
| ---------- | ----------- |
| `EasyMRC`  | 型（クラス）      |
| `checker`  | 変数名         |
| `(config)` | コンストラクタ呼び出し |

Pythonで言うと👇

```python
checker = EasyMRC(config)
```

👉 **クラスからオブジェクトを作っている**

   

## ⑦ ここが最大の疑問

### `EasyMRC::Config load_rule_file(const std::string& filename) { ... }`

> これは何？？
> 型定義？変数？関数？

   

## ⑧ 正体：**関数定義**

これは **関数を定義している行**です。

   

### 分解して見る

```cpp
EasyMRC::Config   load_rule_file   (const std::string& filename)
```

| 部分                | 意味    |
| ----------------- | ----- |
| `EasyMRC::Config` | 戻り値の型 |
| `load_rule_file`  | 関数名   |
| `(filename)`      | 引数    |

👉 **「Config型を返す関数」**

   

### `{}` の中は？

```cpp
{
    // 処理内容
}
```

👉 **関数の本体**

Pythonで完全に同じ👇

```python
def load_rule_file(filename) -> Config:
    ...
```

   

## ⑨ なぜ `EasyMRC::Config` なの？

### 理由

`Config` は **EasyMRCクラスの中で定義された型**だから。

```cpp
class EasyMRC {
public:
    struct Config {
        ...
    };
};
```

👉 **型もクラスに所属できる**

   

## ⑩ つまりこの1行を日本語にすると

```cpp
EasyMRC::Config load_rule_file(...)
```

＝

> **「EasyMRCの中で定義されたConfig型を返す
> load_rule_fileという関数を定義します」**

よくあるC言語と同じ形です：
```
int x = f();
```
   

## ⑪ なぜ `{}` が関数っぽいのか？

👉 **関数そのものだから**

```cpp
return config;
```

が中にあるはず。

   

## ⑫ ここまでの全体整理（超重要）

### C++で混乱する原因はこれ

> **関数・型・名前空間・オブジェクトが
> 全部「::」と「.」で表現される**

でもルールは単純👇

  

### 見た目ルールまとめ

| 書き方               | 意味      |
| ----------------- | ------- |
| `func()`          | 普通の関数   |
| `ns::func()`      | 名前空間の関数 |
| `Type var;`       | 変数定義    |
| `Type(var)`       | コンストラクタ |
| `obj.func()`      | メンバ関数   |
| `Type::InnerType` | 型の中の型   |

   

## ⑬ 最後に一言（重要）

あなたはもう **「理解できる所まで来ている」**。

混乱している理由は
**「ルールが分からない」のではなく
「全部同時に出てきている」だけ**。

   

## 次にやると一気に楽になるテーマ（おすすめ順）

1️⃣ **関数宣言 vs 定義（.hpp / .cpp 分離）**
2️⃣ **コンストラクタとは何か**
3️⃣ **Type::func と obj.func の違い**

どれから行く？
資料形式でそのまま続けられるよ。
