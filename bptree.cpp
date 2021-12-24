#include <stdio.h>
#include <stdlib.h>
#include <stack>

using namespace std;

// ERROR DEFINE
#define WRONG_INPUT_ERR	1
#define WRITE_FAIL_ERR	2
#define READ_FAIL_ERR	3

//전역변수 초기화
int g_blockSize = 0;
int g_rootBlockId = 1;
int g_depth = 0;
int g_numOfEntry = 0;
int g_emptySize = 0;

FILE* g_bptreeFile = nullptr;
FILE* g_inputFile = nullptr;
FILE* g_outputFile = nullptr;

stack<int>* g_blockIdStack = nullptr;

class Node {
public:
	int* key;
	int* nonKey;	//value 또는 BID 배열 담을 포인터
	int next;		//nextNode 또는 next level Node 담을 포인터
	int depth;
	Node();
	void WriteNode(int blockId);
	void WriteInsertNode(int blockId, int targetKey, int targetNonkey);
	void ReadNode(int blockId);
	void ReadNextLevel(int target);
	void ClearNode();
	bool IsFull();
	int Split(int targetKey, int targetNonKey);
};

void WriteIntToFile(FILE* pFile, int address, int n, int seek_state);
int ReadFileToInt(FILE* pFile, int address, int seek_state);
void SetFileHeader();
void UpdateFileHeader();
int GetInput();
int CalAddress(int blockId);
int CalBlockId(int address);
void SearchNode(Node* curNode, int target);
void Search();
void RangeSearch();
void Print();
int PrintNode(Node* curNode);
void Insert();

int main(int argc, char** argv)
{
	char command = argv[1][0];

	switch (command)
	{
	case 'c':
		// create index file
		g_blockSize = atoi(argv[3]);
		fopen_s(&g_bptreeFile, argv[2], "wb");
		if (g_bptreeFile != nullptr)
		{
			UpdateFileHeader();
			fclose(g_bptreeFile);
		}
		break;
	case 'i':
		// insert records from [records data file], ex) records.txt
		fopen_s(&g_bptreeFile, argv[2], "r+b");
		fopen_s(&g_inputFile, argv[3], "r");
		if (g_bptreeFile != nullptr && g_inputFile != nullptr)
		{
			SetFileHeader();
			Insert();
			fclose(g_bptreeFile);
			fclose(g_inputFile);
		}
		break;
	case 's':
		// search keys in [input file] and print results to [output file]
		fopen_s(&g_bptreeFile, argv[2], "rb");
		fopen_s(&g_inputFile, argv[3], "r");
		fopen_s(&g_outputFile, argv[4], "w");
		if (g_bptreeFile != nullptr && g_inputFile != nullptr && g_outputFile != nullptr)
		{
			SetFileHeader();
			Search();
			fclose(g_bptreeFile);
			fclose(g_inputFile);
			fclose(g_outputFile);
		}
		break;
	case 'r':
		// search keys in [input file] and print results to [output file]
		fopen_s(&g_bptreeFile, argv[2], "rb");
		fopen_s(&g_inputFile, argv[3], "r");
		fopen_s(&g_outputFile, argv[4], "w");
		if (g_bptreeFile != nullptr && g_inputFile != nullptr && g_outputFile != nullptr)
		{
			SetFileHeader();
			RangeSearch();
			fclose(g_bptreeFile);
			fclose(g_inputFile);
			fclose(g_outputFile);
		}
		break;
	case 'p':
		// print B+-Tree structure to [output file]
		fopen_s(&g_bptreeFile, argv[2], "rb");
		fopen_s(&g_outputFile, argv[3], "wt");
		if (g_bptreeFile != nullptr && g_outputFile != nullptr)
		{
			SetFileHeader();
			Print();
			fclose(g_bptreeFile);
			fclose(g_outputFile);
		}
		break;
	}
	return (0);
}

void WriteIntToFile(FILE* pFile, int address, int n, int seek_state)
{
	fseek(pFile, address, seek_state);
	for (int i = 0; i < 4; i++)
	{
		if (fputc(n, pFile) == -1)
		{
			fputs("WRITE FAIL", stderr);
			exit(WRITE_FAIL_ERR);
		}
		if (n > 0)
			n /= 256;
	}
}

