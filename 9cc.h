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
	TK_RETURN, // returnトークン
	TK_IF, // ifトークン
	TK_ELSE, // elseトークン
	TK_WHILE, // whileトークン
	TK_FOR, // forトークン
    TK_IDENT, //変数トークン
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

typedef struct Type Type;

typedef enum {
	TY_INT,
	TY_PTR,
} TypeKind;

struct Type {
	TypeKind kind;
	Type *ptr_to;
};

typedef enum {
	ND_EQ, // ==
	ND_NE, // !=
	ND_LT, // <
	ND_LE, // <=
    ND_ASSIGN,
	ND_RETURN, // return
	ND_IF, // if
	ND_ELSE, // else
	ND_WHILE, // while
	ND_FOR, // for
	ND_BLOCK, //block
    ND_LVAR, // ローカル変数
	ND_NUM, // 整数
	ND_CALL,// 関数呼び出し
	ND_ADDR,
	ND_DEREF,
} NodeKind;

typedef struct Node Node;

struct Node {
	int kind; // ノードの型
	Node *lhs; // 左辺
	Node *rhs; // 右辺
	Node *els;
	Node *next;
    int offset; // ND_LVARの場合のみ使う
	int val; //kindがND_NUMの場合のみ使う
	char *name;
	int name_len;
	Node *args;
	Type *ty;
};

typedef struct LVar LVar;
struct LVar{
	LVar *next;
	char *name;
	int len;
	int offset;
	Type *ty;
};
extern LVar *locals;

typedef struct Function Function;

struct Function {
	Function *next; //次の関数
	char *name; // 関数名
	int name_len; // 関数名の長さ
	Node *node; // 関数本体の先頭
	LVar *locals; // この関数のローカル変数リスト
	int params_len;
	int stack_size; // この関数が必要とするスタックサイズ
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
Node *new_node_if(Node *cond, Node *then, Node *els);

void gen(Node *node);
void gen_lval(Node * node);

Function *program(void);