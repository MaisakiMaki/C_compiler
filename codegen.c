#include "9cc.h"

int label_count = 0;

void gen_lval(Node *node) {
    if (node -> kind == ND_LVAR) {
        printf("    mov rax, rbp\n");
        printf("    sub rax, %d\n", node -> offset);
        printf("    push rax\n");
        return;
    }
    
    if (node -> kind == ND_DEREF) {
        gen(node -> lhs);
        return;
    }
    error("代入の左辺値が変数ではありません");
}

void gen(Node *node) {

    switch (node -> kind) {
    case ND_NUM:
        printf("	push %d\n", node -> val);
		return;
    case ND_LVAR:
        gen_lval(node);
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
        return;
    case ND_ASSIGN:
        gen_lval(node -> lhs);
        gen(node -> rhs);

        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    mov [rax], rdi\n");
        printf("    push rdi\n");
        return;
    case ND_RETURN:
        // 戻り値（左辺）の計算
        gen(node -> lhs);

        // スタックの一番上にある戻り値をraxに持ってくる
        printf("    pop rax\n");

        // mainに後片付けをさせずに、今すぐ脱出する
        printf("    mov rsp, rbp\n");
        printf("    pop rbp\n");
        printf("    ret\n");
        return;
    case ND_IF: {
        int c = label_count++; //ユニークなラベル番号を取得

        //条件式(lhs)の計算
        gen(node -> lhs);
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");

        //偽だったらelse_labelにジャンプ
        printf("    je .L.else.%d\n", c);

        gen(node -> rhs);
        printf("    pop rax\n");
        printf("    jmp .L.end.%d\n", c);

        printf(".L.else.%d:\n", c);
        if (node -> els) {
            gen(node -> els);
            printf("    pop rax\n");
        }
        
        printf(".L.end.%d:\n", c);
        return;
    }
    case ND_WHILE: {
        int c = label_count++;

        // ループ開始ラベル
        printf(".L.begin.%d:\n", c);

        // 条件判定(lhs)の計算
        gen(node -> lhs);
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");

        //偽だったらendにジャンプ
        printf("    je .L.end.%d\n", c);

        // 真だったらbodyブロック(rhs)を実行
        gen(node -> rhs);
        printf("    pop rax\n");

        // 無条件にループの開始に戻る
        printf("    jmp .L.begin.%d\n", c);

        printf(".L.end.%d:\n", c);
        return;
    }
    case ND_BLOCK: {
        // node -> lhs(body)から始まる文のリストを
        // 1個ずつnextでたどりながら、全部genする
        for (Node *n = node -> lhs; n; n = n -> next) {
            gen(n);
            // 各文の計算結果をスタックから捨てる
            // (お掃除)
            printf("    pop rax\n");
        }

        printf("    push 0\n");
        return;
    }
    case ND_CALL: {

        // 引数を渡すレジスタのリスト
        char *arg_reg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

        // 引数をスタックに積む
        int arg_count = 0;
        for (Node *arg = node -> args; arg; arg = arg -> next) {
            gen(arg);
            arg_count++;
        }

        for (int i = arg_count - 1; i >= 0; i--) {
            printf("    pop %s\n", arg_reg[i]);
        }
        //関数名を%.*sで出力する。
        //node -> name_lenと node -> nameをprintfにわたす
        printf("    call %.*s\n", node -> name_len, node -> name);

        //C言語のルールで返り値はraxに入る
        //このコンパイラは式の値をスタックに積むというルールなので
        // raxの値をスタックにpushしておく
        printf("    push rax\n");
        return;
    }

    case ND_ADDR:
        // アドレスを取りたい変数を gen_lval に渡すと
        // その「アドレス」をスタックに積んでくれる
        gen_lval(node -> lhs);
        return;

    case ND_DEREF:
        // p(アドレス)を計算してスタックに積む
        gen(node -> lhs);

        // スタックからアドレスを取り出す
        printf("    pop rax\n");

        // そのアドレスにある値を読みだして、raxに入れる
        printf("    mov rax, [rax]\n");

        // 値をスタックに積む
        printf("    push rax\n");
        return;
    case '+':
    case '-':
    case '*':
    case '/':
    case ND_EQ:
    case ND_NE:
    case ND_LT:
    case ND_LE:
        break;
	default:
		break;
    }

	gen(node -> lhs);
	gen(node -> rhs);

	printf("	pop rdi\n");
	printf("	pop rax\n");

	switch (node -> kind) {
    case '+':
		printf("	add rax, rdi\n");
		break;
	case '-':
		printf("	sub rax, rdi\n");
		break;
	case '*':
		printf("	imul rax, rdi\n");
		break;
	case '/':
		printf("		cqo\n");
		printf("	idiv rdi\n");
		break;
	case ND_EQ:
		printf("	cmp rax, rdi\n");
		printf("	sete al\n");
		printf("	movzb rax, al\n");
		break;
	case ND_NE:
		printf("	cmp rax, rdi\n");
		printf("	setne al\n");
		printf("	movzb rax, al\n");
        break;
	case ND_LT:
		printf("    cmp rax, rdi\n");
        printf("    setl al\n");
        printf("    movzb rax, al\n");
        break;
	case ND_LE:
		printf("    cmp rax, rdi\n");
        printf("    setle al\n");
        printf("    movzb rax, al\n");
        break;
	default:
		break;
    }

	printf("	push rax\n");
}