int ReadFileToInt(FILE* pFile, int address, int seek_state)
{
	int value = 0;
	int temp;
	fseek(pFile, address, seek_state);
	for (int i = 0; i < 4; i++)
	{
		temp = fgetc(pFile);
		if (temp == -1)
		{
			fputs("READ FAIL", stderr);
			exit(READ_FAIL_ERR);
		}
		for (int j = 0; j < i; j++)
			temp *= 256;
		value += temp;
	}
	return value;
}

int GetInput()
{
	int value = 0;
	int temp = 0;
	temp = fgetc(g_inputFile);
	while (temp != -1)
	{
		if (temp == '\n' || temp == ',')
			return value;
		temp -= '0';
		value *= 10;
		value += temp;
		temp = fgetc(g_inputFile);
	}
	return -1;
}

void SetFileHeader()
{
	g_blockSize = ReadFileToInt(g_bptreeFile, 0, SEEK_SET);
	g_rootBlockId = ReadFileToInt(g_bptreeFile, 0, SEEK_CUR);
	g_depth = ReadFileToInt(g_bptreeFile, 0, SEEK_CUR);
	g_numOfEntry = (g_blockSize - 4) / 8;
	g_emptySize = (g_blockSize - 4) % 8;
	g_blockIdStack = new stack<int>;
}

void UpdateFileHeader()
{
	WriteIntToFile(g_bptreeFile, 0, g_blockSize, SEEK_SET);
	WriteIntToFile(g_bptreeFile, 0, g_rootBlockId, SEEK_CUR);
	WriteIntToFile(g_bptreeFile, 0, g_depth, SEEK_CUR);
}

int CalAddress(int blockId)
{
	return (12 + (blockId - 1) * g_blockSize);
}

int CalBlockId(int address)
{
	if (address == 12)
		return 1;
	else
		return ((address - 12) / g_blockSize + 1);
}

void Search()
{
	Node* curNode = new Node();
	int targetKey = GetInput();
	while (targetKey != -1)
	{
		SearchNode(curNode, targetKey);
		for (int i = 0; i < g_numOfEntry; i++)
		{
			if (targetKey == curNode->key[i])
			{
				fprintf(g_outputFile, "%d,%d\n", curNode->key[i], curNode->nonKey[i]);
				break;
			}
			else if (targetKey < curNode->key[i] || curNode->key[i] == 0)
			{
				fprintf(g_outputFile, "key %d is not in here!!\n", targetKey);
				break;
			}
		}
		targetKey = GetInput();
		while (!g_blockIdStack->empty())
			g_blockIdStack->pop();
	}
}

void RangeSearch()
{
	Node* curNode = new Node();
	int targetIndex = 0;
	int targetKey = GetInput();
	int targetDst = GetInput();
	while (targetKey != -1)
	{
		SearchNode(curNode, targetKey);
		while (targetKey > curNode->key[targetIndex] && targetIndex < g_numOfEntry && curNode->key[targetIndex] != 0)
			targetIndex++;
		if (targetIndex >= g_numOfEntry || curNode->key[targetIndex] == 0)
		{
			if (curNode->next != 0)
				curNode->ReadNode(curNode->next);
			targetIndex = 0;
		}
		while (targetDst >= curNode->key[targetIndex])
		{
			fprintf(g_outputFile, "%d,%d\t", curNode->key[targetIndex], curNode->nonKey[targetIndex]);
			targetIndex++;
			if (targetIndex >= g_numOfEntry || curNode->key[targetIndex] == 0)
			{
				targetIndex = 0;
				if (curNode->next != 0)
					curNode->ReadNode(curNode->next);
				else
					break;
			}
		}
		fseek(g_outputFile, -1, SEEK_CUR);
		fprintf(g_outputFile, "\n");
		targetKey = GetInput();
		targetDst = GetInput();
		targetIndex = 0;
		while (!g_blockIdStack->empty())
			g_blockIdStack->pop();
	}
}

