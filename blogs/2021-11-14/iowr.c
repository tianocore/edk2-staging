//
// I/O write tool
//
#include <stdio.h>
#include <stdlib.h>

int setitem(int port, int data) 
{
    outp(port, data);
    return 0;
}

int main(int argc, char** argv)
{
    int port, data, t;

    if (argc < 3) {
        printf("ERROR: IOwr missing parameter, use:\n    IOwr portx value -- hex format\n");
        exit(1);
    }

    t = sscanf(argv[1], "%x", &port);
    t = sscanf(argv[2], "%x", &data);
    // additionally the numbers of tokens scanned can be checked, ignored here
    
    setitem(port, data);

}