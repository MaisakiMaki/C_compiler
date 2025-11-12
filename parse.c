#include "9cc.h"

typedef struct LVar LVar;

struct LVar{
	LVar *next;
	char *name;
	int len;
	int offset;
};

LVar *locals;

void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}
// エラー箇所を報告をするための関数
void error_at(char *loc, char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);

	int pos = loc - user_input;
	fprintf(stderr, "%s\n", user_input);
	fprintf(stderr, "%*s\n", pos, " "); // pos分の空白を出力
	fprintf(stderr, "^");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

// 次のトークンが期待している記号の時には、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char *op) {
	if (token -> kind != TK_RESERVED || 
		strlen(op) != token -> len || 
		memcmp(token -> str, op, token -> len))
		return false;
	token = token -> next;
	return true;
}

// 次のトークンが期待している記号の時には、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char *op) {
	if (token -> kind != TK_RESERVED || 
		strlen(op) != token -> len || 
		memcmp(token -> str, op, token -> len))
		error_at(token -> str, "'%s'ではありません", op);
	token = token -> next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number() {
	if (token -> kind != TK_NUM) 
		error_at(token -> str, "数ではありません");
	int val = token -> val;
	token = token -> next;
	return val;
}

bool at_eof() {
	return token -> kind  == TK_EOF;
}
// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
	Token *tok = calloc(1, sizeof(Token));
	tok -> kind = kind;
	tok -> str = str;
	tok -> len = len;
	cur -> next = tok;
	return tok;
}

Token *tokenize(char *p) {
	Token head;
	head.next = NULL;
	Token *cur = &head;

	while (*p) {
		// 空白文字をスキップ
		if (isspace(*p)) {
			p++;
			continue;
		}

		if (strncmp(p, "==", 2) == 0) {
			cur = new_token(TK_RESERVED, cur, p, 2);
			p += 2;
			continue;
		}

		if (strncmp(p, "!=", 2) == 0) {
			cur = new_token(TK_RESERVED, cur, p, 2);
			p += 2;
			continue;
		}

		if (strncmp(p, "<=", 2) == 0) {
			cur = new_token(TK_RESERVED, cur, p, 2);
			p += 2;
			continue;
		}

		if (strncmp(p, ">=", 2) == 0) {
			cur = new_token(TK_RESERVED, cur, p, 2);
			p += 2;
			continue;
		}

		if (*p == '<') {
			cur = new_token(TK_RESERVED, cur, p, 1);
			p++;
			continue;
		}

		if (*p == '>') {
			cur = new_token(TK_RESERVED, cur, p, 1);
			p++;
			continue;
		}

		if (*p == '+') {
			cur = new_token(TK_RESERVED, cur, p, 1);
			p++;
			continue;
		}

		if (*p == '-') {
			cur = new_token(TK_RESERVED, cur, p, 1);
			p++;
			continue;
		}

		if (*p == '*') {
			cur = new_token(TK_RESERVED, cur, p, 1);
			p++;
			continue;
		}

		if (*p == '/') {
			cur = new_token(TK_RESERVED, cur, p, 1);
			p++;
			continue;
		}

		if (*p == '(') {
			cur = new_token(TK_RESERVED, cur, p, 1);
			p++;
			continue;
		}

		if (*p == ')') {
			cur = new_token(TK_RESERVED, cur, p, 1);
			p++;
			continue;
		}

		if (*p == '{') {
			cur = new_token(TK_RESERVED, cur, p++, 1);
			continue;
		}

		if (*p == '}') {
			cur = new_token(TK_RESERVED, cur, p++, 1);
			continue;
		}

        if (*p == ';') {
            cur = new_token(TK_RESERVED, cur, p, 1);
            p++;
            continue;
        }

        if (*p == '=') {
            cur = new_token(TK_RESERVED, cur, p, 1);
            p++;
            continue;
        }

        if (isalpha(*p)) {
			char *start = p;
			while (isalnum(*p) || *p == '_') {
				p++;
			}

			int len = p - start;

			if (len == 6 && memcmp(start, "return", 6) == 0) {
				cur = new_token(TK_RETURN, cur, start, 6);
			} else if (len == 2 && memcmp(start, "if", 2) == 0) {
				cur = new_token(TK_IF, cur, start, 2);
			} else if (len == 4 && memcmp(start, "else", 4) == 0) {
				cur =  new_token(TK_ELSE, cur, start, 4);
			} else if (len == 5 && memcmp(start, "while", 5) == 0) {
				cur = new_token(TK_WHILE, cur, start, 5);
			} else if (len == 3 && memcmp(start, "for", 3) == 0) {
				cur = new_token(TK_FOR, cur, start, 3);
			} else {
				cur = new_token(TK_IDENT, cur, start, len);
			}
            continue;
        }

		if (isdigit(*p)) {
			char *start = p;
			int val = strtol(p, &p, 10);
			int len = p - start;
			cur = new_token(TK_NUM, cur, start, len);
			cur -> val = val;
			continue;
		}

		error_at(p, "トークナイズできません");
	}

	new_token(TK_EOF, cur, p, 0);
	return head.next;
}

