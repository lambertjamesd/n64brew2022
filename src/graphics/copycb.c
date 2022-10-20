#include "copycb.h"

#include "graphics.h"

// group u16 to u64
// 0x([0-9a-f]{4}), 0x([0-9a-f]{4}), 0x([0-9a-f]{4}), 0x([0-9a-f]{4})
// 0x$1$2$3$4

u64 gFirePallete[] = {
    0x0001000100010001, 0x0801080108010801, 0x1001100110011801, 0x1801180118012001, 
    0x2001200120012801, 0x2801280130013001, 0x3001380138013801, 0x3801400140014001, 
    0x4001480148014801, 0x5001500150015001, 0x5801580160016801, 0x7001780180018801, 
    0x90019801a001a801, 0xb001b001b001b001, 0xb841b841b841b841, 0xb881b881b881c083, 

    0xc0c3c0c3c0c3c103, 0xc103c103c103c943, 0xc945c945c985c985, 0xc985c985d1c5d1c5, 
    0xd1c5d205d207d207, 0xd207da07da47da47, 0xda47da47da47da87, 0xda87da87da87da87, 
    0xda87e2c7e2c7e2c9, 0xe2c9e2c9e2c9e309, 0xe309e309e309e309, 0xe349eb49eb49eb49, 
    0xeb49eb49eb8beb8b, 0xeb8beb8beb8bf3cb, 0xf3cbf3cbf3cbf3cb, 0xf3cbf40bf40bf40b, 

    0xf40bf40bf40df44b, 0xf44bf44bf44bf44b, 0xf48bf48bf48bf48b, 0xf48bf4cbf4cbf4cb, 
    0xf4cbf4cbf50bf509, 0xf509f509f509f549, 0xf549f549f549f549, 0xf589f589f589f589, 
    0xf589f5c9f5c9f5c7, 0xf5c7f5c7f5c7f607, 0xf607f607f607f607, 0xf647f647f647f647, 
    0xf647f687f687f687, 0xf687f687f687f6c5, 0xf6c5f6c5f6c5f6c5, 0xf6c5f705f705f705, 

    0xf705f705f705f707, 0xf747f747f749f749, 0xf74bf74bf74bf74d, 0xf74df74ff74ff74f, 
    0xf751f751f751f753, 0xf753f755f755f757, 0xf757f757f759f759, 0xf759f75bf75bf75d, 
    0xf75df75df75ff75f, 0xf75ff761f761f763, 0xf763f763f765f765, 0xf765f767f767f767, 
    0xf769f769f76bf76b, 0xf76bf76df76df76d, 0xf76ff76ff76ff771, 0xf771f773f773f773
};

u64 gToonPallete[] = {
    0x18c718c718c718c7, 0x08cb08cb32573257, 0x0a550a5545674567, 0x62036203e615e615,
    0x70c570c5db91db91, 0x784f784fe1d5e1d5, 0x18c718c718c718c7, 0x18c718c718c718c7, 
    0x18c718c718c718c7, 0x18c718c718c718c7, 0x18c718c718c718c7, 0x18c718c718c718c7, 
    0x18c718c718c718c7, 0x18c718c718c718c7, 0x18c718c718c718c7, 0x18c718c718c718c7, 

    0x18c718c718c718c7, 0x18c718c718c718c7, 0x18c718c718c718c7, 0x18c718c718c718c7, 
    0x18c718c718c718c7, 0x18c718c718c718c7, 0x18c718c718c718c7, 0x18c718c718c718c7, 
    0x18c718c718c718c7, 0x18c718c718c718c7, 0x18c718c718c718c7, 0x18c718c718c718c7, 
    0x18c718c718c718c7, 0x18c718c718c718c7, 0x18c718c718c718c7, 0x18c718c718c718c7, 
    
    0x18c718c718c718c7, 0x18c718c718c718c7, 0x18c718c718c718c7, 0x18c718c718c718c7, 
    0x18c718c718c718c7, 0x18c718c718c718c7, 0x18c718c718c718c7, 0x18c718c718c718c7, 
    0x18c718c718c718c7, 0x18c718c718c718c7, 0x18c718c718c718c7, 0x18c718c718c718c7, 
    0x18c718c718c718c7, 0x18c718c718c718c7, 0x18c718c718c718c7, 0x18c718c718c718c7, 
    
    0x18c718c718c718c7, 0x18c718c718c718c7, 0x18c718c718c718c7, 0x18c718c718c718c7, 
    0x18c718c718c718c7, 0x18c718c718c718c7, 0x18c718c718c718c7, 0x18c718c718c718c7, 
    0x18c718c718c718c7, 0x18c718c718c718c7, 0x18c718c718c718c7, 0x18c718c718c718c7, 
    0x18c718c718c718c7, 0x18c718c718c718c7, 0x18c718c718c718c7, 0xffffffffffffffff,
};

