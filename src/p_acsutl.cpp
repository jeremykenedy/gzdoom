/*
** p_acsutl.cpp
** ACS helper utilities
**
**---------------------------------------------------------------------------
** Copyright 2015 Alexey Lysiuk
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**---------------------------------------------------------------------------
**
*/

#include "r_defs.h"
#include "p_acs.h"
#include "c_dispatch.h"
#include "cmdlib.h"
#include "m_swap.h"


namespace
{

struct PCode
{
	const char* name;
	const char* args;
};

// TODO: comment arguments encoding!
// http://zdoom.org/wiki/ACS_PCodes
// http://zdoom.org/wiki/Reserved_ACS_names

static const PCode PCODES[DLevelScript::PCODE_COMMAND_COUNT] =
{
	{ "nop", "" },
	{ "terminate", "" },
	{ "suspend", "" },
	{ "pushnumber", "w" },
	{ "lspec1", "b" },
	{ "lspec2", "b" },
	{ "lspec3", "b" },
	{ "lspec4", "b" },
	{ "lspec5", "b" },
	{ "lspec1direct", "bw" },
	{ "lspec2direct", "bww" },
	{ "lspec3direct", "bwww" },
	{ "lspec4direct", "bwwww" },
	{ "lspec5direct", "bwwwww" },
	{ "add", "" },
	{ "subtract", "" },
	{ "multiply", "" },
	{ "divide", "" },
	{ "modulus", "" },
	{ "eq", "" },
	{ "ne", "" },
	{ "lt", "" },
	{ "gt", "" },
	{ "le", "" },
	{ "ge", "" },
	{ "assignscriptvar", "b" },
	{ "assignmapvar", "b" },
	{ "assignworldvar", "b" },
	{ "pushscriptvar", "b" },
	{ "pushmapvar", "b" },
	{ "pushworldvar", "b" },
	{ "addscriptvar", "b" },
	{ "addmapvar", "b" },
	{ "addworldvar", "b" },
	{ "subscriptvar", "b" },
	{ "submapvar", "b" },
	{ "subworldvar", "b" },
	{ "mulscriptvar", "b" },
	{ "mulmapvar", "b" },
	{ "mulworldvar", "b" },
	{ "divscriptvar", "b" },
	{ "divmapvar", "b" },
	{ "divworldvar", "b" },
	{ "modscriptvar", "b" },
	{ "modmapvar", "b" },
	{ "modworldvar", "b" },
	{ "incscriptvar", "b" },
	{ "incmapvar", "b" },
	{ "incworldvar", "b" },
	{ "decscriptvar", "b" },
	{ "decmapvar", "b" },
	{ "decworldvar", "b" },
	{ "goto", "a" },
	{ "ifgoto", "a" },
	{ "drop", "" },
	{ "delay", "" },
	{ "delaydirect", "w" },
	{ "random", "" },
	{ "randomdirect", "ww" },
	{ "thingcount", "" },
	{ "thingcountdirect", "ww" },
	{ "tagwait", "" },
	{ "tagwaitdirect", "w" },
	{ "polywait", "" },
	{ "polywaitdirect", "w" },
	{ "changefloor", "" },
	{ "changefloordirect", "ww" },
	{ "changeceiling", "" },
	{ "changeceilingdirect", "ww" },
	{ "restart", "" },
	{ "andlogical", "" },
	{ "orlogical", "" },
	{ "andbitwise", "" },
	{ "orbitwise", "" },
	{ "eorbitwise", "" },
	{ "negatelogical", "" },
	{ "lshift", "" },
	{ "rshift", "" },
	{ "unaryminus", "" },
	{ "ifnotgoto", "a" },
	{ "lineside", "" },
	{ "scriptwait", "" },
	{ "scriptwaitdirect", "w" },
	{ "clearlinespecial", "" },
	{ "casegoto", "wa" },
	{ "beginprint", "" },
	{ "endprint", "" },
	{ "printstring", "" },
	{ "printnumber", "" },
	{ "printcharacter", "" },
	{ "playercount", "" },
	{ "gametype", "" },
	{ "gameskill", "" },
	{ "timer", "" },
	{ "sectorsound", "" },
	{ "ambientsound", "" },
	{ "soundsequence", "" },
	{ "setlinetexture", "" },
	{ "setlineblocking", "" },
	{ "setlinespecial", "" },
	{ "thingsound", "" },
	{ "endprintbold", "" },
	{ "activatorsound", "" },
	{ "localambientsound", "" },
	{ "setlinemonsterblocking", "" },
	{ "playerblueskull", "" },        // ???
	{ "playerredskull", "" },         // ???
	{ "playeryellowskull", "" },      // ???
	{ "playermasterskull", "" },      // ???
	{ "playerbluecard", "" },         // ???
	{ "playerredcard", "" },          // ???
	{ "playeryellowcard", "" },       // ???
	{ "playermastercard", "" },       // ???
	{ "playerblackskull", "" },       // ???
	{ "playersilverskull", "" },      // ???
	{ "playergoldskull", "" },        // ???
	{ "playerblackcard", "" },        // ???
	{ "playersilvercard", "" },       // ???
	{ "playeronteam", "" },
	{ "playerteam", "" },
	{ "playerhealth", "" },
	{ "playerarmorpoints", "" },
	{ "playerfrags", "" },
	{ "playerexpert", "" },   // ???
	{ "blueteamcount", "" },
	{ "redteamcount", "" },
	{ "blueteamscore", "" },
	{ "redteamscore", "" },
	{ "isoneflagctf", "" },
	{ "lspec6", "b" },             // ???
	{ "lspec6direct", "bwwwwww" }, // ???
	{ "printname", "" },
	{ "musicchange", "" },
	{ "team2fragpoints", "" },
	{ "consolecommand", "" },
	{ "singleplayer", "" },
	{ "fixedmul", "" },
	{ "fixeddiv", "" },
	{ "setgravity", "" },
	{ "setgravitydirect", "w" },
	{ "setaircontrol", "" },
	{ "setaircontroldirect", "w" },
	{ "clearinventory", "" },
	{ "giveinventory", "" },
	{ "giveinventorydirect", "ww" },
	{ "takeinventory", "" },
	{ "takeinventorydirect", "ww" },
	{ "checkinventory", "" },
	{ "checkinventorydirect", "w" },
	{ "spawn", "" },
	{ "spawndirect", "wwwwww" },
	{ "spawnspot", "" },
	{ "spawnspotdirect", "wwww" },
	{ "setmusic", "" },
	{ "setmusicdirect", "ww" },
	{ "localsetmusic", "" },
	{ "localsetmusicdirect", "ww" },
	{ "printfixed", "" },
	{ "printlocalized", "" },
	{ "morehudmessage", "" },
	{ "opthudmessage", "" },
	{ "endhudmessage", "" },
	{ "endhudmessagebold", "" },
	{ "setstyle", "" },
	{ "setstyledirect", "" },  // ???
	{ "setfont", "" },
	{ "setfontdirect", "w" },
	{ "pushbyte", "" },
	{ "lspec1directb", "" },
	{ "lspec2directb", "" },
	{ "lspec3directb", "" },
	{ "lspec4directb", "" },
	{ "lspec5directb", "" },
	{ "delaydirectb", "" },
	{ "randomdirectb", "" },
	{ "pushbytes", "B" },
	{ "push2bytes", "BB" },
	{ "push3bytes", "BBB" },
	{ "push4bytes", "BBBB" },
	{ "push5bytes", "BBBBB" },
	{ "setthingspecial", "" },
	{ "assignglobalvar", "b" },
	{ "pushglobalvar", "b" },
	{ "addglobalvar", "b" },
	{ "subglobalvar", "b" },
	{ "mulglobalvar", "b" },
	{ "divglobalvar", "b" },
	{ "modglobalvar", "b" },
	{ "incglobalvar", "b" },
	{ "decglobalvar", "b" },
	{ "fadeto", "" },
	{ "faderange", "" },
	{ "cancelfade", "" },
	{ "playmovie", "" },
	{ "setfloortrigger", "" },
	{ "setceilingtrigger", "" },
	{ "getactorx", "" },
	{ "getactory", "" },
	{ "getactorz", "" },
	{ "starttranslation", "" },
	{ "translationrange1", "" },
	{ "translationrange2", "" },
	{ "endtranslation", "" },
	{ "call", "b" },
	{ "calldiscard", "b" },
	{ "returnvoid", "" },
	{ "returnval", "" },
	{ "pushmaparray", "b" },
	{ "assignmaparray", "b" },
	{ "addmaparray", "b" },
	{ "submaparray", "b" },
	{ "mulmaparray", "b" },
	{ "divmaparray", "b" },
	{ "modmaparray", "b" },
	{ "incmaparray", "b" },
	{ "decmaparray", "b" },
	{ "dup", "" },
	{ "swap", "" },
	{ "writetoini", "" },  // ???
	{ "getfromini", "" },  // ???
	{ "sin", "" },
	{ "cos", "" },
	{ "vectorangle", "" },
	{ "checkweapon", "" },
	{ "setweapon", "" },
	{ "tagstring", "" },
	{ "pushworldarray", "b" },
	{ "assignworldarray", "b" },
	{ "addworldarray", "b" },
	{ "subworldarray", "b" },
	{ "mulworldarray", "b" },
	{ "divworldarray", "b" },
	{ "modworldarray", "b" },
	{ "incworldarray", "b" },
	{ "decworldarray", "b" },
	{ "pushglobalarray", "b" },
	{ "assignglobalarray", "b" },
	{ "addglobalarray", "b" },
	{ "subglobalarray", "b" },
	{ "mulglobalarray", "b" },
	{ "divglobalarray", "b" },
	{ "modglobalarray", "b" },
	{ "incglobalarray", "b" },
	{ "decglobalarray", "b" },
	{ "setmarineweapon", "" },
	{ "setactorproperty", "" },
	{ "getactorproperty", "" },
	{ "playernumber", "" },
	{ "activatortid", "" },
	{ "setmarinesprite", "" },
	{ "getscreenwidth", "" },
	{ "getscreenheight", "" },
	{ "thing_projectile2", "" },
	{ "strlen", "" },
	{ "sethudsize", "" },
	{ "getcvar", "" },
	{ "casegotosorted", "w" },  // TODO: variable param count!
	{ "setresultvalue", "" },
	{ "getlinerowoffset", "" },
	{ "getactorfloorz", "" },
	{ "getactorangle", "" },
	{ "getsectorfloorz", "" },
	{ "getsectorceilingz", "" },
	{ "lspec5result", "b" },
	{ "getsigilpieces", "" },
	{ "getlevelinfo", "" },
	{ "changesky", "" },
	{ "playeringame", "" },
	{ "playerisbot", "" },
	{ "setcameratotexture", "" },
	{ "endlog", "" },
	{ "getammocapacity", "" },
	{ "setammocapacity", "" },
	{ "printmapchararray", "" },
	{ "printworldchararray", "" },
	{ "printglobalchararray", "" },
	{ "setactorangle", "" },
	{ "grabinput", "" },         // ???
	{ "setmousepointer", "" },   // ???
	{ "movemousepointer", "" },  // ???
	{ "spawnprojectile", "" },
	{ "getsectorlightlevel", "" },
	{ "getactorceilingz", "" },
	{ "setactorposition", "" },
	{ "clearactorinventory", "" },
	{ "giveactorinventory", "" },
	{ "takeactorinventory", "" },
	{ "checkactorinventory", "" },
	{ "thingcountname", "" },
	{ "spawnspotfacing", "" },
	{ "playerclass", "" },
	{ "andscriptvar", "b" },
	{ "andmapvar", "b" },
	{ "andworldvar", "b" },
	{ "andglobalvar", "b" },
	{ "andmaparray", "b" },
	{ "andworldarray", "b" },
	{ "andglobalarray", "b" },
	{ "eorscriptvar", "b" },
	{ "eormapvar", "b" },
	{ "eorworldvar", "b" },
	{ "eorglobalvar", "b" },
	{ "eormaparray", "b" },
	{ "eorworldarray", "b" },
	{ "eorglobalarray", "b" },
	{ "orscriptvar", "b" },
	{ "ormapvar", "b" },
	{ "orworldvar", "b" },
	{ "orglobalvar", "b" },
	{ "ormaparray", "b" },
	{ "orworldarray", "b" },
	{ "orglobalarray", "b" },
	{ "lsscriptvar", "b" },
	{ "lsmapvar", "b" },
	{ "lsworldvar", "b" },
	{ "lsglobalvar", "b" },
	{ "lsmaparray", "b" },
	{ "lsworldarray", "b" },
	{ "lsglobalarray", "b" },
	{ "rsscriptvar", "b" },
	{ "rsmapvar", "b" },
	{ "rsworldvar", "b" },
	{ "rsglobalvar", "b" },
	{ "rsmaparray", "b" },
	{ "rsworldarray", "b" },
	{ "rsglobalarray", "b" },
	{ "getplayerinfo", "" },
	{ "changelevel", "" },
	{ "sectordamage", "" },
	{ "replacetextures", "" },
	{ "negatebinary", "" },
	{ "getactorpitch", "" },
	{ "setactorpitch", "" },
	{ "printbind", "" },
	{ "setactorstate", "" },
	{ "thingdamage2", "" },
	{ "useinventory", "" },
	{ "useactorinventory", "" },
	{ "checkactorceilingtexture", "" },
	{ "checkactorfloortexture", "" },
	{ "getactorlightlevel", "" },
	{ "setmugshotstate", "" },
	{ "thingcountsector", "" },
	{ "thingcountnamesector", "" },
	{ "checkplayercamera", "" },
	{ "morphactor", "" },
	{ "unmorphactor", "" },
	{ "getplayerinput", "" },
	{ "classifyactor", "" },
	{ "printbinary", "" },
	{ "printhex", "" },
	{ "callfunc", "bh" },
	{ "savestring", "" },
	{ "printmapchrange", "" },
	{ "printworldchrange", "" },
	{ "printglobalchrange", "" },
	{ "strcpytomapchrange", "" },
	{ "strcpytoworldchrange", "" },
	{ "strcpytoglobalchrange", "" },
	{ "pushfunction", "b" },
	{ "callstack", "" },
	{ "scriptwaitnamed", "" },
	{ "translationrange3", "" },
	{ "gotostack", "" },
	{ "assignscriptarray", "b" },
	{ "pushscriptarray", "b" },
	{ "addscriptarray", "b" },
	{ "subscriptarray", "b" },
	{ "mulscriptarray", "b" },
	{ "divscriptarray", "b" },
	{ "modscriptarray", "b" },
	{ "incscriptarray", "b" },
	{ "decscriptarray", "b" },
	{ "andscriptarray", "b" },
	{ "eorscriptarray", "b" },
	{ "orscriptarray", "b" },
	{ "lsscriptarray", "b" },
	{ "rsscriptarray", "b" },
	{ "printscriptchararray", "" },
	{ "printscriptchrange", "" },
	{ "playerblueskull", "" },
};


class ACSDisassembler
{
public:
	ACSDisassembler();

