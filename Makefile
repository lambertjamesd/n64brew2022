#!smake
# --------------------------------------------------------------------
#        Copyright (C) 1998 Nintendo. (Originated by SGI)
#        
#        $RCSfile: Makefile,v $
#        $Revision: 1.1.1.1 $
#        $Date: 2002/05/02 03:27:21 $
# --------------------------------------------------------------------
include /usr/include/n64/make/PRdefs

MIDICVT:=tools/midicvt
SFZ2N64:=tools/sfz2n64
SKELATOOL64:=tools/skeletool64
BLENDER_2_9:=tools/blender/blender

OPTIMIZER		:= -O0
LCDEFS			:= -DDEBUG -g -Isrc/ -I/usr/include/n64/nustd -Werror -Wall
N64LIB			:= -lultra_rom -lnustd

ifeq ($(WITH_DEBUGGER),1)
LCDEFS += -DWITH_DEBUGGER
endif

BASE_TARGET_NAME = build/game

LD_SCRIPT	= game.ld
CP_LD_SCRIPT	= build/game

ASMFILES    =	$(shell find asm/ -type f -name '*.s')

ASMOBJECTS  =	$(patsubst %.s, build/%.o, $(ASMFILES))

CODEFILES = $(shell find src/ -type f -name '*.c')

ifeq ($(WITH_GFX_VALIDATOR),1)
LCDEFS += -DWITH_GFX_VALIDATOR
CODEFILES += gfxvalidator/validator.c gfxvalidator/error_printer.c gfxvalidator/command_printer.c
endif

CODEOBJECTS = $(patsubst %.c, build/%.o, $(CODEFILES))

CODESEGMENT =	build/codesegment

BOOT		=	/usr/lib/n64/PR/bootcode/boot.6102
BOOT_OBJ	=	build/boot.6102.o

OBJECTS		=	$(ASMOBJECTS) $(BOOT_OBJ)

DEPS = $(patsubst %.c, build/%.d, $(CODEFILES)) $(patsubst %.c, build/%.d, $(DATAFILES))

-include $(DEPS)

LCINCS =	-I/usr/include/n64/PR 
LCDEFS +=	-DF3DEX_GBI_2
#LCDEFS +=	-DF3DEX_GBI_2 -DFOG
#LCDEFS +=	-DF3DEX_GBI_2 -DFOG -DXBUS
#LCDEFS +=	-DF3DEX_GBI_2 -DFOG -DXBUS -DSTOP_AUDIO

LDIRT  =	$(BASE_TARGET_NAME).elf $(CP_LD_SCRIPT) $(BASE_TARGET_NAME).z64 $(BASE_TARGET_NAME)_no_debug.map $(ASMOBJECTS)

LDFLAGS =	-L/usr/lib/n64 $(N64LIB)  -L$(N64_LIBGCCDIR) -lgcc

default:	$(BASE_TARGET_NAME).z64

include $(COMMONRULES)

.s.o:
	$(AS) -Wa,-Iasm -o $@ $<

build/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -MM $^ -MF "$(@:.o=.d)" -MT"$@"
	$(CC) $(CFLAGS) -c -o $@ $<

build/%.o: %.s
	@mkdir -p $(@D)
	$(AS) -Wa,-Iasm -o $@ $<

####################
## Assets
####################

LEVEL_LIST = assets/levels/tutorial.blend \
	assets/levels/level_2.blend \
	assets/levels/level_3.blend \
	assets/levels/level_4.blend \
	assets/levels/level_5.blend

LEVEL_LIST_FBX = $(LEVEL_LIST:%.blend=build/%.fbx)
LEVEL_LIST_HEADERS = $(LEVEL_LIST:%.blend=build/%.h)
LEVEL_LIST_OBJECTS = $(LEVEL_LIST:%.blend=build/%_geo.o)

