#include <stdio.h>
#include <sys/unistd.h>

// 2. The client sends the complete path to a file. The server returns back the length of the file and its
//    content in the case the file exists. When the file does not exist the server returns a length of -1
//    and no content. The client will store the content in a file with the same name as the input file with
//    the suffix –copy appended (ex: for f.txt => f.txt-copy).

int main() {
    printf("Hello\n");
    sleep(10);
    return 0;
}
