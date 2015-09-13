#include "utils.h"

int main()
{
    //check if the password file exists, if not create one
    if(!IsPasswordFileExist())
    {
        cout<<"password file does not exist. Creating default file"<<endl;
        CreatePasswordFile();
    }
    return 0;
}
