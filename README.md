# nodelist
A Nodelist container is a doubly-linked list where the nodes of the list can be created and stored to avoid memory allocation on insertion. 

## About

C++ 11 Standard

Like with a doubly-linked list, the nodelist container support constant time insertion (attach) and removal (detach). 
It is the user's responsibility to allocate each node of the list. 
When nodes are destructed, they are automatically detached (removed) from the list. 
Nodes can only attach to one list at a time, and attaching a node to one list will automatically detach the node from its current list. 
The list iterator is bidirectional; deleting a node at the iterator's position will invalidate the iterator. 


