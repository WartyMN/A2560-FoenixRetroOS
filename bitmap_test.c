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



void bitmap_test_setup(void)	// this is called EVERY test
{
// 	foo = 7;
// 	bar = 4;
// 	
}


void bitmap_test_teardown(void)	// this is called EVERY test
{

}



// **** speed tests

MU_TEST(bitmap_test_tiling)
{
	long start1;
	long end1;
	long start2;
	long end2;
	int		i;
	int		times_to_run = 25;
	
	Screen* the_screen = Sys_GetScreen(global_system, ID_CHANNEL_B);
	Theme*	the_theme = Sys_GetTheme(global_system);
	Bitmap*	the_pattern = Theme_GetDesktopPattern(the_theme);
	Bitmap*	the_target_bitmap = the_screen->bitmap_;
	
	
	// test speed of first variant
	start1 = mu_timer_real();

	// speed test 1 goes here
	for (i = 0; i < times_to_run; i++)
	{
		Bitmap_TileV1(the_pattern, 0, 0, the_target_bitmap, 16, 16);
	}


	
	
	end1 = mu_timer_real();
	
	// test speed of 2nd variant
	start2 = mu_timer_real();
	


	// speed test 2 goes here
	for (i = 0; i < times_to_run; i++)
	{
		Bitmap_Tile(the_pattern, 0, 0, the_target_bitmap, 16, 16);
	}
	
	
	end2 = mu_timer_real();
	
	printf("\nSpeed results: first routine completed in %li ticks; second in %li ticks\n", end1 - start1, end2 - start2);
	DEBUG_OUT(("\nSpeed results: first routine completed in %li ticks; second in %li ticks\n", end1 - start1, end2 - start2));
}



	// speed tests
MU_TEST_SUITE(bitmap_test_suite_speed)
{	
	MU_SUITE_CONFIGURE(&bitmap_test_setup, &bitmap_test_teardown);
	
	MU_RUN_TEST(bitmap_test_tiling);
}


// unit tests
MU_TEST_SUITE(bitmap_test_suite_units)
{	
	MU_SUITE_CONFIGURE(&bitmap_test_setup, &bitmap_test_teardown);
	
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
	
// 	printf("Hiya from graphic world.");
// 	
// 	Sys_SetModeGraphics(global_system);
// 	printf("now in graphics mode");
// 	getchar();
// 	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], 1, 4, (char*)"graphics mode?", FG_COLOR_YELLOW, BG_COLOR_BLACK);
// 	Sys_SetModeText(global_system, false);
// 	printf("now in normal text mode");
// 	getchar();
// 	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], 1, 4, (char*)"overlay mode??", FG_COLOR_YELLOW, BG_COLOR_BLACK);

	

// 		MU_RUN_SUITE(bitmap_test_suite_units);
	MU_RUN_SUITE(bitmap_test_suite_speed);
	MU_REPORT();

	Sys_SetModeText(global_system, true);

	return MU_EXIT_CODE;
	
	return 0;
}
