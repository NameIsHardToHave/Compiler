#include "base.h"

// 一个遍历语法树，模拟运行的工具
class Imitate: public Base{
    protected:
        // 变量表节点
        typedef struct IdNode{
            std::string text;
            int value;
            bool flag;
        }IdNode, *PIdNode;
        std::list<IdNode> Idlist;           // 变量表

        std::list<Imitate::IdNode>::iterator findIdentifier(std::string text);     // 在变量表中查找变量
        void insertIdNode(std::string text, int value, bool flag);                 // 创建并插值入变量表

    public:
        void showIdTable();                         // 展示变量表
};

// 一个用于根据语法树计算 show 中的函数值的工具
class Evaluate: public Imitate{
    private:
        std::vector<int> resultlist;    // show 结果表
        // 用于循环控制
        typedef struct Loops
        {
            bool continues;
            bool breaks;
        }Loops;
        std::list<Loops> Loopslist;     // 循环嵌套结构（总是用头插法插入新的循环）
        std::list<Loops>::iterator cur_loop = Loopslist.begin();

        int Op_expression(PASTNode node);           // 操作四则运算表达式
        int Op_boole(PASTNode node);                // 操作布尔表达式的值
        void Op_body(PASTNode node);                // 操作可能的函数体

    public:   
        void run(PASTNode node, std::ostream &out); // 开始运行   
};

// 一个用于生成中间代码的工具
class Mediate: public Imitate{
    private:
        std::string mediate_codes;          // 中间代码结果
        std::list<std::string> exp_list;    // 借助列表生成中间代码
        int temp_exp_order = 0;             
        int temp_bool_order = 0;
        int temp_goto_order = 0;            // 初始化三类临时变量的序号
        // 用于循环控制
        typedef struct Loops
        {
            std::string inlet;
            std::string outlet;
        }Loops;
        std::list<Loops> Loopslist;     // 循环嵌套结构（总是用头插法插入新的循环）
        std::list<Loops>::iterator cur_loop = Loopslist.begin();

        std::string exp_record(PASTNode node);          // 四则运算表达式的缓冲函数
        std::string bool_record(PASTNode node);         // 布尔表达式的缓冲函数
        std::string Op_expression(PASTNode node);       // 操作四则运算表达式
        std::string Op_boole(PASTNode node);            // 操作布尔表达式的值
        void Op_body(PASTNode node);                    // 操作可能的函数体
    
    public:   
        void run(PASTNode node, std::ostream &out);     // 开始运行   
};

// 展示变量表
void Imitate::showIdTable() {
    os << "VariableName" << '\t' << "Value" << '\n';
    for (auto id: Idlist) {
        std::string value;
        if (id.flag) {
            value = std::to_string(id.value);
        }
        else {
            value = "No defined";
        }
        os << id.text << '\t' << value << '\n';
    }
}

// 找到变量表中满足的一项（若无则返回尾迭代器）
std::list<Imitate::IdNode>::iterator Imitate::findIdentifier(std::string text) {
    auto iter = Idlist.begin();
    for (; iter!=Idlist.end(); iter++){
        if (iter->text == text){
            break;
        }
    }
    return iter;
}

// 创建并插入新变量至变量表（若有同名变量会覆盖）
void Imitate::insertIdNode(std::string text, int value, bool flag) {
    auto iter = findIdentifier(text);
    if (iter != Idlist.end()) {
        iter->value = value;
        iter->flag = flag;
    }
    else {
        IdNode node = {
            .text = text,
            .value = value,
            .flag = flag,
        };
        Idlist.push_back(node);
    }
}

