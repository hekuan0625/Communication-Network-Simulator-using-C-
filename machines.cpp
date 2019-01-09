#include <iostream>
#include <stdlib.h>
#include <string>

#include "machines.h"

//*****************************************//
// node class functions

node::node(string n, IPAddress a) {
    name = new string;
    *name = n;
    my_address = a;
}

node::~node() {
    delete name;
}

void node::display() {
    
    cout << "   Name: " << *name << "   IP address: ";
    my_address.display();
}

int node::amIThisComputer(IPAddress a) {
    if(my_address.sameAddress(a))
        return 1;
    else
        return 0;
}

int node::myType() {
    return 0;
}

IPAddress node::myAddress(){
    return this->my_address;
    
}

//*****************************************//
// laptop class functions

laptop::laptop(string n, IPAddress a) : node(n,a)  {
    incoming = outgoing = NULL;
    my_server.parse("0.0.0.0");
}

int laptop::myType() {
    return LAPTOP;
}

void laptop::initiateDatagram(datagram* d) {
    outgoing = d;
}

void laptop::receiveDatagram(datagram* d) {
    incoming = d;
}

int  laptop::canAcceptConnection(int machine_type) {
    if(machine_type!=SERVER) return 0;
    return my_server.isNULL(); //we can only connect if the server is empty
}

void laptop::connect(IPAddress x, int machine_type) {
    if(machine_type==SERVER) my_server = x;
}

void laptop::transferDatagram() {
    int i;
    extern node* network[MAX_MACHINES];
    
    if(outgoing==NULL) return;
    if(my_server.isNULL()) return;
    for (i = 0; i < MAX_MACHINES; i++)
    {
        if (network[i] != NULL)
        {
            if (network[i]->amIThisComputer(my_server))
                break;
        }
    }
    network[i]->receiveDatagram(outgoing);
    outgoing = NULL;
}

void laptop::display() {
    
    cout << "Laptop computer:  ";
    node::display();
    
    if(my_server.isNULL()) {
        cout << "\n    Not connected to any server.\n";
    }
    else {
        cout << "\nConnected to ";
        my_server.display();
        cout << "\n";
    }
    
    cout << "\n   Incoming datagram:  ";
    if(incoming==NULL) cout << "No datagram.";
    else               {cout << "\n";  incoming->display(); }
    
    cout << "\n   Outgoing datagram:  ";
    if(outgoing==NULL) cout << "No datagram.";
    else               {cout << "\n"; outgoing->display(); }
    
}

/**************new*************/
laptop::~laptop()
{
    if (incoming != NULL)
        delete incoming;
    if (outgoing != NULL)
        delete outgoing;
}

void laptop::consumeDatagram(){
    this->incoming = NULL;
}

int laptop::canReceiveDatagram(){
    if (incoming == NULL) {
        return 1;
    } else {
        return 0;
    }
}
/**************new*************/

//*****************************************//
// server class functions

server::server(string n, IPAddress a) : node(n,a)  {
    number_of_laptops = number_of_wans = 0;
    dlist = new msg_list;
}


int server::myType() {
    return SERVER;
}

int  server::canAcceptConnection(int machine_type) {
    if(machine_type==LAPTOP)
        return (number_of_laptops<8);
    else if(machine_type==WAN_MACHINE)
        return (number_of_wans<4);
    return 0;
}

void server::connect(IPAddress x, int machine_type) {
    if(machine_type==LAPTOP)
        laptop_list[number_of_laptops++] = x;
    else if(machine_type==WAN_MACHINE)
        WAN_list[number_of_wans++] = x;
}

void server::receiveDatagram(datagram* d) {
    dlist->append(d);
}

void server::display() {
    int i;
    
    cout << "Server computer:  ";
    node::display();
    
    cout << "\n   Connections to laptops: ";
    if(number_of_laptops==0) cout << "    List is empty.";
    else for(i=0; i<number_of_laptops; i++) {
        cout << "\n      Laptop " << i+1 << ":   ";
        laptop_list[i].display();
    }
    cout << "\n\n   Connections to WANs:    ";
    if(number_of_wans==0) cout << "    List is empty.";
    else for(i=0; i<number_of_wans; i++) {
        cout << "\n         WAN " << i+1 << ":   ";
        WAN_list[i].display();
    }
    
    cout << "\n\n   Message list:\n";
    dlist->display();
    
    cout << "\n\n";
    
}

/**************new*************/
server::~server()
{
    dlist->deleteList();
}

