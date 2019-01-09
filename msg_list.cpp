#include <iostream>

#include "msg_list.h"

msg_list::msg_list(){
	front = back = NULL;
}

void msg_list::display() {
	msg_list_node *tmp;  int i;
	
	if(front==NULL) {
		cout << "** List is empty. **\n";
		return;
	}
	
	tmp = front;  i = 1;
	while(tmp!=NULL) {
		cout << "Datagram " << i++ << ":  \n";
		(tmp->d)->display();
		cout << "\n";
		tmp = tmp->next;
	}
	
}

void msg_list::append(datagram *x) {
	msg_list_node* tmp = new msg_list_node;
	tmp->next = NULL;
	tmp->d = x;
    
	if(front==NULL)
        front = tmp;
	else
        back->next = tmp;
	back = tmp;
}

datagram* msg_list::returnFront(){
    if (this->front == NULL) {
        return NULL;
    } else {
        msg_list_node* firstNode = new msg_list_node;
        firstNode = this->front;
        datagram* d1 = firstNode->d;
        if (firstNode->next == NULL) {
            this->front = NULL;
            this->back = NULL;
        } else {
            this->front = firstNode->next;
        }
        delete firstNode;
        return d1;
    }
}

void msg_list::deleteList(){
    msg_list_node* currentNode = new msg_list_node;
    while (this->front != NULL) {
        
        currentNode = this->front;
        this->front = (this->front)->next;
        delete currentNode->d;
        delete currentNode;
    }
    this->back = NULL;
}
