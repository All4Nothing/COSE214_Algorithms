#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "node.h"
#include "heap.h"

// 허프만 트리를 순회하며 허프만 코드를 생성하여 codes에 저장
// leaf 노드에서만 코드를 생성
// strdup 함수 사용
// make_huffman_code 함수에서 호출
static void traverse_tree( tNode *root, char *code, int depth, char *codes[]);

// 새로운 노드를 생성
// 좌/우 subtree가 NULL(0)이고 문자(data)와 빈도값(freq)이 저장됨
// make_huffman_tree 함수에서 호출
// return value : 노드의 포인터
static tNode *newNode(unsigned char data, int freq);

////////////////////////////////////////////////////////////////////////////////
// 허프만 코드를 화면에 출력
void print_huffman_code( char *codes[])
{
	int i;
	
	for (i = 0; i < 256; i++)
	{
		printf( "%d\t%s\n", i, codes[i]);
	}
}

////////////////////////////////////////////////////////////////////////////////
// 허프만 트리로부터 허프만 코드를 생성
// traverse_tree 함수 호출
void make_huffman_code( tNode *root, char *codes[])
{
	char code[256];
	
	traverse_tree( root, code, 0, codes);
}

