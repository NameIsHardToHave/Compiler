#include "base.h"

// 遇到错误时会报错终止运行，较小的错误（如缺少分号）只会给予警告

// 仍然存在的不足：
// （1）布尔值中的 ！优先级不够，其后的表达式容易被长匹配
// （2）布尔值无法参与四则运算表达式

class Syntax: public Base{
    private:
        std::list<Token> Tokenlist;		// 保存结果的双向链表                
        std::list<Token>::const_iterator pList = Tokenlist.cbegin();

        int wait_paren = 0;             // 补丁，解决FB的括号无法匹配的问题
        int in_loop = 0;           // 补丁，用于检查break和continue语句的合法性

        void addASTNode(PASTNode node, PASTNode child);     // 把孩子节点添加到双亲结点上
        PASTNode createASTNode(ASTNodeType type, std::string text, PASTNode parent, 
                            PASTChildren children);     // 创建新的语法树节点

        PASTNode boolStatement();           // 布尔表达式以及四个辅助函数
        PASTNode bool_B1(PASTNode pre);
        PASTNode bool_TB();
        PASTNode bool_TB1(PASTNode pre);
        PASTNode bool_FB();
        PASTNode expressionStatement();     // 四则运算表达式以及四个辅助函数
        PASTNode expression_E1(PASTNode pre);
        PASTNode expression_TE();
        PASTNode expression_TE1(PASTNode pre);
        PASTNode expression_F();
           
        void intdeclarationBody(PASTNode node);     // 整型声明语句
        void assignmentBody(PASTNode node);         // 赋值语句
        void showBody(PASTNode node);               // show 语句
        void ifBody(PASTNode node);                 // if 语句
        void whileBody(PASTNode node);              // while 语句
        void breakBody(PASTNode node);              // break 语句
        void continueBody(PASTNode node);           // continue 语句
        PASTNode Body();                            // 函数体

        // 语法节点决定相应的继续分析的产生式
        std::map <ASTNodeType, decltype(&Syntax::boolStatement)> ASTfunc = {
            {ExpressionASTNode, expressionStatement},
            {BooleASTNode, boolStatement},
        };

        // 词法前缀决定函数体产生式
        std::map <TokenType, decltype(&Syntax::showBody)> ASTbody = {
            {Int, intdeclarationBody},
            {Identifier, assignmentBody},
            {Show, showBody},
            {If, ifBody},
            {While, whileBody},
            {Break, breakBody},
            {Continue, continueBody},
        };

        void MatchNext(ASTNodeType type, PASTNode parent, ERRORtype error);     // 匹配下一个词法单元
        void MatchNext(TokenType type, PASTNode parent, ERRORtype error);       // 匹配下一个语法单元

    public:
        Syntax(std::list<Token> &s): Tokenlist(s){};            // 构造函数
        void UpdateTokenlist(std::list<Token> newlist);         // 词法链表
        void dumpAST(PASTNode p, int depth, std::ostream &out); // 遍历并打印语法树
        PASTNode Programming();                                 // 从程序体开始分析
};

// 更新传入的词法链表
void Syntax::UpdateTokenlist(std::list<Token> newlist) {
    Tokenlist = newlist;
    pList = Tokenlist.cbegin();
}

// 创建新的语法树节点
Syntax::PASTNode Syntax::createASTNode(ASTNodeType type, std::string text, PASTNode parent=NULL, 
                            PASTChildren children=NULL) {
    PASTNode node = new ASTNode;
    node->nodetype = type;
    node->text = text;
    node->parent = parent;
    node->children = children;
    return node;
}

// 把孩子节点添加到双亲结点上（要求孩子和双亲节点均为非空）
void Syntax::addASTNode(PASTNode node, PASTNode child){
    if (node != NULL && child != NULL) {
        PASTChildren pchild = new ASTChildren;
        pchild->pnode = child;
        pchild->next = NULL;
        if (node->children == NULL) {
            node->children = pchild;
        }
        else {
            PASTChildren p = node->children;
            while (p->next != NULL) {
                p = p->next;
            }
            p->next = pchild;
        }
        child->parent = node;
    }
}

// 遍历并打印语法树
void Syntax::dumpAST(PASTNode p, int depth, std::ostream &out=std::cout) {
    if (p == NULL) {
        return;
    }
    std::string blank(depth, '\t');
    out << blank << ASTTypeTransfrom[p->nodetype] << '\t' << p->text << '\n';
    PASTChildren children = p->children;
    while (children != NULL) {
        dumpAST(children->pnode, depth+1, out);
        children = children->next;
    }
}