MODEL_LIST = assets/models/conveyor.blend \
	assets/models/table.blend \
	assets/models/table_vertical.blend \
	assets/models/player.blend \
	assets/models/pumpkin.blend \
	assets/models/hat.blend \
	assets/models/brain.blend \
	assets/models/broom.blend \
	assets/models/candle.blend \
	assets/models/cat.blend \
	assets/models/cobweb.blend \
	assets/models/crow.blend \
	assets/models/hand.blend \
	assets/models/rat.blend \
	assets/models/return_bin.blend \
	assets/models/scarecrow.blend \
	assets/models/skull.blend \
	assets/models/spider.blend \
	assets/models/ui/item_prompt.blend \
	assets/models/ui/item_drop_prompt.blend \
	assets/models/ghostjeff.blend \
	assets/models/lantern.blend \
	assets/models/portal.blend

MODEL_HEADERS = $(MODEL_LIST:%.blend=build/%.h)
MODEL_FBX = $(MODEL_LIST:%.blend=build/%.fbx)
MODEL_OBJECTS = $(MODEL_LIST:%.blend=build/%_geo.o)

####################
## Materials
####################

build/assets/materials/pallete.png: assets/materials/half_pallete.png
	@mkdir -p $(@D)
	convert assets/materials/half_pallete.png -sample 48x5\! build/assets/materials/pallete.png

GENERATED_IMAGES = build/assets/materials/pallete.png

IMAGE_LIST = $(shell find assets/ -type f -name '*.png')

ALL_IMAGES = $(GENERATED_IMAGES) $(IMAGE_LIST)

build/assets/materials/static.h: assets/materials/static.skm.yaml $(LEVEL_LIST_FBX) $(MODEL_FBX) $(SKELATOOL64) $(ALL_IMAGES)
	@mkdir -p $(@D)
	$(SKELATOOL64) --name static --ci-buffer -m $< $(LEVEL_LIST_FBX:%=-m %) $(MODEL_FBX:%=-m %) --pallete build/assets/materials/pallete.png --material-output -o build/assets/materials/static.h

build/assets/materials/pallete.h: assets/materials/pallete.skm.yaml assets/materials/half_pallete.png
	@mkdir -p $(@D)
	$(SKELATOOL64) --name pallete -m $< --material-output -o build/assets/materials/pallete.h

build/assets/materials/ui.h: assets/materials/ui.skm.yaml $(SKELATOOL64) $(ALL_IMAGES)
	@mkdir -p $(@D)
	$(SKELATOOL64) --name ui -m $< --material-output -o build/assets/materials/ui.h --default-material default_ui

build/src/level/level.o: build/assets/materials/static.h build/assets/levels/level_list.h

build/src/scene/scene.o: build/assets/materials/static.h build/assets/materials/pallete.h build/assets/materials/ui.h build/src/audio/clips.h

build/src/menu/ui.o: build/assets/materials/ui.h

build/src/ui/nightchilde.o: build/assets/materials/ui.h

build/src/scene/pause_menu.o: build/assets/materials/ui.h

####################
## Models
####################

ANIM_LIST = build/assets/models/player_anim.o \
	build/assets/models/cat_anim.o \
	build/assets/models/crow_anim.o \
	build/assets/models/hand_anim.o \
	build/assets/models/rat_anim.o \
	build/assets/models/spider_anim.o \
	build/assets/models/ghostjeff_anim.o \
	build/assets/models/portal_anim.o

build/assets/models/%.h build/assets/models/%_geo.c build/assets/models/%_anim.c: build/assets/models/%.fbx assets/models/%.flags assets/materials/static.skm.yaml $(ALL_IMAGES) $(SKELATOOL64)
	$(SKELATOOL64) --fixed-point-scale 256 --model-scale 0.01 --name $(<:build/assets/models/%.fbx=%) -m $< $(shell cat $(<:build/assets/models/%.fbx=assets/models/%.flags)) -o $(<:%.fbx=%.h) $<