void SearchNode(Node* curNode, int target)
{
	curNode->ClearNode();
	fseek(g_bptreeFile, 0, SEEK_END);
	int fileSize = ftell(g_bptreeFile);
	g_blockIdStack->push(g_rootBlockId);
	if (fileSize <= 12)
		return;
	curNode->ReadNode(g_rootBlockId);
	while (curNode->depth != g_depth)
		curNode->ReadNextLevel(target);
}

void Print()
{
	Node* curNode = new Node();
	curNode->ClearNode();
	curNode->ReadNode(g_rootBlockId);
	fprintf(g_outputFile, "<0>\n");
	int i = PrintNode(curNode);
	fseek(g_outputFile, -1, SEEK_CUR);
	fprintf(g_outputFile, "\n");
	if (g_depth > 0)
	{
		fprintf(g_outputFile, "<1>\n");
		Node* nextLevelNode = new Node();
		nextLevelNode->ClearNode();
		nextLevelNode->depth = 1;
		nextLevelNode->ReadNode(curNode->next);
		PrintNode(nextLevelNode);
		for (int j = 0; j < i; j++)
		{
			nextLevelNode->ClearNode();
			nextLevelNode->depth = 1;
			nextLevelNode->ReadNode(curNode->nonKey[j]);
			PrintNode(nextLevelNode);
		}
		fseek(g_outputFile, -1, SEEK_CUR);
		fprintf(g_outputFile, "\n");
	}
}

int PrintNode(Node* curNode)
{
	int i;
	for (i = 0; i < g_numOfEntry; i++)
	{
		if (curNode->key[i] == 0)
			break;
		fprintf(g_outputFile, "%d,", curNode->key[i]);
	}
	return i;
}

void Insert()
{
	Node* curNode = new Node();
	int targetKey = GetInput();
	int targetNonKey = GetInput();
	while (targetKey != -1)
	{
		bool flag = true;
		SearchNode(curNode, targetKey);
		while (flag)
		{
			if (curNode->IsFull())
			{
				targetKey = curNode->Split(targetKey, targetNonKey);
				targetNonKey = g_blockIdStack->top();
				g_blockIdStack->pop();
				if (curNode->depth == 0)
				{
					curNode->ClearNode();
					curNode->next = g_blockIdStack->top();
					g_blockIdStack->pop();
					fseek(g_bptreeFile, 0, SEEK_END);
					g_rootBlockId = CalBlockId(ftell(g_bptreeFile));
					g_depth++;
					g_blockIdStack->push(g_rootBlockId);
					UpdateFileHeader();
				}
				else
				{
					g_blockIdStack->pop();
					curNode->depth--;
					curNode->ReadNode(g_blockIdStack->top());
				}
			}
			else
			{
				curNode->WriteInsertNode(g_blockIdStack->top(), targetKey, targetNonKey);
				flag = false;
			}
		}
		while (!g_blockIdStack->empty())
			g_blockIdStack->pop();
		targetKey = GetInput();
		targetNonKey = GetInput();
	}
}

Node::Node()
{
	key = (int*)malloc(g_numOfEntry * sizeof(int));
	nonKey = (int*)malloc(g_numOfEntry * sizeof(int));
	next = 0;
	depth = 0;
}

void Node::WriteNode(int blockId)
{
	fseek(g_bptreeFile, CalAddress(blockId), SEEK_SET);
	if (depth < g_depth)
		WriteIntToFile(g_bptreeFile, 0, next, SEEK_CUR);
	for (int i = 0; i < g_numOfEntry; i++)
	{
		WriteIntToFile(g_bptreeFile, 0, key[i], SEEK_CUR);
		WriteIntToFile(g_bptreeFile, 0, nonKey[i], SEEK_CUR);
	}
	if (depth == g_depth)
		WriteIntToFile(g_bptreeFile, 0, next, SEEK_CUR);
	int count = g_emptySize;
	while (count--)
		fputc(0, g_bptreeFile);
}

