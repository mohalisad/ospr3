#define NUMBER_OF_HIDDEN_CELLS         256   ///< use 256 hidden cells in one hidden layer
#define OUTPUT_THREADS_COUNT           10
#define DEFAULT_HIDDEN_THREADS_COUNT   8

void set_hidden_threads_count(int input);

void init_semaphores();
void close_semaphores();
void get_hidden_node_id(int &i,int &begin,int &end);
int  get_output_node_id();

void run_threads(int count,void *(*start_routine) (void *));
void join_all();

void wait_input_thread();
void wait_hidden_thread(int i);
void wait_output_thread(int i);

void input_thread_finished();
void hidden_thread_finished();
bool output_thread_finished();

void continue_hidden_threads2();
