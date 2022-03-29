/*
 * general_test.c
 *
 *  Created on: March 26, 2022
 *      Author: micahbly
 */






/*****************************************************************************/
/*                                Includes                                   */
/*****************************************************************************/

// unit testing framework
#include "minunit.h"

// project includes

// class being tested
#include "general.h"

// C includes


// A2560 includes
#include <mb/a2560_platform.h>
#include <mb/lib_sys.h>



/*****************************************************************************/
/*                               Definitions                                 */
/*****************************************************************************/



/*****************************************************************************/
/*                               Enumerations                                */
/*****************************************************************************/



/*****************************************************************************/
/*                             Global Variables                              */
/*****************************************************************************/

extern System*			global_system;


/*****************************************************************************/
/*                       Private Function Prototypes                         */
/*****************************************************************************/



/*****************************************************************************/
/*                       Private Function Definitions                        */
/*****************************************************************************/





/*****************************************************************************/
/*                        MinUnit Function Defintions                        */
/*****************************************************************************/



void text_test_setup(void)	// this is called EVERY test
{
// 	foo = 7;
// 	bar = 4;
// 	
}


void text_test_teardown(void)	// this is called EVERY test
{

}


// **** WORD-WRAP UTILITIES *****


// **** MATH UTILITIES *****


MU_TEST(general_test_round)
{
	int		expected_result[18] = {0, 0, 0, 0, 1, 1, 1, 1, 4, 10, 10, 10, 99, 100, 149999, 150001, 0, -1};
	double	input[18] = {0.01, 0.1, 0.4, 0.49999, 0.50, 0.51, 0.99, 1.0, 4.4, 9.50, 9.99, 10.00000001, 98.5, 99.51, 149999.0000049, 150000.6, -0.01, -0.51};
	int		i;
	int		result;
	
	for (i = 0; i < 18; i++)
	{
		result = General_Round(input[i]);
		DEBUG_OUT(("%s %d: i=%i, result='%i', expected_result[i]='%i'", __func__, __LINE__, i, result, expected_result[i]));
		mu_assert_int_eq( expected_result[i], result );
	}
}


// **** NUMBER<>STRING UTILITIES *****


MU_TEST(general_test_make_filesize_readable)
{
	char			temp_buff[30];
	char*			result = temp_buff;
	char*			expected_result[12] = {"1 b", "1023 b", "1.0 kb", "9.9 kb", "10 kb", "100 kb", "101 kb", "999 kb", "1.0 Mb", "9.9 Mb", "10 Mb", "100 Mb"};
	unsigned long	input[12] = {1, 1023, 1024, 10239, 10240, 103420, 103424, 1048575, 1048576, 10485759, 10485760, 104857600};
	int				i;
	
	for (i = 0; i < 12; i++)
	{
		General_MakeFileSizeReadable(input[i], result);
		DEBUG_OUT(("%s %d: result='%s', expected_result[i]='%s'", __func__, __LINE__, result, expected_result[i]));
		mu_assert_string_eq( expected_result[i], result );
	}
}




// **** MISC STRING UTILITIES *****

MU_TEST(general_test_makelower)
{
	char*	string1 = "myfile.doc";
	char*	string1result = "myfile.doc";
	char*	string2 = "MyFiLe";
	char*	string2result = "myfile";
	char*	string3 = "MYFILE.DOC";
	char*	string3result = "myfile.doc";
	char*	string4 = "MYFILE1234!@#$.DOC";
	char*	string4result = "myfile1234!@#$.doc";
	char*	string5 = "My FiLe IS REALLY long and WEIRD";
	char*	string5result = "my file is really long and weird";
	
	mu_assert( General_StrToLower(string1) == false, "string was already lowercase, but function claims to have changed it" );
	mu_assert_string_eq( string1result, string1 );

	mu_assert( General_StrToLower(string2) == true, "string was not all lowercase, but function claims it was" );
	mu_assert_string_eq( string2result, string2 );

	mu_assert( General_StrToLower(string3) == true, "string was not all lowercase, but function claims it was" );
	mu_assert_string_eq( string3result, string3 );

	mu_assert( General_StrToLower(string4) == true, "string was not all lowercase, but function claims it was" );
	mu_assert_string_eq( string4result, string4 );

	mu_assert( General_StrToLower(string5) == true, "string was not all lowercase, but function claims it was" );
	mu_assert_string_eq( string5result, string5 );
}


