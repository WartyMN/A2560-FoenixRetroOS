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
	long	start_ticks;
	long	end_ticks;
	long	test1_ticks;
	long	test2_ticks;
	long	test3_ticks;
	int16_t	i;
	int16_t	times_to_run = 100;
	
	Theme*	the_theme = Sys_GetTheme(global_system);
	Bitmap*	the_pattern = Theme_GetDesktopPattern(the_theme);
	Bitmap*	the_target_bitmap = Sys_GetScreenBitmap(global_system, back_layer);
	
	
	// test speed of first variant
	start_ticks = mu_timer_real();

	// speed test 1 goes here
	for (i = 0; i < times_to_run; i++)
	{
		Bitmap_TileV1(the_pattern, 0, 0, the_target_bitmap, 16, 16);
	}
	
	end_ticks = mu_timer_real();
	test1_ticks = end_ticks - start_ticks;


	
	// test speed of 2nd variant
	start_ticks = mu_timer_real();
	
	// speed test 2 goes here
	for (i = 0; i < times_to_run; i++)
	{
		Bitmap_TileV2(the_pattern, 0, 0, the_target_bitmap, 16, 16);
	}
		
	end_ticks = mu_timer_real();
	test2_ticks = end_ticks - start_ticks;


	
	// test speed of 3rd variant
	start_ticks = mu_timer_real();
	
	// speed test 3 goes here
	for (i = 0; i < times_to_run; i++)
	{
		Bitmap_Tile(the_pattern, 0, 0, the_target_bitmap, 16, 16);
	}
		
	end_ticks = mu_timer_real();
	test3_ticks = end_ticks - start_ticks;
	
	printf("\nSpeed results: 1st: %li ticks; 2nd: %li ticks; 3rd: %li ticks\n", test1_ticks, test2_ticks, test3_ticks);
	DEBUG_OUT(("Speed results: 1st: %li ticks; 2nd: %li ticks; 3rd: %li ticks", test1_ticks, test2_ticks, test3_ticks));
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
// 	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], 1, 4, (char*)"graphics mode?", FG_COLOR_BRIGHT_YELLOW, BG_COLOR_BLACK);
// 	Sys_SetModeText(global_system, false);
// 	printf("now in normal text mode");
// 	getchar();
// 	Text_DrawStringAtXY(global_system->screen_[ID_CHANNEL_B], 1, 4, (char*)"overlay mode??", FG_COLOR_BRIGHT_YELLOW, BG_COLOR_BLACK);

	

	MU_RUN_SUITE(bitmap_test_suite_units);
// 	MU_RUN_SUITE(bitmap_test_suite_speed);
	MU_REPORT();

	Sys_SetModeText(global_system, true);

	return MU_EXIT_CODE;
	
	return 0;
}