// 四则运算表达式子树的结果计算
int Evaluate::Op_expression(PASTNode node) {
    if (node == NULL) {
        msg = "Meeting null node when evaluating expression";
        tackleERROR(ERROR);
    }
    PASTNode child1 = NULL, child2 = NULL;
    std::list<Imitate::IdNode>::iterator iter;
    switch (node->nodetype){
        case Plustive:
            child1 = node->children->pnode;
            child2 = node->children->next->pnode;
            return Op_expression(child1) + Op_expression(child2);
        
        case Subtracttive:
            child1 = node->children->pnode;
            child2 = node->children->next->pnode;
            return Op_expression(child1) - Op_expression(child2);

        case Multiplytive:
            child1 = node->children->pnode;
            child2 = node->children->next->pnode;
            return Op_expression(child1) * Op_expression(child2);

        case Dividetive:
            child1 = node->children->pnode;
            child2 = node->children->next->pnode;
            return Op_expression(child1) / Op_expression(child2);

        case Modtive:
            child1 = node->children->pnode;
            child2 = node->children->next->pnode;
            return Op_expression(child1) % Op_expression(child2);

        case IntLiteralASTnode: // 遇到整形字面量则返回其数值
            return atoi(node->text.c_str());

        case IdentifierASTnode: // 遇到变量名则查找并返回其值
            iter = findIdentifier(node->text);
            if (iter == Idlist.end()) {
                msg = "The variable is not defined when evaluating";
                tackleERROR(ERROR, node->text);
            }
            if (iter->flag == false) {
                msg = "The variable is not assigned when evaluating";
                tackleERROR(ERROR, node->text);
            }
            return iter->value;
    
        default:
            msg = "Meeting uncorrect node when evaluating expression";
            tackleERROR(ERROR, node->text);
            return -1;
    }
}

// Bool 表达式子树的结果计算
int Evaluate::Op_boole(PASTNode node) {
    if (node == NULL) {
        msg = "Meeting null node when evaluating boole";
        tackleERROR(ERROR);
    }
    PASTNode child1 = NULL, child2 = NULL;
    switch (node->nodetype){
        case BooleASTNode:
            child1 = node->children->pnode;
            return Op_boole(child1);
        
        case GTASTNode:
            child1 = node->children->pnode;
            child2 = node->children->next->pnode;
            return Op_expression(child1) > Op_expression(child2);

        case GTEASTNode:
            child1 = node->children->pnode;
            child2 = node->children->next->pnode;
            return Op_expression(child1) >= Op_expression(child2);

        case LTASTNode:
            child1 = node->children->pnode;
            child2 = node->children->next->pnode;
            return Op_expression(child1) < Op_expression(child2);

        case LTEASTNode:
            child1 = node->children->pnode;
            child2 = node->children->next->pnode;
            return Op_expression(child1) <= Op_expression(child2);

        case EqualASTNode:
            child1 = node->children->pnode;
            child2 = node->children->next->pnode;
            return Op_expression(child1) == Op_expression(child2);

        case NOTEqualASTNode:
            child1 = node->children->pnode;
            child2 = node->children->next->pnode;
            return Op_expression(child1) != Op_expression(child2);

        case ANDASTNode:
            child1 = node->children->pnode;
            child2 = node->children->next->pnode;
            return Op_boole(child1) && Op_boole(child2);
         
        case ORASTNode:
            child1 = node->children->pnode;
            child2 = node->children->next->pnode;
            return Op_boole(child1) || Op_boole(child2);

        case NOTASTNode:
            child1 = node->children->pnode;
            return !Op_boole(child1);

        case TrueASTNode:
            return true;
            
        case FalseASTNode:
            return false;
    
        default:
            return bool(Op_expression(node));
    }
}

