#include <stdio.h>
#include <stdlib.h>

#define PEASANT 0x08
#define WOLF 0x04
#define GOAT 0x02
#define CABBAGE 0x01

// 주어진 상태 state의 이름(마지막 4비트)을 화면에 출력
// 예) state가 7(0111)일 때, "<0111>"을 출력
static void print_statename(FILE *fp, int state);

// 주어진 상태 state에서 농부, 늑대, 염소, 양배추의 상태를 각각 추출하여 p, w, g, c에 저장
// 예) state가 7(0111)일 때, p = 0, w = 1, g = 1, c = 1
static void get_pwgc(int state, int *p, int *w, int *g, int *c);

// 허용되지 않는 상태인지 검사
// 예) 농부없이 늑대와 염소가 같이 있는 경우 / 농부없이 염소와 양배추가 같이 있는 경우
// return value: 1 허용되지 않는 상태인 경우, 0 허용되는 상태인 경우
static int is_dead_end(int state);

// state1 상태에서 state2 상태로의 전이 가능성 점검
// 농부 또는 농부와 다른 하나의 아이템이 강 반대편으로 이동할 수 있는 상태만 허용
// 허용되지 않는 상태(dead-end)로의 전이인지 검사
// return value: 1 전이 가능한 경우, 0 전이 불이가능한 경우
static int is_possible_transition(int state1, int state2);

// 상태 변경: 농부 이동
// return value : 새로운 상태
static int changeP(int state);

// 상태 변경: 농부, 늑대 이동
// return value : 새로운 상태, 상태 변경이 불가능한 경우: -1
static int changePW(int state);

// 상태 변경: 농부, 염소 이동
// return value : 새로운 상태, 상태 변경이 불가능한 경우: -1
static int changePG(int state);

// 상태 변경: 농부, 양배추 이동
// return value : 새로운 상태, 상태 변경이 불가능한 경우: -1
static int changePC(int state);

// 주어진 state가 이미 방문한 상태인지 검사
// return value : 1 visited, 0 not visited
static int is_visited(int visited[], int depth, int state);

// 방문한 경로(상태들)을 차례로 화면에 출력
static void print_path(int visited[], int depth);

// recursive function
static void dfs_main(int initial_state, int goal_state, int depth, int visited[]);

////////////////////////////////////////////////////////////////////////////////
// 상태들의 인접 행렬을 구하여 graph에 저장
// 상태간 전이 가능성 점검
// 허용되지 않는 상태인지 점검
void make_adjacency_matrix(int graph[][16]);

// 인접행렬로 표현된 graph를 화면에 출력
void print_graph(int graph[][16], int num);

// 주어진 그래프(graph)를 .net 파일로 저장
// pgwc.net 참조
void save_graph(char *filename, int graph[][16], int num);

////////////////////////////////////////////////////////////////////////////////
// 깊이 우선 탐색 (초기 상태 -> 목적 상태)
void depth_first_search(int initial_state, int goal_state)
{
	int depth = 0;
	int visited[16] = {
		0,
	}; // 방문한 정점을 저장

	dfs_main(initial_state, goal_state, depth, visited);
}

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
	int graph[16][16] = {
		0,
	};

	// 인접 행렬 만들기
	make_adjacency_matrix(graph);

	// 인접 행렬 출력 (only for debugging)
	print_graph(graph, 16);

	// .net 파일 만들기
	// save_graph( "pwgc.net", graph, 16);

	// 깊이 우선 탐색
	depth_first_search(0, 15); // initial state, goal state

	return 0;
}

// 주어진 상태 state의 이름(마지막 4비트)을 화면에 출력
// 예) state가 7(0111)일 때, "<0111>"을 출력
// file=printf, 출력을 파일에 작성
// int -> char: + '0'
static void print_statename(FILE *fp, int state)
{
	int p, w, g, c;
	get_pwgc(state, &p, &w, &g, &c);
	char state_bit[4];
	state_bit[0] = p + '0';
	state_bit[1] = w + '0';
	state_bit[2] = g + '0';
	state_bit[3] = c + '0';
	fprintf(fp, "<%s>", state_bit);
}

// 주어진 상태 state에서 농부, 늑대, 염소, 양배추의 상태를 각각 추출하여 p, w, g, c에 저장
// 예) state가 7(0111)일 때, p = 0, w = 1, g = 1, c = 1
// 1000: 8, 0100: 4, 0010: 2, 0001: 1
// 연산자 우선순위 >>, &
static void get_pwgc(int state, int *p, int *w, int *g, int *c)
{
	*p = (state & PEASANT) >> 3;
	*w = (state & WOLF) >> 2;
	*g = (state & GOAT) >> 1;
	*c = state & CABBAGE;
}

