
// spine sample code
// document http://ja.esotericsoftware.com/spine-c#SpineC-Runtime-Documentation

#include "stdafx.h"
#include <stdio.h>
#include <string>
#include <iostream>

#include <vector>
#include <fstream>
#include <sstream>

#include "spine\extension.h"
#include "spine\spine.h"

//#include <crtdbg.h>

//computing vertex buffer
std::vector<float> _worldVertices;
void _worldVertices_setlength(int length) {
	if (length > _worldVertices.size()) {
		_worldVertices.resize(length);
	}
}
void _worldVertices_clear() {
	_worldVertices.resize(1024);
}

//implement function
//extern "C" { // probably unnecessary 

void _spAtlasPage_createTexture(spAtlasPage* self, const char* path) {
	self->rendererObject = nullptr;	//set user class or record pointer
	self->width = 2048;
	self->height = 2048;

	printf("texture.create:%s\n", path);
	//printf("texture.create:%s\n", self->name);
}

void _spAtlasPage_disposeTexture(spAtlasPage* self) {
	//dispose self->rendererObject

	printf("texture.dispose:%s\n", self->name);
}

//not use
char* _spUtil_readFile(const char* path, int* length){
	printf("readfile:%s\n", path);
	return nullptr;	//return byte array pointer
}

void myListener(spAnimationState* state, spEventType type, spTrackEntry* entry, spEvent* event) {
	switch (type) {
		// 
	case SP_ANIMATION_START:
		printf("Animation %s started on track %i\n", entry->animation->name, entry->trackIndex);
		break;
	case SP_ANIMATION_INTERRUPT:
		printf("Animation %s interrupted on track %i\n", entry->animation->name, entry->trackIndex);
		break;
	case SP_ANIMATION_END:
		printf("Animation %s ended on track %i\n", entry->animation->name, entry->trackIndex);
		break;
	case SP_ANIMATION_COMPLETE:
		printf("Animation %s completed on track %i\n", entry->animation->name, entry->trackIndex);
		break;
	case SP_ANIMATION_DISPOSE:
		printf("Track entry for animation %s disposed on track %i\n", entry->animation->name, entry->trackIndex);
		break;
	case SP_ANIMATION_EVENT:
		printf("User defined event for animation %s on track %i\n", entry->animation->name, entry->trackIndex);
		printf("Event: %s: %d, %f, %s\n", event->data->name, event->intValue, event->floatValue, event->stringValue);
		break;
	default:
		printf("Unknown event type: %i", type);
	}
}

//CustomDraw
void addTriangle(float* xx, float* yy, spColor* col, int blendmode) {
	printf("draw.triangle:vertex=(%f,%f)(%f,%f)(%f,%f),argb=(%f,%f,%f,%f),blend(%d)\n", xx[0], yy[0], xx[1], yy[1], xx[2], yy[2], col->a, col->r, col->g, col->b, blendmode);
}

void myCustomDraw_region(spSlot* slot , spAttachment* attachment , spColor* col , int blendmode) {
	// Cast to an spRegionAttachment so we can get the rendererObject
	// and compute the world vertices
	spRegionAttachment* regionAttachment = (spRegionAttachment*)attachment;

	// Check the number of vertices in the mesh attachment.
	_worldVertices_setlength(4*2);	//4 vertex

	// Our engine specific Texture is stored in the spAtlasRegion which was
	// assigned to the attachment on load. It represents the texture atlas
	// page that contains the image the region attachment is mapped to
	void* textureptr = nullptr;
	textureptr = (void*)((spAtlasRegion*)regionAttachment->rendererObject)->page->rendererObject;

	// Computed the world vertices positions for the 4 vertices that make up
	// the rectangular region attachment. This assumes the world transform of the
	// bone to which the slot (and hence attachment) is attached has been calculated
	// before rendering via spSkeleton_updateWorldTransform
	spRegionAttachment_computeWorldVertices(regionAttachment, slot->bone, _worldVertices.data(), 0, 2);

	// Create 2 triangles, with 3 vertices each from the region's
	// world vertex positions and its UV coordinates (in the range [0-1]).

	// 「Computed the world vertices positions for the 4 vertices that make up」
	// が正確な情報で、4頂点分を計算します。
	// つまり、0,1,2 と 2,3,0 の2個の三角形を形成します。

	const float idxtbl[6] = { 0,1,2,2,3,0 };

	float xx[3], yy[3];	//vertex
	float uu[3], vv[3];	//uv
	int triangleindex = 0;
	float* sptr = (float*)_worldVertices.data();
	int n = 2;
	for (int i = 0; i < n; i++) {
		//triangle
		for (int j = 0; j < 3; j++) {
			//vertex
			int idx = idxtbl[triangleindex] * 2;
			triangleindex++;
			xx[j] = sptr[idx + 0];
			yy[j] = sptr[idx + 1];
			uu[j] = regionAttachment->uvs[idx + 0];
			vv[j] = regionAttachment->uvs[idx + 1];
		}
		//add
		addTriangle(xx, yy, col, blendmode);
	}

}

