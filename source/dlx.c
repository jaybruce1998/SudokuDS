#include <stdlib.h>
#include <string.h>

#define LIMIT 2

typedef struct node
{
	struct node *L, *R, *U, *D, *C;
	int n, index;
} Node;

typedef struct cell
{
	int r, c;
} Cell;

Node* header;
Node columns[324];
Node *choices[81], *hi[729];
int solutionsFound, choiceLast=-1, hiLast=-1;
int solution[9][9], givens[9][9], correct[9][9];
int hib[729];
Cell cells[81];
int ind[46656];

void* xmalloc(size_t size)
{
    void* p = malloc(size);
    if(!p)
		abort();
    return p;
}

void appendRow(int id, int* cols)
{
	Node *prev=NULL, *col, *n;
	for(int i=0; i<4; i++)
	{
		col=&columns[cols[i]];
		n=(Node*)xmalloc(sizeof(Node));
		n->L=n->R=n;
		n->C=n->D=col;
		n->n=id;
		n->U=col->U;
		col->U->D=n;
		col->U=n;
		if(prev!=NULL)
		{
			n->L=prev;
			n->R=prev->R;
			prev->R->L=n;
			prev->R=n;
		}
		prev=n;
	}
}

void setupDLX()
{
	int cols[4]={0, 81, 162, 207};
	int i, id=0, r, c, d;
	header=(Node*)xmalloc(sizeof(Node));
	header->L=header->R=header;
	Node *last=header;
	for(i=0; i<324; i++)
	{
		columns[i].index=i;
		columns[i].U=&columns[i];
		columns[i].R=last->R;
		columns[i].L=last;
		columns[i].n=9;
		last->R->L = &columns[i];
		last->R = &columns[i];
		last = &columns[i];
	}
	for(r=0; r<9; r++)
	{
		if(r%3==0)
			cols[3]+=27;
		for(c=0; c<9; c++)
		{
			if(c%3==0)
				cols[3]+=9;
			for(d=0; d<9; d++)
			{
				appendRow(id++, cols);
				cols[1]++;
				cols[2]++;
				cols[3]++;
			}
			cols[0]++;
			cols[1]-=9;
			cols[3]-=9;
		}
		cols[1]+=9;
		cols[2]=162;
		cols[3]-=27;
	}
	i=0;
	for(r=0; r<9; r++)
		for(c=0; c<9; c++)
		{
			cells[i].r=r;
			cells[i++].c=c;
		}
}

void cover(Node* c)
{
	c->R->L = c->L;
	c->L->R = c->R;
	Node* j;
	for (Node* i = c->D; i != c; i = i->D) {
		for (j = i->R; j != i; j = j->R) {
			j->D->U = j->U;
			j->U->D = j->D;
			j->C->n--;
		}
	}
}

void uncover(Node* c)
{
	Node* j;
	for (Node* i = c->U; i != c; i = i->U) {
		for (j = i->L; j != i; j = j->L) {
			j->C->n++;
			j->D->U = j;
			j->U->D = j;
		}
	}
	c->R->L = c;
	c->L->R = c;
}

Node* chooseColumn()
{
	Node* best=header;
	int bestSize = 10;
	for (Node* c = header->R; bestSize>1 && c != header; c = c->R) {
		if (c->n < bestSize) {
			bestSize = c->n;
			best = c;
		}
	}
	return best;
}

void coverFirst(Node* c)
{
	cover(c);
	c=c->D;
	choices[++choiceLast]=c;
	for (Node* j = c->R; j != c; j = j->R)
		cover(j->C);
}

void search()
{
	if (header->R == header)
	{
		if(++solutionsFound==1)
		{
			int id, rc;
			for (int i=choiceLast; i>=0; i--) {
				id = choices[i]->n;
				rc = id / 9;
				solution[rc/9][rc%9] = id%9 + 1;
			}
		}
		return;
	}
	Node *c=chooseColumn(), *j;
	if (c->n == 0)
		return;
	cover(c);
	for (Node* r = c->D; solutionsFound<LIMIT && r!=c; r = r->D) {
		choices[++choiceLast]=r;
		for (j = r->R; j != r; j = j->R)
			cover(j->C);
		search();
		for (j = r->L; j != r; j = j->L)
			uncover(j->C);
		choiceLast--;
	}
	uncover(c);
}

void uncoverGivens()
{
	Node *n, *j;
	while(choiceLast>=0)
	{
		n=choices[choiceLast--];
		for(j=n->L; j!=n; j=j->L)
			uncover(j->C);
		uncover(n->C);
	}
}

int easySearch()
{
	for(Node* c=header->R; c!=header; c=header->R)
	{
		while(c->n>1)
			if(c->index>80)
			{
				uncoverGivens();
				return c->index;
			}
			else
				c=c->R;
		if(c->n==0)
		{
			uncoverGivens();
			return 0;
		}
		coverFirst(c);
	}
	uncoverGivens();
	return 1;
}

