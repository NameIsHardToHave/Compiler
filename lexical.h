#include "base.h"

// 检测到错误时，会丢弃不正确的字符

// 一个词法分析类
class Lexical: public Base{
	private:
		// 状态表
		typedef enum dfaStat
		{
			InitialStat,	// 对应DFA中的初始状态  
			LetterStat,		// 对应DFA中的状态1
			DigitStat,		// 对应DFA中的状态2
			CompareStat,	// 比较运算符的统一状态
		}dfastat;

		// 保留字
		std::map <std::string, TokenType> ReservedWord = {
			{"if", If},
			{"else", Else},
			{"while", While},
			{"main", Main},
			{"show", Show},
			{"int", Int},
			{"char", Char},
			{"true", True},
			{"false", False},
			{"break", Break},
			{"continue", Continue},
		};

		// 无前缀的单个字符
		std::map <std::string, TokenType> SingleWord = {
			{"+", Plus},
			{"-", Subtract},
			{"*", Multiply},
			{"/", Divide},
			{"%", Mod},
			{",", Comma},
			{";", SemiColon},
			{"(", LeftParen},
			{")", RightParen},
			{"{", LeftBrace},
			{"}", RightBrace}
		};

		// 比较运算符
		std::map <std::string, TokenType> CompareWord = {
			{">=", GTE},   
            {">", GT},
            {"<=", LTE},        
            {"<", LT},	
            {"=", Assignment},
            {"==", Equal},    
            {"!=", NOTEqual},  
            {"!", NOT},  
            {"||", OR},     
            {"&&", AND}       
		};

		std::string TokenText;			// 储存当前读取的字符串
		TokenType Tokentype;			// 储存当前的词法单元
        std::list<Token> Tokenlist;		// 保存结果的双向链表

		bool iscompare(char c);			// 判断是否是比较字符的合法前缀
		void addList();					// 将当前词法单元添加到链表中
		dfastat initToken(char c);		// 初始状态处理函数
		dfaStat digitToken(char c);		// 数字状态处理函数
		dfaStat letterToken(char c);	// 字符状态处理函数
		dfaStat compareToken(char c);	// 比较运算符的处理函数

	public:
		std::istream &Input;						// 输入流
		Lexical(std::istream &is): Input(is){};		// 构造函数
		std::list<Token> *TokenReader();			// 词法分析函数（返回双向链表的引用）
		void TokenShow(std::ostream &out);			// 结果展示函数
};

void Lexical::addList(){
	// 如果当前进入DFA的字符串不为空，则将其加入Token列表
	if (!TokenText.empty())	{
		Token token = {
			.type = Tokentype,
			.text = TokenText,
		};
		Tokenlist.push_back(token);
		TokenText.clear();
	}
}

inline bool Lexical::iscompare(char c){
	// 判断是否是比较字符的合法前缀
	static std::set<char> comparewordfirst = {'<', '>', '=', '!', '|', '&'};
	return comparewordfirst.find(c) != comparewordfirst.end();
}

Lexical::dfastat Lexical::initToken(char c){
	// 初始状态下的处理
	if (isspace(c)) {		// 对空值的处理
		return InitialStat;
	}
	TokenText += c;
	if (isalpha(c)) {
		return LetterStat;
	}
	else if (isdigit(c)) {
		return DigitStat;
	}
	else if (iscompare(c)){
		return CompareStat;
	}
	else {
		for (auto iter=SingleWord.cbegin(); iter!=SingleWord.cend(); iter++) {
			if (iter->first == TokenText) {		// 判断是否是单个字符即可确认的状态
				Tokentype = iter->second;
				addList();
				return InitialStat;
			}
		}
		if (int(TokenText[0]) != -1){
			msg = "该字符不能被识别，将丢弃";
			tackleERROR(WARNING, TokenText);
		}
		TokenText.clear();
		return InitialStat;
	}
}

Lexical::dfaStat Lexical::letterToken(char c){
	// 字符串状态下的处理
	if (isdigit(c) || isalpha(c)) {
		TokenText += c;
		return LetterStat;
	}
	else {
		bool reserve_flag = false;		// 判断是否是保留字
		for (auto iter=ReservedWord.cbegin(); iter!=ReservedWord.cend(); iter++) {
			if (iter->first == TokenText) {
				Tokentype = iter->second;
				reserve_flag = true;
			}
		}
		if (!reserve_flag) {
			Tokentype = Identifier;
		}
		addList();
		return initToken(c);
	}
}

Lexical::dfaStat Lexical::digitToken(char c){
	// 数字状态下的处理
	if (isdigit(c)) {
		TokenText += c;
		return DigitStat;
	}
	else {
		if (TokenText.size() > 1 && TokenText[0] == '0'){
			msg = "该数字的首位是0";
			tackleERROR(WARNING, TokenText);
		}
		Tokentype = IntLiteral;
		addList();
		return initToken(c);
	}
}

Lexical::dfaStat Lexical::compareToken(char c){
	// 比较运算符的处理
	TokenText += c;
	if (CompareWord.find(TokenText) == CompareWord.end()){
		TokenText.pop_back();
		Tokentype = CompareWord[TokenText];
		addList();
		return initToken(c);
	}
	return CompareStat;
}

std::list<Base::Token> *Lexical::TokenReader(){
	// 读取文件，返回词法分析后的链表头节点
	TokenText.clear();
	char c;
	dfastat stat = InitialStat;
    //从源文件中开始读取内容，并得到相关Token
	while (!Input.eof()) {
		c = Input.get();
		switch (stat) {		// 根据不同的状态判断不同的输入
			case InitialStat:
				stat = initToken(c);
				break;
			case LetterStat:
				stat = letterToken(c);
				break;
			case DigitStat:
				stat = digitToken(c);
				break;
			case CompareStat:
				stat = compareToken(c);
				break;
			default: 
				msg = "进入了未定义的DFA状态";
				tackleERROR(ERROR);
				break;
		}
	}
	return &Tokenlist;
}

void Lexical::TokenShow(std::ostream &out=std::cout){
	// 打印词法分析的结果
	if (Tokenlist.empty()) {
		msg = "词法分析结果的链表为空";
		tackleERROR(WARNING, TokenText);
		return;
	}
	out << "词法分析" << '\n';
	for (Token p: Tokenlist){
		out << "Token = " << p.text << "\t" << "Type = " << p.type << "\n";
	}
}