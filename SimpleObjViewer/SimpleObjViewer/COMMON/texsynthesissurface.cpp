#include "stdafx.h"

#include "texsynthesissurface.h"
#include "expmap.h"




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









inline double t_distance2D_sq(const EVec3d &x1, const EVec3d &x2){
	return (x1[0] - x2[0]) * (x1[0] - x2[0]) + 
		   (x1[1] - x2[1]) * (x1[1] - x2[1]);
}
inline double t_distance2D( const EVec3d& x1, const EVec3d &x2){
	return sqrt( t_distance2D_sq(x1, x2) ); 
}



// ph = p0 + t * (lineP1 - lineP0) / |lineP1 - lineP0)|
inline double t_distPointToLineSegment2D( 
	const EVec3d &p , 
	const EVec3d &lineP0, 
	const EVec3d &lineP1,
	double &t)
{
	t =  (p[0] - lineP0[0]) * (lineP1[0] - lineP0[0]) + 
		 (p[1] - lineP0[1]) * (lineP1[1] - lineP0[1]);
	t /= t_distance2D_sq(lineP0,lineP1);

	if( t < 0 ) { t = 0; return t_distance2D( p, lineP0 );}
	if( t > 1 ) { t = 1; return t_distance2D( p, lineP1 );}

	double x = lineP0[0]  + t * (lineP1[0]-lineP0[0]) - p[0];
	double y = lineP0[1]  + t * (lineP1[1]-lineP0[1]) - p[1];

	return sqrt( x*x + y*y);
}


/*
  pixel p1 is inside the triangle 
  pixel p2 run out from the triangle

      seam edge
	      e
          |
          |
   poly1  |   poly2  
      p1  -   p2  
(inside)  |  (outside)
          |
          |
  
  map p2 considering seam on the atlas
*/
EVec3i t_getNeighbors(
	const int W,
	const int H,
	const TTexMesh &mesh,
	const int      *polyIdTex,
	const EVec3i   &p1, 
	const EVec3i   &p2 )
{

	const int   polyI = polyIdTex[p1[2]];
	if (polyI == -1)
	{
		fprintf( stderr, "never comes here\n");
		return EVec3i(-1,-1,-1);
	}

	const TPoly &poly1 = mesh.m_polys[polyI];

	//get closest edge
	const EVec3d &uv0 = mesh.m_uvs[ poly1.tIdx[0] ];
	const EVec3d &uv1 = mesh.m_uvs[ poly1.tIdx[1] ];
	const EVec3d &uv2 = mesh.m_uvs[ poly1.tIdx[2] ];


	EVec3d pixPos( (p1[0]+0.5) / W, (p1[1]+0.5)/H, 0);
	double t0,t1,t2;
	double d0 = t_distPointToLineSegment2D( pixPos, uv0, uv1, t0 );
	double d1 = t_distPointToLineSegment2D( pixPos, uv1, uv2, t1 );
	double d2 = t_distPointToLineSegment2D( pixPos, uv2, uv0, t2 );

	//k = 0(01), 1(12), 2(20)
	int k = ( d0<= d1 && d0 <= d2 && mesh.m_edges[ poly1.edge[0] ].bAtlsSeam ) ? 0 : 
		    ( d1<= d0 && d1 <= d2 && mesh.m_edges[ poly1.edge[1] ].bAtlsSeam ) ? 1 : 
			(                        mesh.m_edges[ poly1.edge[2] ].bAtlsSeam ) ? 2 : -1;
	if( k == -1 ) return EVec3i(-1,-1,-1);


	const int poly1v0 = poly1.vIdx[ k       ];
	const int poly1v1 = poly1.vIdx[ (k+1)%3 ];


	const TEdge &e     = mesh.m_edges[ poly1.edge[k] ];
	const TPoly &poly2 = mesh.m_polys[ (polyI != e.p[0]) ? e.p[0] : e.p[1] ];

	//poly1‚ª¶‰ñ‚è‚È‚Ì‚Å poly 2‚Í‰E‰ñ‚è
	const EVec3d &p2uv0 = mesh.m_uvs[ poly2.tIdx[0] ];
	const EVec3d &p2uv1 = mesh.m_uvs[ poly2.tIdx[1] ];
	const EVec3d &p2uv2 = mesh.m_uvs[ poly2.tIdx[2] ];

	EVec3d uv;
	if ( poly1v0 == poly2.vIdx[1] && poly1v1 == poly2.vIdx[0] ) uv = t0 * (p2uv0 - p2uv1 ) + p2uv1; 
	if ( poly1v0 == poly2.vIdx[2] && poly1v1 == poly2.vIdx[1] ) uv = t1 * (p2uv1 - p2uv2 ) + p2uv2; 
	if ( poly1v0 == poly2.vIdx[0] && poly1v1 == poly2.vIdx[2] ) uv = t2 * (p2uv2 - p2uv0 ) + p2uv0; 

	int u = uv[0] * W;
	int v = uv[1] * H;
	int I = u + v * W;
	if( polyIdTex[I] != -1 ) return EVec3i(u,v,I);
	return EVec3i(-1,-1,-1);
}



