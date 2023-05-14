/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_sprite.cpp
 *     Graphics sprites support library.
 * @par Purpose:
 *     Functions for reading/writing, decoding/encodeing of sprites.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     25 Nov 2008 - 09 Jan 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "bflib_basics.h"
#include "bflib_dernc.h"
#include "bflib_sprite.h"
#include <string>
#include <stdexcept>
#include <vector>
#include "post_inc.h"

namespace {

#pragma packed(1)
struct TbRawSprite {
	uint32_t offset;
#ifdef SPRITE_FORMAT_V2
	uint16_t width;
	uint16_t height;
#else
	uint8_t width;
	uint8_t height;
#endif
};
#pragma packed()

#ifdef SPRITE_FORMAT_V2
const auto raw_sprite_size = 8;
#else
const auto raw_sprite_size = 6;
#endif

} // local

struct SpriteSheet {

	std::vector<uint8_t> data;
	std::vector<TbSprite> sprites;

	SpriteSheet(
		const std::string & basename
	) {
		load_data(basename + ".dat");
		load_table(basename + ".tab");
	}

	void load_data(
		const std::string & filename
	) {
		const auto unpacked_size = LbFileLengthRnc(filename.c_str());
		if (unpacked_size < 0) {
			throw std::runtime_error("Cannot access sprite data " + filename);
		} else if (unpacked_size == 0) {
			throw std::runtime_error("Sprite data " + filename + " empty");
		}
		data.resize(unpacked_size);
		if (LbFileLoadAt(filename.c_str(), data.data()) != data.size()) {
			throw std::runtime_error("Error loading sprite data from " + filename);
		}
	}

	void load_table(
		const std::string & filename
	) {
		const auto unpacked_size = LbFileLengthRnc(filename.c_str());
		if (unpacked_size < 0) {
			throw std::runtime_error("Cannot access sprite table " + filename);
		} else if (unpacked_size == 0) {
			throw std::runtime_error("Sprite table " + filename + " empty");
		}
		std::vector<uint8_t> raw(unpacked_size);
		if (LbFileLoadAt(filename.c_str(), raw.data()) != raw.size()) {
			throw std::runtime_error("Error loading sprite table from " + filename);
		}
		const auto num_sprites = raw.size() / raw_sprite_size;
		sprites.resize(num_sprites);
		for (int i = 0; i < num_sprites; ++i) {
			const auto src = reinterpret_cast<const TbRawSprite *>(&raw[i * raw_sprite_size]);
			auto & dst = sprites[i];
			dst.SWidth = src->width;
			dst.SHeight = src->height;
			dst.Data = &data[src->offset];
		}
	}
};

extern "C" SpriteSheet * LoadSprites(
	const char * basename
) {
	try {
		return new SpriteSheet(basename);
	} catch (const std::exception & e) {
		ERRORLOG("%s", e.what());
	}
	return nullptr;
}

extern "C" void DeleteSprites(
	SpriteSheet ** sheet
) {
	if (sheet && *sheet) {
		delete *sheet;
		*sheet = nullptr;
	}
}

extern "C" const struct TbSprite * GetSprite(
	const struct SpriteSheet * sheet,
	size_t index
) {
	if (sheet && index < sheet->sprites.size()) {
		return &sheet->sprites[index];
	}
	return nullptr;
}

size_t CountSprites(
	const struct SpriteSheet * sheet
) {
	if (sheet) {
		return sheet->sprites.size();
	}
	return 0;
}