// 匹配下一个词法单元（不会添加孩子节点）
void Syntax::MatchNext(TokenType type, PASTNode parent, ERRORtype error=ERROR) {
    if (pList != Tokenlist.cend() && pList->type == type) {
        pList++;
    } 
    else {
        msg = "in " + ASTTypeTransfrom[parent->nodetype] + " statement, expecting a " + TokenTypeTransfrom[type];
        tackleERROR(error);
    }
}

// 匹配下一个语法单元（会自动添加孩子节点）
void Syntax::MatchNext(ASTNodeType type, PASTNode parent, ERRORtype error=ERROR) {
    PASTNode child = NULL;
    if (pList != Tokenlist.cend() && (child = (this->*(ASTfunc[type]))()) != NULL) {
        addASTNode(parent, child);
    } 
    else {
        msg = "in " + ASTTypeTransfrom[parent->nodetype] + " statement, unvalid " + ASTTypeTransfrom[type];
        tackleERROR(error);
    }
}

// E1 -> + TE E1 | - TE E1 | ε
Syntax::PASTNode Syntax::expression_E1(PASTNode pre) {
    static std::map <TokenType, ASTNodeType> E1_table = {
        {Plus, Plustive},
        {Subtract, Subtracttive}
    };
    if (pList != Tokenlist.cend() && E1_table.find(pList->type) != E1_table.end()) {
        PASTNode parent = createASTNode(E1_table[pList->type], TokenTypeTransfrom[pList->type]);
        pList++;
        addASTNode(parent, pre);
        addASTNode(parent, expression_TE());
        parent = expression_E1(parent);
        return parent;
    }
    else {
        return pre;
    }
}

// TE -> F TE1
Syntax::PASTNode Syntax::expression_TE() {
    PASTNode front = expression_F();
    return expression_TE1(front);
}

// TE1 -> * F TE1 | / F TE1 | ε
Syntax::PASTNode Syntax::expression_TE1(PASTNode pre) {
    static std::map <TokenType, ASTNodeType> TE1_table = {
        {Multiply, Multiplytive},
        {Divide, Dividetive},
        {Mod, Modtive}
    };
    if (pList != Tokenlist.cend() && TE1_table.find(pList->type) != TE1_table.end()) {
        PASTNode parent = createASTNode(TE1_table[pList->type], TokenTypeTransfrom[pList->type]);
        pList++;
        addASTNode(parent, pre);
        addASTNode(parent, expression_F());
        parent = expression_TE1(parent);
        return parent;
    }
    else {
        return pre;
    }
}

// F -> id | num | (E) （提供了足够的错误检查，确保返回正确值）
Syntax::PASTNode Syntax::expression_F(){
    PASTNode node = NULL; 
    if (pList != Tokenlist.cend()) {
        if (pList->type == IntLiteral) {
            node = createASTNode(IntLiteralASTnode, pList->text);
            pList++;
        }
        else if (pList->type == Identifier) {
            node = createASTNode(IdentifierASTnode, pList->text);
            pList++;
        }
        else if (pList->type == LeftParen) {
            pList++;
            node = expressionStatement();
            if (node != NULL) {
                if (pList!=Tokenlist.cend() && pList->type == RightParen) {
                    pList++;
                }
                else if (wait_paren == 1) {
                    wait_paren = 2;
                }
                else {
                    msg = "in primary statement, expecting )";
                    tackleERROR(ERROR);
                }
            }
            else {
                msg = "in primary statement, expecting an expression";
                tackleERROR(ERROR);
            }
        }
        else {
            msg = "no matched primary statement";
            tackleERROR(ERROR);
        }
    }
    else {
        msg = "Program end too early, expecting a primary statement";
        tackleERROR(ERROR);
    }
    return node;
}

// 四则运算表达式 E -> TE E1
Syntax::PASTNode Syntax::expressionStatement() {
    PASTNode front = expression_TE();
    return expression_E1(front);
}

// B1 -> ‘||’ TB B1 | ε
Syntax::PASTNode Syntax::bool_B1(PASTNode pre) {
    if (pList != Tokenlist.cend() && pList->type == OR) {
        PASTNode parent = createASTNode(ORASTNode, "||");
        pList++;
        addASTNode(parent, pre);
        addASTNode(parent, bool_TB());
        parent = bool_B1(parent);
        return parent;
    }
    else {
        return pre;
    }
}

