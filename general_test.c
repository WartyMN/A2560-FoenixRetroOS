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
