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
	Node *code[100];
	int i = 0;

	while (!at_eof()) {
		code[i++] = stmt();
	}

	printf(".intel_syntax noprefix\n");
	printf(".globl main\n");
	printf("main:\n");

	//プロローグ
	printf("	push rbp\n");
	printf("	mov rbp, rsp\n");

	//必要なオフセットを計算してスタック確保
	//locals はリストの先頭（一番最後に登録された変数）を指す。
	//そのoffsetが必要なサイズになる
	int offset = locals ? locals -> offset : 0;
	printf("	sub rsp, %d\n", offset);

	//貯めておいた木を順番にgen
	for (int j = 0; j < i; j++) {
		gen(code[j]);

		//式の評価結果をスタックから捨てる
		//(ND_RETURNの場合はgenがretするので不要)
		if (code[j] -> kind != ND_RETURN) {
			printf("	pop rax\n");
		}
	}

	printf("	mov rsp, rbp\n");
	printf("	pop rbp\n");
	printf("	ret\n");
	return 0;
}