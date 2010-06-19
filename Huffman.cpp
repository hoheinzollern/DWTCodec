#include "Huffman.h"

#include <queue>
#include <vector>

Huffman::Node::Node(unsigned char t, unsigned int occ) : root(0), inner(t), occ(occ) {}

Huffman::Node::~Node() {}

Huffman::InnerNode::InnerNode(Huffman::Node *left, Huffman::Node *right) : Huffman::Node(1, left->occ + right->occ) {
	children[0] = left;
	children[1] = right;
	left->root = this;
	right->root = this;
}

Huffman::InnerNode::~InnerNode() {
	delete children[0];
	delete children[1];
}

Huffman::Leaf::Leaf(unsigned int occ, unsigned short info) : Node(0, occ), info(info) {}

Huffman::Leaf::~Leaf() {}

Huffman::Huffman(unsigned int bpp) : bitFile(0), tree(0), bpp(bpp) {}

Huffman::~Huffman()
{
	delete tree;
}

void Huffman::buildTree(unsigned int *occurrences, int n)
{
	priority_queue<Node *, vector<Node *>, gt> q;
	for (int i = 0; i < n; i++) {
		if (occurrences[i] > 0) {
			Leaf *n = new Leaf(occurrences[i], i);
			leafs[i] = n;
			q.push(n);
		}
	}
	while (!q.empty()) {
		Node *n1 = q.top();
		q.pop();
		if (q.empty()) {
			tree = n1;
		} else {
			Node *n2 = q.top();
			q.pop();
			q.push(new InnerNode(n1, n2));
		}
	}
}

void Huffman::setFile(bit_file_t *file)
{
	bitFile = file;
}

void Huffman::writeNode(Huffman::Node *n)
{
	BitFilePutBit(n->inner, bitFile);
	if (n->inner) {
		writeNode(((InnerNode*)n)->children[0]);
		writeNode(((InnerNode*)n)->children[1]);
	} else {
		BitFilePutBitsInt(bitFile, &((Leaf*)n)->info, bpp, sizeof(unsigned short));
	}
}

void Huffman::writeTree()
{
	writeNode(tree);
}

Huffman::Node *Huffman::readNode(bool mapLeafs)
{
	unsigned char t = BitFileGetBit(bitFile);
	if (t == 1) {
		Node *l = readNode(mapLeafs);
		Node *r = readNode(mapLeafs);
		return new InnerNode(l, r);
	} else {
		unsigned short info = 0;
		BitFileGetBitsInt(bitFile, &info, bpp, sizeof(unsigned short));
		Leaf *n = new Leaf(0, info);
		if (mapLeafs) leafs[info] = n;
		return n;
	}
}

void Huffman::readTree(bool mapLeafs)
{
	tree = readNode(mapLeafs);
}

void Huffman::readTree()
{
	tree = readNode(false);
}

void Huffman::writeSymRec(Huffman::Node *n)
{
	if (n == tree) return;
	if (n->root != tree) writeSymRec(n->root);
	BitFilePutBit(((InnerNode*)n->root)->children[1]==n, bitFile);
}

void Huffman::writeSymbol(unsigned short sym)
{
	writeSymRec(leafs[sym]);
}

unsigned short Huffman::readSymRec(Huffman::Node *n)
{
	if (!n->inner)
		return ((Leaf*)n)->info;
	int child = BitFileGetBit(bitFile)?1:0;
	return readSymRec(((InnerNode*)n)->children[child]);
}

unsigned short Huffman::readSymbol()
{
	return readSymRec(tree);
}