MU_TEST(general_test_strlcpy_with_alloc)
{
	char*	result;
	char*	expected_result[9] = {"1 b", "1023 b", "1.0 kb", "ACME CORPORATION", "12345678", "123456789", "1234567890", "- - - - - / ", ""};
	char*	input[9] = {"1 b", "1023 b", "1.0 kb", "ACME CORPORATION", "1234567890", "1234567890", "1234567890", "- - - - - / ", "abc"};
	int		max_len[9] = {100, 20, 20, 17, 9, 10, 11, 100000, -1};
	int		i;
	
	for (i = 0; i < 9; i++)
	{
		result = General_StrlcpyWithAlloc(input[i], max_len[i]);
		DEBUG_OUT(("%s %d: i=%i, result='%s' (%p), expected_result[i]='%s', input[i]='%s' (%p)", __func__, __LINE__, i, result, result, expected_result[i], input[i], input[i]));
		if (result == NULL)
		{
			mu_assert_int_eq(strlen(expected_result[i]), 0);
		}
		else
		{
			mu_assert_string_eq( result, expected_result[i] );
			f_free(result, MEM_STANDARD);
		}
	}
}


MU_TEST(general_test_strlcpy)
{
	char		temp_buff[30];
	char*		copy = temp_buff;
	signed long	result;
	signed long	expected_len[10] = {3, 6, 6, 16, 10, 10, 10, 10, 10, -1};
	char*		expected_result[10] =	{"1 b", "1023 b", "1.0 kb", "ACME CORPORATION", "12345678",   "123456789",  "1234567890", "1234567890", "- - - - - / ", ""};
	char*		input[10] = 			{"1 b", "1023 b", "1.0 kb", "ACME CORPORATION", "1234567890", "1234567890", "1234567890", "1234567890", "- - - - - / ", "abc"};
	int			max_len[10] = {100, 20, 20, 17, 9, 10, 11, 100, 100000, -1};
	int			i;
	
	for (i = 0; i < 10; i++)
	{
		result = General_Strlcpy(copy, input[i], max_len[i]);
		DEBUG_OUT(("%s %d: i=%i, result=%lu, max_len[i]=%lu, expected_len[i]=%lu, input[i]='%s', copy='%s', expected_result[i]='%s'", __func__, __LINE__, i, result, max_len[i], expected_len[i], input[i], copy, expected_result[i]));

		if (result == -1)
		{
			mu_assert_int_eq(strlen(expected_result[i]), 0);
		}
		else
		{
			mu_assert_string_eq( copy, expected_result[i] );
		}
	}
}


MU_TEST(general_test_strlcat)
{
	char		temp_buff[30];
	signed long	result;
	signed long	padding[10] = 			{3, 		5, 				10, 				12, 				6, 			 7, 			7, 				7, 				12, 			3};
	signed long	expected_len[10] = 		{6, 		11, 			14, 				16, 				9, 			 10, 			10, 			10, 			11, 			3};
	char*		expected_result[10] = 	{"1 byte",	"1023 kbytes",	"1.0 megajoules", 	"ACME CORPORATION", "12345678",  "123456789", 	"1234567890", 	"1234567890",	"- - - - - /", "abc"};
	char*		input[10] = 			{"1 b   ",	"1023 k     ", 	"1.0           ", 	"ACME            ", "123      ", "123       ", 	"123       ",   "123       ", 	"            ", "abc   "};
	char*		to_add[10] = 			{"yte",		"bytes", 		"megajoules", 		" CORPORATION", 	"4567890",	 "4567890", 	"4567890", 		"4567890", 		"- - - - - / ", "def"};
	int			max_len[10] = 			{100, 		20,				20, 				17, 				9, 			 10, 			11, 			100, 			12, 			-1};
	int			i;
	
	// LOGIC: 
	//   the string to be added to has extra padding on definition, to make sure it has enough storage. 
	//   it is then terminated before the padding, so that the cat will only add to the end of the first bit of content, simulating a real use scenario
	
	for (i = 0; i < 10; i++)
	{
		int the_end = strlen(input[i]) - padding[i];
		
		input[i][the_end] = 0;
		
		result = General_Strlcat(input[i], to_add[i], max_len[i]);
		DEBUG_OUT(("%s %d: i=%i, result=%i, max_len[i]=%i, expected_len[i]=%i, input[i]='%s', to_add[i]='%s', expected_result[i]='%s'", __func__, __LINE__, i, result, max_len[i], expected_len[i], input[i], to_add[i], expected_result[i]));

		if (result == -1)
		{
			mu_assert_int_eq(strlen(expected_result[i]), 0);
		}
		else
		{
			mu_assert_string_eq( input[i], expected_result[i] );
		}
	}
}