void server::transferDatagram(){
    extern node* network[MAX_MACHINES];
    msg_list* temporyList = new msg_list;
    datagram* myData = (this->dlist)->returnFront();
    
    while (myData != NULL) {
        IPAddress myDest = myData->destinationAddress();
        int myFirstOctad = myDest.firstOctad();
        
        if ((myFirstOctad - (this->my_address).firstOctad()) == 0) {
            for (int i = 0; i < this->number_of_laptops; i++) {
                if (myDest.sameAddress((this->laptop_list)[i])) {
                    for (int j = 0; j < MAX_MACHINES; j++) {
                        if (network[j] != NULL) {
                            if (network[j]->amIThisComputer(myDest)) {
                                if (((laptop*)network[j])->canReceiveDatagram()) { // not connected or cannot
                                    network[j]->receiveDatagram(myData);
                                    break;
                                } else {
                                    temporyList->append(myData);
                                    break;
                                }
                            }
                        }
                    }
                    break;
                }
                if (i == 7) {
                    temporyList->append(myData);
                }
            }
        } else {    // transfer data to WAN at this TIME CLICK, Not next time!
            if (this->number_of_wans == 0) {
                temporyList->append(myData);
            } else {
                int a[this->number_of_wans];
                for (int i = 0; i < this->number_of_wans; i++) {
                    a[i] = abs(myFirstOctad - ((this->WAN_list)[i]).firstOctad());
                }
                
                int min = a[0];
                int number = 0;
                for (int i = 0; i < this->number_of_wans; i++) {
                    if (a[i] < min || a[i] == min) {
                        min = a[i];
                        number = i;
                    }
                }
                
                IPAddress nextWan = WAN_list[number];
                
                for (int j = 0; j < MAX_MACHINES; j++) {
                    if (network[j] != NULL) {
                        if (network[j]->amIThisComputer(nextWan)) {
                            network[j]->receiveDatagram(myData);
                            break;
                        }
                    }
                }
            }
        }
        //delete myData;
        myData = (this->dlist)->returnFront();
    }
    delete dlist;
    this->dlist = temporyList;
}
/**************new*************/

//*****************************************//
// WAN class functions

WAN::WAN(string n, IPAddress a) : node(n,a)  {
    number_of_servers = number_of_wans = 0;
    dlist = new msg_list;
}

int WAN::myType() {
    return WAN_MACHINE;
}

int  WAN::canAcceptConnection(int machine_type) {
    if(machine_type==SERVER)
        return (number_of_servers<4);
    else if(machine_type==WAN_MACHINE)
        return (number_of_wans<4);
    
    return 0;
}

void WAN::connect(IPAddress x, int machine_type) {
    if(machine_type==SERVER)
        server_list[number_of_servers++] = x;
    else if(machine_type==WAN_MACHINE)
        WAN_list[number_of_wans++] = x;
}

void WAN::receiveDatagram(datagram* d) {
    dlist->append(d);
}


void WAN::display() {
    int i;
    
    cout << "WAN computer:  ";
    node::display();
    
    cout << "\n   Connections to servers: ";
    if(number_of_servers==0) cout << "    List is empty.";
    else for(i=0; i<number_of_servers; i++) {
        cout << "\n      Server " << i+1 << ":   ";
        server_list[i].display();
    }
    cout << "\n\n   Connections to WANs:    ";
    if(number_of_wans==0) cout << "    List is empty.";
    else for(i=0; i<number_of_wans; i++) {
        cout << "\n         WAN " << i+1 << ":   ";
        WAN_list[i].display();
    }
    
    cout << "\n\n   Message list:\n";
    dlist->display();
    
    cout << "\n\n";
    
}

/**************new*************/
WAN::~WAN()
{
    dlist->deleteList();
}


void WAN::transferDatagram(){
    extern node* network[MAX_MACHINES];
    msg_list* temporyList = new msg_list;
    datagram* myData = (this->dlist)->returnFront();
    
    while (myData != NULL) { // problem:
        IPAddress myDest = myData->destinationAddress();
        int myFirstOctad = myDest.firstOctad();
        
        bool canWeSendToServer = false;
        int serverNo = 0;
        
        if (number_of_servers > 0) {
            for (int i = 0; i < number_of_servers; i++) {
                if ((myFirstOctad - ((this->server_list)[i]).firstOctad()) == 0) {
                    canWeSendToServer = true;
                    serverNo = i;
                    break;
                }
            }
        }
        
        if (canWeSendToServer) {  // transfer data to SERVER at this TIME CLICK, Not next time!
            for (int j = 0; j < MAX_MACHINES; j++) {
                if (network[j] != NULL) {
                    if (network[j]->amIThisComputer((this->server_list)[serverNo])) {
                        network[j]->receiveDatagram(myData);
                        break;
                    }
                }
            }

        } else {    // transfer data to WAN at this TIME CLICK, Not next time!
            if (this->number_of_wans == 0) {
                temporyList->append(myData);
            } else {
                int a[this->number_of_wans];
                for (int i = 0; i < this->number_of_wans; i++) {
                    a[i] = abs(myFirstOctad - ((this->WAN_list)[i]).firstOctad());
                }
                
                int min = a[0];
                int number = 0;
                for (int i = 0; i < this->number_of_wans; i++) {
                    if (a[i] < min || a[i] == min) {
                        min = a[i];
                        number = i;
                    }
                }
                
                IPAddress nextWan = WAN_list[number];
                
                for (int j = 0; j < MAX_MACHINES; j++) {
                    if (network[j] != NULL) {
                        if (network[j]->amIThisComputer(nextWan)) {
                            network[j]->receiveDatagram(myData);
                            break;
                        }
                    }
                }
            }
        }
        //delete myData;
        myData = (this->dlist)->returnFront(); // problem: returnFront is called twice
    }
    delete dlist;
    this->dlist = temporyList;
}
/**************new*************/





















