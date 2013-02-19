#include "server.h"


int main()
{
    server* s = server::get_instance();
    s->loop();
    server::kill_instance();
    return 0;
}

