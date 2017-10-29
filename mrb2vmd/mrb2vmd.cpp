/**
 mrb_file.c

 Copyright (c) 2017 kapuusagi

 This software is released under the MIT License.
 http://opensource.org/licenses/mit-license.php
*/

#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <math.h>

#pragma warning(disable:4996)

#include "mrb_file.h"
#include "vmd_file.h"

static std::string generate_new_path(const char *path);
static int mrb2vmd(const char *mrb_path, const char *vmd_path);
static int write_vmd(const struct MRBAnimationData &data, const char *path);
static std::string generate_modelname(const char *path);

/**
 * �A�v���P�[�V�����̃G���g���|�C���g
 * 
 * @param ac �����̐�
 * @param av �����i.exe�t�@�C����D&D���ꂽ�p�X�������ɓn��j
 */
int
main(int ac, char **av)
{
    for (int i = 1; i < ac; i++) {
        std::string new_path = generate_new_path(av[i]);
        if (mrb2vmd(av[i], new_path.c_str()) == 0) {
            printf("Converting %s -> %s done.\n", av[i], new_path.c_str());
        }
        else {
            fprintf(stderr, "Converting %s failure.\n", av[i]);

        }
    }

    printf("Press enter to exit.\n");
    fgetc(stdin);

    return 0;
}

/**
 * MRB�t�@�C��������VMD�t�@�C�����𐶐�����B
 * ��̓I�ɂ͊g���q��VMD�ɂ��ĕԂ��B
 *
 * �g���q�����Ă�O��̂�낵���Ȃ��R�[�h�B
 * �g���q�Ȃ��Ńf�B���N�g����.���������Ƃ��ɂǂ�����񂾁[�B
 *
 * @param path MRB�t�@�C���p�X
 * @return VMD�t�@�C���p�X
 */
static std::string
generate_new_path(const char *path)
{
    std::string ret(path);

    size_t pos = ret.find_last_of('.');
    if (pos == std::string::npos) {
        return ret + ".vmd";
    }
    else {
        return ret.substr(0, pos) + ".vmd";
    }
}

/**
 * path�Ŏw�肳�ꂽMRB�t�@�C����ǂݍ��݁Avmd_path�Ŏw�肳�ꂽVMD�t�@�C���ɏ����o���B
 * 
 * @param mrb_path MRB�t�@�C���p�X
 * @param vmd_path VMD�t�@�C���p�X
 * @return ����������0�D���s������G���[�ԍ��B
 */
static int
mrb2vmd(const char *mrb_path, const char *vmd_path)
{
    printf("%s -> %s\n", mrb_path, vmd_path);

    struct MRBFile mrb_file;
    int s = read_mrb_file(mrb_path, &mrb_file);
    if (s != 0) {
        fprintf(stderr, "Could not read file. %s\n", mrb_path);
        return s;
    }

    for (const struct MRBData &mrb_data : mrb_file.data) {
        if (mrb_data.header.type == MRB_TYPE_ANIMATION) {
            write_vmd(mrb_data.anim, vmd_path);
        }
    }


    return 0;
}

/**
 * VMD�t�@�C���������o���B
 * @note 
 *   MRB�t�@�C���̃f�[�^��ϊ�����̂͂����ɓ����B
 * @param data MRB �A�j���[�V�����f�[�^
 * @param path VMD�t�@�C���p�X
 * @return ���������ꍇ�ɂ�0�A���s�����ꍇ�ɂ̓G���[�ԍ��B
 */
