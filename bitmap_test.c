/*
 * bitmap_test.c
 *
 *  Created on: Mar 10, 2022
 *      Author: micahbly
 */






/*****************************************************************************/
/*                                Includes                                   */
/*****************************************************************************/

// unit testing framework
#include "minunit.h"

// project includes

// class being tested
#include "bitmap.h"

// C includes
#include <stdbool.h>


// A2560 includes
#include <mb/a2560_platform.h>
#include <mb/general.h>
#include <mb/text.h>
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



// **** speed tests

MU_TEST(text_test_hline_speed)
{
	long start1;
	long end1;
	long start2;
	long end2;
	
	// test speed of first variant
	start1 = mu_timer_real();
	


	// speed test 1 goes here
	
	
	end1 = mu_timer_real();
	
	// test speed of 2nd variant
	start2 = mu_timer_real();
	


	// speed test 2 goes here
	
	
	end2 = mu_timer_real();
	
	printf("\nSpeed results: first routine completed in %li ticks; second in %li ticks\n", end1 - start1, end2 - start2);
}



	// speed tests
MU_TEST_SUITE(text_test_suite_speed)
{	
	MU_SUITE_CONFIGURE(&text_test_setup, &text_test_teardown);
	
// 	MU_RUN_TEST(text_test_hline_speed);
}


// unit tests
MU_TEST_SUITE(text_test_suite_units)
{	
	MU_SUITE_CONFIGURE(&text_test_setup, &text_test_teardown);
	
// 	MU_RUN_TEST(font_replace_test);
}




/*****************************************************************************/
/*                        Public Function Definitions                        */
/*****************************************************************************/



int main(int argc, char* argv[])
{
	if ( Sys_InitSystem() == false)
	{
		DEBUG_OUT(("%s %d: Couldn't initialize the system", __func__, __LINE__));
		exit(0);
	}
	
	printf("Hiya from graphic world.");
	
	Sys_SetModeGraphics(global_system);
	printf("now in graphics mode");
	getchar();
	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], 1, 4, (char*)"graphics mode?", FG_COLOR_YELLOW, BG_COLOR_BLACK);
	Sys_SetModeText(global_system, false);
	printf("now in normal text mode");
	getchar();
	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], 1, 4, (char*)"overlay mode??", FG_COLOR_YELLOW, BG_COLOR_BLACK);
	Sys_SetModeText(global_system, true);
	printf("now in overlay text mode");

	

	#if defined(RUN_TESTS)
// 		MU_RUN_SUITE(text_test_suite_units);
// 		MU_RUN_SUITE(text_test_suite_speed);
// 		MU_REPORT();
		return MU_EXIT_CODE;
	#endif

	return 0;
}
