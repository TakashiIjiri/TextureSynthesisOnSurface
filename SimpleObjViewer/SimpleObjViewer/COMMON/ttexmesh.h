#pragma once

#pragma warning(disable:4996)

#include "togl.h"
#include "timage.h"
#include <list>
#include <vector>
using namespace std;


class TPoly
{
public:
	int vIdx[3];
	int tIdx[3];

    TPoly(int v0=0, int v1=0, int v2=0, int t0=0, int t1=0, int t2=0){ 
		vIdx[0] = v0; vIdx[1] = v1; vIdx[2] = v2;
		tIdx[0] = t0; tIdx[1] = t1; tIdx[2] = t2;
	}
    TPoly(const TPoly &p ){ Set(p); }

	void Set( const TPoly &p){
		memcpy( vIdx, p.vIdx, sizeof( int ) * 3);
		memcpy( tIdx, p.tIdx, sizeof( int ) * 3);
	}

	TPoly& operator= (const TPoly  &v){ Set(v); return *this; }
};



inline bool t_intersectRayToTriangle
(
	const EVec3d &rayP,
	const EVec3d &rayD,
	const EVec3d &x0,
	const EVec3d &x1,
	const EVec3d &x2,
	EVec3d &pos
	)
{
	Eigen::Matrix3d A;
	A << x1[0] - x0[0], x2[0] - x0[0], -rayD[0],
		 x1[1] - x0[1], x2[1] - x0[1], -rayD[1],
		 x1[2] - x0[2], x2[2] - x0[2], -rayD[2];

	EVec3d stu = A.inverse()*(rayP - x0);

	if (0 <= stu[0] && stu[0] <= 1 &&
		0 <= stu[1] && stu[1] <= 1 &&
		0 <= stu[0] + stu[1] && stu[0] + stu[1] <= 1)
	{
		pos = rayP + stu[2] * rayD;
		return true;
	}

	return false;
}






class TTexMesh
{
public:
	int    m_vSize ;
	EVec3d *m_verts  ;
	EVec3d *m_v_norms;

	vector<vector<int>> m_v_RingPs;
	vector<vector<int>> m_v_RingVs;


	int m_uvSize;
	EVec3d *m_uvs    ;

	int m_pSize     ;
	TPoly *m_polys  ;
	EVec3d *m_p_norms;



	TTexMesh()
	{
		m_vSize   = 0;
		m_verts   = 0;
		m_v_norms = 0;
	
		m_uvSize  = 0;
		m_uvs     = 0;

		m_pSize   = 0;
		m_polys   = 0;
	}
	TTexMesh(const TTexMesh &src) { Set(src); }
	TTexMesh& operator=(const TTexMesh &src) { Set(src); return *this; }

	~TTexMesh()
	{
		clear();
	}

	void clear()
	{
		m_vSize  = 0;
		m_uvSize = 0;
		m_pSize  = 0;
		if( m_verts   != 0) delete[] m_verts ;
		if( m_v_norms != 0) delete[] m_v_norms ;
		if( m_uvs     != 0) delete[] m_uvs ;
		if( m_polys   != 0) delete[] m_polys ;
	}

	void Set( const TTexMesh &m)
	{
		clear();
		m_vSize = m.m_vSize;
		if (m_vSize != 0)
		{
			m_verts = new EVec3d[m_vSize];
			m_v_norms = new EVec3d[m_vSize];
			memcpy(m_verts, m.m_verts, sizeof(EVec3d)*m_vSize);
			memcpy(m_v_norms, m.m_v_norms, sizeof(EVec3d)*m_vSize);
		}

		m_uvSize  = m.m_uvSize;
		if (m_uvSize != 0)
		{
			m_uvs = new EVec3d[m_uvSize];
			memcpy(m_uvs, m.m_uvs, sizeof(EVec3d)*m_uvSize);
		}

		m_pSize   = m.m_pSize;
		if (m_pSize != 0)
		{
			m_polys = new TPoly[m_pSize];
			memcpy(m_polys, m.m_polys, sizeof(TPoly)*m_pSize);
		}
		m_v_RingPs = m.m_v_RingPs;
		m_v_RingVs = m.m_v_RingVs;
	}