// 函数体结果计算，同时记录 show 函数的值
void Evaluate::Op_body(PASTNode node){
    // 检查节点状态
    if (node == NULL) {
        msg = "Meeting null node when evaluating body";
        tackleERROR(ERROR);
    }
    if (node->children == NULL && (node->nodetype != IntDeclaration 
                                && node->nodetype != BreakASTNode 
                                && node->nodetype != ContinueASTNode)) {
        msg = "Meeting non-children node when evaluating body";
        tackleERROR(ERROR, node->text);
    }
    // 检查是否处于已经终止的循环内部
    if (cur_loop != Loopslist.end()) {
        if (cur_loop->breaks == true || cur_loop->continues == true) {
            return;
        }
    }
    // 开始分析
    PASTChildren child = node->children;
    std::list<Imitate::IdNode>::iterator iter;
    bool bo;
    switch (node->nodetype) {
        case BodyASTNode:       // 遇到函数体语句则执行具体的子节点
            while (child) {
                Op_body(child->pnode);
                child = child -> next;
            }
            break;
        
        case IntDeclaration:    // 遇到变量声明语句，插入至变量表
            if (child != NULL) {
                insertIdNode(node->text, Op_expression(child->pnode), true);
            } 
            else {
                insertIdNode(node->text, 0, false);
            }
            break;

        case AssignmentASTnode: // 遇到赋值语句，检查变量表并填入新值
            iter = findIdentifier(node->text);
            if (iter == Idlist.end()) {
                msg = "The variable is not defined when assigning";
                tackleERROR(ERROR, node->text);
            }
            if (child != NULL) {
                iter->value = Op_expression(child->pnode);
                iter->flag = true;
            }
            break;

        case ShowASTNode:       // 遇到show语句则记录具体的值
            resultlist.push_back(Op_expression(child->pnode));
            break;

        case IfASTNode:         // 遇到if语句则查看是否执行该分支
            bo = Op_boole(child->pnode);
            if (child->next != NULL) {
                if (child->next->pnode->nodetype == BodyASTNode) {
                    if (bo) {
                        Op_body(child->next->pnode);
                    }
                }
                else {
                    if (!bo) {
                        Op_body(child->next->pnode->children->pnode);
                    }
                }
                if (child->next->next != NULL && !bo){
                    Op_body(child->next->next->pnode->children->pnode);
                }
            }
            break;

        case ElseASTNode:       // 这里只在if语句未奏效时才会进入，依赖于If的检查，可以不写
            break;

        case WhileASTNode:      // 遇到while语句则循环执行（可能会因此使分析程序产生死循环）
            Loopslist.push_front({
                false,
                false,
            });
            cur_loop = Loopslist.begin();
            while(true) {
                if (cur_loop->breaks == true) {
                    Loopslist.pop_front();
                    cur_loop = Loopslist.begin();
                    break;
                }
                if (cur_loop->continues == true) {
                    cur_loop->continues = false;
                }
                if (Op_boole(child->pnode)) {
                    if (child->next != NULL) {
                        Op_body(child->next->pnode);
                    }
                    else {
                        msg = "Meeting endless loop when evaluating whileASTNode";
                        tackleERROR(ERROR);
                    }
                }
                else {
                    break;
                }
            }
            break;
        
        case BreakASTNode:      // 无条件跳出最近的循环
            cur_loop->breaks = true;
            break;

        case ContinueASTNode:   // 无条件重新执行循环
            cur_loop->continues = true;
            break;

        case ExpressionASTNode: // 未使用的节点
            msg = "ExpressionASTNode not used";
            tackleERROR(ERROR, node->text);
            break;

        case Primary:           // 未使用的节点
            msg = "PrimaryASTNode not used";
            tackleERROR(ERROR, node->text);
            break;

        default:
            msg = "Meeting uncorrect node when evaluating body";
            tackleERROR(ERROR, node->text);
            break;
    }
}

// 整体的结果计算，同时记录 show 函数的值
void Evaluate::run(PASTNode node, std::ostream &out=std::cout){
    Idlist.clear();     // 清空已有的变量表和结果表的记录
    resultlist.clear();
    if (node != NULL && node->nodetype == Program && node->children != NULL){
        Op_body(node->children->pnode);
        for (auto res: resultlist) {
            out << res << '\n';
        }
    }
    else {
        msg = "Uncorrect or Blank Program";
        tackleERROR(WARNING);
    }
}