// 허용되지 않는 상태인지 검사
// 예) 농부없이 늑대와 염소가 같이 있는 경우 / 농부없이 염소와 양배추가 같이 있는 경우
// 011_ 100_ / 0_11 1_00 /
// return value: 1 허용되지 않는 상태인 경우, 0 허용되는 상태인 경우
static int is_dead_end(int state)
{
	int p, w, g, c;
	get_pwgc(state, &p, &w, &g, &c);
	if (w == g && p != w)
	{
		return 1;
	}
	else if (g == c && p != g)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

// state1 상태에서 state2 상태로의 전이 가능성 점검
// 농부 또는 농부와 다른 하나의 아이템이 강 반대편으로 이동할 수 있는 상태만 허용
// 허용되지 않는 상태(dead-end)로의 전이인지 검사
// return value: 1 전이 가능한 경우, 0 전이 불이가능한 경우
static int is_possible_transition(int state1, int state2)
{
	int p1, w1, g1, c1;
	int p2, w2, g2, c2;

	get_pwgc(state1, &p1, &w1, &g1, &c1);
	get_pwgc(state2, &p2, &w2, &g2, &c2);

	// PEASANT NOT MOVE
	if (p1 == p2)
	{
		return 0;
	}

	// MOVE WITHOUT PEASANT
	if (p1 != w1 && w1 != w2)
	{
		return 0;
	}
	else if (p1 != g1 && g1 != g2)
	{
		return 0;
	}
	else if (p1 != c1 && c1 != c2)
	{
		return 0;
	}

	// MOVE MORE THAN 2
	if (w1 != w2 && g1 != g2)
	{
		return 0;
	}
	else if (w1 != w2 && c1 != c2)
	{
		return 0;
	}
	else if (c1 != c2 && g1 != g2)
	{
		return 0;
	}

	// NEWSTAE IS DEAD-END
	if (is_dead_end(state2))
	{
		return 0;
	}

	return 1;
}

// 상태 변경: 농부 이동
// & 1: 값 유지  &0: 값 0으로
// state & 8: P가 1이냐?
// state & 7: P 1 -> 0
// state | 8: P 0 -> 1
// return value : 새로운 상태
static int changeP(int state)
{
	return state & PEASANT ? state & ~PEASANT : state | PEASANT;
}

// 상태 변경: 농부, 늑대 이동
// return value : 새로운 상태, 상태 변경이 불가능한 경우: -1
static int changePW(int state)
{
	int newState;
	newState = state & PEASANT ? state & ~PEASANT : state | PEASANT;
	newState = newState & WOLF ? newState & ~WOLF : newState | WOLF;
	if (is_possible_transition(state, newState))
	{
		return newState;
	}
	else
	{
		return -1;
	}
}

// 상태 변경: 농부, 염소 이동
// return value : 새로운 상태, 상태 변경이 불가능한 경우: -1
static int changePG(int state)
{
	int newState;
	newState = state & PEASANT ? state & ~PEASANT : state | PEASANT;
	newState = newState & GOAT ? newState & ~GOAT : newState | GOAT;
	if (is_possible_transition(state, newState))
	{
		return newState;
	}
	else
	{
		return -1;
	}
}

// 상태 변경: 농부, 양배추 이동
// return value : 새로운 상태, 상태 변경이 불가능한 경우: -1
static int changePC(int state)
{
	int newState;
	newState = state & PEASANT ? state & ~PEASANT : state | PEASANT;
	newState = newState & CABBAGE ? newState & ~CABBAGE : newState | CABBAGE;
	if (is_possible_transition(state, newState))
	{
		return newState;
	}
	else
	{
		return -1;
	}
}

// 주어진 state가 이미 방문한 상태인지 검사
// return value : 1 visited, 0 not visited
static int is_visited(int visited[], int depth, int state)
{
	for (int i = 0; i < depth; i++)
	{
		if (state == visited[i])
		{
			return 1;
		}
	}
	return 0;
}

// 방문한 경로(상태들)을 차례로 화면에 출력
static void print_path(int visited[], int depth)
{
	for (int i = 0; i < depth + 1; i++)
	{
		print_statename(stdout, visited[i]);
		printf("\n");
	}
}

// recursive function
// print current state
// check dead-end or visited
// is current state == 1111 -> Goal-state found -> print_path
// stdout: 표준출력, stdin: 표준입력
// p이동, pw이동, pg이동, pc이동
// Depth First Search
static void dfs_main(int initial_state, int goal_state, int depth, int visited[])
{
	printf("current state is ");
	print_statename(stdout, initial_state);
	printf(" (depth %d)\n", depth);

	visited[depth] = initial_state;

	if (initial_state == goal_state)
	{
		visited[depth] = initial_state;
		printf("Goal-state found!\n");
		print_path(visited, depth);
		return;
	}

	int nextState;
	nextState = changeP(initial_state);
	if (is_dead_end(nextState))
	{
		printf("\tnext state ");
		print_statename(stdout, nextState);
		printf(" is dead-end\n");
	}
	else if (is_visited(visited, depth, nextState))
	{
		printf("\tnext state ");
		print_statename(stdout, nextState);
		printf(" has been visited\n");
	}
	else
	{
		dfs_main(nextState, goal_state, depth + 1, visited);
		printf("back to ");
		print_statename(stdout, initial_state);
		printf(" (%d)\n", depth);
	}

	nextState = changePW(initial_state);
	if (nextState == -1)
	{
		int newState;
		newState = initial_state & PEASANT ? initial_state & ~PEASANT : initial_state | PEASANT;
		newState = newState & WOLF ? newState & ~WOLF : newState | WOLF;
		if (is_dead_end(newState))
		{
			printf("\tnext state ");
			print_statename(stdout, newState);
			printf(" is dead-end\n");
		}
	}
	else
	{
		if (is_visited(visited, depth, nextState))
		{
			printf("\tnext state ");
			print_statename(stdout, nextState);
			printf(" has been visited\n");
		}
		else
		{
			dfs_main(nextState, goal_state, depth + 1, visited);
			printf("back to ");
			print_statename(stdout, initial_state);
			printf(" (%d)\n", depth);
		}
	}

	nextState = changePG(initial_state);
	if (nextState == -1)
	{
		int newState;
		newState = initial_state & PEASANT ? initial_state & ~PEASANT : initial_state | PEASANT;
		newState = newState & GOAT ? newState & ~GOAT : newState | GOAT;
		if (is_dead_end(newState))
		{
			printf("\tnext state ");
			print_statename(stdout, newState);
			printf(" is dead-end\n");
		}
	}
	else
	{
		if (is_visited(visited, depth, nextState))
		{
			printf("\tnext state ");
			print_statename(stdout, nextState);
			printf(" has been visited\n");
		}
		else
		{
			dfs_main(nextState, goal_state, depth + 1, visited);
			printf("back to ");
			print_statename(stdout, initial_state);
			printf(" (%d)\n", depth);
		}
	}

	nextState = changePC(initial_state);
	if (nextState == -1)
	{
		int newState;
		newState = initial_state & PEASANT ? initial_state & ~PEASANT : initial_state | PEASANT;
		newState = newState & CABBAGE ? newState & ~CABBAGE : newState | CABBAGE;
		if (is_dead_end(newState))
		{
			printf("\tnext state ");
			print_statename(stdout, newState);
			printf(" is dead-end\n");
		}
	}
	else
	{
		if (is_visited(visited, depth, nextState))
		{
			printf("\tnext state ");
			print_statename(stdout, nextState);
			printf(" has been visited\n");
		}
		else
		{
			dfs_main(nextState, goal_state, depth + 1, visited);
			printf("back to ");
			print_statename(stdout, initial_state);
			printf(" (%d)\n", depth);
		}
	}

	return;
}

////////////////////////////////////////////////////////////////////////////////
// 상태들의 인접 행렬을 구하여 graph에 저장
// 상태간 전이 가능성 점검
// 허용되지 않는 상태인지 점검
// i -> n1, n2, n3, n4
void make_adjacency_matrix(int graph[][16])
{
	int n1, n2, n3, n4;
	for (int i = 0; i < 16; i++)
	{
		if (is_dead_end(i))
		{
			continue;
		}
		n1 = changeP(i);
		n2 = changePW(i);
		n3 = changePG(i);
		n4 = changePC(i);

		if (!is_dead_end(n1))
		{
			graph[i][n1] = 1;
			graph[n1][i] = 1;
		}
		if (n2 != -1)
		{
			graph[i][n2] = 1;
			graph[n2][i] = 1;
		}
		if (n3 != -1)
		{
			graph[i][n3] = 1;
			graph[n3][i] = 1;
		}
		if (n4 != -1)
		{
			graph[i][n4] = 1;
			graph[n4][i] = 1;
		}
	}
}

// 인접행렬로 표현된 graph를 화면에 출력
void print_graph(int graph[][16], int num)
{
	for (int i = 0; i < num; i++)
	{
		for (int j = 0; j < num; j++)
		{
			printf("%d ", graph[i][j]);
		}
		printf("\n");
	}
}

// 주어진 그래프(graph)를 .net 파일로 저장
// pgwc.net 참조
void save_graph(char *filename, int graph[][16], int num);