void t_getNeighbors(
	const int W,
	const int H,
	const TTexMesh &mesh,
	const int      *polyIdTex,
	const EVec3i   &p, 
	EVec3i   &n1, 
	EVec3i   &n2, 
	EVec3i   &n3,
	EVec3i   &n4)
{
	n1 << -1,-1,-1;
	n2 << -1,-1,-1;
	n3 << -1,-1,-1;
	n4 << -1,-1,-1;

	if (0 <= p[0] )
	{
		n1 << p[0]-1, p[1], p[2]-1;
		if( polyIdTex[n1[2]] == -1 )  n1 = t_getNeighbors(W,H, mesh, polyIdTex, p, n1 );
	}	
	if (p[0] < W )
	{
		n2 << p[0]+1, p[1], p[2]+1;
		if( polyIdTex[n2[2]] == -1 )  n2 = t_getNeighbors(W,H, mesh, polyIdTex, p, n2 );
	}

	if (0 <= p[1] )
	{
		n3 << p[0], p[1]-1, p[2]-W;
		if( polyIdTex[n3[2]] == -1 )  n3 = t_getNeighbors(W,H, mesh, polyIdTex, p, n3 );
	}

	if (p[1] < H)
	{
		n4 << p[0], p[1]+1, p[2]+W;
		if( polyIdTex[n4[2]] == -1 )  n4 = t_getNeighbors(W,H, mesh, polyIdTex, p, n4 );
	}

}











// point p in (f0,f1,f2) is mapped on (t0,t1,t2)
EVec3d t_mapPointByTriangle
(
	const EVec3d &p ,
	const EVec3d &f0, const EVec3d &f1, const EVec3d &f2,
	const EVec3d &t0, const EVec3d &t1, const EVec3d &t2
	)
{
	EVec3d d1 = f1-f0;
	EVec3d d2 = f2-f0;
	EVec3d b  = p -f0;
	double s, t;
	t_solve2by2LinearEquation(d1.dot(d1), d1.dot(d2), 
		                      d2.dot(d1), d2.dot(d2), 
		                      d1.dot(b ), d2.dot(b ), s, t);
	return t0 + s * (t1-t0) + t * (t2-t0);
}
	

EVec3d t_mapPoint_UVto3D(const TTexMesh &mesh, const int trgtPolyIdx, const EVec2d &uv)
{
	const int   *tidx  = mesh.m_polys[ trgtPolyIdx ].tIdx;
	const int   *vidx  = mesh.m_polys[ trgtPolyIdx ].vIdx;
	return t_mapPointByTriangle(uv, 
			mesh.m_uvs  [tidx[0]], mesh.m_uvs  [tidx[1]], mesh.m_uvs  [tidx[2]],
			mesh.m_verts[vidx[0]], mesh.m_verts[vidx[1]], mesh.m_verts[vidx[2]] );
}






