#pragma once
#include "date.h"

enum class LogicalOperation{
    And,Or
};

enum class Comparison{
    Less,LessOrEqual,Greater,GreaterOrEqual,Equal,NotEqual
};

class Node{
public:
    virtual bool Evaluate(Date date,string s) = 0;
};

class EmptyNode : public Node{
public:
    bool Evaluate(Date date,string s) override;
};

class DateComparisonNode : public Node{
public:

    DateComparisonNode(Comparison cmp,Date date);// : cmp_(cmp),date_(date){}
    bool Evaluate(Date date,string s) override;

private:
    Comparison cmp_;
    Date date_;
};

class EventComparisonNode : public Node{
public:
    EventComparisonNode(Comparison cmp,string s);
    bool Evaluate(Date date,string s) override;

private:
    Comparison cmp_;
    string s_;
};

class LogicalOperationNode : public Node{
public:
    LogicalOperationNode(LogicalOperation cmp,shared_ptr<Node> r,shared_ptr<Node> l);
    bool Evaluate(Date date,string s) override;

private:
    LogicalOperation cmp_;
    shared_ptr<Node> r_;
    shared_ptr<Node> l_;
};

class AlwaysFalseNode : public Node {
public:
    bool Evaluate(Date date,string event) override;
};
