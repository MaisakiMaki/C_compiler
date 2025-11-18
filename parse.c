#include "9cc.h"

Type *new_type_int() {
	Type *ty = calloc(1, sizeof(Type));
	ty -> kind = TY_INT;
	return ty;
}

Type *new_type_ptr(Type *base) {
	Type *ty = calloc(1, sizeof(Type));
	ty -> kind = TY_PTR;
	return ty;
}

void add_type(Node *node) {
	if (!node || node -> ty) return;

	add_type(node -> lhs);
	add_type(node -> rhs);
	add_type(node -> els);
	add_type(node -> next);
	add_type(node -> args);

	switch (node -> kind) {
		case ND_ADD:
		case ND_SUB:
		case ND_MUL:
		case ND_DIV:
		case ND_ASSIGN:
			node -> ty = node -> lhs -> ty;
			return;
		
		case ND_EQ:
		case ND_NE:
		case ND_LT:
		case ND_LE:
		case ND_NUM:
		case ND_CALL:
			node -> ty = new_type_int();
			return;
		
		case ND_LVAR:
			return;
			
		case ND_ADDR:
			node -> ty = new_type_ptr(node -> lhs -> ty);
			return;

		case ND_DEREF:
			if (node -> lhs -> ty -> ptr_to)
				node -> ty = node -> lhs -> ty -> ptr_to;
			else
				node -> ty = new_type_int();
			return;
	}
}

Node *new_node_add(Node *lhs, Node *rhs) {
	add_type(lhs);
	add_type(rhs);

	if (lhs -> ty -> kind == TY_INT && rhs -> ty -> kind == TY_PTR) {
		Node *tmp = lhs;
		lhs = rhs;
		rhs = tmp;
	}

	if (lhs -> ty -> kind == TY_PTR && rhs -> ty -> kind == TY_INT) {
		rhs = new_node(ND_MUL, rhs, new_node_num(8));

		add_type(rhs);
	}

	Node *node = new_node(ND_ADD, lhs, rhs);
	node -> ty = lhs -> ty;
	return node;
}

Node *new_node_sub(Node *lhs, Node *rhs) {
	add_type(lhs);
	add_type(rhs);

	if (lhs -> ty -> kind == TY_PTR && rhs -> ty -> kind == TY_INT) {
		rhs = new_node(ND_MUL, rhs, new_node_num(8));
		add_type(rhs);

		Node *node = new_node(ND_SUB, lhs, rhs);
		node -> ty = lhs -> ty;
		return node;
	}

	Node *node = new_node(ND_SUB, lhs, rhs);
	node -> ty = lhs -> ty;
	return node;
}

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