// TB -> FB TB1
Syntax::PASTNode Syntax::bool_TB() {
    PASTNode parent = bool_FB();
    return bool_TB1(parent);
}

// TB1 -> && FB TB1 | ε
Syntax::PASTNode Syntax::bool_TB1(PASTNode pre) {
    if (pList != Tokenlist.cend() && pList->type == AND) {
        PASTNode parent = createASTNode(ANDASTNode, "&&");
        pList++;
        addASTNode(parent, pre);
        addASTNode(parent, bool_FB());
        parent = bool_TB1(parent);
        return parent;
    }
    else {
        return pre;
    }
}

// FB -> E >[=] E | E <[=] E | E == E | E != E | E | !B | TRUE | FALSE
Syntax::PASTNode Syntax::bool_FB() {
    PASTNode node = NULL, child1 = NULL, child2 = NULL; 
    if (pList != Tokenlist.cend()) {
        if (pList->type == True) {
            node = createASTNode(TrueASTNode, "true");
            pList++;
        }
        else if (pList->type == False) {
            node = createASTNode(FalseASTNode, "false");
            pList++;
        }
        else if (pList->type == NOT) {
            node = createASTNode(NOTASTNode, "!");
            pList++;
            child1 = boolStatement();
            if (child1 != NULL) {
                addASTNode(node, child1);
            }
            else {
                msg = "in not statement, expecting boolstatement";
                tackleERROR(ERROR);
            }
        }
        else {
            if (pList->type == LeftParen && wait_paren == 0) {
                wait_paren = 1;
            }
            child1 = expressionStatement();
            static std::map <TokenType, ASTNodeType> FB_table = {
                {GT, GTASTNode},
                {GTE, GTEASTNode},
                {LT, LTASTNode},
                {LTE, LTEASTNode},
                {Equal, EqualASTNode},
                {NOTEqual, NOTEqualASTNode},
            };
            if (pList != Tokenlist.cend() && FB_table.find(pList->type) != FB_table.end()) {
                node = createASTNode(FB_table[pList->type], TokenTypeTransfrom[pList->type]);
                pList++;
                child2 = expressionStatement();
                addASTNode(node, child1);
                addASTNode(node, child2);
            }
            else {
                node = child1;
            }
            if (pList->type == RightParen && wait_paren == 2) {
                pList ++;
                wait_paren = 0;
            }
        }
    }
    else {
        msg = "Program end too early, expecting a simple bool statement";
        tackleERROR(ERROR);
    }
    return node;
}

// 布尔表达式 B -> TB B1 （这个函数外面可以加括号）
Syntax::PASTNode Syntax::boolStatement() {
    PASTNode node = createASTNode(BooleASTNode, "Boole");
    bool paren = false;
    if (pList != Tokenlist.cend() && pList->type == LeftParen) {
        paren = true;
        MatchNext(LeftParen, node);
    }
    PASTNode parent = bool_TB();
    parent = bool_B1(parent);
    addASTNode(node, parent);
    if (paren) {
        MatchNext(RightParen, node);
    }
    return node;
}

// 整型变量声明语句 -> 类型 id [= E] id_list
void Syntax::intdeclarationBody(PASTNode node) {
    PASTNode parent = createASTNode(IntDeclaration, "int");         // 分析主声明语句
    MatchNext(Int, parent);
    if (pList != Tokenlist.cend() && pList->type == Identifier) {
        parent->text = pList->text;
    }
    MatchNext(Identifier, parent);
    if (pList != Tokenlist.cend() && pList->type == Assignment) {
        MatchNext(Assignment, parent);
        MatchNext(ExpressionASTNode, parent);
    }
    addASTNode(node, parent);
    while (true) {
        if (pList != Tokenlist.cend() && pList->type == Comma) {    // 分析逗号之后的声明语句
            parent = createASTNode(IntDeclaration, "int");
            MatchNext(Comma, parent);
            if (pList != Tokenlist.cend() && pList->type == Identifier) {
                parent->text = pList->text;
            }
            MatchNext(Identifier, parent);
            if (pList != Tokenlist.cend() && pList->type == Assignment) {
                MatchNext(Assignment, parent);
                MatchNext(ExpressionASTNode, parent);
            }
            addASTNode(node, parent);
        }
        else {                                                      // 遇到分号退出
            MatchNext(SemiColon, parent, WARNING);
            break;
        }
    }
}

