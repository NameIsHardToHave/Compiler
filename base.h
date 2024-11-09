#pragma once

#include<string>
#include<list>
#include<map>
#include<set>
#include<vector>
#include<new>
#include<iostream>
#include<fstream>

// 基类定义了编译器各个部分都可能使用到的数据结构
class Base{
    public:
        std::ostream &os = std::cout;		// 输出流

    protected:
        // 词法单元的类型
        typedef enum TokenType
        {
            Plus, 			// +
            Subtract,		// -
            Multiply,		// *
            Divide, 		// /
            Mod,            // %
            GTE,            // >=
            GT, 			// >
            LTE,            // <=
            LT, 			// <
            Assignment, 	// =
            Equal,          // == 
            NOTEqual,       // !=
            NOT,            // !
            OR,             // ||
            AND,            // &&
            Comma,			// ,
            SemiColon, 		// ;
            LeftParen, 		// (
            RightParen, 	// )
            LeftBrace, 		// { 
            RightBrace, 	// }
            If,         	// if
            Else,          	// else
            While,			// while
            Int,			// int
            Char,			// char
            True,           // true
            False,          // false
            Identifier,  	// 标识符
            IntLiteral,  	// 整形字面量
            Main,        	// main
            Show,        	// show
            Break,          // break
            Continue,       // continue
        }TokenType;

        // 词法单元的字符串展示
        std::vector<std::string> TokenTypeTransfrom = {
            "+", 			
            "-",		
            "*",		
            "/", 		
            "%",            
            ">=",           
            ">", 			
            "<=",            
            "<", 			
            "=", 	
            "==",          
            "!=",       
            "!",            
            "||",             
            "&&",            
            ",",			
            ";", 		
            "(", 		
            ")", 	
            "{", 		
            "}", 	
            "if",         	
            "else",          	
            "while",			
            "int",			
            "char",			
            "true",           
            "false",          
            "identifier",  
            "intliteral",  	
            "main",        	
            "show",       
            "break",
            "continue", 	
        };

        // 词法单元
        typedef struct Token
        {
            TokenType type;
            std::string text;
        }Token;		

        // 语法分析器中相关数据结构的定义
        typedef enum ASTNodeType{
            Program,                // 程序入口
            IntDeclaration,         // 整型变量声明
            ExpressionASTNode,      // 表达式语句（未使用）
            AssignmentASTnode,      // 赋值语句
            Primary,                // 基础表达式（未使用）
            Plustive,               // 加法表达式
            Subtracttive,           // 减法表达式
            Multiplytive,           // 乘法表达式
            Dividetive,             // 除法表达式
            Modtive,                // 模运算表达式
            IdentifierASTnode,      // 标识符
            IntLiteralASTnode,      // 整型字面量
            BodyASTNode,			// 函数体
            IfASTNode,              // if 语句
            ElseASTNode,            // else 语句
            WhileASTNode,           // while 语句
            BooleASTNode,           // 布尔表达式（已使用）
            GTEASTNode,             // >= 表达式
            GTASTNode, 			    // > 表达式
            LTEASTNode,             // <= 表达式
            LTASTNode, 			    // < 表达式
            EqualASTNode,           // == 表达式
            NOTEqualASTNode,        // != 表达式
            NOTASTNode,             // ! 表达式
            ORASTNode,              // || 表达式
            ANDASTNode,             // && 表达式
            TrueASTNode,            // true 表达式
            FalseASTNode,           // false 表达式
            ShowASTNode,            // show 语句
            BreakASTNode,           // break 语句
            ContinueASTNode,        // continue 语句
        }ASTNodeType;

        // 数据结构的直观字符串转化
        std::vector <std::string> ASTTypeTransfrom = {
            "Program",
            "IntDeclaration",
            "Expression",
            "Assignment",
            "Primary",
            "Addtive",
            "Subtracttive",
            "Multiplytive",
            "Dividetive",
            "Modtive",
            "Identifier",
            "IntLiteral",
            "Body",
            "If",
            "Else",
            "While",
            "Boole",
            "GTE",
            "GT", 
            "LTE",          
            "LT", 		
            "Equal",         
            "NOTEqual",     
            "NOT",        
            "OR",           
            "AND",          
            "True",     
            "False",        
            "Show",
            "Break",
            "Continue",
        };

        // 语法树节点
        struct ASTChildren;                 // 结构体的事先声明（因为出现了互引用）
        typedef struct ASTNode{
            ASTNodeType nodetype;           // 结点的语法类型（赋值，表达式等）
            std::string text;				// 结点类型对应的实体名称
            ASTNode *parent;				
            ASTChildren *children;
        }ASTNode, *PASTNode;
        typedef struct ASTChildren{
            ASTNode *pnode;
            ASTChildren *next;
        }ASTchildren, *PASTChildren;

        // 错误类型
        typedef enum ERRORtype{
            ERROR,
            INVALID,
            WARNING,
        }ERRORtype;

        // 错误消息
        std::string msg;

        // 错误处理函数（可重写）
        void tackleERROR(ERRORtype type, std::string text);
};

// 错误情况处理函数
void Base::tackleERROR(ERRORtype type, std::string text=""){
	switch (type){
        case ERROR:
            os << "ERROR! " << text << ' ' << msg << '\n';
            exit(0);
            break;
        case INVALID:
            os << "INVALID! " << text << ' '  << msg << '\n';
            break;
        case WARNING:
            os << "WARNING! " << text << ' '  << msg << '\n';
            break;
        default:
            os << "tackleERROR Program Failed!\n";
            break;
    }
}