void myCustomDraw_mesh(spSlot* slot , spAttachment* attachment , spColor* col , int blendmode) {
	// Cast to an spMeshAttachment so we can get the rendererObject
	// and compute the world vertices
	spMeshAttachment* mesh = (spMeshAttachment*)attachment;

	// Check the number of vertices in the mesh attachment. If it is bigger
	// than our scratch buffer, we don't render the mesh. We do this here
	// for simplicity, in production you want to reallocate the scratch buffer
	// to fit the mesh.
	// if (mesh->super.worldVerticesLength > MAX_VERTICES_PER_ATTACHMENT) continue;
	_worldVertices_setlength(mesh->super.worldVerticesLength);	//worldVerticesLength buffer length. x,y,x,y,x,y,x,y...x,y

	// Our engine specific Texture is stored in the spAtlasRegion which was
	// assigned to the attachment on load. It represents the texture atlas
	// page that contains the image the mesh attachment is mapped to
	void* textureptr = nullptr;
	textureptr = (void*)((spAtlasRegion*)mesh->rendererObject)->page->rendererObject;	//set user pointer in _spAtlasPage_createTexture

	// Computed the world vertices positions for the vertices that make up
	// the mesh attachment. This assumes the world transform of the
	// bone to which the slot (and hence attachment) is attached has been calculated
	// before rendering via spSkeleton_updateWorldTransform
	// SUPER = mesh->super
	spVertexAttachment_computeWorldVertices(SUPER(mesh), slot, 0, mesh->super.worldVerticesLength, _worldVertices.data(), 0, 2);	//probably stride value float x,y count 2. support 3D(x,y,z) feature?

	// Mesh attachments use an array of vertices, and an array of indices to define which
	// 3 vertices make up each triangle. We loop through all triangle indices
	// and simply emit a vertex for each triangle's vertex.
	//for (int i = 0; i < mesh->trianglesCount; ++i) {
	//	int index = mesh->triangles[i] << 1;
	//	addVertex(worldVerticesPositions[index], worldVerticesPositions[index + 1],
	//		mesh->uvs[index], mesh->uvs[index + 1],
	//		tintR, tintG, tintB, tintA, &vertexIndex);
	//}
	
	//ややこしいですが、
	//triangleCount分の(x,y)頂点indexが入っていて（三角形の数じゃないんだ...）、3頂点ごとに1三角形を構成します。
	//4頂点は存在しません。

	float xx[3], yy[3];	//vertex
	float uu[3], vv[3];	//uv
	int triangleindex = 0;
	float* sptr = (float*)_worldVertices.data();
	int n = mesh->trianglesCount / 3;
	for (int i = 0; i < n; i++) {
		//triangle
		for (int j = 0; j < 3; j++) {
			//vertex
			int idx = mesh->triangles[triangleindex] * 2;
			triangleindex++;
			xx[j] = sptr[idx + 0];
			yy[j] = sptr[idx + 1];
			uu[j] = mesh->uvs[idx + 0];
			vv[j] = mesh->uvs[idx + 1];
		}
		//add
		addTriangle(xx,yy,col,blendmode); 
	}
	
}

void myCustomDraw(spSkeleton* skeleton) {
	for (int i = 0; i < skeleton->slotsCount; i++) {
		spSlot* slot = skeleton->drawOrder[i];
		
		// Fetch the currently active attachment, continue
		// with the next slot in the draw order if no
		// attachment is active on the slot
		spAttachment* attachment = slot->attachment;
		if (!attachment) continue;

		// Fetch the blend mode from the slot and
		// translate it to the engine blend mode
		int blendmode = 0;
		switch (slot->data->blendMode) {
			case SP_BLEND_MODE_NORMAL:		//alpha blend : src.rgb * (src.a) + dest.rgb * (1.0-src.a)
				blendmode = 0;
				break;
			case SP_BLEND_MODE_ADDITIVE:	//add : src.rgb * src.a + dest.rgb
				blendmode = 1;
				break;
			case SP_BLEND_MODE_MULTIPLY:	//multiply/modulate : (src.rgb * src.a) * dest.rgb
				blendmode = 2;
				break;
			case SP_BLEND_MODE_SCREEN:		//screen : (src.rgb * src.a + dest.rgb) - (src.rgb * src.a * dest.rgb) == ((src.rgb * src.a) * inv(dest.rgb)) + (1.0 * dest.rgb)
				blendmode = 3;
				break;
		}

		// Calculate the tinting color based on the skeleton's color
		// and the slot's color. Each color channel is given in the
		// range [0-1], you may have to multiply by 255 and cast to
		// and int if your engine uses integer ranges for color channels.
		spColor col;
		col.r = skeleton->color.r * slot->color.r;
		col.g = skeleton->color.g * slot->color.g;
		col.b = skeleton->color.b * slot->color.b;
		col.a = skeleton->color.a * slot->color.a;

		// Fill the vertices array depending on the type of attachment
		if (attachment->type == SP_ATTACHMENT_REGION) {
			myCustomDraw_region(slot,attachment , &col , blendmode);
		}
		else if (attachment->type == SP_ATTACHMENT_MESH) {
			myCustomDraw_mesh(slot,attachment, &col , blendmode);
		}
		else {
			//unsupport
		}

	}
}

