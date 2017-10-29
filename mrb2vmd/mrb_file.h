/**
 mrb_file.h

 Copyright (c) 2017 kapuusagi

 This software is released under the MIT License.
 http://opensource.org/licenses/mit-license.php
*/
#ifndef MRB_FILE_H
#define MRB_FILE_H

#include <stdint.h>
#include <vector>

/**
 * MRB file header.
 */
struct MRBFileHeader {
    char magic[4]; /* Magic number "MRB\0" */
    uint32_t tmp; 
    uint32_t data_count; /* Data count of MRB file. */
};

struct MRBDataHeader {
    uint32_t type;
#define MRB_TYPE_MODEL      1
#define MRB_TYPE_BONE       2
#define MRB_TYPE_ANIMATION  3
#define MRB_TYPE_4          4
#define MRB_TYPE_5          5
    char name[32];
    uint32_t length;          // 44 + data length.
    uint32_t flag;
};

struct MRBBone {
    std::string name;
    uint32_t flag;
    uint8_t no;
    uint8_t parent;
    uint8_t child;
    uint8_t sibling_no;
    struct {
        float x; 
        float y;
        float z;
    } move;
    struct {
        float a1;
        float a2;
        float a3;
        float a4;
    } quaternion;
    struct {
        float x;
        float y;
        float z;
    } scale;
};

struct MRBBoneData {
    std::vector<struct MRBBone> bones;
};

struct MRBAnimationArrayKey {
    struct {
        float x;
        float y;
        float z;
    } vector;
    struct {
        float x;
        float y;
        float z;
        float w;
    } quat;
};

struct MRBAnimationEffect {
    uint32_t frame;
    char data[64];
};



struct MRBAnimationBoneData {
    struct {
        float x;
        float y;
        float z;
    } move;
    struct {
        float x;
        float y;
        float z;
        float w;
    } rotation;
    struct {
        float x;
        float y;
        float z;
    } expansion;
};
struct MRBAnimationBone {
    std::string name;
    std::vector<MRBAnimationBoneData> frames;
};

struct MRBAnimationData {
    std::vector<uint32_t> keys;
    std::vector<struct MRBAnimationArrayKey> arrayKeys;
    std::vector<struct MRBAnimationEffect> effects;
    std::vector<MRBAnimationBone> data;
};


/**
 * MRBファイル内の1つのデータセクションを表す構造体。
 * unionを使っていないのは各構造体の中で使っているectorなどが
 * うまく動くとは思えないため。
 * 例えば bone内で使っているvectorを変更したら、 animで使っているvectorのメモリが破壊されるでしょう？
 * C++で union は注意しないといけない。
 *
 * あるいはインタフェース定義して動的に確保する手段もあるけれど、
 * 動的に確保はエラーチェックいれたりで面倒だからやめました。
 * 共通インタフェースじゃないですしね。
 */
struct MRBData {
    struct MRBDataHeader header;
    struct MRBBoneData bone;
    struct MRBAnimationData anim;
};

/**
 * MRBファイルデータ。
 * MRBファイルの詳細構造は ECO Wikiなどを参照してもらうのがいい。
 * ちょっとだけ構造にふれると、
 * 
 *
 * MRBFile 
 *    +- N個のデータ
 *          データは種類がある。どんな種類かはHeaderに記述されている。
 */
struct MRBFile {
    MRBFileHeader header;
    std::vector<MRBData> data; /* MRBデータ。 1つのMRBファイルには複数のデータが含まれている。 */
};

int read_mrb_file(const char *path, struct MRBFile *mrb);


#endif /* MRB_FILE_H */