// 四则运算表达式的翻译
std::string Mediate::Op_expression(PASTNode node) {
    if (node == NULL) {
        msg = "Meeting null node when evaluating expression";
        tackleERROR(ERROR);
    }
    PASTNode child1 = NULL, child2 = NULL;
    std::string temp_name, temp_part;
    std::list<Imitate::IdNode>::iterator iter;
    switch (node->nodetype){
        case Plustive:
            child1 = node->children->pnode;
            child2 = node->children->next->pnode;
            temp_part = " = " + Op_expression(child1) + " + " + Op_expression(child2);
            temp_name = "T" + std::to_string(temp_exp_order++);
            exp_list.push_back(temp_name + temp_part);
            return temp_name;
        
        case Subtracttive:
            child1 = node->children->pnode;
            child2 = node->children->next->pnode;
            temp_part = " = " + Op_expression(child1) + " - " + Op_expression(child2);
            temp_name = "T" + std::to_string(temp_exp_order++);
            exp_list.push_back(temp_name + temp_part);
            return temp_name;
        
        case Multiplytive:
            child1 = node->children->pnode;
            child2 = node->children->next->pnode;
            temp_part = " = " + Op_expression(child1) + " * " + Op_expression(child2);
            temp_name = "T" + std::to_string(temp_exp_order++);
            exp_list.push_back(temp_name + temp_part);
            return temp_name;

        case Dividetive:
            child1 = node->children->pnode;
            child2 = node->children->next->pnode;
            temp_part = " = " + Op_expression(child1) + " / " + Op_expression(child2);
            temp_name = "T" + std::to_string(temp_exp_order++);
            exp_list.push_back(temp_name + temp_part);
            return temp_name;

        case Modtive:
            child1 = node->children->pnode;
            child2 = node->children->next->pnode;
            temp_part = " = " + Op_expression(child1) + " % " + Op_expression(child2);
            temp_name = "T" + std::to_string(temp_exp_order++);
            exp_list.push_back(temp_name + temp_part);
            return temp_name;

        case IntLiteralASTnode:     // 遇到整形字面量则返回其字面值
            return node->text;

        case IdentifierASTnode:     // 遇到变量名则查找并返回
            iter = findIdentifier(node->text);
            if (iter == Idlist.end()) {
                msg = "The variable is not defined when evaluating";
                tackleERROR(ERROR, node->text);
            }
            if (iter->flag == false) {
                msg = "The variable is not assigned when evaluating";
                tackleERROR(WARNING, node->text);
            }
            return iter->text;

        default:
            msg = "Meeting uncorrect node when evaluating expression";
            tackleERROR(ERROR, node->text);
            return temp_name;
    }
}

