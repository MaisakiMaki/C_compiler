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

assert 5 "main() { return 5; }"
assert 21 "main() { return 5+20-4; }"
assert 1 "main() { return 1 < 2 ; }"
assert 6 "main() { a=3; b=3; return a + b; }"
assert 10 "main() { foo=3; bar=7; foo = foo + bar; return foo; }"

# ステップ16 (呼び出し) テスト
assert 3 "foo() { return 3; } main() { return foo(); }"
assert 42 "foo() { return 42; } main() { return foo(1, 2, 3); }"
assert 10 "foo() { a=10; return a; } main() { a=5; return foo(); }"

assert 3 "main() { x=3; y=&x; return *y; }"
assert 3 "main() { x=3; y=&x; z=&y; return **z; }"
assert 5 "main() { x=3; y=&x; *y=5; return x; }"

echo OK