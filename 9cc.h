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
	ND_ADD, // +
	ND_SUB, // -
	ND_MUL, // * 
	ND_DIV, // /
	ND_EQ, // ==
	ND_NE, // !=
	ND_LT, // <
	ND_LE, // <=
	ND_NUM, // 整数
} NodeKind;

typedef struct Node Node;

struct Node {
	NodeKind kind; // ノードの型
	Node *lhs; // 左辺
	Node *rhs; // 右辺
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

Token *tokenize(char *p);
void error_at(char *loc, char *fmt, ...);
bool consume(char *op);
void expect(char *op);
int expect_number(void); // (void) は「引数なし」って意味
bool at_eof(void);
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);

void gen(Node *node);