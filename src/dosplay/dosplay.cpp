// license:BSD-3-Clause
// copyright-holders:Takeda Toshiya,Miodrag Milanovic

#include "emu.h"
#include "cpu/i86/i86.h"
#include "cpu/i86/i86inline.h"

#define VECTOR_TOP	0
#define VECTOR_SIZE	0x400
#define BIOS_TOP	(VECTOR_TOP + VECTOR_SIZE)
#define BIOS_SIZE	0x100
#define WORK_TOP	(BIOS_TOP + BIOS_SIZE)
#define WORK_SIZE	0x300
#define IRET_TOP	(WORK_TOP + WORK_SIZE)
#define IRET_SIZE	0x100

class dosplay_state : public driver_device
{
public:
	dosplay_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, "ram")
	{
	}

	void msdos_syscall(uint8_t num);
private:
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_ram;
	virtual void machine_reset() override;
};

class dosplay_i8088_cpu_device : public i8088_cpu_device
{
public:
	// construction/destruction
	dosplay_i8088_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: i8088_cpu_device(mconfig, tag, owner, clock) { };
	
	virtual bool common_op(uint8_t op) override;
};

const device_type I8088_DOSPLAY = &device_creator<dosplay_i8088_cpu_device>;

bool dosplay_i8088_cpu_device::common_op(uint8_t op)
{
	switch(op)
	{			
		case 0xcf: // i_iret
			{
				uint32_t old = m_pc - 1;

				m_ip = POP();
				m_sregs[CS] = POP();
				i_popf();
				CLK(IRET);
				
				// MS-DOS system call
				if(IRET_TOP <= old && old < (IRET_TOP + IRET_SIZE)) {
					dosplay_state *state = machine().driver_data<dosplay_state>();
					state->msdos_syscall(old - IRET_TOP);
				}				
			}
			break;
		default:
			return i8088_cpu_device::common_op(op);
	}
	return true;
}	


void dosplay_state::machine_reset()
{
	memset(m_ram, 0, 0xa0000);
	
	for(int i = 0; i < 0x80; i++) {
		*(uint16_t *)(m_ram + 4 * i + 0) = i;
		*(uint16_t *)(m_ram + 4 * i + 2) = (IRET_TOP >> 4);
	}
	*(uint16_t *)(m_ram + 4 * 0x08 + 0) = 0xfea5;
	*(uint16_t *)(m_ram + 4 * 0x08 + 2) = 0xf000;
	*(uint16_t *)(m_ram + 4 * 0x22 + 0) = 0xfff0;
	*(uint16_t *)(m_ram + 4 * 0x22 + 2) = 0xf000;
	memset(m_ram + IRET_TOP, 0xcf, IRET_SIZE);
	
	uint8_t *ROM = memregion("bios")->base();
	// have irq0 call system timer tick
	ROM[0xfea5] = 0xcd;	// int 1ch
	ROM[0xfea6] = 0x1c;
	ROM[0xfea7] = 0xea;	// jmp 80:08
	ROM[0xfea8] = 0x08;
	ROM[0xfea9] = 0x00;
	ROM[0xfeaa] = ((IRET_TOP >> 4)     ) & 0xff;
	ROM[0xfeab] = ((IRET_TOP >> 4) >> 8) & 0xff;
	
	// boot
	ROM[0xfff0] = 0xf4;	// halt
	ROM[0xfff1] = 0xcd;	// int 21h
	ROM[0xfff2] = 0x21;
	ROM[0xfff3] = 0xcb;	// retf
}

void dosplay_state::msdos_syscall(uint8_t num)
{
	switch(num) {
		case 0x20: 
			machine().schedule_exit();
			break;
		default:
			printf("unhandled interrupt %02x", num);
	}	
}
/******************************************************************************
 Address Maps
******************************************************************************/

static ADDRESS_MAP_START( pc8_map, AS_PROGRAM, 8, dosplay_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x9ffff) AM_RAM AM_SHARE("ram")
	AM_RANGE(0xf0000, 0xfffff) AM_ROM AM_REGION("bios", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START(pc8_io, AS_IO, 8, dosplay_state )
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

/******************************************************************************
 Input Ports
******************************************************************************/
static INPUT_PORTS_START( dosplay )
INPUT_PORTS_END

/******************************************************************************
 Machine Drivers
******************************************************************************/

static MACHINE_CONFIG_START( dosplay, dosplay_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",  I8088_DOSPLAY, 4772720)
	MCFG_CPU_PROGRAM_MAP(pc8_map)
	MCFG_CPU_IO_MAP(pc8_io)
MACHINE_CONFIG_END



/******************************************************************************
 ROM Definitions
******************************************************************************/

ROM_START(dosplay)
	ROM_REGION(0x10000,"bios", 0)
ROM_END



/******************************************************************************
 Drivers
******************************************************************************/

/*    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT   INIT      COMPANY                     FULLNAME                                                    FLAGS */
COMP( 198?, dosplay,   0,          0,      dosplay,   dosplay, driver_device, 0,      "MAMEdev",   "DosPlay", MACHINE_NO_SOUND_HW )
