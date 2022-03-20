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
#if 0
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
#else
    const char* variants[] =
    {
        "0x1c",
        "536",
        "536L",
        "0x5a3b6e",

    };
#endif
    for (const auto s : variants)
    {
        test(s);
    }

    WaitKeyAndQuit();
    return 0;
}

void test(const char* str)
{
#if 0
    fnr::NumberReader<float> nrd;

    printf( "NumberReader<float>:\n" );

	for ( int i = 0 ; str[i] && nrd.put( str[i] ) ; ++i );
	if ( nrd.valid() )
		printf( "value %s = %g\n", str, nrd.value() );
	else
		printf( "invalid float value (%s)\n", str );
	printf("\n");
#else
    fnr::NumberReader<long> nri;

    printf( "NumberReader<long>:\n" );

	for ( int i = 0 ; str[i] && nri.put( str[i] ) ; ++i );
	if ( nri.valid() )
		printf( "value %s = %d\n", str, nri.value() );
	else
		printf( "invalid long value (%s)\n", str );
	printf("\n");
#endif
}
