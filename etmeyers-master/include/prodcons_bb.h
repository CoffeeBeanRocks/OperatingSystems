/* Global variables for producer_bb*/
extern sid32 write_mutex;
extern sid32 read_mutex;
extern int arr[5];
extern int head;
extern int tail;
/* Function Prototype */
void producer_bb(int count);
void consumer_bb(int count);
