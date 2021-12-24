- What you have implemented and what you have not

구현한 기능 : 5가지 기능 모두 구현하였습니다.

구현하지 못한 기능 : 없습니다.

- Brief explanation of your implementation (Avoid any fancy designs and make it less than 1 page)

저는 Node 클레스와 9개의 전역변수를 사용하였습니다. 처음에 초기화한 뒤 블락사이즈, 루트 아이디, 트리의 depth, 엔트리의 수, 남는 공간의 사이즈, 읽고 쓸 파일들은 전역 변수로 사용하였습니다.

search시 경로의 블락아이디를 저장하는 블락아이디스택 또한 전역 변수로 사용하였습니다.

Node 클레스는 leaf Node와 non leaf Node에 모두 사용 가능하게 구현하였습니다.

Main 함수에서는 argument에 따라서 file들을 먼저 open 하고 적절한 함수들을 호출하고 실행이 끝나면 close해주었습니다.

파일을 int 단위로 읽고 쓰기 위해 WriteIntToFile 함수와 ReadFileToInt 함수를, Node 단위로 읽고 쓰기 위해 Node 클래스의 함수로 ReadNode 함수와 WriteNode 함수를 만들어 사용하였습니다.

Node에 한개의 엔트리를 insert 하면서 write 할때는 WriteInsertNode 함수를 사용하였습니다. Node 단위로 읽고 쓰는 함수에서는 Node의 depth이 g_depth(트리의 depth)와 같은지 확인하여 leaf 인지 확인하였고 leaf 인 경우에는 next를 마지막에 읽거나 쓰고 non leaf 인 경우에는 next를 먼저 읽거나 쓰기를 하여 leaf와 non-leaf 모두 하나의 함수로 처리할 수 있도록 하였습니다.

creation은 file header를 작성해주었습니다. root bid는 1, depth는 0으로 하였습니다.

print는 2개의 node를 사용하였습니다. 먼저 root Node를 print 한뒤 root Node의 bid에 해당하는 Node를 차례대로 read한뒤 print 해주었습니다.

search 와 range search는 target key의 범위의 leafNode를 찾는 searchNode 함수를 사용하였습니다. search는 searchNode로 찾은 leafNode를 확인하여 target key와 일치하는 key를 찾으면 key,value를 write하고 찾지 못하면 key is not in here를 write 하게 하였습니다. range search는 start key의 범위의 leafNode에서 start key 보다 크거나 같은 key 부터 end key보다 큰 key가 나오기 전까지 write 하도록 하였습니다.

insertion은 searchNode 함수를 사용하여 insert key 범위의 leafNode로 이동한 뒤 node가 full이 아니면 insert를 full이면 split을 하였습니다. node가 full이고 node의 depth가 0인 경우, 즉 full인 root node 인 경우 split 이후 전역변수와 파일에서 depth와 root id를 업데이트하도록 하였습니다.

split에서는 split 이후 상위 노드에 insert 할 newNode의 bid를 bid stack에 push하고 key 를 리턴하여 new Node에 대한 entry를 insertion 했습니다. split은 new Node를 생성한 뒤 curNode의 entry를 뒤에서 부터 하나씩 이동시키다가 이동시키려는 entry의 키가 insert 하려는 key보다 작을때 insert entry를 대신 이동시켜주는 방식으로 구현하였습니다. 또한 split시 curNode의 depth와 g_depth를 비교하여 leaf Node와 non leafNode를 구분하여 처리해주었습니다.

- How to compile and run

visual studio의 명령 인수에서 argument들을 입력해준 뒤 ctrl + f5 버튼으로 실행하였습니다.

- Talk about your experience of doing this project

프로젝트를 진행 할 때 코드를 작성전에 미리 디자인하는 것의 중요성을 느꼈습니다.

처음에는 프로젝트에 대한 디자인 없이 코드를 작성하였는데 계속하여 수정해야 될 부분들이 많아지고 비효율적인 함수와 코드들이 늘어나 결국 디자인을 먼저 한 뒤 코드를 처음부터 다시 작성하였습니다.

또한 bptree에 대하여 어느정도 이해를 하고 있다 생각했는데 직접 구현을 하다 보니 잘못 알고 있었던 부분들을 알 수 있었고 이를 바로잡고 bptree에 대해 좀 더 깊게 이해하게 되었습니다.