MU_TEST(general_test_strncmp)
{
	signed int	result;
	signed int	expected_result[12] = 	{1, 		1, 				1, 					-1, 				0, 			 1, 			1, 				0, 				0, 				0,				0,		0};
	char*		string1[12] = 			{"1 byte",	"1023 kbytes",	"1.0 megajoules", 	"ACME CORPORATION", "CORPORATI", "123456789", 	"1234567890", 	"1234567890",	"!@#$%^&*()_+", "!@#$%^&*()XX", "abc", "abc"};
	char*		string2[12] = 			{"1 b",		"1023 k     ", 	"1.0 Megajoules", 	"acme corporation", "CORPORATI", "123       ", 	"123",   		"1234567890", 	"!@#$%^&*()_+", "!@#$%^&*()ZZ", "abc", "abc"};
	int			max_len[12] = 			{100, 		20,				20, 				17, 				9, 			 10, 			11, 			100, 			12, 			10,				-1,		0};
	int			i;
	
	// LOGIC: 
	//   function could return 0, any negative, or any positive. 
	//   our expected result will be -1, 0, or 1, with -1 and 1 standing for any negative/positive result. 
	//   will not try to calculate the actual results, as they don't matter. 
	
	for (i = 0; i < 12; i++)
	{
		result = General_Strncmp(string1[i], string2[i], max_len[i]);
		DEBUG_OUT(("%s %d: i=%i, result=%i, max_len[i]=%i, expected_result[i]=%i, string1[i]='%s', string2[i]='%s'", __func__, __LINE__, i, result, max_len[i], expected_result[i], string1[i], string2[i]));

		if (result == 0)
		{
			mu_assert(expected_result[i] == 0, "compare says strings same, but we expected them to show different");
		}
		else if (result > 0)
		{
			mu_assert(expected_result[i] > 0, "compare says string1 > string2, but we expected different result");
		}
		else
		{
			mu_assert(expected_result[i] < 0, "compare says string1 < string2, but we expected different result");
		}
	}
}


MU_TEST(general_test_strncasecmp)
{
	signed int	result;
	signed int	expected_result[12] = 	{1, 		1, 				0, 					0, 					0, 			 1, 			1, 				0, 				0, 				0,				0,		0};
	char*		string1[12] = 			{"1 byte",	"1023 kbytes",	"1.0 megajoules", 	"ACME CORPORATION", "CORPORATI", "123456789", 	"1234567890", 	"1234567890",	"!@#$%^&*()_+", "!@#$%^&*()XX", "abc", "abc"};
	char*		string2[12] = 			{"1 b",		"1023 k     ", 	"1.0 Megajoules", 	"acme corporation", "CORPORATI", "123       ", 	"123",   		"1234567890", 	"!@#$%^&*()_+", "!@#$%^&*()ZZ", "abc", "abc"};
	int			max_len[12] = 			{100, 		20,				20, 				17, 				9, 			 10, 			11, 			100, 			12, 			10,				-1,		0};
	int			i;
	
	// LOGIC: 
	//   function could return 0, any negative, or any positive. 
	//   our expected result will be -1, 0, or 1, with -1 and 1 standing for any negative/positive result. 
	//   will not try to calculate the actual results, as they don't matter. 
	
	for (i = 0; i < 12; i++)
	{
		result = General_Strncasecmp(string1[i], string2[i], max_len[i]);
		DEBUG_OUT(("%s %d: i=%i, result=%i, max_len[i]=%i, expected_result[i]=%i, string1[i]='%s', string2[i]='%s'", __func__, __LINE__, i, result, max_len[i], expected_result[i], string1[i], string2[i]));

		if (result == 0)
		{
			mu_assert(expected_result[i] == 0, "compare says strings same, but we expected them to show different");
		}
		else if (result > 0)
		{
			mu_assert(expected_result[i] > 0, "compare says string1 > string2, but we expected different result");
		}
		else
		{
			mu_assert(expected_result[i] < 0, "compare says string1 < string2, but we expected different result");
		}
	}
}


