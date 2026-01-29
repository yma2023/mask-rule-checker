# src/easymrc/parallel.hpp 解説（超詳細）

このファイルは **並列処理（マルチスレッド）** で高速化する部分です。
Type (a)/(b) の空間チェックや幅チェックを複数スレッドで分割実行します。

---

## 1. ParallelSpaceChecker
### コンストラクタ
```cpp
ParallelSpaceChecker(polygons, pairs, R, multiplier, num_threads)
```
- `polygons`：全ポリゴン
- `pairs`：候補ペア（candidate_pairs.hpp で生成）
- `R`：ルール距離
- `multiplier`：サンプリング半径係数
- `num_threads`：0なら自動（CPUコア数）

### スレッド数の決定
```cpp
if (num_threads <= 0) {
  num_threads_ = std::thread::hardware_concurrency();
  if (num_threads_ == 0) num_threads_ = 4;
}
```
- `hardware_concurrency()` は CPU コア数を返す。
- 0が返る場合もあるので、その時は4にする。

---

### check_parallel() の流れ
1. ペア数をスレッド数で分割
2. 各スレッドで担当範囲のペアを処理
3. 各スレッドの結果を集約

#### スレッド内の処理
- 各ペアに対して
  1) サンプリング半径 r の計算
  2) 代表点・代表エッジのサンプリング
  3) Type (a) / Type (b) 違反を検出
  4) スレッド専用の結果配列に保存

#### 結果の集約
- `thread_results_a` / `thread_results_b` を最後に結合。
- ここで `insert` を使って1つの配列にまとめる。

---

## 2. ParallelWidthChecker
### check_parallel() の流れ
- ポリゴン単位で分割して並列処理。
- 各スレッドが `check_width_violations` を実行。
- 最後に結果を結合する。

---

## 3. インライン関数
### parallel_space_check
- `ParallelSpaceChecker` を簡単に使うための関数。

### parallel_width_check
- `ParallelWidthChecker` を簡単に使うための関数。

---

## まとめ
- 並列化の単位は「候補ペア」または「ポリゴン」。
- 共有データへの書き込みは行わず、
  スレッドごとの結果配列にためることで競合を避けている。
- 最後にまとめて結合するので安全。
