#include "9cc.h"

Token *token;
char *user_input;

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "引数の個数が正しくありません\n");
		return 1;
	}

	// トークナイズしてパースする
	user_input = argv[1];
	token = tokenize(argv[1]);

	// アセンブリの前半部分を出力
	printf(".intel_syntax noprefix\n");
	printf(".globl main\n");
	printf("main:\n");

    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, 208\n");

    while (!at_eof()) {
        Node *node = stmt();
        gen(node);
		if (node -> kind != ND_RETURN) {
			printf("	pop rax\n");
		}
    }

	// 式の最初は数でなければならないので、それをチェックして
	// 最初のmov命令を出力
    printf("    mov rsp, rbp\n");
	printf("    pop rbp\n");
	printf("	ret\n");
	return 0;
}