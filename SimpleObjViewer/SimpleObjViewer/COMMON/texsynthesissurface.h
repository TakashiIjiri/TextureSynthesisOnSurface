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
	const TImage2D &trgtTex
);

