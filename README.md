# data-structures

## description
List 
insertion, deletion, search takes Log(N) time

## inrernal structure

binary tree.  
each child knows how bigger it is, then its parent
root has its real index

### insertion


```mermaid
graph TD;
	a((0));

```

`insert(1)`

```mermaid
graph TD;
	a((0))-.->b((1));

```


`insert(2)`

```mermaid
graph TD;
	a((0))-->b((1));
	b-.->c((2))

```

rotate


```mermaid
graph TD;
	a-->c((0))
	a((1))-->b((2))

```
lets create more complicated example

```mermaid
graph TD;
	a((a))-->b((b))
	a-->c((c))
	b-->d((d))
	b-->e((e))
	c-->f((f))
	c-->g((g))
	g-->h((h))
	g-->i((i))

```



```mermaid
graph TD;
	a((3))-->b((1))
	a-->c((c))
	b-->d((0))
	b-->e((2))
	c-->f((f))
	c-->g((g))
	g-->h((h))
	g-->i((i))

```

