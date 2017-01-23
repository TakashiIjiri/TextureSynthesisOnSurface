#include "stdafx.h"

#include "texsynthesissurface.h"




#ifndef MIN3
#define MIN3(  a,b,c)	((a)<(b)?((a)<(c)?(a):(c)):((b)<(c)?(b):(c)))
#define MAX3(  a,b,c)	((a)>(b)?((a)>(c)?(a):(c)):((b)>(c)?(b):(c)))
#define MIN3ID(a,b,c)	((a)<(b)?((a)<(c)?(0):(2)):((b)<(c)?(1):(2)))
#define MAX3ID(a,b,c)	((a)>(b)?((a)>(c)?(0):(2)):((b)>(c)?(1):(2)))
#endif



//Mathematical functions -------------------------------------------------------

//	  | a b | |s|    w1
//    | c d | |t|  = w2
inline bool t_solve2by2LinearEquation
(
	const double a, const double b,
	const double c, const double d,
	const double w1, const double w2,
	double &s, double &t
	)
{
	double det = (a*d - b*c);
	if (det == 0) return false;
	det = 1.0 / det;
	s = (d*w1 - b*w2) * det;
	t = (-c*w1 + a*w2) * det;
	return true;
}




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool t_isInTriangle2D( const EVec3d &p0, const EVec3d &p1,const EVec3d &p2, const EVec3d P)
{
	// p = p0 + s * (p1-p0) + t * (p2-p0)
	//--> p-p0 = (p1-p0, p2-p0) (s,t)^T

	double s,t;
	t_solve2by2LinearEquation( p1[0] - p0[0], p2[0] - p0[0],
		                       p1[1] - p0[1], p2[1] - p0[1],   P[0]-p0[0], P[1]-p0[1],  s,t);
	return 0<=s && 0 <= t && s+t <= 1;
}


// generate Polygon ID texture
// 
// mesh       : surface model with uv on each vertex
// int W, H   : 
// polyIdxImg : int[W*H] image painted by texture ids
void t_genPolygonIDtexture
(
	const TTexMesh &mesh,
	const int W,
	const int H,
	int *polyIdTex
)
{
	const double pX = 1.0 / W;
	const double pY = 1.0 / H;

	for( int i=0,WH=W*H; i < WH; ++i) polyIdTex[i] = -1;

	for( int pi=0; pi < mesh.m_pSize; ++pi)
	{
		const int   *idx = mesh.m_polys[ pi ].tIdx; 
		const EVec3d &uv0 = mesh.m_uvs[ idx[0] ];
		const EVec3d &uv1 = mesh.m_uvs[ idx[1] ];
		const EVec3d &uv2 = mesh.m_uvs[ idx[2] ];

		int minU = max(  0, (int)( MIN3( uv0[0], uv1[0], uv2[0]) * W ) - 1);
		int minV = max(  0, (int)( MIN3( uv0[1], uv1[1], uv2[1]) * H ) - 1);
		int maxU = min(W-1, (int)( MAX3( uv0[0], uv1[0], uv2[0]) * W ) + 1);
		int maxV = min(H-1, (int)( MAX3( uv0[1], uv1[1], uv2[1]) * H ) + 1);

		//fprintf( stderr, "%d %d %d %d\n", minU, minV,maxU, maxV );

		for( int u = minU; u < maxU; ++u)
		{
			for( int v = minV; v < maxV; ++v)
			{
				EVec3d p( (u+0.5) / W, (v+0.5)/H, 0);
				if( t_isInTriangle2D( uv0, uv1, uv2, p) )
				{
					polyIdTex[u + v*W] = pi;
				}
			}
		}
	}

	//update 2015/12/20
	//thicken one-ring pixels (N-times)
	const int N = 3;
	for (int i = 0; i < N; ++i)
	{
		for (int y = 0; y < H; ++y)
		{
			for (int x = 0; x < W; ++x)
			{
				int I = x + y * W;
				if (polyIdTex[I] != -1) continue;
				if (x > 0     && polyIdTex[I - 1] >= 0) { polyIdTex[I] = -2; continue; }
				if (y > 0     && polyIdTex[I - W] >= 0) { polyIdTex[I] = -2; continue; }
				if (x < W - 1 && polyIdTex[I + 1] >= 0) { polyIdTex[I] = -2; continue; }
				if (y < H - 1 && polyIdTex[I + W] >= 0) { polyIdTex[I] = -2; continue; }
			}
		}

		for (int y = 0; y < H; ++y)
		{
			for (int x = 0; x < W; ++x)
			{
				int I = x + y * W;
				if (polyIdTex[I] != -2) continue;
				if (x > 0     && polyIdTex[I - 1] >= 0) { polyIdTex[I] = polyIdTex[I - 1]; continue; }
				if (y > 0     && polyIdTex[I - W] >= 0) { polyIdTex[I] = polyIdTex[I - W]; continue; }
				if (x < W - 1 && polyIdTex[I + 1] >= 0) { polyIdTex[I] = polyIdTex[I + 1]; continue; }
				if (y < H - 1 && polyIdTex[I + W] >= 0) { polyIdTex[I] = polyIdTex[I + W]; continue; }
			}
		}
	}



	//debug texture
	TImage2D forDebug;
	forDebug.AllocateImage( W,H, 0);
	for( int i=0,WH=W*H; i < WH; ++i) 
	{
		int pi = polyIdTex[i];
		if(      pi == -1)    forDebug.setPix( 4*i, 0  ,0  ,255,0);
		else if( pi % 5 == 0) forDebug.setPix( 4*i, 255,0  ,0  ,0);
		else if( pi % 5 == 1) forDebug.setPix( 4*i, 0  ,255,0  ,0);
		else if( pi % 5 == 2) forDebug.setPix( 4*i, 64 ,255,0  ,0);
		else if( pi % 5 == 3) forDebug.setPix( 4*i, 255,255,0  ,0);
		else                  forDebug.setPix( 4*i, 128,128,0  ,0);
	}

	forDebug.FlipInY();
	forDebug.saveAsFile( "idRefImg.bmp", 0);
}

































void TextureSynthesisOnSurface
(
	const TTexMesh  &mesh, //mesh should be have tex coord
	const vector<EVec3d> &p_Flow, 

	const TImage2D  &sampleTex,


	const int    winR ,
	const double pitch,
	const int W,
	const int H, 
	TImage2D &trgtTex
)
{
	trgtTex.AllocateImage(W,H,0);

	int *polyIdTex = new int[W*H];
	t_genPolygonIDtexture( mesh, W,H, polyIdTex);




}
