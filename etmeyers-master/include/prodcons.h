/* Global variable for producer consumer */
extern int n; /* this is just declaration */
extern sid32 w_mutex;
extern sid32 r_mutex;
extern sid32 p_mutex;
/* Function Prototype */
void consumer(int count);
void producer(int count);
