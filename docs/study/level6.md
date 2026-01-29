> ## 優先順位（これだけやれば読める順）
> 
> 1. **`std::string`** 
> 2. 参照 / const    
> 3. 例外処理 
> 4. `ifstream` / `ofstream` + `getline`  👈今回はここ！！
> 5. `vector` + `size_t` + `auto&`
> 6. クラス / namespace（EasyMRC周り）
> 7. `argc` / `argv`
> 8. `chrono`
> 9. `thread`（おまけ）<br>
><br>
>
<br>
<br>

いい流れ。
では **`vector + size_t + auto&`** を、
さっきの **`ifstream file("rules.txt")` を腑に落とした時と同じレベル感**で、
**気持ち悪さを一つずつ潰す**形で解説するね。

   

# 第2弾-⑤

## `vector` + `size_t` + `auto&` を完全に普通の感覚にする

   

## 第1章：まず C の配列を思い出す（出発点）

### Cでの配列

```c
int a[3] = {1, 2, 3};
```

ここで分かっていること：

* 型は `int`
* 要素数は 3
* サイズは **固定**

👉 Cでは
「配列は作った瞬間に大きさが決まる」

   

## 第2章：Cの配列の不便なところ

### 例えばこうしたい時

```c
// 要素数が実行時に決まる
int n = get_number();
int a[n];  // ← 危険 or コンパイラ依存
```

* 実行時にサイズ変更しづらい
* 大きな配列はスタック破壊の危険
* 自分でサイズ管理が必要

👉 **実務ではつらい**

   

## 第3章：`std::vector` の正体（ここ重要）

### 3-1. `vector` は何者？

```cpp
std::vector<int> v;
```

これは日本語にすると：

> **「int を並べて入れられる箱 v を作る」**

* 中に何個入るかは **まだ決まっていない**
* 必要に応じて勝手に大きくなる

   

### 3-2. C配列との対応

| C          | C++                  |
| ---------- | -------------------- |
| `int a[3]` | `std::vector<int> v` |
| 固定サイズ      | 可変サイズ                |
| 手動管理       | 自動管理                 |

   

## 第4章：`vector` を実際に使ってみる

### 4-1. 要素を追加する

```cpp
#include <vector>
#include <iostream>

int main(){
    std::vector<int> v;

    v.push_back(10);
    v.push_back(20);
    v.push_back(30);

    std::cout << v[0] << "\n";
}
```

👉 `push_back` = **後ろに追加**

   

### 4-2. 初期値付き

```cpp
std::vector<int> v = {1, 2, 3};
```

👉 Cの `{}` 初期化とほぼ同じ感覚。

   

## 第5章：`size_t` が出てきて気持ち悪い問題

### 5-1. `.size()` の戻り値

```cpp
std::vector<int> v = {1,2,3};
std::cout << v.size();
```

ここで返ってくる型が **`size_t`**。

   

### 5-2. `size_t` って何？

一言で言うと：

> **「サイズ専用の整数型」**

* マイナスにならない（符号なし）
* 32bit or 64bit は環境依存

   

### 5-3. なぜ `int` じゃダメ？

* 配列サイズはマイナスにならない
* 巨大配列でも安全に扱える

👉 **設計上の安全装置**

   

## 第6章：for文で一緒に出てくる理由

### よく見る形

```cpp
for(size_t i = 0; i < v.size(); i++){
    std::cout << v[i] << "\n";
}
```

これは日本語にすると：

> 「0 から 要素数-1 まで安全に回す」

👉 `int` でも動くことは多い
👉 **でも size_t を使うのが正解**

   

## 第7章：`auto` が突然出てくる問題

### 7-1. まず auto の正体

```cpp
auto x = 10;
```

これは：

```cpp
int x = 10;
```

と **完全に同じ**。

👉 **型をコンパイラに決めさせる**

   

### 7-2. なぜ vector で auto が必要？

```cpp
std::vector<int> v = {1,2,3};
```

ここから 1 要素取り出す型は？

```cpp
std::vector<int>::value_type
```

👉 書きたくない
👉 だから `auto`

   

## 第8章：`auto&` の意味（ここが核心）

### 8-1. & が付かない場合

```cpp
auto x = v[0];
```

* `x` は **コピー**
* 重いデータだと無駄

   

### 8-2. & を付ける

```cpp
auto& x = v[0];
```

意味：

> **「v[0] そのものを見る」**

   

### 8-3. const を付けた完成形

```cpp
const auto& x = v[0];
```

意味：

> **「コピーしない・書き換えない」**

👉 実務の黄金形。

   

## 第9章：あなたのコードとの対応（重要）

### JSON出力ループ

```cpp
for(size_t i = 0; i < results.width_violations.size(); ++i) {
    const auto& vio = results.width_violations[i];
}
```

これを日本語にすると：

> 「違反リストの i 番目を、
> コピーせず、変更せずに vio として扱う」

   

## 第10章：HelloWorld的ミニ例題（試してほしい）

```cpp
#include <iostream>
#include <vector>

int main(){
    std::vector<int> v = {10,20,30};

    for(size_t i = 0; i < v.size(); i++){
        const auto& x = v[i];
        std::cout << x << "\n";
    }
}
```

### コメントアウト実験

* `const` を消す
* `&` を消す
* `auto` を `int` にする

👉 どれが **意味的に変わるか**確認。

   

## 最後に：気持ち悪さの正体まとめ

| 気持ち悪さ       | 正体          |
| ----------- | ----------- |
| vector      | 可変配列        |
| size_t      | サイズ専用の安全な整数 |
| auto        | 型省略         |
| auto&       | コピー回避       |
| const auto& | 実務最適解       |

   

## 次の自然な流れ

ここまで来たら：

1️⃣ **argc / argv**（CLIツール完成）
2️⃣ **class / namespace（EasyMRCの正体）**
3️⃣ **chrono（処理時間測定）**
4️⃣ **thread（おまけ）**

次、どれ行く？
同じ「気持ち悪さを潰す路線」で続けるよ。