	bool initialize(const char *fName)
	{	
		FILE* fp = fopen(fName,"r") ;
		if( !fp ) return false;

		list<EVec3d>  vs_list   , uvs_list;
		list<TPoly >  polys_list;


		char buf[512] ;		
		while( fgets(buf,255,fp) )
		{
			char* bkup  = _strdup( buf        );
			char* token =  strtok( buf, " \t" );


			if( stricmp( token,"vt" ) == 0 )
			{ 
				EVec3d vt ;
				sscanf( bkup,"vt %lf %lf",&vt[0], &vt[1] ) ;
				uvs_list.push_back( vt ) ;
			} 
			else if( stricmp( token,"v" ) == 0 )
			{ 
				EVec3d v;
				sscanf( bkup,"v %lf %lf %lf",&v[0],&v[1],&v[2] ) ;
				vs_list.push_back( v ) ;
			} 
			else if( stricmp( token,"f" ) == 0 )
			{
				TPoly p(0,0,0,0,0,0);
				int tmp;

				int vtxnum = sscanf( bkup,"f %d %d %d %d", &p.vIdx[0], &p.vIdx[1], &p.vIdx[2], &tmp) ;//sscanfの返り値は正常に読めたフィールド数 (/が入ったら2フィールドしか読めない)

				//t_info( "(%d %d %d)", p.vIdx[0], p.vIdx[1], p.vIdx[2]);
			
				if( vtxnum < 3 ) vtxnum = sscanf( bkup,"f %d/%d %d/%d %d/%d %d/%d" ,            &p.vIdx[0], &p.tIdx[0],
																								&p.vIdx[1], &p.tIdx[1],
																								&p.vIdx[2], &p.tIdx[2], &tmp, &tmp)/2 ;

				if( vtxnum < 3 ) vtxnum = sscanf( bkup,"f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d", &p.vIdx[0], &p.tIdx[0], &tmp,
																								&p.vIdx[1], &p.tIdx[1], &tmp,
																								&p.vIdx[2], &p.tIdx[2], &tmp, &tmp, &tmp, &tmp)/3 ;
				if( vtxnum < 3 ) vtxnum = sscanf( bkup,"f %d//%d %d//%d %d//%d %d//%d" ,        &p.vIdx[0], &tmp,  
																								&p.vIdx[1], &tmp,
																								&p.vIdx[2], &tmp,  &tmp, &tmp )/2 ;
				--p.vIdx[0]; --p.vIdx[1]; --p.vIdx[2];
				--p.tIdx[0]; --p.tIdx[1]; --p.tIdx[2];
				polys_list  .push_back( p ) ;
			}

			free(bkup) ;
		}
		fclose(fp) ;



		m_vSize   = (int) vs_list   .size();
		m_uvSize  = (int) uvs_list  .size();
		m_pSize   = (int) polys_list.size();
		fprintf(stderr, "loaded object file info : %d %d %d\n", m_vSize, m_uvSize, m_pSize);
	
		m_verts   = new EVec3d[m_vSize ];
		m_v_norms = new EVec3d[m_vSize ];
		m_polys   = new TPoly [m_pSize ];
		m_p_norms = new EVec3d[m_pSize ];
		m_uvs     = new EVec3d[m_uvSize];

		int c = 0;
		for( auto it = vs_list   .begin(); it != vs_list   .end(); ++it) m_verts[c++] = *it;
		c = 0;
		for( auto it = uvs_list  .begin(); it != uvs_list  .end(); ++it) m_uvs  [c++] = *it;
		c = 0;
		for( auto it = polys_list.begin(); it != polys_list.end(); ++it) m_polys[c++] = *it;


		updateNormal();
		updateRingInfo();
		fprintf(stderr, "loaded object file info : %d %d %d\n", m_vSize, m_uvSize, m_pSize);
		return true;
	}


	void updateNormal()
	{
		for(int i=0; i<m_vSize; ++i) m_v_norms[i].Zero();


		for(int i=0; i<m_pSize; ++i)
		{
			int *idx = m_polys[i].vIdx;
			m_p_norms[i] = (m_verts[ idx[1] ] - m_verts[ idx[0] ]).cross( m_verts[ idx[2] ] - m_verts[ idx[0] ]);

			if( m_p_norms[i].norm() > 0 )
			{
				m_p_norms[i].normalize();
				m_v_norms[ idx[0] ] += m_p_norms[i];
				m_v_norms[ idx[1] ] += m_p_norms[i];
				m_v_norms[ idx[2] ] += m_p_norms[i];
			}
		}

		for(int i=0; i<m_vSize; ++i) m_v_norms[i].normalize();
	}


	
	void updateRingInfo()
	{
		m_v_RingPs.clear();
		m_v_RingVs.clear();
		m_v_RingPs.resize(m_vSize);
		m_v_RingVs.resize(m_vSize);

		for( int i =0; i < m_pSize; ++i)
		{
			int *idx = m_polys[i].vIdx;
			m_v_RingPs[ idx[0] ].push_back(i);
			m_v_RingPs[ idx[1] ].push_back(i);
			m_v_RingPs[ idx[2] ].push_back(i);
			m_v_RingVs[ idx[0] ].push_back(idx[1]);
			m_v_RingVs[ idx[0] ].push_back(idx[2]);
			m_v_RingVs[ idx[1] ].push_back(idx[0]);
			m_v_RingVs[ idx[1] ].push_back(idx[2]);
			m_v_RingVs[ idx[2] ].push_back(idx[0]);
			m_v_RingVs[ idx[2] ].push_back(idx[1]);
		}

 		for (int i = 0; i < m_vSize; ++i) 
		{
			sort  (m_v_RingVs[i].begin(), m_v_RingVs[i].end());
			auto it = unique(m_v_RingVs[i].begin(), m_v_RingVs[i].end());
			m_v_RingVs[i].erase( it, m_v_RingVs[i].end());
		}
	}
		




