
// SimpleObjViewerView.cpp : CSimpleObjViewerView �N���X�̎���
//

#include "stdafx.h"
// SHARED_HANDLERS �́A�v���r���[�A�k���ŁA����ь����t�B���^�[ �n���h���[���������Ă��� ATL �v���W�F�N�g�Œ�`�ł��A
// ���̃v���W�F�N�g�Ƃ̃h�L�������g �R�[�h�̋��L���\�ɂ��܂��B
#ifndef SHARED_HANDLERS
#include "SimpleObjViewer.h"
#endif

#include "SimpleObjViewerDoc.h"
#include "SimpleObjViewerView.h"

#include "texsynthesissurface.h"

#include <map>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSimpleObjViewerView

IMPLEMENT_DYNCREATE(CSimpleObjViewerView, CView)

BEGIN_MESSAGE_MAP(CSimpleObjViewerView, CView)
	// �W������R�}���h
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_PAINT()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
END_MESSAGE_MAP()





















// CSimpleObjViewerView �R���X�g���N�V����/�f�X�g���N�V����

CSimpleObjViewerView::CSimpleObjViewerView()
{
	m_bL = m_bR = m_bM = false;

	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, "wavefront obj (*.obj)|*.obj||");
	if (dlg.DoModal() != IDOK) exit(0);
	
	m_mesh.initialize(dlg.GetPathName());

	EVec3d gc = m_mesh.getGravityCenter();
	m_mesh.Translate( -gc );
	m_mesh.updateNormal();

	m_expMap.resize(m_mesh.m_vSize);

	CFileDialog dlg1(TRUE, NULL, NULL, OFN_HIDEREADONLY, "texture (*.bmp;*.jpg)|*.bmp;*.jpg||");
	if (dlg1.DoModal() != IDOK) exit(0);
	bool flip;
	sample.AllocateFromFile( dlg1.GetPathName(), flip, 0);



	//compute flow field
	vector<EVec3d> pFlow( m_mesh.m_pSize );

	for( int i=0; i < m_mesh.m_pSize ; ++i)
	{	
		EVec3d &n = m_mesh.m_p_norms[i];
		pFlow[i] << 1,0,0;
		pFlow[i] = pFlow[i] - pFlow[i].dot(n) * n;
		pFlow[i].normalize();
	}

	TextureSynthesisOnSurface( m_mesh, pFlow, sample, 5, 0.01, 2000, 2000, m_texture);



}



CSimpleObjViewerView::~CSimpleObjViewerView()
{
}

BOOL CSimpleObjViewerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: ���̈ʒu�� CREATESTRUCT cs ���C������ Window �N���X�܂��̓X�^�C����
	//  �C�����Ă��������B

	return CView::PreCreateWindow(cs);
}

// CSimpleObjViewerView �`��

