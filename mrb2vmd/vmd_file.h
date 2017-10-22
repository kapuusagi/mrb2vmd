/**
 mrb_file.c

 Copyright (c) 2017 kapuusagi

 This software is released under the MIT License.
 http://opensource.org/licenses/mit-license.php
*/
#ifndef VMD_FILE_H
#define VMD_FILE_H

#include <stdint.h>

namespace vmd_file {

#define VMD_FILE_SIGNATURE "Vocaloid Motion Data 0002"

#pragma pack(1)
struct VMDMotionRecord {
	char bone_name[15];
	uint32_t frame_no;
	struct {
		float x;
		float y;
		float z;
	} pos;
	struct {
		float x;
		float y;
		float z;
		float w;
	} rotation;
	uint8_t interpolation[64];
};

struct VMDSkinRecord {
	char name[15];
	uint32_t frame_no;
	float weight;
};
struct VMDCameraRecord {
	uint32_t frame_no;
	float length;
	struct {
		float x;
		float y;
		float z;
	} location;
	struct {
		float x;
		float y;
		float z;
	} rotation;
	uint8_t interpolation[24];
	uint32_t angle;
	uint8_t perspective;
};

struct VMDLightRecord {
	uint32_t frame_no;
	struct {
		float r;
		float g;
		float b;
	} color;
	struct {
		float x;
		float y;
		float z;
	} location;
};

struct VMDShadowRecord {
	uint8_t data[28];
	uint32_t frame_no;
	uint8_t mode;
	float distance;
};
#pragma pack()

} // namespace vmd_file


#endif /* VMD_FILE_H */