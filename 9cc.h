#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの種類
typedef enum {
	TK_RESERVED, // 記号
	TK_NUM, // 整数トークン
    TK_IDENT,
	TK_EOF, // 入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

struct Token {
	TokenKind kind; // トークンの型
	Token *next; // 次の入力トークン
	int val; // kindがTK_NUMの場合、その数値
	char *str; // トークン文字列
	int len; // トークンの長さ
};

// 現在着目しているトークン
extern Token *token;

// 入力データ
extern char *user_input;

typedef enum {
	ND_EQ, // ==
	ND_NE, // !=
	ND_LT, // <
	ND_LE, // <=
    ND_ASSIGN,
    ND_LVAR, // ローカル変数
	ND_NUM, // 整数
} NodeKind;

typedef struct Node Node;

struct Node {
	int kind; // ノードの型
	Node *lhs; // 左辺
	Node *rhs; // 右辺
    int offset; // ND_LVARの場合のみ使う
	int val; //kindがND_NUMの場合のみ使う
};

// 関数ライブラリ
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *primary();
Node *unary();
Node *assign();
Node *stmt();

Token *tokenize(char *p);
void error(char* fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool consume(char *op);
void expect(char *op);
int expect_number(void); // (void) は「引数なし」って意味
bool at_eof(void);
Node *new_node(int kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *new_node_lvar(int offset);

void gen(Node *node);
void gen_lval(Node * node);