static int
write_vmd(const struct MRBAnimationData &data, const char *path)
{
    FILE *fp;

    if (data.data.empty()) {
        return ENODATA;
    }

    fp = fopen(path, "wb");
    if (fp == nullptr) {
        return errno;
    }

    // Write header.
    {
        char signature[30];
        char name[20];

        memset(signature, 0x0, sizeof(signature));
        memset(name, 0x0, sizeof(name));
        snprintf(signature, sizeof(signature), "Vocaloid Motion Data 0002");
        snprintf(name, sizeof(name), generate_modelname(path).c_str());
        fwrite(signature, 30, 1, fp); // Bad code. TODO : checking error.
        fwrite(name, 20, 1, fp);

    }

    // Motion data.
    {
        uint32_t record_count;
        uint32_t frame_count;
        uint32_t bone_count;

        bone_count = (uint32_t)(data.data.size());
        frame_count = (uint32_t)(data.data.at(0).frames.size());

        record_count = bone_count * frame_count;
        fwrite(&record_count, 4, 1, fp);

        for (uint32_t frame = 0; frame < frame_count; frame++) {
            for (uint32_t bone = 0; bone < bone_count; bone++) {
                const struct MRBAnimationBone &b = data.data.at(bone);
                const struct MRBAnimationBoneData &d = b.frames.at(frame);
                struct vmd_file::VMDMotionRecord record;
                memset(record.bone_name, 0x0, sizeof(record.bone_name));
                memset(record.interpolation, 0x0, sizeof(record.interpolation));
                if (b.name.length() >= 15) {
                    fprintf(stderr, " [warning] Too large bone name. [%s]\n", b.name.c_str());
                }
                //snprintf(record.bone_name, sizeof(record.bone_name), "%s", b.name.c_str());
                snprintf(record.bone_name, sizeof(record.bone_name), "Bone%d", bone);
                record.frame_no = frame;

                /* MRB �t�@�C���͐�΍��W�n�ŁAVMD�͐e������̑��΍��W�ɂȂ��Ă���炵���B
                 * �Ȃ񂩕ϊ����K�v�����悭�킩���I */
                /* �ʒu */
#if 0
                record.pos.x = -d.move.x;
                record.pos.y = d.move.y;
                record.pos.z = d.move.z;
#else
                record.pos.x = 0;
                record.pos.y = 0;
                record.pos.z = 0;
#endif

#if 0

                record.rotation.x = d.rotation.x;
                record.rotation.y = d.rotation.y;
                record.rotation.z = d.rotation.z;
                record.rotation.w = d.rotation.w;
#elif 1
                {
                    float x = d.rotation.x;
                    float y = d.rotation.y;
                    float z = d.rotation.z;
                    float w = d.rotation.w;
                    float r2 = sqrt(2.0f);

                    record.rotation.x = x / r2 + z / r2;
                    record.rotation.y = y / r2 - w / r2;
                    record.rotation.z = z / r2 - x / r2;
                    record.rotation.w = y / r2 + x / r2;
                }
#endif
                fwrite(&record, sizeof(record), 1, fp);
            }
        }

    }

    // Skin data.
    {
        uint32_t skin_count = 1;
        struct vmd_file::VMDSkinRecord record;
        memset(&record, 0x0, sizeof(record));
        fwrite(&skin_count, sizeof(skin_count), 1, fp);
        fwrite(&record, sizeof(record), 1, fp);
    }

    // Camera data
    {
        uint32_t camera_data_count = 1;
        struct vmd_file::VMDCameraRecord record;
        fwrite(&camera_data_count, sizeof(camera_data_count), 1, fp);
        memset(&record, 0x0, sizeof(record));
        fwrite(&record, sizeof(record), 1, fp);
    }

    // Light data
    {
        uint32_t light_data_count = 1;
        struct vmd_file::VMDLightRecord record;
        fwrite(&light_data_count, sizeof(light_data_count), 1, fp);
        memset(&record, 0x0, sizeof(record));
        fwrite(&record, sizeof(record), 1, fp);
    }

    // Shadow data
    {
        uint32_t shadow_data_count = 1;
        struct vmd_file::VMDShadowRecord record;
        fwrite(&shadow_data_count, sizeof(shadow_data_count), 1, fp);
        memset(&record, 0x0, sizeof(record));
        fwrite(&record, sizeof(record), 1, fp);
    }

    fclose(fp);

    return 0;
}

/**
 * MRB�t�@�C���f�[�^�̖��O����f�B���N�g�������������ă��f�����ɂ���B 
 * 
 * @param path MRB�t�@�C���̃p�X
 * @return ���f����(=�t�@�C����)
 */
static std::string
generate_modelname(const char *path)
{
    const char *p;
    std::string base;
    size_t pos;

    p = strrchr(path, '/');
    if (p == nullptr) {
        p = strrchr(path, '\\');
    }
    if (p != nullptr) {
        base = p + 1;
    }
    else {
        base = path;
    }

    pos = base.rfind('.');
    return (pos != base.npos) ? base.substr(0, pos) : base;
}
