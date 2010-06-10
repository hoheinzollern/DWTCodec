#ifndef HUFFMAN_H_
#define HUFFMAN_H_

#include "bitfile.h"
#include <map.h>

class Huffman {
private:
	class Node {
	public:
		Node *root;
		unsigned char inner;
		unsigned int occ;
		Node(unsigned char t, unsigned int occ);
		~Node();
	};
	struct less
	{
		bool operator()(Node *a, Node *b) const
		{
			return (*a).occ > (*b).occ;
		}
	};

	class InnerNode : public Node {
	public:
		InnerNode(Node *left, Node *right);
		~InnerNode();
		Node *children[2];
	};

	class Leaf : public Node {
	public:
		Leaf(unsigned int occ, unsigned short info);
		~Leaf();
		unsigned short info;
	};

	bit_file_t *bitFile;
	Node *tree;
	map<unsigned short, Leaf *>leafs;

	void writeNode(Node *n);
	Node *readNode(bool mapLeafs);
	void writeSymRec(Node *n);
	unsigned short readSymRec(Node *n);
public:
	Huffman();
	~Huffman();

	void setFile(bit_file_t *file);

	void buildTree(unsigned int *occurrences, int n);

	void writeTree();
	void readTree(bool mapLeafs);
	void readTree();

	void writeSymbol(unsigned short sym);
	unsigned short readSymbol();
};

#endif