u64 gIcePallete[] = {
    0x000b000b000b000b, 0x000b000b000b000b, 0x000b000b000b000b, 0x004d004d004d004d, 
    0x004d004d004d004d, 0x004d004d004d004d, 0x004d004f004f008f, 0x004f008f008f008f, 
    0x008f008f088f008f, 0x088f088f088f088f, 0x0891089108910891, 0x08d108d108d108d1, 
    0x08d108d108d108d1, 0x08d108d308d308d3, 0x08d308d308d308d3, 0x08d3091309130913, 
    
    0x0913091309130915, 0x1115091509151115, 0x1115111511151115, 0x1115115511551157, 
    0x1157115711571157, 0x1157115711571157, 0x1157115711571157, 0x1197119711991199, 
    0x119919991999199b, 0x19db19db19db19dd, 0x19dd1a1d1a1d1a1f, 0x1a1f221f225f225f, 
    0x22612261226122a1, 0x22a322a322a322a3, 0x22e52ae52ae52ae5, 0x2ae52ae72b272b27, 
    
    0x2b272b272b272b27, 0x2b272b672b672b67, 0x2b67236923692369, 0x23a923a923a923a9, 
    0x23a923a923a923a9, 0x23eb23eb23eb23eb, 0x23eb23eb23eb23eb, 0x242b242b242b242b, 
    0x242b1c2b1c2d1c2d, 0x1c6d1c6d1c6d1c6d, 0x1c6d1c6d1c6d1cad, 0x1cad1caf1caf1caf, 
    0x1caf1caf1cef1cef, 0x1cef1cef1cef1cef, 0x1cef1cef1cef1d31, 0x1d31153115311531, 
    
    0x1531153115711571, 0x1571157115711573, 0x157315b315b315b3, 0x15b315b315b315f3, 
    0x15f315f315f50df5, 0x0df50df50df50e35, 0x0e350e350e350e35, 0x0e350e370e770e77, 
    0x0e770e770e770e77, 0x0eb70eb70eb70eb7, 0x06b706b906b906f9, 0x06f906f906f906f9, 
    0x06f906f907390739, 0x0739073b073b073b, 0x073b077b077b077b, 0x077b077b077b077d,
};

u64 gYesWeCan[] = {
    0x18c718c718c718c7, 0x18c718c718c718c7, 0x18c718c718c718c7, 0x18c718c718c718c7, 
    0x18c718c718c718c7, 0x18c718c718c718c7, 0x18c718c718c718c7, 0x18c718c718c718c7, 
    0x18c718c718c718c7, 0x18c718c718c718c7, 0x18c718c718c718c7, 0x18c718c718c718c7, 
    0x18c718c718c718c7, 0x18c718c718c718c7, 0x18c718c718c718c7, 0x18c718c718c718c7, 
    
    0x0193019301930193, 0x0193019301930193, 0x0193019301930193, 0x0193019301930193, 
    0xc909c909c909c909, 0xc909c909c909c909, 0xc909c909c909c909, 0xc909c909c909c909, 
    0xc909c909c909c909, 0xc909c909c909c909, 0xc909c909c909c909, 0xc909c909c909c909, 
    0x74e974e974e974e9, 0x74e974e974e974e9, 0x74e974e974e974e9, 0x74e974e974e974e9, 
    
    0x74e974e974e974e9, 0x74e974e974e974e9, 0x74e974e974e974e9, 0x74e974e974e974e9, 
    0xbde7bde7bde7bde7, 0xbde7bde7bde7bde7, 0xbde7bde7bde7bde7, 0xbde7bde7bde7bde7, 
    0xf6e9f6e9f6e9f6e9, 0xf6e9f6e9f6e9f6e9, 0xf6e9f6e9f6e9f6e9, 0xf6e9f6e9f6e9f6e9, 
    0xf6e9f6e9f6e9f6e9, 0xf6e9f6e9f6e9f6e9, 0xf6e9f6e9f6e9f6e9, 0xf6e9f6e9f6e9f6e9, 
    
    0xf727f727f727f727, 0xf727f727f727f727, 0xf727f727f727f727, 0xf727f727f727f727, 
    0xf727f727f727f727, 0xf727f727f727f727, 0xf727f727f727f727, 0xf727f727f727f727, 
    0xf727f727f727f727, 0xf727f727f727f727, 0xf727f727f727f727, 0xf727f727f727f727, 
    0xf727f727f727f727, 0xf727f727f727f727, 0xf727f727f727f727, 0xf727f727f727f727,
};


