// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    main.c

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "xmlfile.h"
#include "modules/lib/osdobj_common.h"
#include "ui/uimain.h"

#include "drivenum.h"

GAME_EXTERN(dosplay);

const game_driver * const driver_list::s_drivers_sorted[2] =
{
	&GAME_NAME(___empty),
	&GAME_NAME(dosplay),
};

int driver_list::s_driver_count = 2;

// ======================> dosplay_machine_manager

class dosplay_machine_manager : public machine_manager
{
private:
	DISABLE_COPYING(dosplay_machine_manager);
	// construction/destruction
	dosplay_machine_manager(emu_options &options, osd_interface &osd) : machine_manager(options, osd) { }
public:
	static dosplay_machine_manager *instance(emu_options &options, osd_interface &osd)
	{
		if (!m_manager)
		{
			m_manager = new dosplay_machine_manager(options, osd);
		}
		return m_manager;
	}

	static dosplay_machine_manager *instance() { return m_manager; }

	~dosplay_machine_manager() { delete m_manager;  m_manager = nullptr; }

	int execute()
	{
		auto system = &GAME_NAME(dosplay);
		machine_config config(*system, m_options);
		running_machine machine(config, *this);
		return machine.run(false);
	}

	virtual ui_manager* create_ui(running_machine& machine) override {
		m_ui = std::make_unique<ui_manager>(machine);
		return m_ui.get();
	}

private:
	static dosplay_machine_manager* m_manager;
	std::unique_ptr<ui_manager> m_ui;
};

//**************************************************************************
//  MACHINE MANAGER
//**************************************************************************

dosplay_machine_manager* dosplay_machine_manager::m_manager = nullptr;

int emulator_info::start_frontend(emu_options &options, osd_interface &osd, int argc, char *argv[])
{
	std::string error_string;
	options.set_value(OSDOPTION_VIDEO, "none", OPTION_PRIORITY_MAXIMUM, error_string);
	options.set_value(OSDOPTION_SOUND, "none", OPTION_PRIORITY_MAXIMUM, error_string);
	options.set_value(OPTION_DEBUG, true, OPTION_PRIORITY_MAXIMUM, error_string);
	//options.set_value(OPTION_THROTTLE, false, OPTION_PRIORITY_MAXIMUM, error_string);
	dosplay_machine_manager::instance(options,osd)->execute();
	return 0;
}

const char * emulator_info::get_bare_build_version() { return nullptr; }

const char * emulator_info::get_build_version() { return nullptr; }

void emulator_info::display_ui_chooser(running_machine& machine) { }

void emulator_info::draw_user_interface(running_machine& machine) { }

void emulator_info::periodic_check() { }

bool emulator_info::frame_hook() { return false; }

void emulator_info::layout_file_cb(util::xml::data_node &layout) { }

const char * emulator_info::get_appname() { return nullptr; }

const char * emulator_info::get_appname_lower() { return "dosplay"; }

const char * emulator_info::get_configname() { return nullptr; }

const char * emulator_info::get_copyright() { return nullptr; }

const char * emulator_info::get_copyright_info() { return nullptr; }

bool emulator_info::standalone() { return true; }