// 抽象構文木のノードの種類


Node *new_node(int kind, Node *lhs, Node *rhs) {
	Node *node = calloc(1, sizeof(Node));
	node -> kind = kind;
	node -> lhs = lhs;
	node -> rhs = rhs;
	return node;
}

Node *new_node_num(int val) {
	Node *node = calloc(1, sizeof(Node));
	node -> kind = ND_NUM;
	node -> val = val;
	return node;
}

Node *new_node_if(Node *cond, Node  *then, Node *els) {
	Node *node = calloc(1, sizeof(Node));
	node -> kind = ND_IF;
	node -> lhs = cond;
	node -> rhs = then;
	node -> els = els;
	return node;
}

LVar *find_lvar(Token *tok) {
	for (LVar *var = locals; var; var =  var -> next) {
		if (var -> len == tok -> len && memcmp(tok -> str, var -> name, tok -> len) == 0) {
			return var;
		}
	}
	return NULL;
}

Node *new_node_lvar(int offset) {
    Node *node = calloc(1, sizeof(Node));
    node -> kind = ND_LVAR;
    node -> offset = offset;
    return node;
}

Node *stmt() {
	if (token -> kind == TK_RETURN) {
		token = token -> next; // TK_RETURNを消費

		Node *node = expr();
		expect(";");

		return new_node(ND_RETURN, node, NULL);
	} else if (token -> kind == TK_IF) {
		token = token -> next;

		expect("(");
		Node *cond = expr();
		expect(")");

		Node *then = stmt();

		Node *els = NULL;
		if (token -> kind == TK_ELSE) {
			token = token -> next;
			els = stmt();
		}
		return new_node_if(cond, then, els);
	} else if (token -> kind == TK_WHILE) {
		token = token -> next;

		expect("(");
		Node *cond = expr();
		expect(")");

		Node *body = stmt();

		return new_node(ND_WHILE, cond, body);
	} else if (token -> kind == TK_RESERVED && token -> str[0] == '{') {
		token = token -> next; //"{"を消費

		Node head = {}; // リストのダミーの先頭
		Node *cur = &head;

		while (token -> kind != TK_RESERVED || token -> str[0] != '}') {
			cur -> next = stmt(); // stmt()を再帰呼び出し
			cur = cur -> next; // リストの次に進む
		}
		token = token -> next; // "}"を消費

		// ND_BLOCKノードにくっつけて、head.next(リストの本当の先頭)を
		// lhs(body)にくっつけて返す
		return new_node(ND_BLOCK, head.next, NULL);
	} else if (token -> kind == TK_FOR) {
		token = token -> next; // TK_FORを消費
		expect("(");

		// --- for (A; B; C) の A, B, C をパース ---
		// (A) 初期化式 無くてもいい
		Node *init = NULL;
		if (token -> kind != TK_RESERVED || token -> str[0] != ';') {
			init = expr();
		}
		expect(";");

		// (B) 条件式 無くてもいい(ない場合は真(1)扱い)
		Node *cond = NULL;
		if (token -> kind != TK_RESERVED || token -> str[0] != ';') {
			cond = expr();
		}
		if (cond == NULL) {
			cond = new_node_num(1);
		}
		expect(";");

		// (C)インクリメント式 無くてもいい
		Node *inc = NULL;
		if (token -> kind != TK_RESERVED || token -> str[0] != ')') {
			inc = expr();
		}
		expect(")");

		Node *body = stmt();

		// --- for を whileに組み換え ---
		//{D; C;}のブロックを作る
		Node *block_body = new_node(ND_BLOCK, body, NULL);
		if (inc) {
			body -> next = inc;
		}

		// while (B) {D; C; }のwhileの木を作る
		Node *while_node = new_node(ND_WHILE, cond, block_body);

		//{A; while(B) {...}}の外側のブロックをつくる
		if (init == NULL) {
			return while_node;
		} else {
			init -> next = while_node;
			return new_node(ND_BLOCK, init, NULL);
		}

	}
    Node *node = expr();
    expect(";");
    return node;
}

