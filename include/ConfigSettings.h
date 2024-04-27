/*-----------------------------------------------------------------------------
	6502 Macroassembler and Simulator

Copyright (C) 1995-2003 Michal Kowalski

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
-----------------------------------------------------------------------------*/

#ifndef CONFIG_SETTINGS_H__
#define CONFIG_SETTINGS_H__

namespace CConfigSettings
{
    extern wxFontInfo *const fonts[];    // LOGFONT structures throughout the program
    extern wxFont *const cfonts[];     // CFont structures throughout the program
    extern wxColour *text_color[];
    extern wxColour *bkgnd_color[];
    extern wxColour *color_syntax[];
    extern uint8_t *syntax_font_style[];
};

#endif /* CONFIG_SETTINGS_H__ */
