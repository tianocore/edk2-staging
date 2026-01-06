//
// I/O write 16 tool
//
#include <stdio.h>
#include <stdlib.h>

int setitem(int port, int data) 
{
    outpw(port, data);
    return 0;
}

int main(int argc, char** argv)
{
    int port, data, t;

    if (argc < 3) {
        printf("ERROR: IOwr16 missing parameter, use:\n    IOwr16 portx value -- hex format\n");
        exit(1);
    }

    t = sscanf(argv[1], "%x", &port);
    t = sscanf(argv[2], "%x", &data);
    // additionally the numbers of tokens scanned can be checked, ignored here
    
    setitem(port, data);

}