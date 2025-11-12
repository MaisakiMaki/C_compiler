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

assert 0 "0;"
assert 42 "42;"
assert 21 "5+20-4;"
assert 41 " 12 + 34 - 5 ;"
assert 4 "(3+5)/2;"
assert 47 "5+6*7;"
assert 15 "5*(9-6);"
assert 4 "(3+5)/2;"
assert 10 "-10+20;"
assert 1 "1 < 2 ;"
assert 1 "5 == 2 + 3 ;"

# ↓↓↓↓ 俺が前回「挑戦状」で出したヤツも、こうやって追加するんだぞ
assert 3 "a = 3; a;"
assert 8 "a = 3; b = 5; a + b;"
assert 6 "a = b = 3; a + b;"
assert 13 "a = 3; b = 5; a + b * 2;"
assert 2 "a=1; b=2; c=a+b; d=c*a+b; e=d-c; e;"

assert 5 "foo = 5; foo;"
assert 8 "foo = 3; bar = 5; foo + bar;"

# 2. ちゃんと「名簿」が機能してるか
assert 3 "foo = 1; bar = 2; foo + bar;"
assert 7 "foo = 3; bar = 4; foo + bar;"

# 3.「代入」の合わせ技
assert 10 "foo = 3; bar = 7; foo = foo + bar; foo;"
assert 10 "foo = 10; bar = foo; bar;"

# 4. 「名簿」検索の「衝突」テスト（一番意地悪なヤツ）
# "foo" (len=3) と "foobar" (len=6) は「別人」として扱えるか？
assert 13 "foo = 3; foobar = 10; foo + foobar;"


echo OK