// 赋值语句 -> id = E ;
void Syntax::assignmentBody(PASTNode node) {
    PASTNode parent = createASTNode(AssignmentASTnode, "assignment");
    if (pList != Tokenlist.cend() && pList->type == Identifier) {
        parent->text = pList->text;
    }
    MatchNext(Identifier, parent);
    MatchNext(Assignment, parent);
    MatchNext(ExpressionASTNode, parent);
    MatchNext(SemiColon, parent, WARNING);
    addASTNode(node, parent);
}

// show (E);
void Syntax::showBody(PASTNode node) {
    PASTNode parent = createASTNode(ShowASTNode, "show");
    MatchNext(Show, parent);
    MatchNext(LeftParen, parent);
    MatchNext(ExpressionASTNode, parent);
    MatchNext(RightParen, parent);
    MatchNext(SemiColon, parent, WARNING);
    addASTNode(node, parent);
}

// if (B) {函数体} [else {函数体}]
void Syntax::ifBody(PASTNode node) {
    PASTNode parent = createASTNode(IfASTNode, "if");
    MatchNext(If, parent);
    MatchNext(LeftParen, parent);
    MatchNext(BooleASTNode, parent);
    MatchNext(RightParen, parent);
    MatchNext(LeftBrace, parent);
    addASTNode(parent, Body());
    MatchNext(RightBrace, parent);
    if (pList != Tokenlist.cend() && pList->type == Else) {
        PASTNode child = createASTNode(ElseASTNode, "else");
        MatchNext(Else, parent);
        MatchNext(LeftBrace, child);
        addASTNode(child, Body());
        MatchNext(RightBrace, child);
        addASTNode(parent, child);
    }
    addASTNode(node, parent);
}

// while (B) {函数体}
void Syntax::whileBody(PASTNode node) {
    PASTNode parent = createASTNode(WhileASTNode, "while");
    MatchNext(While, parent);
    MatchNext(LeftParen, parent);
    MatchNext(BooleASTNode, parent);
    MatchNext(RightParen, parent);
    MatchNext(LeftBrace, parent);
    in_loop += 1;
    addASTNode(parent, Body());
    in_loop -= 1;
    MatchNext(RightBrace, parent);
    addASTNode(node, parent);
}

// break ;
void Syntax::breakBody(PASTNode node) {
    PASTNode parent = createASTNode(BreakASTNode, "break");
    MatchNext(Break, parent);
    MatchNext(SemiColon, parent, WARNING);
    // 有效性检查
    if (in_loop == 0) {
        msg = "Break statement should be in while loop";
        tackleERROR(ERROR);
    }
    else {
        addASTNode(node, parent);
    }
}

// continue ;
void Syntax::continueBody(PASTNode node) {
    PASTNode parent = createASTNode(ContinueASTNode, "continue");
    MatchNext(Continue, parent);
    MatchNext(SemiColon, parent, WARNING);
    // 有效性检查
    if (in_loop == 0) {
        msg = "Continue statement should be in while loop";
        tackleERROR(ERROR);
    }
    else {
        addASTNode(node, parent);
    }
}

// 分析函数体不同前缀下的产生式
Syntax::PASTNode Syntax::Body() {
    PASTNode node = createASTNode(BodyASTNode, "Body");;
    if (pList != Tokenlist.cend()) {
        if (ASTbody.find(pList->type) != ASTbody.end()) {   // 检查合适的前缀字符
            (this->*(ASTbody[pList->type]))(node);
            addASTNode(node, Body());
        } 
        else if (pList->type == RightBrace){                // 函数体 -> \epsilon
            node = NULL;
        }
        else {                                              // 未知的前缀字符
            msg = "unknown statement in main body";
            tackleERROR(ERROR);
        }
    }
    else {                                                  // 文件指针提前终止
        msg = "Program Lack of RightBrace or Code End too Early";
        tackleERROR(ERROR);
    }
    return node;
}

// 程序体 -> main() {函数体}
Syntax::PASTNode Syntax::Programming() {
    PASTNode parent = createASTNode(Program, "main");
    MatchNext(Main, parent);
    MatchNext(LeftParen, parent);
    MatchNext(RightParen, parent);
    MatchNext(LeftBrace, parent);
    addASTNode(parent, Body());
    MatchNext(RightBrace, parent);
    if (pList != Tokenlist.cend()) {                    // 末尾之后的代码将被忽略
        msg = "The following content will be discarded";
        tackleERROR(WARNING);
    }
    return parent;
}