// Bool 表达式的翻译
std::string Mediate::Op_boole(PASTNode node) {
    if (node == NULL) {
        msg = "Meeting null node when evaluating boole";
        tackleERROR(ERROR);
    }
    PASTNode child1 = NULL, child2 = NULL;
    std::string temp_name, temp_part;
    switch (node->nodetype){
        case BooleASTNode:
            child1 = node->children->pnode;
            return Op_boole(child1);
        
        case GTASTNode:
            child1 = node->children->pnode;
            child2 = node->children->next->pnode;
            temp_part = " = " + Op_expression(child1) + " > " + Op_expression(child2);
            temp_name = "B" + std::to_string(temp_bool_order++);
            exp_list.push_back(temp_name + temp_part);
            return temp_name;

        case GTEASTNode:
            child1 = node->children->pnode;
            child2 = node->children->next->pnode;
            temp_part = " = " + Op_expression(child1) + " >= " + Op_expression(child2);
            temp_name = "B" + std::to_string(temp_bool_order++);
            exp_list.push_back(temp_name + temp_part);
            return temp_name;

        case LTASTNode:
            child1 = node->children->pnode;
            child2 = node->children->next->pnode;
            temp_part = " = " + Op_expression(child1) + " < " + Op_expression(child2);
            temp_name = "B" + std::to_string(temp_bool_order++);
            exp_list.push_back(temp_name + temp_part);
            return temp_name;

        case LTEASTNode:
            child1 = node->children->pnode;
            child2 = node->children->next->pnode;
            temp_part = " = " + Op_expression(child1) + " <= " + Op_expression(child2);
            temp_name = "B" + std::to_string(temp_bool_order++);
            exp_list.push_back(temp_name + temp_part);
            return temp_name;

        case EqualASTNode:
            child1 = node->children->pnode;
            child2 = node->children->next->pnode;
            temp_part = " = " + Op_expression(child1) + " == " + Op_expression(child2);
            temp_name = "B" + std::to_string(temp_bool_order++);
            exp_list.push_back(temp_name + temp_part);
            return temp_name;

        case NOTEqualASTNode:
            child1 = node->children->pnode;
            child2 = node->children->next->pnode;
            temp_part = " = " + Op_expression(child1) + " != " + Op_expression(child2);
            temp_name = "B" + std::to_string(temp_bool_order++);
            exp_list.push_back(temp_name + temp_part);
            return temp_name;

        case ANDASTNode:
            child1 = node->children->pnode;
            child2 = node->children->next->pnode;
            temp_part = " = " + Op_boole(child1) + " && " + Op_boole(child2);
            temp_name = "B" + std::to_string(temp_bool_order++);
            exp_list.push_back(temp_name + temp_part);
            return temp_name;
        
        case ORASTNode:
            child1 = node->children->pnode;
            child2 = node->children->next->pnode;
            temp_part = " = " + Op_boole(child1) + " || " + Op_boole(child2);
            temp_name = "B" + std::to_string(temp_bool_order++);
            exp_list.push_back(temp_name + temp_part);
            return temp_name;

        case NOTASTNode:
            child1 = node->children->pnode;
            temp_part = " = ! " + Op_boole(child1);
            temp_name = "B" + std::to_string(temp_bool_order++);
            exp_list.push_back(temp_name + temp_part);
            return temp_name;

        case TrueASTNode:
            return node->text;
            
        case FalseASTNode:
            return node->text;
    
        default:
            return Op_expression(node);
    }
}

// 打印四则运算表达式的缓冲函数
std::string Mediate::exp_record(PASTNode node) {
    std::string temp_name = Op_expression(node);
    for (auto line: exp_list){
        mediate_codes += "\t" + line + "\n";
    }
    exp_list.clear();
    return temp_name;
}

// 打印布尔表达式的缓冲函数
std::string Mediate::bool_record(PASTNode node) {
    std::string temp_name = Op_boole(node);
    for (auto line: exp_list){
        mediate_codes += "\t" + line + "\n";
    }
    exp_list.clear();
    return temp_name;
}

