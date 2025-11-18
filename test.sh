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

echo OK