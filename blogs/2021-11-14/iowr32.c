//
// I/O write 32 tool
//
#include <stdio.h>
#include <stdlib.h>

int setitem(int port, int data) 
{
    outpd(port, data);
    return 0;
}

int main(int argc, char** argv)
{
    int port, data, t;

    if (argc < 3) {
        printf("ERROR: IOwr32 missing parameter, use:\n    IOwr32 portx value -- hex format\n");
        exit(1);
    }

    t = sscanf(argv[1], "%x", &port);
    t = sscanf(argv[2], "%x", &data);
    // additionally the numbers of tokens scanned can be checked, ignored here
    
    setitem(port, data);

}