// 不同函数体的翻译
void Mediate::Op_body(PASTNode node){
    if (node == NULL) {
        msg = "Meeting null node when evaluating body";
        tackleERROR(ERROR);
    }
    if (node->children == NULL && (node->nodetype != IntDeclaration
                                && node->nodetype != BreakASTNode 
                                && node->nodetype != ContinueASTNode)) {
        msg = "Meeting non-children node when evaluating body";
        tackleERROR(ERROR, node->text);
    }
    PASTChildren child = node->children;
    std::list<Imitate::IdNode>::iterator iter;
    std::string temp_name, temp_name2, temp_name3;
    switch (node->nodetype) {
        case BodyASTNode:       // 遇到函数体语句则执行具体的子节点
            while (child) {
                Op_body(child->pnode);
                child = child -> next;
            }
            break;
        
        case IntDeclaration:    // 遇到变量声明语句，插入至变量表
            if (child != NULL) {
                temp_name = exp_record(child->pnode);
                mediate_codes += "\t" + node->text + " = " + temp_name + "\n";
            }
            else {
                mediate_codes += "\t" + node->text + "=0\n";
            }
            insertIdNode(node->text, 0, true);
            break;

        case AssignmentASTnode: // 遇到赋值语句，同时需要检查变量表
            iter = findIdentifier(node->text);
            if (iter == Idlist.end()) {
                msg = "The variable is not defined when assigning";
                tackleERROR(ERROR, node->text);
            }
            if (child != NULL) {
                temp_name = exp_record(child->pnode);
                mediate_codes += "\t" + iter->text + " = " + temp_name + "\n";
                iter->flag = true;
            }
            break;

        case ShowASTNode:       // 遇到show语句则记录具体的值
            if (child != NULL){
                temp_name = exp_record(child->pnode);
            }
            mediate_codes += "\t" + temp_name + "=" + temp_name + "+0\n";
            break;

        case IfASTNode:         // 遇到if语句则记录跳转分支
            temp_name = bool_record(child->pnode);
            temp_name2 = "L" + std::to_string(temp_goto_order++);
            mediate_codes += "\tBRp " + temp_name + " GOTO " + temp_name2 + "\n";
            // 先生成 Else 语句的中间代码
            if (child->next != NULL) {
                if (child->next->pnode->nodetype == ElseASTNode) {
                    Op_body(child->next->pnode->children->pnode);
                }
                else {
                    if (child->next->next != NULL && child->next->next->pnode->nodetype == ElseASTNode) {
                        Op_body(child->next->next->pnode->children->pnode);
                    }
                }
            }
            temp_name = "L" + std::to_string(temp_goto_order++);
            mediate_codes += "\tGOTO " + temp_name + "\n";
            mediate_codes += temp_name2 + " :\n";
            // 再生成肯定跳转语句的代码
            if (child->next != NULL) {
                if (child->next->pnode->nodetype == BodyASTNode) {
                    Op_body(child->next->pnode);
                }
            }
            mediate_codes += temp_name + " :\n";
            break;

        case ElseASTNode:       // 已在If语句中实现，可以不写
            break;

        case WhileASTNode:      // 遇到while语句则需要判断跳转
            temp_name = "L" + std::to_string(temp_goto_order++);
            mediate_codes += temp_name + " :\n";
            temp_name2 = bool_record(child->pnode);
            temp_name3 = "L" + std::to_string(temp_goto_order++);
            mediate_codes += "\tBRnz " + temp_name2 + " GOTO " + temp_name3 + "\n";
            // 添加和删除循环结构
            Loopslist.push_front({
                temp_name,
                temp_name3,
            });
            cur_loop = Loopslist.begin();
            if (child->next != NULL) {
                Op_body(child->next->pnode);
            }
            Loopslist.pop_front();
            cur_loop = Loopslist.begin();
            mediate_codes += "\tGOTO " + temp_name + "\n";
            mediate_codes += temp_name3 + " :\n";
            break;

        case BreakASTNode:      // 退出当前循环
            mediate_codes += "\tGOTO " + cur_loop->outlet + "\n";
            break;

        case ContinueASTNode:   // 重新开始当前循环
            mediate_codes += "\tGOTO " + cur_loop->inlet + "\n";
            break;

        case ExpressionASTNode: // 未使用的节点
            msg = "ExpressionASTNode not used";
            tackleERROR(ERROR, node->text);
            break;

        case Primary:           // 未使用的节点
            msg = "PrimaryASTNode not used";
            tackleERROR(ERROR, node->text);
            break;

        default:
            msg = "Meeting uncorrect node when evaluating body";
            tackleERROR(ERROR, node->text);
            break;
    }
}

// 整体的结果处理，同时输出中间代码
void Mediate::run(PASTNode node, std::ostream &out=std::cout){
    // 初始化
    Idlist.clear();
    mediate_codes.clear();
    exp_list.clear();
    temp_exp_order = 0;
    temp_bool_order = 0;
    temp_goto_order = 0;
    // 开始自顶向下分析
    if (node != NULL && node->nodetype == Program && node->children != NULL){
        Op_body(node->children->pnode);
        out << mediate_codes;
    }
    else {
        msg = "Uncorrect or Blank Program";
        tackleERROR(WARNING);
    }
}