build/src/scene/player.o: build/assets/models/player.h build/assets/models/ui/item_drop_prompt.h build/assets/materials/static.h
build/src/scene/conveyor.o: build/assets/models/conveyor.h build/assets/materials/static.h 
build/src/scene/spot_light.o: build/assets/models/lantern.h build/assets/materials/static.h 
build/src/scene/table.o: build/assets/models/table.h build/assets/models/table_vertical.h build/assets/materials/static.h tools/table_export.lua
build/src/scene/item_requester.o: build/assets/models/ui/item_prompt.h build/assets/models/portal.h build/assets/materials/static.h
build/src/scene/shadow_volume_group.o: build/assets/materials/static.h 
build/src/scene/tutorial.o: build/assets/materials/ui.h 
build/src/scene/end_screen.o: build/assets/materials/ui.h 
build/src/scene/return_bin.o: build/assets/models/return_bin.h build/assets/materials/static.h 

build/src/scene/item.o: build/assets/materials/static.h \
	build/assets/models/pumpkin.h \
	build/assets/models/hat.h \
	build/assets/models/brain.h \
	build/assets/models/broom.h \
	build/assets/models/candle.h  \
	build/assets/models/cat.h \
	build/assets/models/cobweb.h \
	build/assets/models/crow.h \
	build/assets/models/hand.h \
	build/assets/models/rat.h \
	build/assets/models/scarecrow.h \
	build/assets/models/skull.h \
	build/assets/models/spider.h

build/src/scene/bezos.o: build/assets/models/ghostjeff.h build/assets/materials/static.h

build/anims.ld: $(ANIM_LIST) tools/generate_animation_ld.js
	@mkdir -p $(@D)
	node tools/generate_animation_ld.js $@ $(ANIM_LIST)

####################
## Levels
####################

LEVEL_GENERATION_SCRIPT = $(shell find tools/generate_level/ -type f -name '*.lua')

build/%.fbx: %.blend
	@mkdir -p $(@D)
	$(BLENDER_2_9) $< --background --python tools/export_fbx.py -- $@

build/assets/levels/%.h build/assets/levels/%_geo.c: build/assets/levels/%.fbx assets/levels/%.json assets/materials/static.skm.yaml build/assets/materials/static.h build/src/audio/clips.h tools/generate_level.lua $(LEVEL_GENERATION_SCRIPT) $(SKELATOOL64) $(ALL_IMAGES)
	@mkdir -p $(@D)
	$(SKELATOOL64) --script tools/generate_level.lua --ci-buffer --fixed-point-scale 256 --model-scale 0.01 --name $(<:build/assets/levels/%.fbx=%) -m assets/materials/static.skm.yaml --pallete build/assets/materials/pallete.png -o $(<:%.blend=build/%.h) $<

build/assets/levels/%.o: build/assets/levels/%.c build/assets/materials/static.h
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -MM $^ -MF "$(@:.o=.d)" -MT"$@"
	$(CC) $(CFLAGS) -c -o $@ $<

build/levels.ld: $(LEVEL_LIST_OBJECTS) tools/generate_level_ld.js
	@mkdir -p $(@D)
	node tools/generate_level_ld.js $@ $(LEVEL_LIST_OBJECTS)

build/assets/levels/level_list.h: $(LEVEL_LIST_HEADERS) tools/generate_level_list.js
	@mkdir -p $(@D)
	node tools/generate_level_list.js $@ $(LEVEL_LIST_HEADERS)

####################
## Sounds
####################


SOUND_CLIPS = $(shell find assets/ -type f -name '*.wav') $(shell find assets/ -type f -name '*.aif')

build/assets/sound/sounds.sounds build/assets/sound/sounds.sounds.tbl: $(SOUND_CLIPS)
	@mkdir -p $(@D)
	$(SFZ2N64) --compress -o $@ $^