int mediumSearch()
{
	Node* c;
	while(header->R!=header)
	{
		c=chooseColumn();
		if(c->n!=1)
		{
			uncoverGivens();
			return c->n;
		}
		coverFirst(c);
	}
	uncoverGivens();
	return 1;
}

void uncoverHard()
{
	Node *r, *j;
	while(hiLast>=0)
	{
		r=hi[hiLast];
		if(hib[hiLast--])
		{
			for(j=r->L; j!=r; j=j->L)
				uncover(j->C);
			uncover(r->C);
		}
		else
		{
			for(j=r->L; j!=r; j=j->L)
			{
				j->D->U=j;
				j->U->D=j;
				j->C->n++;
			}
			r->D->U = r;
			r->U->D = r;
			r->C->n++;
		}
	}
}

int hardSearch(int easy)
{
	Node *c, *r, *j;
	for(int i=0; i<=hiLast; i++)
		hib[i]=1;
	int b=1, s;
	while(header->R!=header)
	{
		c=chooseColumn();
		if(c->n==1)
		{
			cover(c);
			r=c->D;
			hi[++hiLast]=r;
			hib[hiLast]=1;
			for (j = r->R; j != r; j = j->R)
				cover(j->C);
			continue;
		}
		b=0;
		for(c=header->R; c!=header; c=c->R)
		{
			if(c->index>80)
				break;
			cover(c);
			for(r=c->D; r!=c; r=r->D)
			{
				for (j = r->R; j != r; j = j->R)
					cover(j->C);
				s=easy?easySearch():mediumSearch();
				for (j = r->L; j != r; j = j->L)
					uncover(j->C);
				switch(s)
				{
					case 0:
						hi[++hiLast]=r;
						hib[hiLast]=0;
						uncover(c);
						r->D->U = r->U;
						r->U->D = r->D;
						for(j=r->R; j!=r; j=j->R)
						{
							j->D->U=j->U;
							j->U->D=j->D;
							j->C->n--;
						}
						c->n--;
						b=1;
						break;
					case 1:
						uncover(c);
						uncoverHard();
						return 1;
				}
				if(b)
					break;
			}
			if(b)
				break;
			else
				uncover(c);
		}
		if(!b)
			break;
	}
	uncoverHard();
	return b;
}

int coverGiven(int r, int c, int d, Node** stack, int l) {
	if(d--==0)
		return 0;
	Node* cellCol=&columns[r*9+c];
	for(Node* n=cellCol->D; n!=cellCol; n=n->D)
		if(d==n->n%9)
		{
			stack[l+1]=n;
			cover(n->C);
			for (Node* j = n->R; j != n; j = j->R)
				cover(j->C);
			return 1;
		}
	uncoverGivens();
	abort();
}

int coverGivens(Node** stack)
{
	int l=-1;
	for(int i=0; i<9; i++)
		for(int j=0; j<9; j++)
			l+=coverGiven(i, j, givens[i][j], stack, l);
	return l;
}

int setSolution()
{
	choiceLast=coverGivens(choices);
	solutionsFound=0;
	search();
	uncoverGivens();
	return solutionsFound;
}

void fillGridInd()
{
	memcpy(givens, correct, sizeof(correct));
	for(int i=0; i<81; i++)
		ind[i]=i;
}

Cell* removeCell(int l)
{
	int j=rand()%l--;
	Cell* c=&cells[ind[j]];
	memmove(&ind[j], &ind[j+1], (l-j)*sizeof(int));
	return c;
}

void setRandom()
{
	Cell* c;
	int d;
	fillGridInd();
	for(int i=81; i>0; i--)
	{
		c=removeCell(i);
		d=givens[c->r][c->c];
		givens[c->r][c->c]=0;
		if(setSolution()==LIMIT)
			givens[c->r][c->c]=d;
	}
}

void setGivens(int d)
{
	if(d==4)
	{
		setRandom();
		return;
	}
	Cell* c;
	while(1)
	{
		fillGridInd();
		switch(d)
		{
			case 0:
				for(int i=81; i>0; i--)
				{
					c=removeCell(i);
					givens[c->r][c->c]=0;
					choiceLast=coverGivens(choices);
					if(easySearch()!=1)
						givens[c->r][c->c]=correct[c->r][c->c];
				}
				return;
			case 1:
				for(int i=81; i>0; i--)
				{
					c=removeCell(i);
					givens[c->r][c->c]=0;
					choiceLast=coverGivens(choices);
					if(mediumSearch()!=1)
						givens[c->r][c->c]=correct[c->r][c->c];
				}
				choiceLast=coverGivens(choices);
				if(easySearch()!=1)
					return;
			case 2:
				for(int i=81; i>0; i--)
				{
					c=removeCell(i);
					givens[c->r][c->c]=0;
					hiLast=coverGivens(hi);
					if(!hardSearch(1)||setSolution()!=1)
						givens[c->r][c->c]=correct[c->r][c->c];
				}
				choiceLast=coverGivens(choices);
				if(mediumSearch()!=1)
					return;
			default:
				setRandom();
				hiLast=coverGivens(hi);
				if(!hardSearch(1))
					return;
		}
	}
}