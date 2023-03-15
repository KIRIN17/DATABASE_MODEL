#include "node.h"


bool EmptyNode::Evaluate(Date date,string s) {return true;}
bool AlwaysFalseNode::Evaluate(Date date, string event) {return false;}
DateComparisonNode::DateComparisonNode(Comparison cmp,Date date) : cmp_(cmp),date_(date){}

bool DateComparisonNode::Evaluate(Date date,string s) {
    if(cmp_ == Comparison::Equal)
        return date_ == date;
    if(cmp_ == Comparison::NotEqual)
        return date_ != date;
    if(cmp_ == Comparison::Greater)
        return date_ < date;
    if(cmp_ == Comparison::GreaterOrEqual)
        return date_ <= date;
    if(cmp_ == Comparison::Less)
        return date_ > date;
    if(cmp_ == Comparison::LessOrEqual)
        return date_ >= date;
}

EventComparisonNode::EventComparisonNode(Comparison cmp, string s) : cmp_(cmp),s_(s) {}

bool EventComparisonNode::Evaluate(Date date, string s) {


    if(cmp_ == Comparison::GreaterOrEqual)
        return s_ <= s;
    if(cmp_ == Comparison::LessOrEqual)
        return s_ >= s;
    if(cmp_ == Comparison::Greater)
        return s_ < s;
    if(cmp_ == Comparison::Less)
        return s_ > s;
    if(cmp_ == Comparison::Equal)
        return s_ == s;
    if(cmp_ == Comparison::NotEqual)
        return s_ != s;
}

LogicalOperationNode::LogicalOperationNode(LogicalOperation cmp, shared_ptr<Node> r, shared_ptr<Node> l) : cmp_(cmp),r_(r),l_(l) {}

bool LogicalOperationNode::Evaluate(Date date, string s) {
    if(cmp_ == LogicalOperation::And)
        return r_->Evaluate(date,s) && l_->Evaluate(date,s);
    if(cmp_ == LogicalOperation::Or)
        return r_->Evaluate(date,s) || l_->Evaluate(date,s);
}