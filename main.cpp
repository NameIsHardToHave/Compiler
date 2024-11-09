#define _CRT_SECURE_NO_WARNINGS
#include "lexical.h"
#include "syntax.h"
#include "imitate.h"

int main(){
	// 输入文件流
	std::ifstream input_file("test/test3_4.txt");
	if (!input_file) {
		std::cerr << "打开输入文件失败！" << "\n";
		return -1;
	}

	// 输出文件流
	std::ofstream output_file("result/result1_1.txt");
	if (!output_file) {
		std::cerr << "打开输出文件失败！" << "\n";
		return -1;
	}

	// 指定输出的位置
	std::ostream &output = std::cout;		// 词法分析、语法树、show结果的输出
	std::ostream &output2 = output_file;	// 中间代码生成的输出

	// 词法分析
	Lexical lexical(input_file);
	auto TokenList = *lexical.TokenReader();
	lexical.TokenShow(output);
		
	// 语法分析
	Syntax syntax(TokenList);
	output << "\n-语法分析\n\n" << "--打印语法树\n";
  	auto AST = syntax.Programming();
	syntax.dumpAST(AST, 0, output);

	// show结果展示
	Evaluate evaluate;
	output << "\n--显示show函数中的变量值\n";
	evaluate.run(AST, output);

	// 中间代码生成
	Mediate mediate;
	output2 << "\n--打印中间代码\n\n";
    mediate.run(AST, output2);

	// 文件指针关闭
	input_file.close();
	output_file.close();
	return 0;
}