MU_TEST(general_test_strnlen)
{
	signed int	result;
	signed int	expected_result[12] = 	{6, 		11, 			14, 				16, 				9, 			 9, 			10, 			10, 			12, 			10,				3,		0};
	char*		string1[12] = 			{"1 byte",	"1023 kbytes",	"1.0 megajoules", 	"ACME CORPORATION", "CORPORATI", "123456789", 	"1234567890", 	"1234567890",	"!@#$%^&*()_+", "!@#$%^&*()XX", "abc", "abc"};
	size_t		max_len[12] = 			{100, 		20,				20, 				17, 				9, 			 9, 			10, 			100, 			12, 			10,				-1,		0};
	int			i;
	
	for (i = 0; i < 12; i++)
	{
		result = General_Strnlen(string1[i], max_len[i]);
		DEBUG_OUT(("%s %d: i=%i, result=%i, max_len[i]=%i, expected_result[i]=%i, string1[i]='%s'", __func__, __LINE__, i, result, max_len[i], expected_result[i], string1[i]));
		mu_assert_int_eq(result, expected_result[i]);
	}
}


MU_TEST(general_test_compare_string_len)
{
	boolean		result;
	boolean		expected_result[12] = 	{true, 		false, 			false, 				false, 				false, 		 false, 		true, 			false, 			true, 			false,			true,	false};
	char*		string1[12] = 			{"1 byte",	"1023 kbytes",	"1.0 megajoules", 	"ACME CORPORATION", "       ",	"123456789", 	"1234567890", 	"1234567890",	"!@#$%^&*()_+", "!@#$%^&*()X",	"a",
		"123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 "};
	char*		string2[12] = 			{"1 b",		"1023 k     ", 	"1.0 Megajoules", 	"acme corporation", "       ",	"123       ", 	"123",   		"1234567890", 	"!@#$%^&*()_",	"!@#$%^&*()ZZ", "", 	
		"123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 "};
	int			i;
	
	for (i = 0; i < 12; i++)
	{
		result = General_CompareStringLength(string1[i], string2[i]);
		DEBUG_OUT(("%s %d: i=%i, result=%i, expected_result[i]=%i, string1[i]='%s', string2[i]='%s'", __func__, __LINE__, i, result, expected_result[i], string1[i], string2[i]));
		mu_assert_int_eq(result, expected_result[i]);
	}
}


// **** RECTANGLE UTILITIES *****


// **** FILENAME AND FILEPATH UTILITIES *****