u64 gHeat[] = {
    0x0897089710971097, 0x1097189718971897, 0x1897209720972097, 0x2897289728972897, 
    0x3097309730973897, 0x3897389740974097, 0x4097409748974897, 0x4897509750975097, 
    0x58975897589758d7, 0x60d76097609768d7, 0x68d768d770d770d7, 0x70d778d778d778d7, 
    0x78d780d780d580d5, 0x88d588d588d588d5, 0x90d590d590d598d5, 0x8113811381138113, 
    
    0x8913891389159115, 0x911590d590d598d5, 0x98d598d5a097a097, 0xa095a095a895a895, 
    0xa895a893a893b0d3, 0xb093b0d3b0d3b0d1, 0xb0d1b8d1b8d1b8d1, 0xb8d1b8cfb8cfc0cf, 
    0xc0cfc0cfc0cfc0cd, 0xc8cdc8cdc8cdc8cd, 0xc8cdc8cbd0cbd0cb, 0xd0cbd0cbd0c9d8c9, 
    0xd8c9d8c9d909d909, 0xd907d907e107e107, 0xe107e107e105e905, 0xe905e905e905e905, 
    
    0xe903e943e943e943, 0xe983e983e983e9c3, 0xe9c3e9c3ea03f203, 0xf203f243f243f243, 
    0xf283f283f283f2c3, 0xf2c3f2c3f2c3f303, 0xf303f303f343f343, 0xf343f383f383f383, 
    0xf3c3f3c3f3c3f403, 0xf403f403f441f441, 0xf441f481f481f481, 0xf4c1f4c1f4c1f501, 
    0xf501f501f541f541, 0xf541f581f581f581, 0xf5c1f5c1f5c1f5c1, 0xf601f601f601f641, 
    
    0xf641f641f681f681, 0xf681f6c1f6c1f6c1, 0xf701f701f701f741, 0xf741f741f741f783, 
    0xf785f785f787f787, 0xf789f789f78bf78d, 0xf78df78ff791f791, 0xf793f793f795f797, 
    0xf797f799f799f79b, 0xf79df79df79ff79f, 0xf7a1f7a3f7a3f7a5, 0xf7a5f7a7f7a7f7a9, 
    0xf7abf7abf7adf7af, 0xf7aff7b1f7b1f7b3, 0xf7b5f7b5f7b7f7b7, 0xf7b9f7bbf7bbf7bd,
};

u64 gRainbow[] = {
    0xf001f001f003f005, 0xf007f007f009f00b, 0xf00bf00df00ff011, 0xf011f013f015f017, 
    0xf017f019f01bf01b, 0xf01df01ff021f021, 0xf023f025f027f027, 0xf029f02bf02bf02d, 
    0xf02ff031f031f033, 0xf035f037f037f039, 0xf03bf03df03df03d, 0xe83de03dd83dd83d, 
    0xd03dc83dc83dc03d, 0xb83db83db03da83d, 0xa03da03d983d903d, 0x883d883d803d783d, 

    0x783d703d683d603d, 0x603d583d503d483d, 0x483d403d383d383d, 0x303d283d203d203d, 
    0x183d103d083d083d, 0x003d003d003d007d, 0x00bd00fd00fd013d, 0x017d017d01bd01fd, 
    0x023d023d027d02bd, 0x02fd02fd033d037d, 0x037d03bd03fd043d, 0x043d047d04bd04fd, 
    0x04fd053d057d057d, 0x05bd05fd063d063d, 0x067d06bd06bd06fd, 0x073d077d077d07bd,

    0x07bd07bb07b907b9, 0x07b707b507b507b3, 0x07b107af07af07ad, 0x07ab07a907a907a7, 
    0x07a507a507a307a1, 0x079f079f079d079b, 0x0799079907970795, 0x079307930791078f, 
    0x078f078d078b0789, 0x0789078707850785, 0x0783078107810781, 0x0f8117811f811f81, 
    0x27812f8137813781, 0x3f81478147814f81, 0x57815f815f816781, 0x6f81778177817f81, 

    0x878187818f819781, 0x9f819f81a781af81, 0xaf81b781bf81c781, 0xc781cf81d781df81, 
    0xdf81e781ef81ef81, 0xf781f781f741f701, 0xf701f6c1f681f681, 0xf641f601f5c1f5c1, 
    0xf581f541f541f501, 0xf4c1f481f481f441, 0xf401f3c1f3c1f381, 0xf341f341f301f2c1, 
    0xf281f281f241f201, 0xf1c1f1c1f181f141, 0xf141f101f0c1f081, 0xf081f041f001f001,
};

