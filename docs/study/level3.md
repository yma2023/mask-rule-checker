> ## 優先順位（これだけやれば読める順）
> 
> 1. **`std::string`** 
> 2. 参照 / const     👈今回はここ！！
> 3. 例外処理
> 4. `ifstream` / `ofstream` + `getline`
> 5. `vector` + `size_t` + `auto&`
> 6. クラス / namespace（EasyMRC周り）
> 7. `argc` / `argv`
> 8. `chrono`
> 9. `thread`（おまけ）<br>
><br>
>
<br>
<br>


OK。
では **さっきの HelloWorld → std::string の延長線**として、
**「参照 / const」だけをまず丁寧に**いきます。
（今日はここを“完全に腑に落とす”のが目的）

  

# 第2弾-②：参照（`&`）と `const`

※ ここは **CからC++に来た人が一番つまずく所**なので、かなり噛み砕きます。

  

## 第1章：まず「値渡し」を思い出す（Cの感覚）

### 1-1. Cでよくある関数

```c
#include <stdio.h>

void print(int x){
    printf("%d\n", x);
}

int main(void){
    int a = 10;
    print(a);
}
```

### 起きていること

* `a` の **値（10）** がコピーされて `x` に入る
* `x` を変えても `a` は変わらない

👉 **値渡し = コピー**

  

## 第2章：C++でも同じ「値渡し」はある

### 2-1. C++で string を値渡しすると？

```cpp
#include <iostream>
#include <string>

void print(std::string s){
    std::cout << s << "\n";
}

int main(){
    std::string a = "Hello";
    print(a);
}
```

### 何が起きている？

* `a` の中身 `"Hello"` が **丸ごとコピー**
* コピーされたものが `s`

👉 小さい文字列なら問題ない
👉 **巨大な文字列・配列だと重い**

  

## 第3章：参照（`&`）＝「コピーしない」

### 3-1. 参照を使う形

```cpp
void print(std::string& s){
    std::cout << s << "\n";
}
```

### ここが超重要

* `&` は **「元の変数そのものを見る」**
* コピーは作られない

イメージ：

```
a ───▶ "Hello"
s ───▶ 同じ "Hello" を指している
```

👉 **速い・メモリ節約**

  

## 第4章：でも危険になる（勝手に変えられる）

### 4-1. 問題のある例

```cpp
void print(std::string& s){
    s = "Changed!";
}

int main(){
    std::string a = "Hello";
    print(a);
    std::cout << a << "\n";
}
```

### 結果

```
Changed!
```

👉 呼び出し元の `a` が書き換わった
👉 「見るだけ」のつもりが事故る

  

## 第5章：`const` を付ける理由（ここが肝）

### 5-1. 正解の形

```cpp
void print(const std::string& s){
    std::cout << s << "\n";
}
```

### 意味を分解すると

* `&`：コピーしない
* `const`：**中身を書き換え禁止**

👉
**「重くならない ＋ 安全」**

  

## 第6章：HelloWorldで試す参照＋const

### 6-1. まずは値渡し

```cpp
#include <iostream>
#include <string>

void show(std::string s){
    std::cout << s << "\n";
}

int main(){
    std::string msg = "Hello World";
    show(msg);
}
```

### 6-2. 参照に変える（&）

```cpp
void show(std::string& s){
    std::cout << s << "\n";
}
```

### 6-3. const を付ける（完成形）

```cpp
void show(const std::string& s){
    std::cout << s << "\n";
}
```

👉 **この形が実務の基本形**

  

## 第7章：あなたのコードでの対応箇所

### 7-1. trim関数

```cpp
std::string trim(const std::string& str)
```

意味：

* `str` は大きいかもしれない → コピーしない
* 中身を変えたら危険 → `const`

  

### 7-2. JSON出力ループ

```cpp
const auto& vio = results.space_violations_type_a[i];
```

意味：

* `vio` は「resultsの中の1要素」
* コピーすると重い
* 書き換えない → `const`

  

## 第8章：超重要な見分け方（実務基準）

### 関数引数はこう判断する

* **読むだけ** → `const T&`
* **書き換える** → `T&`
* **小さい型（int,double）** → 値渡し

👉 これだけ覚えればOK。

  

## 今の到達点

あなたは今：

* char配列 → `std::string`
* 値渡し → 参照
* 事故防止 → `const`

という **C++の核心ルール**を踏んだ。

  

## 次の章の選択

同じノリで次はどれ行く？

1️⃣ **ifstream / getline**（ファイル読み込みHelloWorld）
2️⃣ **vector + auto&**（配列のC++版）
3️⃣ **例外処理（try/catch）**
4️⃣ **クラス / namespace（EasyMRCの正体）**

どれから行くか言って。
同じく **コメントアウトしながら試せる形**で続ける。