EVec3d t_mapPoint_UVto3D(const TTexMesh &mesh, const int trgtPolyIdx, const EVec2d &uv)
{
	const int    *tidx = mesh.m_polys[trgtPolyIdx].tIdx;
	const EVec3d &p0   = mesh.m_uvs[tidx[0]];
	const EVec3d &p1   = mesh.m_uvs[tidx[1]];
	const EVec3d &p2   = mesh.m_uvs[tidx[2]];

	double s, t;
	t_solve2by2LinearEquation(p1[0] - p0[0], p2[0] - p0[0], 
		                      p1[1] - p0[1], p2[1] - p0[1], 
		                      uv[0] - p0[0], uv[1] - p0[1], s, t);
	
	const int   *vidx = mesh.m_polys[trgtPolyIdx].vIdx;
	const EVec3d &v0 = mesh.m_verts[vidx[0]];
	const EVec3d &v1 = mesh.m_verts[vidx[1]];
	const EVec3d &v2 = mesh.m_verts[vidx[2]];
	return v0 + s * (v1-v0) + t * (v2-v0);
}


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
	TImage2D &patch//should be allocated
)
{
	//pixOnTex --> 3D Point 
	EVec3d pixUV( (pixOnTex[0] + 0.5) / W, (pixOnTex[1] + 0.5) / H );
	EVec3d pos3D = t_mapPoint_UVto3D(mesh, pixPolyI, pixUV);

	//compute exponential Mapping
	vector< ExpMapVtx > expMapVs;
	expnentialMapping( mesh, pos3D, pixPolyI, DBL_MAX, expMapVs);

	//sampling pixel by using 

}





















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
)
{
	const int WH = W * H;
	trgtTex.AllocateImage(W,H,0);

	int *polyIdTex = new int[W*H];
	t_genPolygonIDtexture( mesh, W,H, polyIdTex);


	//get initial pixel 
	int startPixel = -1;
	for (int i = 0; i < WH; ++i) if( polyIdTex[i] != -1) {
		startPixel = i; 
		break;
	}

	if (startPixel == -1)
	{
		fprintf(stderr, "no foregrund\n");
		return;
	}

	int *dist = new int[WH];
	for (int i = 0; i < WH; ++i)
	{
		dist[i] = -1;
	}

	
	//grow from it
	list<EVec3i> Q;
	Q.push_back( EVec3i( startPixel%W, startPixel/W, startPixel));
	dist[ startPixel ] = 0;

	while( !Q.empty() )
	{
		EVec3i p = Q.front();
		Q.pop_front();

		TImage2D patch;
		t_getNeighboringPatch(W,H,trgtTex,mesh, p,  5, 0.01, patch);
		

		EVec3i n1, n2, n3, n4;
		t_getNeighbors(W,H,mesh, polyIdTex, p, n1, n2, n3, n4);

		if (n1[0] != -1 && dist[n1[2]] == -1){ dist[n1[2]] = dist[p[2]] + 1; Q.push_back( n1 ); }
		if (n2[0] != -1 && dist[n2[2]] == -1){ dist[n2[2]] = dist[p[2]] + 1; Q.push_back( n2 ); }
		if (n3[0] != -1 && dist[n3[2]] == -1){ dist[n3[2]] = dist[p[2]] + 1; Q.push_back( n3 ); }
		if (n4[0] != -1 && dist[n4[2]] == -1){ dist[n4[2]] = dist[p[2]] + 1; Q.push_back( n4 ); }
	}


	for (int i = 0; i < WH; ++i) if( polyIdTex[i] != -1)
	{
		if( dist[i] == -1) trgtTex.setPix(4*i, 0,0,255,0 );
		else if( dist[i]%80 == 1 || dist[i]%80 == 0) trgtTex.setPix(4*i, 256,0,0,0);
		else trgtTex.setPix(4*i, 64,64,0,0);
	}
}




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
)
{
	const int WH = W * H;
	trgtTex.AllocateImage(W, H,0);

	int *polyIdTex = new int[W*H];
	t_genPolygonIDtexture( mesh, W,H, polyIdTex);




	//get initial pixel 
	int startPixel = -1;
	for (int i = 0; i < WH; ++i) if( polyIdTex[i] != -1) {
		startPixel = i; 
		break;
	}

	if (startPixel == -1)
	{
		fprintf(stderr, "no foregrund\n");
		return;
	}

	int *dist = new int[WH];
	for (int i = 0; i < WH; ++i)
	{
		dist[i] = -1;
	}

	
	//grow from it
	list<EVec3i> Q;
	Q.push_back( EVec3i( startPixel%W, startPixel/W, startPixel));
	dist[ startPixel ] = 0;

	while( !Q.empty() )
	{
		EVec3i p = Q.front();
		Q.pop_front();

		EVec3i n1, n2, n3, n4;
		t_getNeighbors(W,H,mesh, polyIdTex, p, n1, n2, n3, n4);

		if (n1[0] != -1 && dist[n1[2]] == -1){ dist[n1[2]] = dist[p[2]] + 1; Q.push_back( n1 ); }
		if (n2[0] != -1 && dist[n2[2]] == -1){ dist[n2[2]] = dist[p[2]] + 1; Q.push_back( n2 ); }
		if (n3[0] != -1 && dist[n3[2]] == -1){ dist[n3[2]] = dist[p[2]] + 1; Q.push_back( n3 ); }
		if (n4[0] != -1 && dist[n4[2]] == -1){ dist[n4[2]] = dist[p[2]] + 1; Q.push_back( n4 ); }
	}


	for (int i = 0; i < WH; ++i) if( polyIdTex[i] != -1)
	{
		if( dist[i] == -1) trgtTex.setPix(4*i, 0,0,255,0 );
		else if( dist[i]%80 == 1 || dist[i]%80 == 0) trgtTex.setPix(4*i, 256,0,0,0);
		else trgtTex.setPix(4*i, 64,64,0,0);
	}
}
