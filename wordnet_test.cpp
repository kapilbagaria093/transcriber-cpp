#include <iostream>

extern "C" {
    #include "wn.h"
}

int main()
{
    if (wninit() != 0)
    {
        std::cerr << "Failed to initialize WordNet\n";
        return 1;
    }

    char word[] = "beautiful";

    IndexPtr result = index_lookup(word, NOUN);

    if (result)
        std::cout << word << " IS a noun\n";
    else
        std::cout << word << " is NOT a noun\n";

    return 0;
}