void Node::WriteInsertNode(int blockId, int targetKey, int targetNonKey)
{
	int cur = g_numOfEntry - 1;
	while (cur > 0)
	{
		key[cur] = key[cur - 1];
		nonKey[cur] = nonKey[cur - 1];
		if (targetKey >= key[cur] && key[cur] != 0)
			break;
		cur--;
	}
	key[cur] = targetKey;
	nonKey[cur] = targetNonKey;
	WriteNode(blockId);
}

void Node::ReadNode(int blockId)
{
	fseek(g_bptreeFile, CalAddress(blockId), SEEK_SET);
	if (depth < g_depth)
		next = ReadFileToInt(g_bptreeFile, 0, SEEK_CUR);
	for (int i = 0; i < g_numOfEntry; i++)
	{
		this->key[i] = ReadFileToInt(g_bptreeFile, 0, SEEK_CUR);
		this->nonKey[i] = ReadFileToInt(g_bptreeFile, 0, SEEK_CUR);
	}
	if (depth == g_depth)
		next = ReadFileToInt(g_bptreeFile, 0, SEEK_CUR);
}

void Node::ReadNextLevel(int target)
{
	depth++;
	int cur = 0;
	while (cur < g_numOfEntry)
	{
		if (target < key[cur] || key[cur] == 0)
			break;
		cur++;
	}
	if (cur == 0)
	{
		g_blockIdStack->push(next);
		ReadNode(next);
	}
	else
	{
		g_blockIdStack->push(nonKey[cur - 1]);
		ReadNode(nonKey[cur - 1]);
	}
}

void Node::ClearNode()
{
	for (int i = 0; i < g_numOfEntry; i++)
	{
		key[i] = 0;
		nonKey[i] = 0;
	}
	next = 0;
	depth = 0;
}

bool Node::IsFull()
{
	if (key[g_numOfEntry - 1] == 0)
		return false;
	return true;
}

int Node::Split(int targetKey, int targetNonKey)
{
	fseek(g_bptreeFile, 0, SEEK_END);
	int targetBlockId = CalBlockId(ftell(g_bptreeFile));
	Node* newNode = new Node();
	newNode->ClearNode();
	newNode->depth = this->depth;
	int curIndex = g_numOfEntry;
	int rightIndex = (g_numOfEntry + 1) / 2;
	int leftIndex = g_numOfEntry + 1 - rightIndex;
	int rtValue = 0;
	if (this->depth != g_depth)
		leftIndex > rightIndex ? leftIndex-- : rightIndex--;
	bool flag = true;
	while (rightIndex--)
	{
		curIndex--;
		if (flag && targetKey > this->key[curIndex])
		{
			flag = false;
			newNode->key[rightIndex] = targetKey;
			newNode->nonKey[rightIndex] = targetNonKey;
			rightIndex--;
		}
		if (rightIndex < 0)
		{
			curIndex++;
			break;
		}
		newNode->key[rightIndex] = this->key[curIndex];
		newNode->nonKey[rightIndex] = this->nonKey[curIndex];
	}
	if (this->depth == g_depth)
	{
		newNode->next = this->next;
		this->next = targetBlockId;
		rtValue = newNode->key[0];
	}
	else
	{
		curIndex--;
		if (flag && targetKey > this->key[curIndex])
		{
			flag = false;
			newNode->next = targetNonKey;
			rtValue = targetKey;
			curIndex++;
		}
		else
		{
			newNode->next = this->nonKey[curIndex];
			rtValue = this->key[curIndex];
		}
	}
	int temp = g_numOfEntry - 1;
	while (temp >= leftIndex)
	{
		this->key[temp] = 0;
		this->nonKey[temp] = 0;
		temp--;
	}
	if (flag)
	{
		this->key[temp] = 0;
		this->nonKey[temp] = 0;
		WriteInsertNode(g_blockIdStack->top(), targetKey, targetNonKey);
	}
	else
		WriteNode(g_blockIdStack->top());
	newNode->WriteNode(targetBlockId);
	g_blockIdStack->push(targetBlockId);
	return rtValue;
}
