#include "./find.h"

#define IDX_HEAD(q) (q).idxHead
#define IDX_TAIL(q) (q).idxTail
#define     HEAD(q) (q).buffer[(q).idxHead]
#define     TAIL(q) (q).buffer[(q).idxTail]

void CreateQueue(Queue *q){
	IDX_HEAD(*q) = IDX_UNDEF;
	IDX_TAIL(*q) = IDX_UNDEF;
}

bool isEmpty(Queue q){
	return ((IDX_HEAD(q) == IDX_UNDEF) && (IDX_TAIL(q) == IDX_UNDEF));
}

bool isFull(Queue q){
	if (IDX_HEAD(q) == 0) return (IDX_TAIL(q) == (CAPACITY-1));
	else return (IDX_TAIL(q) == (IDX_HEAD(q)-1));
}

void enqueue(Queue *q, ElType val){
	if (isEmpty(*q)){
		IDX_HEAD(*q) = 0;
		IDX_TAIL(*q) = 0;
	}
	else{
		IDX_TAIL(*q) = (IDX_TAIL(*q) + 1) % CAPACITY;
	}
	TAIL(*q) = val;
}

void dequeue(Queue *q, ElType *val){
	*val = HEAD(*q);
	if (IDX_HEAD(*q) == IDX_TAIL(*q)){
		IDX_HEAD(*q) = IDX_UNDEF;
		IDX_TAIL(*q) = IDX_UNDEF;
	}
	else{
		IDX_HEAD(*q) = (IDX_HEAD(*q) + 1) % CAPACITY;
	}
}

uint32_t convertToClusterNumber(int i){
    return (uint32_t) ((state.curr_dir.table[i].cluster_high >> 16) | state.curr_dir.table[i].cluster_low);
}

void print_find(int i){
    print_curr_dir(state);
    put_char('/');

    put_chars(state.curr_dir.table[i].name, BIOS_LIGHT_GREEN);
    if (strlen(state.curr_dir.table[i].ext) != 0 && state.curr_dir.table[i].attribute != ATTR_SUBDIRECTORY){
        put_char('.');
        put_chars(state.curr_dir.table[i].ext, BIOS_LIGHT_GREEN);
    }
    if (state.curr_dir.table[i].attribute == ATTR_SUBDIRECTORY) put_char('/');
    put_char('\n');
}

void process_find(Queue *q, char* arg){
    ElType cn;
    dequeue(q, &cn);

    state.current_directory = cn;
    updateDirectoryTable(cn);

    char name[8];
    char ext[3];

    for (int i = 2; i < TOTAL_DIRECTORY_ENTRY; i++){
        if (state.curr_dir.table[i].user_attribute == UATTR_NOT_EMPTY){
            if (state.curr_dir.table[i].attribute == ATTR_SUBDIRECTORY){
                cn = convertToClusterNumber(i);
                enqueue(q, cn);
            }
            extract_filename(arg, name);
            extract_file_extension(arg, ext);
            if (memcmp(state.curr_dir.table[i].name, name, 8) == 0 &&
                (strlen(state.curr_dir.table[i].ext) == 0 || memcmp(state.curr_dir.table[i].ext, ext, 3) == 0)){
                print_find(i);
            }
        }
    }
}

void find(char* arg){
    uint32_t curr_cluster_number = state.current_directory;

    Queue bfs;
    CreateQueue(&bfs);

    //Initialize Queue with root
    ElType root = ROOT_CLUSTER_NUMBER;
    enqueue(&bfs, root);

    while (!isEmpty(bfs) && !isFull(bfs)){
        process_find(&bfs, arg);
    }

    if (isFull(bfs)) put_chars("find selesai karena depth terlalu dalam\n", BIOS_LIGHT_GREEN);

    //End dirTable with curr_cluster_number before find()
    state.current_directory = curr_cluster_number;
    updateDirectoryTable(curr_cluster_number);
}