build/asm/sound_data.o: build/assets/sound/sounds.sounds build/assets/sound/sounds.sounds.tbl

build/src/audio/clips.h: tools/generate_sound_ids.js $(SOUND_CLIPS)
	@mkdir -p $(@D)
	node tools/generate_sound_ids.js -o $@ -p SOUNDS_ $(SOUND_CLIPS)


build/src/menu/main_menu.o: build/src/audio/clips.h

####################
## Linking
####################

MATERIAL_OBJECTS = build/assets/materials/static_mat.o build/assets/materials/pallete_mat.o build/assets/materials/ui_mat.o

DATA_OBJECTS = $(MODEL_OBJECTS) 

$(BOOT_OBJ): $(BOOT)
	$(OBJCOPY) -I binary -B mips -O elf32-bigmips $< $@

# without debugger

CODEOBJECTS_NO_DEBUG = $(CODEOBJECTS) $(DATA_OBJECTS)

ifeq ($(WITH_DEBUGGER),1)
CODEOBJECTS_NO_DEBUG += build/debugger/debugger_stub.o build/debugger/serial.o 
endif

$(CODESEGMENT)_no_debug.o:	$(CODEOBJECTS_NO_DEBUG)
	$(LD) -o $(CODESEGMENT)_no_debug.o -r $(CODEOBJECTS_NO_DEBUG) $(LDFLAGS)


$(CP_LD_SCRIPT)_no_debug.ld: $(LD_SCRIPT) build/levels.ld build/anims.ld
	cpp -P -Wno-trigraphs $(LCDEFS) -DCODE_SEGMENT=$(CODESEGMENT)_no_debug.o -o $@ $<

$(BASE_TARGET_NAME).z64: $(CODESEGMENT)_no_debug.o $(OBJECTS) $(LEVEL_LIST_OBJECTS) $(CP_LD_SCRIPT)_no_debug.ld $(MATERIAL_OBJECTS)
	$(LD) -L. -T $(CP_LD_SCRIPT)_no_debug.ld -Map $(BASE_TARGET_NAME)_no_debug.map -o $(BASE_TARGET_NAME).elf
	$(OBJCOPY) --pad-to=0x100000 --gap-fill=0xFF $(BASE_TARGET_NAME).elf $(BASE_TARGET_NAME).z64 -O binary
	makemask $(BASE_TARGET_NAME).z64

# with debugger
CODEOBJECTS_DEBUG = $(CODEOBJECTS) $(DATA_OBJECTS)

ifeq ($(WITH_DEBUGGER),1)
CODEOBJECTS_DEBUG += build/debugger/debugger.o build/debugger/serial.o 
endif

$(CODESEGMENT)_debug.o:	$(CODEOBJECTS_DEBUG)
	$(LD) -o $(CODESEGMENT)_debug.o -r $(CODEOBJECTS_DEBUG) $(LDFLAGS)

$(CP_LD_SCRIPT)_debug.ld: $(LD_SCRIPT) build/levels.ld build/anims.ld
	cpp -P -Wno-trigraphs $(LCDEFS) -DCODE_SEGMENT=$(CODESEGMENT)_debug.o -o $@ $<

$(BASE_TARGET_NAME)_debug.z64: $(CODESEGMENT)_debug.o $(OBJECTS) $(LEVEL_LIST_OBJECTS) $(CP_LD_SCRIPT)_debug.ld $(MATERIAL_OBJECTS)
	$(LD) -L. -T $(CP_LD_SCRIPT)_debug.ld -Map $(BASE_TARGET_NAME)_debug.map -o $(BASE_TARGET_NAME)_debug.elf
	$(OBJCOPY) --pad-to=0x100000 --gap-fill=0xFF $(BASE_TARGET_NAME)_debug.elf $(BASE_TARGET_NAME)_debug.z64 -O binary
	makemask $(BASE_TARGET_NAME)_debug.z64

clean:
	rm -rf build