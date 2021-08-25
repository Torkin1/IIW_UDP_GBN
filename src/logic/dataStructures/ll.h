#ifndef LL_H_INCLUDED
#define LL_H_INCLUDED

// The fundamental element of linked list
typedef struct structNode{

	char *value;                // Value of node.
	struct structNode *next;    // Pointer to next node of list

} Node;

// Creates a node with given value
Node *newNode (char *value);

// prints content of linked list using given format string
void printLL (Node *current, char *format);

// frees memory associated with ll
void destroyLL (Node *current);

// creates a node with given value and adds it to the end of list
void appendLL (Node **currentAddr, char *value);

// Stores value of node at given index in destination and removes such node from list. If destination is NULL the value is not stored
int popLL(Node **headAddr, int index, char **destination);

// Returns leńgth of list
int lenLL(Node *current);

#endif // LL_H_INCLUDED