MU_TEST(general_test_extract_file_extension)
{
	char*	filename1 = "myfile.doc";
	char*	filename1result = "doc";
	char*	filename2 = ".txt";
	char*	filename2result = "txt";
	char*	filename3 = "myfile";
	char*	filename3result = "";
	char*	filename4 = "myfile.reallybigextension";
	char*	filename4result = "reallybigextension";
	char*	filename5 = "myfile.UPPERCASEext";
	char*	filename5result = "uppercaseext";
	char	temp_buff[30];
	char*	the_extension = temp_buff;
	
	mu_assert( General_ExtractFileExtensionFromFilename(filename1, the_extension) == true, "Could not extract file extension" );
	mu_assert_string_eq( filename1result, the_extension );

	mu_assert( General_ExtractFileExtensionFromFilename(filename2, the_extension) == true, "Could not extract file extension" );
	mu_assert_string_eq( filename2result, the_extension );

	the_extension[0] = 0;
	mu_assert( General_ExtractFileExtensionFromFilename(filename3, the_extension) == false, "Extension extractor returned success, but there was no extension" );
	mu_assert( the_extension[0] == 0, "Extension should have been empty string, but wasn't." );

	mu_assert( General_ExtractFileExtensionFromFilename(filename4, the_extension) == true, "Could not extract file extension" );
	mu_assert_string_eq( filename4result, the_extension );

	mu_assert( General_ExtractFileExtensionFromFilename(filename5, the_extension) == true, "Could not extract file extension" );
	mu_assert_string_eq( filename5result, the_extension );
}




// **** speed tests

MU_TEST(text_test_hline_speed)
{
	long start1;
	long end1;
	long start2;
	long end2;
	signed int		x;
	signed int		y;
	signed int		line_len;
	unsigned char	the_char;
	signed int		i;
	signed int		num_passes = 90;
	signed int		j;
	signed int		num_cycles = 10;

	
	// test speed of first variant
	start1 = mu_timer_real();
	
	for (j = 0; j < num_cycles; j++)
	{
		for (i=0; i < num_passes; i++)
		{
			//mu_assert( Text_DrawHLineSlow(global_system->screen_[ID_CHANNEL_A], x, y + i, line_len, the_char, FG_COLOR_GREEN, BG_COLOR_BLACK, CHAR_ONLY) == true, "Text_DrawHLine failed" );
		}
	}
		
	end1 = mu_timer_real();
	
	// test speed of second variant
	x++;
	start2 = mu_timer_real();
	
	for (j = 0; j < num_cycles; j++)
	{
		for (i=0; i < num_passes; i++)
		{
			//mu_assert( Text_DrawHLine(global_system->screen_[ID_CHANNEL_A], x, y + i, line_len, the_char, FG_COLOR_RED, BG_COLOR_BLACK, CHAR_ONLY) == true, "Text_DrawHLine failed" );
		}
	}
	
	end2 = mu_timer_real();
	
	printf("\nSpeed results: first routine completed in %li ticks; second in %li ticks\n", end1 - start1, end2 - start2);	
}



	// speed tests
MU_TEST_SUITE(text_test_suite_speed)
{	
	MU_SUITE_CONFIGURE(&text_test_setup, &text_test_teardown);
	
	MU_RUN_TEST(text_test_hline_speed);
}


// unit tests
MU_TEST_SUITE(text_test_suite_units)
{	
	MU_SUITE_CONFIGURE(&text_test_setup, &text_test_teardown);

	MU_RUN_TEST(general_test_extract_file_extension);
	MU_RUN_TEST(general_test_makelower);
	//MU_RUN_TEST(general_test_make_filesize_readable);
	MU_RUN_TEST(general_test_round);
	//MU_RUN_TEST(general_test_strlcpy_with_alloc);
	MU_RUN_TEST(general_test_strlcpy);
	MU_RUN_TEST(general_test_strlcat);
	MU_RUN_TEST(general_test_strncmp);
	MU_RUN_TEST(general_test_strncasecmp);
	MU_RUN_TEST(general_test_strnlen);
	MU_RUN_TEST(general_test_compare_string_len);
	
}



/*****************************************************************************/
/*                        Public Function Definitions                        */
/*****************************************************************************/



int main(int argc, char* argv[])
{
	if (Sys_InitSystem() == false)
	{
		DEBUG_OUT(("%s %d: Couldn't initialize the system", __func__, __LINE__));
		exit(0);
	}

	MU_RUN_SUITE(text_test_suite_units);
// 	MU_RUN_SUITE(text_test_suite_speed);
	MU_REPORT();
	
	return MU_EXIT_CODE;
}
