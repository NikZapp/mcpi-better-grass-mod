// just some useful headers
#include <cstdio>
#include <algorithm>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>
#include <mods/misc/misc.h>
//#include "helpers.h"
#include <GLES/gl.h>
//#include <GLFW/glfw3.h>

extern "C"{
    unsigned char *get_minecraft();
    std::string get_world_name();
    bool in_local_world();
}

// Change Grass Color
static int32_t get_color(unsigned char *level_source, int32_t x, int32_t z) {
    unsigned char *level_source_vtable = *(unsigned char **) level_source;
    LevelSource_getBiome_t LevelSource_getBiome = *(LevelSource_getBiome_t *) (level_source_vtable + LevelSource_getBiome_vtable_offset);
    unsigned char *biome = (*LevelSource_getBiome)(level_source, x, z);
    if (biome == NULL) {
        return 0;
    }
    return *(int32_t *) (biome + Biome_color_property_offset);
}

#define BIOME_BLEND_SIZE 5
#define COLOR_RANDOM_RANGE 30
static Level_getTile_t Level_getData = (Level_getTile_t) 0xa3324;
static int32_t GrassTile_getColor_injection(__attribute__((unused)) unsigned char *tile, unsigned char *level_source, int32_t x, __attribute__((unused)) int32_t y, int32_t z) {
    int r_sum = 0;
    int g_sum = 0;
    int b_sum = 0;
    int color_sum = 0;
    int x_start = x - (BIOME_BLEND_SIZE / 2);
    int z_start = z - (BIOME_BLEND_SIZE / 2);
    for (int x_offset = 0; x_offset < BIOME_BLEND_SIZE; x_offset++) {
        for (int z_offset = 0; z_offset < BIOME_BLEND_SIZE; z_offset++) {
            int32_t color = get_color(level_source, x_start + x_offset, z_start + z_offset);
            r_sum += (color >> 16) & 0xff;
            g_sum += (color >> 8) & 0xff;
            b_sum += color & 0xff;
            color_sum++;
        }
    }

    int r_avg = r_sum / color_sum;
    int g_avg = g_sum / color_sum;
    int b_avg = b_sum / color_sum;
    r_avg += (rand() % COLOR_RANDOM_RANGE) - (COLOR_RANDOM_RANGE / 2);
    g_avg += (rand() % COLOR_RANDOM_RANGE) - (COLOR_RANDOM_RANGE / 2);
    b_avg += (rand() % COLOR_RANDOM_RANGE) - (COLOR_RANDOM_RANGE / 2);
    unsigned char *level = *(unsigned char **) (get_minecraft() + Minecraft_level_property_offset);
    int depth = 0;
	if ((Level_getTile(level, x, y, z) == 2 && Level_getData(level, x, y, z) == 1) ||
		(Level_getTile(level, x, y - 1, z) == 2 && Level_getData(level, x, y - 1, z) == 1)) {
		if (Level_getTile(level, x, y, z) == 31) depth = 1;
		r_avg = Level_getData(level, x, y - 1 - depth, z) * 0x11;
		g_avg = Level_getData(level, x, y - 2 - depth, z) * 0x11;
		b_avg = Level_getData(level, x, y - 3 - depth, z) * 0x11;
	}
    r_avg = std::min(0xff, std::max(0x00, r_avg));
    g_avg = std::min(0xff, std::max(0x00, g_avg));
    b_avg = std::min(0xff, std::max(0x00, b_avg));
    return (r_avg << 16) | (g_avg << 8) | b_avg;
}

static int32_t TallGrass_getColor_injection(unsigned char *tile, unsigned char *level_source, int32_t x, int32_t y, int32_t z) {
    int32_t original_color = (*TallGrass_getColor)(tile, level_source, x, y, z);
    if (original_color == 0x339933) {
        return GrassTile_getColor_injection(tile, level_source, x, y, z);
    } else {
        return original_color;
    }
}


static void add_grass_item(unsigned char *filling_container) {
    ItemInstance *dyed_grass_instance = new ItemInstance;
    ALLOC_CHECK(dyed_grass_instance);
    dyed_grass_instance->count = 255;
    dyed_grass_instance->auxiliary = 1;
    dyed_grass_instance->id = 2;
    (*FillingContainer_addItem)(filling_container, dyed_grass_instance);
}

static void grass_recipes_injection(unsigned char *recipes) {

	// Add grass related recipes
    Recipes_Type ingredient_dirt = {
        .item = 0,
        .tile = 0,
        .instance = {
            .count = 1,
            .id = 3,
            .auxiliary = 0
        },
        .letter = 'a'
    };
    Recipes_Type ingredient_seeds = {
        .item = 0,
        .tile = 0,
        .instance = {
            .count = 1,
            .id = 295,
            .auxiliary = 0
        },
        .letter = 'b'
    };
    ItemInstance result_grass_block = {
        .count = 1,
        .id = 2,
        .auxiliary = 0
    };
    (*Recipes_addShapelessRecipe)(recipes, result_grass_block, {ingredient_dirt, ingredient_seeds});

    ItemInstance result_grass = {
		.count = 1,
		.id = 31,
		.auxiliary = 1
	};
    (*Recipes_addShapelessRecipe)(recipes, result_grass, {ingredient_seeds});

    ItemInstance result_fern = {
		.count = 1,
		.id = 31,
		.auxiliary = 3
	};
    (*Recipes_addShapelessRecipe)(recipes, result_fern, {ingredient_seeds});

    Recipes_Type ingredient_grass_block = {
		.item = 0,
		.tile = 0,
		.instance = {
			.count = 1,
			.id = 2,
			.auxiliary = 0
		},
		.letter = 'a'
	};

	Recipes_Type ingredient_lightblue_dye = {
		.item = 0,
		.tile = 0,
		.instance = {
			.count = 1,
			.id = 351,
			.auxiliary = 9
		},
		.letter = 'a'
	};

	Recipes_Type ingredient_lime_dye = {
		.item = 0,
		.tile = 0,
		.instance = {
			.count = 1,
			.id = 351,
			.auxiliary = 10
		},
		.letter = 'a'
	};

	Recipes_Type ingredient_pink_dye = {
		.item = 0,
		.tile = 0,
		.instance = {
			.count = 1,
			.id = 351,
			.auxiliary = 12
		},
		.letter = 'a'
	};

    ItemInstance result_dyed_grass_block = {
		.count = 1,
		.id = 2,
		.auxiliary = 1
	};
    (*Recipes_addShapelessRecipe)(recipes, result_dyed_grass_block, {ingredient_grass_block, ingredient_lightblue_dye, ingredient_lime_dye, ingredient_pink_dye});
}


__attribute__((constructor)) static void init() {
    misc_run_on_recipes_setup(grass_recipes_injection);
    patch_address((void *) GrassTile_getColor_vtable_addr, (void *) GrassTile_getColor_injection);
    patch_address((void *) TallGrass_getColor_vtable_addr, (void *) TallGrass_getColor_injection);
    misc_run_on_creative_inventory_setup(add_grass_item);

    INFO("%s", "Nik's better grass mod loaded!");
}
