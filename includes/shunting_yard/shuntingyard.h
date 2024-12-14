#pragma once

#include "../../includes/tokenizer/token.h"
#include "../../includes/queue/queue.h"
#include "../../includes/stack/stack.h"

#include <cassert>

using namespace std;

class ShuntingYard {
public:
    ShuntingYard(Queue<Token*> infix) : queue(infix) {}
    Queue<Token*> postfix();
private:
    Queue<Token*> queue;
};

Queue<Token*> ShuntingYard::postfix() {
    Stack<Token*> operator_stack;
    Queue<Token*> postfix;
    Token* t;

    while (!queue.empty()) {
        t = queue.pop();

        switch (t->type_of()) {
        case TOKEN_STR:
            postfix.push(t);
            break;

        case RELATIONAL:
        case LOGICAL:
        case AND:
        case OR:

            while (!operator_stack.empty() && operator_stack.top()->type_of() != LEFT_PAR) {
                Token* top = operator_stack.top();

                if (top->type_of() >= t->type_of()) {
                    postfix.push(operator_stack.pop());
                }
                else {
                    break;
                }
            }
            operator_stack.push(t);

            break;
        case LEFT_PAR:
            operator_stack.push(t);
            break;
        case RIGHT_PAR:
            while (!operator_stack.empty() && operator_stack.top()->type_of() != LEFT_PAR) {
                assert(!operator_stack.empty() && "opstack is empty");
                postfix.push(operator_stack.pop());
            }
            assert(operator_stack.top()->type_of() == LEFT_PAR);
            if (!operator_stack.empty() && operator_stack.top()->type_of() == LEFT_PAR) {
                operator_stack.pop();
            }
            break;
        default:
            break;
        }
    }

    while (!operator_stack.empty()) {
        postfix.push(operator_stack.pop());
    }

    return postfix;
}
