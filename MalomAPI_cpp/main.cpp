#include <iostream>

using namespace std;

int main()
{
    cout << "Hello World!" << endl;

#ifdef WIN32
    system("pause");
#endif
    return 0;
}