void CSimpleObjViewerView::OnDraw(CDC* /*pDC*/)
{
	CSimpleObjViewerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: ���̏ꏊ�Ƀl�C�e�B�u �f�[�^�p�̕`��R�[�h��ǉ����܂��B
}


// CSimpleObjViewerView ���

BOOL CSimpleObjViewerView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// ����̈������
	return DoPreparePrinting(pInfo);
}

void CSimpleObjViewerView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: ����O�̓��ʂȏ�����������ǉ����Ă��������B
}

void CSimpleObjViewerView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: �����̌㏈����ǉ����Ă��������B
}


// CSimpleObjViewerView �f�f

#ifdef _DEBUG
void CSimpleObjViewerView::AssertValid() const
{
	CView::AssertValid();
}

void CSimpleObjViewerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CSimpleObjViewerDoc* CSimpleObjViewerView::GetDocument() const // �f�o�b�O�ȊO�̃o�[�W�����̓C�����C���ł��B
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSimpleObjViewerDoc)));
	return (CSimpleObjViewerDoc*)m_pDocument;
}
#endif //_DEBUG


// CSimpleObjViewerView ���b�Z�[�W �n���h���[


int CSimpleObjViewerView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_ogl.OnCreate(this);
	m_ogl.SetCam( EVec3d(0,0,10), EVec3d(0,0,0), EVec3d(0,1,0));

	return 0;
}


void CSimpleObjViewerView::OnDestroy()
{
	CView::OnDestroy();
	m_ogl.OnDestroy();

	// TODO: �����Ƀ��b�Z�[�W �n���h���[ �R�[�h��ǉ����܂��B
}


BOOL CSimpleObjViewerView::OnEraseBkgnd(CDC* pDC)
{
	return true;
}



void CSimpleObjViewerView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);
	m_ogl.OnSize(cx,cy);

}



static float diff[4] = {1.0f, 1.0f, 1.0f, 0.5f};
static float ambi[4] = {0.6f, 0.5f, .5f, 0.5f};
static float spec[4] = {1.0f, 1.0f, 1.0f, 0.5f};
static float shin[1] = {10.0f};

void CSimpleObjViewerView::OnPaint()
{
	CPaintDC dc(this); 


	
	m_ogl.OnDrawBegin();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glDisable( GL_LIGHTING );
	glLineWidth(5);
	glBegin( GL_LINES );
		glColor3d(1,0,0); glVertex3d(0,0,0); glVertex3d(10,0,0);
		glColor3d(0,1,0); glVertex3d(0,0,0); glVertex3d(0,10,0);
		glColor3d(0,0,1); glVertex3d(0,0,0); glVertex3d(0,0,10);
	glEnd  ();


	glEnable( GL_LIGHTING );
	glEnable( GL_LIGHT0);
	glEnable( GL_LIGHT1);
	glEnable( GL_LIGHT2);
	glEnable( GL_TEXTURE_2D );

	m_texture.Bind(0);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR , spec);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE  , diff);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT  , ambi);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shin);
	EVec3d *Vs = m_mesh.m_verts  ;
	EVec3d *Ns = m_mesh.m_v_norms;
	TPoly  *Ps = m_mesh.m_polys  ;


	glBegin( GL_TRIANGLES );
	for(int p=0; p < m_mesh.m_pSize; ++p)
	{
		int *idx = Ps[p].vIdx;
		glTexCoord2dv(m_expMap[idx[0]].pos.data()); glNormal3dv(Ns[idx[0]].data()); glVertex3dv(Vs[idx[0]].data());
		glTexCoord2dv(m_expMap[idx[1]].pos.data()); glNormal3dv(Ns[idx[1]].data()); glVertex3dv(Vs[idx[1]].data());
		glTexCoord2dv(m_expMap[idx[2]].pos.data()); glNormal3dv(Ns[idx[2]].data()); glVertex3dv(Vs[idx[2]].data());
	}
	glEnd();

	glDisable( GL_TEXTURE_2D );




	
	static vector<int> visPathVerts(0);
	if ((int)visPathVerts.size() == 0)
	{
		for (int k = 0; k < 200; ++k)
		{
			int i = (int)( m_mesh.m_vSize * rand() / (double)RAND_MAX );
			visPathVerts.push_back(i);
		}
	}
	
	glLineWidth(2);
	
	if (m_expMap.size() != 0)
	{
		glDisable( GL_LIGHTING );
		glColor3d(1,1,0.5);

		for (const auto startS : visPathVerts)
		{
			glBegin( GL_LINE_STRIP );
			for( int piv = startS; piv >= 0; piv = m_expMap[piv].from) glVertex3dv( m_mesh.m_verts[piv].data() );
			glEnd();
		}
	}



	m_ogl.OnDrawEnd();

}












void CSimpleObjViewerView::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_bL = true;
	//m_ogl.BtnDown_Trans(point);
}


void CSimpleObjViewerView::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_bL = false;
}



void CSimpleObjViewerView::OnRButtonDown(UINT nFlags, CPoint point)
{
	m_bR = true;
	m_ogl.ButtonDownForRotate(point);
}


void CSimpleObjViewerView::OnRButtonUp(UINT nFlags, CPoint point)
{
	m_bR = false;
	m_ogl.ButtonUp();
}


void CSimpleObjViewerView::OnMButtonDown(UINT nFlags, CPoint point)
{
	m_bM = true;
	m_ogl.ButtonDownForZoom(point);
}

void CSimpleObjViewerView::OnMButtonUp(UINT nFlags, CPoint point)
{
	m_bM = false;
	m_ogl.ButtonUp();
}



void CSimpleObjViewerView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bL)
	{
		int polyIdx;
		EVec3d rayP,rayD, pos;
		m_ogl.GetCursorRay( point , rayP, rayD);

		if (m_mesh.pickByRay(rayP, rayD, pos, polyIdx))
		{
			expnentialMapping( m_mesh, pos, polyIdx, m_expMap);

			double scale = 0.03;
			for( auto &p : m_expMap)
			{
				p.pos *= scale;
				p.pos += EVec2d(0.5f, 0.5f);

				if( p.pos[0] < 0.1) p.pos[0] = 0.1;
				if( p.pos[1] < 0.1) p.pos[1] = 0.1;
				if( p.pos[0] > 0.9) p.pos[0] = 0.9;
				if( p.pos[1] > 0.9) p.pos[1] = 0.9;

			}
		}
		m_ogl.RedrawWindow();
	}


	if (m_bR || m_bM)
	{
		m_ogl.MouseMove( point );
		m_ogl.RedrawWindow();
	}

	CView::OnMouseMove(nFlags, point);
}







