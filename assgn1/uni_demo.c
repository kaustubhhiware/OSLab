#include <unistd.h>

int main(int argc,char* argv[])
{
    int ret;
    ret = execl ("/bin/ls", "ls", "-1", (char *)0);
}
