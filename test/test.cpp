#include <iostream>
#include "NumberReader.h"
#include <stdio.h>

void WaitKeyAndQuit()
{
	std::string q;
	std::cout << "Press any key followed Enter to quit..." << std::endl;
	std::cin >> q;
}

int main()
{
    void test(const char* str);

    const char* variants[] =
    {
        "536.",
        "536e+2",
        "536e",
        "536 e",
        "536e+",
        "536e-2",
        "536e -2",
        "536e- 2",

        "3.14f",
        ".12F"
    };

    for (const auto s : variants)
    {
        test(s);
    }

    WaitKeyAndQuit();
    return 0;
}

void test(const char* str)
{
    fnr::NumberReader<float> nrd;

    printf( "NumberReader<float>:\n" );

	for ( int i = 0 ; str[i] && nrd.put( str[i] ) ; ++i );
	if ( nrd.valid() )
		printf( "value %s = %g\n", str, nrd.value() );
	else
		printf( "invalid float value (%s)\n", str );
	printf("\n");
}
