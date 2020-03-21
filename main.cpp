#include "Database.h"

int main() {
    std::ifstream fin("out.txt");
    std::ofstream fout("output.txt");
    DataBase(fin, fout).ProcessJSON();
    return 0;
}