Node *expr() {
	Node *node = assign();
	return node;
}

Node *assign() {
    Node *node = equality();
    if (consume("="))
        node = new_node(ND_ASSIGN, node, assign());
    return node;
}

Node *equality() {
	Node *node = relational();
	for (;;) {
		if (consume("=="))
			node = new_node(ND_EQ, node, relational());
		else if (consume("!="))
			node = new_node(ND_NE, node, relational());
		else
			return node;
	}
}

Node *relational() {
	Node *node = add();
	for(;;) {
		if (consume("<"))
			node = new_node(ND_LT, node, add());
		else if (consume("<="))
			node = new_node(ND_LE, node, add());
		else if (consume(">"))
			node = new_node(ND_LT, add(), node);
		else if (consume(">="))
			node = new_node(ND_LE, add(), node);
		else
			return node;
	}
}

Node *add() {
	Node *node = mul();
	for (;;) {
		if (consume("+")) 
			node = new_node('+', node, mul());
		else if (consume("-"))
			node = new_node('-', node, mul());
		else
			return node;
	}
}

Node *mul() {
	Node *node = unary();

	for (;;) {
		if (consume("*")) 
			node = new_node('*', node, unary());
		else if (consume("/"))
			node = new_node('/', node, unary());
		else
			return node;
	}
}

Node *unary() {
	if (consume("+"))
		return primary();
	if (consume("-")) 
		return new_node('-', new_node_num(0), primary());
	return primary();
}

Node *primary() {
	if (consume("(")) {
		Node *node = expr();
		expect(")");
		return node;
	}

    if (token -> kind == TK_IDENT) {
		Token *tok = token; // トークンを覚えておく
		token = token -> next; // トークンを1個消費

		// 名簿を検索
		LVar *lvar = find_lvar(tok);

		if (lvar) {
			// いた場合
			// 名簿に書いてある古い住所でノードの制作
			return new_node_lvar(lvar -> offset);
		} else {
			// いなかった場合
			// 新しい名簿の箱を作る
			lvar = calloc(1, sizeof(LVar));
			lvar -> next = locals;
			lvar -> name = tok -> str;
			lvar -> len = tok -> len;

			// 新しい住所を作る
			// 名簿が空なら0
			// 先輩がいたらその人の住所+8
			lvar -> offset = (locals ? locals -> offset : 0) + 8;

			locals = lvar; // 名簿の先頭を新人に更新
			return new_node_lvar(lvar -> offset);
		}
    }

	return new_node_num(expect_number());
}

