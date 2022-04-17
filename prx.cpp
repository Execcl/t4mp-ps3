#include "stdafx.h"

#pragma diag_suppress 77
SYS_MODULE_INFO(t4mp, 0, 1, 1);
SYS_MODULE_START(t4mp_start);
SYS_MODULE_STOP(t4mp_stop);

variables* g_vars;

ScreenPlacement* scrPlaceView = reinterpret_cast<ScreenPlacement*>(static_cast<int>(game_table::scrplaceview));
UiContext* cgDc = reinterpret_cast<UiContext*>(static_cast<int>(game_table::uicontext));;

detour* r_endframe;
detour* sys_io_cellpadgetdata;

void R_EndFrame()
{
	g_change_overtime.update();

	g_menu->on_run();
	g_menu->on_draw();

	r_endframe->invoke<void>();
}

int sys_io_cellPadGetData(unsigned int port, CellPadData* data)
{
	if (port != 0)
		return sys_io_cellpadgetdata->invoke<int>(port, data);

	int returnValue = sys_io_cellpadgetdata->invoke<int>(port, g_input.GetInputData());
	std::memcpy(data, g_input.GetInputData(), sizeof(CellPadData));

	if (g_menu->is_open())
	{
		// Clear button flags
		data->button[CELL_PAD_BTN_OFFSET_DIGITAL1] &= ~(CELL_PAD_CTRL_LEFT | CELL_PAD_CTRL_DOWN | CELL_PAD_CTRL_RIGHT | CELL_PAD_CTRL_UP);
		data->button[CELL_PAD_BTN_OFFSET_DIGITAL2] &= ~(CELL_PAD_CTRL_CROSS | CELL_PAD_CTRL_CIRCLE);

		//if (g_Menu.IsMoving())
		//{
		//	// Clear analog values
		//	data->button[CELL_PAD_BTN_OFFSET_ANALOG_RIGHT_X] = 128;
		//	data->button[CELL_PAD_BTN_OFFSET_ANALOG_RIGHT_Y] = 128;
		//}
	}

	return returnValue;
}

extern "C" int t4mp_start(void)
{
	g_input = CInput();
	g_vars = new variables();

	if (g_vars == nullptr)
		return SYS_PRX_STOP_FAILED;

	g_menu = new menu("CrEaTiiOnBrotherhood");

	if (g_menu == nullptr)
		return SYS_PRX_STOP_FAILED;

	memset(g_menu, 0, sizeof(menu));

	g_menu->on_build();

	r_endframe = new detour(static_cast<int>(game_table::r_endframe), R_EndFrame);
	sys_io_cellpadgetdata = new detour(static_cast<int>(game_table::sys_io_cellpadgetdata), sys_io_cellPadGetData);

	return SYS_PRX_START_OK;
}

extern "C" int t4mp_stop(void)
{
	memset(g_vars, 0, sizeof(variables));
	memset(g_menu, 0, sizeof(menu));

	delete	g_vars;
	delete g_menu;

	r_endframe->~detour();
	sys_io_cellpadgetdata->~detour();

	return SYS_PRX_STOP_OK;
}
