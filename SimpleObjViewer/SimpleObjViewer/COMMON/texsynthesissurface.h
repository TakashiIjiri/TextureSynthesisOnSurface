#pragma once

#include "ttexmesh.h"

#include "timage.h"



void TextureSynthesisOnSurface
(
	const TTexMesh       &mesh, //mesh should be have tex coord
	const vector<EVec3d> &p_Flow, 

	const TImage2D  &sampleTex,


	const int    winR ,
	const double pitch,
	const int W,
	const int H, 
	
	TImage2D &trgtTex
);


void SimpleRegionGrowing
(
	const TTexMesh       &mesh, //mesh should be have tex coord
	const vector<EVec3d> &p_Flow, 

	const TImage2D  &sampleTex,


	const int    winR ,
	const double pitch,
	const int W,
	const int H, 
	
	TImage2D &trgtTex
);







void t_genPolygonIDtexture
(
	const TTexMesh &mesh,
	const int W,
	const int H,
	int *polyIdTex
);

//sampling texture patch around pixel by using exponential mapping
void t_getNeighboringPatch(
	const int &W ,
	const int &H ,
	const TImage2D &trgtTex ,
	const TTexMesh &mesh    ,

	const EVec3i   &pixOnTex,
	const int      &pixPolyI,

	const int    patchR,
	const double samplePitch, 
	TImage2D &patch, //should be allocated
	bool printDebug
);