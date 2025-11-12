#include "9cc.h"

int label_count = 0;

void gen_lval(Node *node) {
    if (node -> kind != ND_LVAR)
        error("代入の左辺値が変数ではありません");
    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", node -> offset);
    printf("    push rax\n");

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