u8 __attribute__((aligned(64))) indexColorBuffer[SCREEN_HT * SCREEN_WD];

#define UNLIT_TEXTURE   0, 0, 0, TEXEL0, 0, 0, 0, ENVIRONMENT

#define COPY_IMAGE_TILE(x, y, w, h)                             \
    gsDPLoadTextureTile(                                        \
        SOURCE_CB,                                              \
        G_IM_FMT_CI, G_IM_SIZ_8b,                               \
        SCREEN_WD, SCREEN_HT,                                   \
        (x), (y), (x) + (w) - 1, (y) + (h) - 1,                 \
        0,                                                      \
        G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMIRROR | G_TX_CLAMP, \
        G_TX_NOMASK, G_TX_NOMASK,                               \
        G_TX_NOLOD, G_TX_NOLOD                                  \
    ),                                                          \
    gsSPTextureRectangle(                                       \
        (x) << 2, (y) << 2,                                     \
        ((x) + (w)) << 2, ((y) + (h)) << 2,                     \
        G_TX_RENDERTILE,                                        \
        (x) << 5, (y) << 5,                                     \
        1 << 10, 1 << 10                                        \
    )

#define COPY_HALF_IMAGE_ROW(x, y, w, h)                         \
    COPY_IMAGE_TILE((x) + 0, y, w, h),                          \
    COPY_IMAGE_TILE((x) + 64, y, w, h),                         \
    COPY_IMAGE_TILE((x) + 128, y, w, h),                        \
    COPY_IMAGE_TILE((x) + 192, y, w, h),                        \
    COPY_IMAGE_TILE((x) + 256, y, w, h)

#define COPY_FULL_IMAGE_ROW(y, w, h)    COPY_HALF_IMAGE_ROW(0, y, w, h), COPY_HALF_IMAGE_ROW(320, y, w, h)

Gfx gCopyCBMaterial[] = {
    gsDPPipeSync(),
    gsDPSetCombineMode(UNLIT_TEXTURE, UNLIT_TEXTURE),
    gsDPSetTextureLUT(G_TT_RGBA16),
    gsDPSetTextureFilter(G_TF_POINT),
    gsDPSetRenderMode(G_RM_OPA_SURF, G_RM_OPA_SURF2),
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPSetEnvColor(255, 255, 255, 255),
    gsSPClearGeometryMode(G_ZBUFFER),
    gsSPTexture(0xffff, 0xffff, 0, G_TX_RENDERTILE, G_ON),
    gsSPEndDisplayList(),
};

Gfx gCopyCBScaryMaterial[] = {
    gsDPPipeSync(),
    gsDPSetCombineMode(UNLIT_TEXTURE, UNLIT_TEXTURE),
    gsDPSetTextureLUT(G_TT_RGBA16),
    gsDPSetTextureFilter(G_TF_POINT),
    gsDPSetRenderMode(G_RM_OPA_SURF, G_RM_OPA_SURF2),
    gsDPSetAlphaCompare(G_AC_DITHER),
    gsDPSetAlphaDither(G_AD_NOISE),
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPSetEnvColor(255, 255, 255, 190),
    gsSPClearGeometryMode(G_ZBUFFER),
    gsSPTexture(0xffff, 0xffff, 0, G_TX_RENDERTILE, G_ON),
    gsSPEndDisplayList(),
};

Gfx gCopyCB[] = {
    COPY_FULL_IMAGE_ROW(0, 64, 32),
    COPY_FULL_IMAGE_ROW(32, 64, 32),
    COPY_FULL_IMAGE_ROW(64, 64, 32),
    COPY_FULL_IMAGE_ROW(96, 64, 32),
    COPY_FULL_IMAGE_ROW(128, 64, 32),
    COPY_FULL_IMAGE_ROW(160, 64, 32),
    COPY_FULL_IMAGE_ROW(192, 64, 32),
    COPY_FULL_IMAGE_ROW(224, 64, 32),
    COPY_FULL_IMAGE_ROW(256, 64, 32),
    COPY_FULL_IMAGE_ROW(288, 64, 32),
    COPY_FULL_IMAGE_ROW(320, 64, 32),
    COPY_FULL_IMAGE_ROW(352, 64, 32),
    COPY_FULL_IMAGE_ROW(384, 64, 32),
    COPY_FULL_IMAGE_ROW(416, 64, 32),
    COPY_FULL_IMAGE_ROW(448, 64, 32),
    
    gsSPEndDisplayList(),
};