	void drawWithTex( TImage2D &img )
	{
		img.Bind(0);
		glEnable( GL_TEXTURE_2D );
		glEnable( GL_LIGHTING   );

		glBegin ( GL_TRIANGLES  );

		const EVec3d *verts = m_verts  ;
		const EVec3d *norms = m_v_norms;
		const EVec3d *uvs   = m_uvs    ;

		for (int i = 0; i < m_pSize; ++i)
		{
			const int *idx   = m_polys[i].vIdx;
			const int *uvIdx = m_polys[i].tIdx;
			glNormal3dv( norms[idx[0] ].data()); glTexCoord2dv( uvs[uvIdx[0]].data()); glVertex3dv( verts[idx[0]].data());
			glNormal3dv( norms[idx[1] ].data()); glTexCoord2dv( uvs[uvIdx[1]].data()); glVertex3dv( verts[idx[1]].data());
			glNormal3dv( norms[idx[2] ].data()); glTexCoord2dv( uvs[uvIdx[2]].data()); glVertex3dv( verts[idx[2]].data());
		}
		glEnd();

		glDisable( GL_TEXTURE_2D );
	}
	
	void drawEdge( float lineWidth)
	{
		glColor3d(1,1,1);
		glLineWidth( lineWidth );

		glDisable( GL_LIGHTING   );

		glBegin  ( GL_LINES);

		const EVec3d *verts = m_verts  ;

		for (int i = 0; i < m_pSize; ++i)
		{
			const int *idx   = m_polys[i].vIdx;
			glVertex3dv( verts[idx[0]].data()); glVertex3dv( verts[idx[1]].data());
			glVertex3dv( verts[idx[1]].data()); glVertex3dv( verts[idx[2]].data());
			glVertex3dv( verts[idx[2]].data()); glVertex3dv( verts[idx[0]].data());
		}
		glEnd();
	}




	void Translate(EVec3d t)
	{
		for( int i=0; i < m_vSize; ++i ) m_verts[i] += t;
	}

	void Scale(double d)
	{
		for( int i=0; i < m_vSize; ++i ) m_verts[i] *= d;
	}



	EVec3d getGravityCenter()
	{
		EVec3d p;
		for( int i=0; i < m_vSize; ++i ) p += m_verts[i];
		return p / (double)m_vSize;
	}



	//compute collision to all polygons ald (p0-p1)
	//ignor back surface (p1-p0) * norm > 0
	bool intersectToLineSegment_ignorBackSurface(const EVec3d &p0, const EVec3d &p1, int ignorVid) const
	{
		EMat3d M;
		EVec3d b, stk, cRay(p1 - p0);

		for (int i = 0; i < m_pSize; ++i)
		{
			if (m_polys[i].vIdx[0] == ignorVid ||
				m_polys[i].vIdx[1] == ignorVid ||
				m_polys[i].vIdx[2] == ignorVid || cRay.dot(m_p_norms[i]) > 0) continue;

			const EVec3d &v0 = m_verts[m_polys[i].vIdx[0]];
			const EVec3d &v1 = m_verts[m_polys[i].vIdx[1]];
			const EVec3d &v2 = m_verts[m_polys[i].vIdx[2]];

			M << v1[0] - v0[0], v2[0] - v0[0], -(p1[0] - p0[0]),
				 v1[1] - v0[1], v2[1] - v0[1], -(p1[1] - p0[1]),
				 v1[2] - v0[2], v2[2] - v0[2], -(p1[2] - p0[2]);

			if (M.determinant() == 0) continue;
			stk = M.partialPivLu().solve( p0 - v0 );

			if (0 <= stk[0] && 0 <= stk[1] && stk[0] + stk[1] <= 1 && 0 <= stk[2] && stk[2] <= 1) return true;
		}


		return false;
	}




	
	bool pickByRay( const EVec3d &rayP, const EVec3d &rayD, EVec3d &pos, int &pIdx ) const
	{
		double depth = DBL_MAX;
		EVec3d tmpPos;
		pIdx  = -1;
		for (int pi = 0; pi < m_pSize; ++pi)
		{
			const int *p = m_polys[pi].vIdx;

			if (t_intersectRayToTriangle( rayP, rayD, m_verts[p[0]], m_verts[p[1]], m_verts[p[2]], tmpPos) )
			{
				double d = (tmpPos - rayP).norm();
				if (d < depth)
				{
					depth = d;
					pos   = tmpPos;
					pIdx  = pi;
				}
			}
		}
		return depth != DBL_MAX;
	}



};





