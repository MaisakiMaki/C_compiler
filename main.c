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

	//　全関数のパース
	Function *func_list = program();

	// アセンブリの前半部分を出力

	printf(".intel_syntax noprefix\n");
	printf(".globl main\n");
	for (Function *func = func_list; func; func = func -> next) {
		printf("%.*s:\n", func -> name_len, func -> name);

		//プロローグ
		printf("	push rbp\n");
		printf("	mov rbp, rsp\n");

		//必要なスタックサイズをfuncから持ってくる
		printf("	sub rsp, %d\n", func -> stack_size);

		//genが正しいローカル変数リストをつかえるように
		//グローバル変数”local"を、今からgenする関数のものに入れ替える
		locals = func -> locals;

		for (Node *node = func -> node; node; node = node -> next) {
			gen(node);
			if (node -> kind != ND_RETURN) {
				printf("	pop rax\n");
			}
		}
		printf("	mov rsp, rbp\n");
		printf("	pop rbp\n");
		printf("	ret\n");
	}
}