	bool Init(const char* const nameOrScript);
	bool Next(FString& assembly);

private:
	const FBehavior* m_module;
	const ScriptPtr* m_code;

	const int* m_startPC;
	const int* m_pc;

	bool m_endOfScript;

	int GetNextInstruction();

	int GetNextByte();
	int GetNextFormatByte();

	int GetNextShort();
	int GetNextFormatShort();

	int GetNextWord();
};

ACSDisassembler::ACSDisassembler()
: m_module(NULL)
, m_code(NULL)
, m_startPC(NULL)
, m_pc(NULL)
, m_endOfScript(true)
{
}

bool ACSDisassembler::Init(const char* const nameOrScript)
{
	const int script = IsNum(nameOrScript)
		? atoi(nameOrScript)
		: -FName(nameOrScript);

	FBehavior* module = NULL;
	m_code   = FBehavior::StaticFindScript(script, module);
	m_module = module;

	if (NULL == m_code || NULL == m_module)
	{
		return false;
	}

	m_pc = m_startPC = module->GetScriptAddress(m_code);
	m_endOfScript = NULL == m_pc;

	return !m_endOfScript;
}

bool ACSDisassembler::Next(FString& assembly)
{
	if (m_endOfScript)
	{
		return false;
	}

	const DWORD addr = sizeof(int) * static_cast<DWORD>(m_pc - m_startPC) + m_code->Address;
	const int pcd = GetNextInstruction();

	if (pcd < 0 || pcd >= DLevelScript::PCODE_COMMAND_COUNT)
	{
		assembly.Format("$%04x: <unknown>", addr);
		m_endOfScript = true;
		return true;
	}

	const PCode& pcode = PCODES[pcd];
	assembly.Format("$%04x: %s", addr, pcode.name);

	for (const char* c = pcode.args; 0 != *c; ++c)
	{
		assembly += pcode.args == c ? " " : ", ";

		switch (*c)
		{
		case 'a':
			assembly.AppendFormat("$%04x", GetNextWord());
			break;

		case 'B':
			assembly.AppendFormat("%d", GetNextByte());
			break;

		case 'b':
			assembly.AppendFormat("%d", GetNextFormatByte());
			break;

		case 'H':
			assembly.AppendFormat("%d", GetNextShort());
			break;

		case 'h':
			assembly.AppendFormat("%d", GetNextFormatShort());
			break;

		case 'w':
			assembly.AppendFormat("%d", GetNextWord());
			break;

		default:
			assert(!"Unknown parameter type");
			return false;
		}
	}

	m_endOfScript = DLevelScript::PCD_TERMINATE == pcd;

	return true;
}

int ACSDisassembler::GetNextInstruction()
{
	if (ACS_LittleEnhanced == m_module->GetFormat())
	{
		static const int ACS_LE_PCODE = 256 - 16;

		int result = GetNextByte();

		if (result >= ACS_LE_PCODE)
		{
			result = ACS_LE_PCODE + ((result - ACS_LE_PCODE) << 8) + GetNextByte();
		}

		return result;
	}

	return GetNextWord();
}

int ACSDisassembler::GetNextByte()
{
	const BYTE* const bytePtr = reinterpret_cast<const BYTE*>(m_pc);
	m_pc = reinterpret_cast<const int*>(bytePtr + 1);
	return *bytePtr;
}

int ACSDisassembler::GetNextFormatByte()
{
	return ACS_LittleEnhanced == m_module->GetFormat()
		? GetNextByte()
		: GetNextWord();
}

int ACSDisassembler::GetNextShort()
{
	const SWORD* const shortPtr = reinterpret_cast<const SWORD*>(m_pc);
	m_pc = reinterpret_cast<const int*>(shortPtr + 1);
	return LittleShort(*shortPtr);
}

int ACSDisassembler::GetNextFormatShort()
{
	return ACS_LittleEnhanced == m_module->GetFormat()
		? GetNextShort()
		: GetNextWord();
}

int ACSDisassembler::GetNextWord()
{
	// Potentially unaligned read
#if defined _M_IX86 || defined _M_X64 || defined __i386__ || defined __x86_64__
	const int result = *m_pc;
#else
	const BYTE* const bytes = reinterpret_cast<const BYTE*>(m_pc);
	const int result = bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
#endif

	++m_pc;

	return LittleLong(result);
}

} // unnamed namespace


CCMD(acsdisasm)
{
	const int argc = argv.argc();

	if (argc < 2 || argc > 6)
	{
		Printf("Usage: acsdisasm <script> ...\n");
		return;
	}

	for (int i = 1; i < argc; ++i)
	{
		if (i > 1)
		{
			Printf("\n");
		}

		ACSDisassembler disasm;

		if (!disasm.Init(argv[i]))
		{
			// TODO: report error
			continue;
		}

		// TODO: print header

		FString assembly;

		while (disasm.Next(assembly))
		{
			Printf("%s\n", assembly.GetChars());
		}
	}
}
