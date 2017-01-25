
// SimpleObjViewerView.cpp : CSimpleObjViewerView クラスの実装
//

#include "stdafx.h"
// SHARED_HANDLERS は、プレビュー、縮小版、および検索フィルター ハンドラーを実装している ATL プロジェクトで定義でき、
// そのプロジェクトとのドキュメント コードの共有を可能にします。
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
	// 標準印刷コマンド
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





















// CSimpleObjViewerView コンストラクション/デストラクション

CSimpleObjViewerView::CSimpleObjViewerView()
{
	m_bL = m_bR = m_bM = false;

	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, "wavefront obj (*.obj)|*.obj||");
	if (dlg.DoModal() != IDOK) exit(0);
	
	m_mesh.initialize(dlg.GetPathName());

	EVec3d gc = m_mesh.getGravityCenter();
	m_mesh.Translate( -gc );
	m_mesh.updateNormal();


	TImage2D sampleTex;
	CFileDialog dlg1(TRUE, NULL, NULL, OFN_HIDEREADONLY, "texture (*.bmp;*.jpg)|*.bmp;*.jpg||");
	if (dlg1.DoModal() != IDOK) exit(0);
	bool flip;
	sampleTex.AllocateFromFile( dlg1.GetPathName(), flip, 0);



	//compute flow field
	vector<EVec3d> pFlow( m_mesh.m_pSize );

	for( int i=0; i < m_mesh.m_pSize ; ++i)
	{	
		EVec3d &n = m_mesh.m_p_norms[i];
		pFlow[i] << 1,0,0;
		pFlow[i] = pFlow[i] - pFlow[i].dot(n) * n;
		pFlow[i].normalize();
	}

	//m_texture = sampleTex;
	TextureSynthesisOnSurface( m_mesh, pFlow, sampleTex, 5, 0.01, 2000, 2000, m_texture);



}



CSimpleObjViewerView::~CSimpleObjViewerView()
{
}

BOOL CSimpleObjViewerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: この位置で CREATESTRUCT cs を修正して Window クラスまたはスタイルを
	//  修正してください。

	return CView::PreCreateWindow(cs);
}

// CSimpleObjViewerView 描画

void CSimpleObjViewerView::OnDraw(CDC* /*pDC*/)
{
	CSimpleObjViewerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: この場所にネイティブ データ用の描画コードを追加します。
}


// CSimpleObjViewerView 印刷

BOOL CSimpleObjViewerView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 既定の印刷準備
	return DoPreparePrinting(pInfo);
}

void CSimpleObjViewerView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 印刷前の特別な初期化処理を追加してください。
}

void CSimpleObjViewerView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 印刷後の後処理を追加してください。
}


// CSimpleObjViewerView 診断

#ifdef _DEBUG
void CSimpleObjViewerView::AssertValid() const
{
	CView::AssertValid();
}

void CSimpleObjViewerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CSimpleObjViewerDoc* CSimpleObjViewerView::GetDocument() const // デバッグ以外のバージョンはインラインです。
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSimpleObjViewerDoc)));
	return (CSimpleObjViewerDoc*)m_pDocument;
}
#endif //_DEBUG


// CSimpleObjViewerView メッセージ ハンドラー


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

	// TODO: ここにメッセージ ハンドラー コードを追加します。
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

	EVec3d *Ts = m_mesh.m_uvs    ;

	glBegin( GL_TRIANGLES );
	for(int p=0; p < m_mesh.m_pSize; ++p)
	{
		int *vidx = Ps[p].vIdx;
		int *tidx = Ps[p].tIdx;
		glTexCoord2dv( Ts[tidx [0]].data()); glNormal3dv(Ns[vidx[0]].data()); glVertex3dv(Vs[vidx[0]].data());
		glTexCoord2dv( Ts[tidx [1]].data()); glNormal3dv(Ns[vidx[1]].data()); glVertex3dv(Vs[vidx[1]].data());
		glTexCoord2dv( Ts[tidx [2]].data()); glNormal3dv(Ns[vidx[2]].data()); glVertex3dv(Vs[vidx[2]].data());
	}
	glEnd();


	glDisable( GL_TEXTURE_2D );
	glDisable(GL_LIGHTING );
	glBegin( GL_LINES );
	for(int ei=0; ei < m_mesh.m_eSize; ++ei)
	{
		int* v=m_mesh.m_edges[ei].v;

		if( m_mesh.m_edges[ei].bAtlsSeam ) glColor3d(1,0,0);
		else                              continue; 
		glVertex3dv(Vs[v[0]].data());
		glVertex3dv(Vs[v[1]].data());
	}
	glEnd();
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