Token *expect_ident() {
	if (token -> kind != TK_IDENT) 
		error_at(token -> str, "識別子ではありません");
	Token *tok = token;
	token = token -> next;
	return tok;
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

		if (*p == '&') {
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

		if (*p == ',') {
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
			} else if (len == 3 && memcmp(start, "int", 3) == 0) {
				cur = new_token(TK_RESERVED, cur, start, 3);
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

Type *parse_base_type() {
	if (consume("int"))
		return new_type_int();
	return NULL;
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
	node -> ty = new_type_int();
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

Node *new_node_call(Token *tok) {
	Node *node = calloc(1, sizeof(Node));
	node -> kind = ND_CALL;
	node -> name = tok -> str;
	node -> name_len = tok -> len;
	return node;
}

Function *function() {
	// 新しい関数に入ったので、ローカル変数をリセットする
	locals = NULL;
	Type *ty = parse_base_type();
	if (!ty) error_at(token -> str, "型ではありません");
	while(consume("*")) {
		ty = new_type_ptr(ty);
	}

	Token *tok = expect_ident(); //関数名を期待

	// 関数ノードを作成
	Function *func = calloc(1, sizeof(Function));
	func -> name = tok -> str;
	func -> name_len = tok -> len;

	expect("(");
	int params = 0;
	if (consume(")")) {
		expect("{");
	} else {
		params++;
		ty = parse_base_type();
		if (!ty) error_at(token -> str, "型ではありません");
		while(consume("*")) {
			ty = new_type_ptr(ty);
		}
		Token *tok = expect_ident();

		LVar *lvar = calloc(1, sizeof(LVar));
		lvar -> next = locals;
		lvar -> name = tok -> str;
		lvar -> len = tok -> len;
		lvar -> offset = (locals ? locals -> offset : 0) + 8;
		lvar -> ty = ty;
		locals = lvar;

		while (consume(",")) {
			params++;
			ty = parse_base_type();
			if (!ty) error_at(token -> str, "型ではありません");
			while(consume("*")) {
				ty = new_type_ptr(ty);
			}

			tok = expect_ident();

			lvar = calloc(1, sizeof(LVar));
			lvar -> next = locals;
			lvar -> name = tok -> str;
			lvar -> len = tok -> len;
			lvar -> offset = (locals ? locals -> offset : 0) + 8;
			lvar -> ty = ty;
			locals = lvar;
		}
		expect(")");
		expect("{");
	}
	
	

	Node head = {};
	Node *cur = &head;
	while(!consume("}")) {
		cur -> next = stmt();
		cur = cur -> next;
	}
	
	func -> node = head.next; // 文のリストを関数ノードにつなぐ
	func -> locals = locals;

	func -> params_len = params;
	
	int offset = 0;
    if (locals) {
        offset = locals->offset; //
    }
    func->stack_size = (offset + 15) / 16 * 16; // Function構造体に保存

	return func;
}

//program = function*

Function *program(void) {
	Function head = {};
	Function *cur = &head;

	while (!at_eof()) {
		cur -> next = function();
		cur = cur -> next;
	}
	return head.next;
}

Node *stmt() {
	Type *ty = parse_base_type();
	if (ty) {

		while(consume("*")) {
			ty = new_type_ptr(ty);
		}

		Token *tok = expect_ident();

		LVar *lvar = calloc(1, sizeof(LVar));
		lvar -> next = locals;
		lvar -> name = tok -> str;
		lvar -> len = tok -> len;
		lvar -> offset = (locals ? locals -> offset : 0) + 8;
		lvar -> ty = ty;
		locals = lvar;

		if (consume("=")) {
			Node *lhs = new_node_lvar(lvar -> offset);
			Node *rhs = expr();
			expect(";");
			return new_node(ND_ASSIGN, lhs, rhs);
		}
		expect(";");

		return new_node(ND_BLOCK, NULL, NULL);
	}
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
			node = new_node_add(node, mul());
		else if (consume("-"))
			node = new_node_sub(node, mul());
		else
			return node;
	}
}

Node *mul() {
	Node *node = unary();

	for (;;) {
		if (consume("*")) 
			node = new_node(ND_MUL, node, unary());
		else if (consume("/"))
			node = new_node(ND_DIV, node, unary());
		else
			return node;
	}
}

Node *unary() {
	if (consume("+"))
		return primary();
	if (consume("-")) 
		return new_node(ND_SUB, new_node_num(0), primary());
	if (consume("&"))
		return new_node(ND_ADDR, unary(), NULL);
	if (consume("*"))
		return new_node(ND_DEREF, unary(), NULL);

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

		if (consume("(")) {
			Node *node = new_node_call(tok);
			//空っぽの引数リスト
			if (consume(")")) {
				return node;
			}

			// 引数リストあり
			Node head = {};
			Node *cur = &head;

			cur -> next = expr();
			cur = cur -> next;
			while(consume(",")) {
				cur -> next = expr();
				cur = cur -> next;
			}
			expect(")");
			node -> args = head.next;
			return node;
		}

		// 名簿を検索
		LVar *lvar = find_lvar(tok);

		if (lvar) {
			// いた場合
			// 名簿に書いてある古い住所でノードの制作
			Node *node = new_node_lvar(lvar -> offset);
			node -> ty = lvar -> ty;
			return node;
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
			Node *node = new_node_lvar(lvar -> offset);
			node -> ty = lvar -> ty;
			return node;
		}
    }

	return new_node_num(expect_number());
}