//}
//misc

std::vector<char>* _readfile(const std::string &filename) {
	std::ifstream fs;
	fs.open(filename, std::ios::in | std::ios::binary);
	if (!fs) {
		printf("error.file.open:%s\n",filename.c_str());
		getchar();
		return nullptr;
	}

	//get file size
	std::vector<char>* buf = new std::vector<char>();
	fs.seekg(0, std::ios::end);
	std::streampos size = fs.tellg();
	fs.seekg(0, std::ios::beg);
	
	//read from filestream
	buf->resize(size);
	fs.read(buf->data(), size);
	
	fs.close();
	return buf;
}

int main(int argc, char* argv[])
{
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);	//memory leak debug

	std::string base = "examples\\spineboy\\export\\";
	std::string f;

	//atlas
	//call create texture
	f = base + "spineboy.atlas";
	std::vector<char>* ms = _readfile(f);
	spAtlas* atlas = spAtlas_create(ms->data(), ms->size(), f.c_str(), 0);
	delete ms;
	if (atlas == nullptr) {
		printf("Atlas.faild\n");
		getchar();
		return -1;
	}
	printf("Atlas.complete\n");

	//SkeletonData
	ms = _readfile(base + "spineboy-pro.json");
	spSkeletonJson* skeletonJson = spSkeletonJson_create(atlas);
	spSkeletonData* skeletonData = spSkeletonJson_readSkeletonData(skeletonJson, ms->data());
	delete ms;
	spSkeletonJson_dispose(skeletonJson);
	if (skeletonData == nullptr) {
		printf("SkentonData.faild\n");
		getchar();
		return -1;
	}
	printf("SkeletonData.complete\n");

	//AnimationData
	spAnimationStateData* animationStateData = spAnimationStateData_create(skeletonData);
	//spAnimationStateData_setDefaultMix(animationStateData);	//probabry disuse
	animationStateData->defaultMix = 0.1f;
	spAnimationStateData_setMixByName(animationStateData, "walk", "run", 0.2f);
	spAnimationStateData_setMixByName(animationStateData, "walk", "shot", 0.1f);
	printf("AnimationStateData.complete\n");

	//play onject state
	spSkeleton* skeleton = spSkeleton_create(skeletonData);
	spAnimationState* animationState = spAnimationState_create(animationStateData);

	skeleton->x = 500;
	skeleton->y = 500;
	spSkeleton_updateWorldTransform(skeleton);	//x,y transform after , next call spSkeleton_updateWorldTransform

	spAnimationState_clearTracks(animationState);						//animation clear
	spAnimationState_setAnimationByName(animationState, 0, "walk", 1);	//animationstate , track , name , loop

	//event
	animationState->listener = myListener;

	//main loop
	for (int i = 0; i < 1; i++) {
		//update (time move) 60fps
		spAnimationState_update(animationState, 1.0/60);	//probably trunc micro seconds better

		//apply skeleton 
		spAnimationState_apply(animationState, skeleton);

		//transform world
		//spAnimationState_updateWorldTransform(skeleton);	//probabry disuse

		//custom draw function
		myCustomDraw(skeleton);

		printf("---frame---\n");
	}

	//dispose
	spAnimationState_dispose(animationState);
	spSkeleton_dispose(skeleton);

	spAnimationStateData_dispose(animationStateData);
	spSkeletonData_dispose(skeletonData);
	spAtlas_dispose(atlas);

	getchar();
	return 0;
}

//animation
//spAnimationState_setEmptyAnimation
//spAnimationState_clearTracks
//spAnimationState_clearTrack
//spAnimationState_setAnimationByName
