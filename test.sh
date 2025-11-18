#!/bin/bash
assert() {
	expected="$1"
	input="$2"

	./9cc "$input" > tmp.s
	cc -o tmp tmp.s
	./tmp
	actual="$?"

	if [ "$actual" = "$expected" ]; then
		echo "$input => $actual"
	else
		echo "$input => $expected expected, but got $actual"
		exit 1
	fi
}

assert 3 "int main() { return 9 - 6; }"
assert 0 "int main() { return 0; }"
assert 42 "int main() { return 42; }"
assert 21 "int main() { return 5+20-4; }"
assert 41 "int main() { return 12 + 34 - 5; }"
assert 47 "int main() { return 5+6*7; }"
assert 15 "int main() { return 5*(9-6); }"
assert 4 "int main() { return (3+5)/2; }"
assert 10 "int main() { return -10+20; }"
assert 1 "int main() { return 1 < 2; }"
assert 0 "int main() { return 1 > 2; }"

# 変数宣言と代入
assert 3 "int main() { int a; a=3; return a; }"
assert 3 "int main() { int a=3; return a; }"
assert 8 "int main() { int a=3; int z=5; return a+z; }"

# ポインタ
assert 3 "int main() { int x=3; int *y=&x; return *y; }"
assert 3 "int main() { int x=3; int *y=&x; int **z=&y; return **z; }"
assert 5 "int main() { int x=3; int *y=&x; *y=5; return x; }"

# 関数呼び出し
assert 3 "int foo() { return 3; } int main() { return foo(); }"
assert 42 "int foo() { return 42; } int main() { return foo(1, 2, 3); }"
assert 10 "int foo(int a) { return a; } int main() { return foo(10); }"
assert 7 "int add(int a, int b) { return a+b; } int main() { return add(3, 4); }"
assert 55 "int fib(int n) { if (n<=1) return n; return fib(n-1) + fib(n-2); } int main() { return fib(10); }"

# ステップ19: ポインタ演算
# ポインタに 1 を足すと、8バイト（intのサイズ分）進むか？
# x(rbp-8), y(rbp-16) なので、&y + 1 は &x になるはず
assert 3 "int main() { int x; int y; x=3; y=5; return *(&y + 1); }"

# ポインタから 1 を引くと、8バイト戻るか？
# &x - 1 は &y になるはず
assert 5 "int main() { int x; int y; x=3; y=5; return *(&x - 1); }"

# 変数を使ったポインタ演算
# int *p; p = &y; p = p + 1; *p は x になるはず
assert 3 "int main() { int x; int y; x=3; y=5; int *p; p=&y; p = p + 1; return *p; }"

# 整数 + ポインタ (逆順) も動くか？
# 1 + &y も &x になるはず
assert 3 "int main() { int x; int y; x=3; y=5; return *(1 + &y); }"

# 配列っぽい使い方のテスト
# malloc はまだ無いけど、スタック上の変数を並べて擬似的に配列として扱う
assert 3 "int main() { int x; int y; x=3; y=5; return *(&y + 1); }"
assert 5 "int main() { int x; int y; x=3; y=5; return *(&x - 1); }"

assert 3 "int main() { int a[2]; *a = 1; *(a + 1) = 2; int *p; p = a; return *p + *(p + 1); }"
assert 3 "int main() { int a[2]; a[0] = 1; a[1] = 2; return a[0] + a[1]; }"

# 2. ポインタ演算との組み合わせ
# a[2] は *(a + 2) と同じか？
assert 5 "int main() { int a[2]; a[1] = 5; return *(a + 1); }"
assert 5 "int main() { int a[2]; *(a + 1) = 5; return a[1]; }"
# "3[a]" みたいな変態的な書き方もC言語では合法（*(3+a)と同じだから）。君のコンパイラでも動くはずだ。
assert 5 "int main() { int a[2]; *a = 1; *(a + 1) = 5; return 1[a]; }" 

# 3. ループでの書き込み
# 0から9まで書き込んで、正しく読めるか？（オフセット計算がズレてないかチェック）
assert 9 "int main() { int a[10]; int i; for (i=0; i<10; i=i+1) a[i]=i; return a[9]; }"
assert 0 "int main() { int a[10]; int i; for (i=0; i<10; i=i+1) a[i]=i; return a[0]; }"
assert 45 "int main() { int a[10]; int i; int sum; sum=0; for (i=0; i<10; i=i+1) a[i]=i; for (i=0; i<10; i=i+1) sum=sum+a[i]; return sum; }"

# 4. 配列のサイズ計算
# int a[10] は 8バイト * 10 = 80バイト確保されているか？
# a[0] と a[9] のアドレスの差は 72 (9 * 8) になるはず。
# (注意: 戻り値は int なのでアドレスそのものは返せないが、差分で検証)
# ポインタ同士の引き算は「要素数」を返すから、&a[9] - &a[0] は 9 になるはず。
assert 9 "int main() { int a[10]; return &a[9] - &a[0]; }"
assert 8 "int main() { int x; return sizeof(x); }"
assert 8 "int main() { int x; return sizeof(x + 3); }"
assert 8 "int main() { int *y; return sizeof(y); }"
assert 8 "int main() { int x; return sizeof(x); }"
assert 8 "int main() { return sizeof(1); }"
assert 32 "int main() { int a[4]; return sizeof(a); }" # 8バイト * 4要素 = 32

echo OK