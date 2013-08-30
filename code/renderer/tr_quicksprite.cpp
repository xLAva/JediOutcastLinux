// tr_QuickSprite.cpp: implementation of the CQuickSpriteSystem class.
//
//////////////////////////////////////////////////////////////////////
#include "../server/exe_headers.h"
#include "tr_quicksprite.h"

extern void R_BindAnimatedImage( const textureBundle_t *bundle );


//////////////////////////////////////////////////////////////////////
// Singleton System
//////////////////////////////////////////////////////////////////////
CQuickSpriteSystem SQuickSprite;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CQuickSpriteSystem::CQuickSpriteSystem(void)
{
	int i;

	for (i = 0; i < SHADER_MAX_VERTEXES; i += 4)
	{
		// Bottom right
		mTextureCoords[i + 0][0] = 1.0;
		mTextureCoords[i + 0][1] = 1.0;
		// Top right
		mTextureCoords[i + 1][0] = 1.0;
		mTextureCoords[i + 1][1] = 0.0;
		// Top left
		mTextureCoords[i + 2][0] = 0.0;
		mTextureCoords[i + 2][1] = 0.0;
		// Bottom left
		mTextureCoords[i + 3][0] = 0.0;
		mTextureCoords[i + 3][1] = 1.0;
	}
}

CQuickSpriteSystem::~CQuickSpriteSystem(void)
{
}


void CQuickSpriteSystem::Flush(void)
{
	if (mNextVert==0)
	{
		return;
	}

	//
	// render the main pass
	//
	R_BindAnimatedImage( mTexBundle );
	GL_State(mGLStateBits);

	//
	// set arrays and lock
	//
	#ifdef HAVE_GLES
	unsigned short indexes[(mNextVert/2)*3];
	int idx;
	// split TRIANGLE_FAN in 2 TRIANGLES
	qglEnableClientState( GL_TEXTURE_COORD_ARRAY);
	qglEnableClientState( GL_COLOR_ARRAY);
	/*for (int i=0; i<mNextVert; i+=4) {
		qglTexCoordPointer(2, GL_FLOAT, 0, mTextureCoords+i);
		qglColorPointer( 4, GL_UNSIGNED_BYTE, 0, mColors+i );
		qglVertexPointer (3, GL_FLOAT, 16, mVerts+i);
		qglDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	}*/
	qglTexCoordPointer(2, GL_FLOAT, 0, mTextureCoords);
	qglColorPointer( 4, GL_UNSIGNED_BYTE, 0, mColors );
	qglVertexPointer (3, GL_FLOAT, 16, mVerts);
	idx = 0;
	for (int i=0; i<mNextVert; i+=4) {
		// Triangle 1
		indexes[idx+0]=i;
		indexes[idx+1]=i+1;
		indexes[idx+2]=i+2;
		idx+=3;
		// Triangle 2
		indexes[idx+0]=i;
		indexes[idx+1]=i+2;
		indexes[idx+2]=i+3;
		idx+=3;
	}
	qglDrawElements(GL_TRIANGLES, idx, GL_UNSIGNED_SHORT, indexes);
	
	#else
	qglEnableClientState( GL_TEXTURE_COORD_ARRAY);
	qglTexCoordPointer( 2, GL_FLOAT, 0, mTextureCoords );

	qglEnableClientState( GL_COLOR_ARRAY);
	qglColorPointer( 4, GL_UNSIGNED_BYTE, 0, mColors );

	qglVertexPointer (3, GL_FLOAT, 16, mVerts);

	if ( qglLockArraysEXT )
	{
		qglLockArraysEXT(0, mNextVert);
		GLimp_LogComment( "glLockArraysEXT\n" );
	}

	qglDrawArrays(GL_QUADS, 0, mNextVert);
	#endif

	backEnd.pc.c_vertexes += mNextVert;
	backEnd.pc.c_indexes += mNextVert;
	backEnd.pc.c_totalIndexes += mNextVert;

	if (mUseFog)
	{
		//
		// render the fog pass
		//
		GL_Bind( tr.fogImage );
		GL_State( GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA | GLS_DEPTHFUNC_EQUAL );

		//
		// set arrays and lock
		//
		#ifdef HAVE_GLES
		qglDisableClientState( GL_COLOR_ARRAY );
		qglColor4ubv((GLubyte *)&mFogColor);
		/*for (int i=0; i<mNextVert; i+=4) {
			qglTexCoordPointer(2, GL_FLOAT, 0, mFogTextureCoords+i);
			qglVertexPointer (3, GL_FLOAT, 16, mVerts+i);
			qglDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		}*/
		qglDrawElements(GL_TRIANGLES, idx, GL_UNSIGNED_SHORT, indexes);
		#else
		qglTexCoordPointer( 2, GL_FLOAT, 0, mFogTextureCoords);
//		qglEnableClientState( GL_TEXTURE_COORD_ARRAY);	// Done above

		qglDisableClientState( GL_COLOR_ARRAY );
		qglColor4ubv((GLubyte *)&mFogColor);

//		qglVertexPointer (3, GL_FLOAT, 16, mVerts);	// Done above

		qglDrawArrays(GL_QUADS, 0, mNextVert);
		#endif

		// Second pass from fog
		backEnd.pc.c_totalIndexes += mNextVert;
	}

	// 
	// unlock arrays
	//
	if (qglUnlockArraysEXT) 
	{
		qglUnlockArraysEXT();
		GLimp_LogComment( "glUnlockArraysEXT\n" );
	}

	mNextVert=0;
}


void CQuickSpriteSystem::StartGroup(textureBundle_t *bundle, unsigned long glbits, unsigned long fogcolor )
{
	mNextVert = 0;

	mTexBundle = bundle;
	mGLStateBits = glbits;
	if (fogcolor)
	{
		mUseFog = qtrue;
		mFogColor = fogcolor;
	}
	else
	{
		mUseFog = qfalse;
	}

	int cullingOn;
	qglGetIntegerv(GL_CULL_FACE,&cullingOn);

	if(cullingOn)
	{
		mTurnCullBackOn=true;
	}
	else
	{
		mTurnCullBackOn=false;
	}
	qglDisable(GL_CULL_FACE);
}


void CQuickSpriteSystem::EndGroup(void)
{
	Flush();

	qglColor4ub(255,255,255,255);
	if(mTurnCullBackOn)
	{
		qglEnable(GL_CULL_FACE);
	}
}




void CQuickSpriteSystem::Add(float *pointdata, color4ub_t color, vec2_t fog)
{
	float *curcoord;
	float *curfogtexcoord;
	unsigned long *curcolor;

	if (mNextVert>SHADER_MAX_VERTEXES-4)
	{
		Flush();
	}

	curcoord = mVerts[mNextVert];
	memcpy(curcoord, pointdata, 4*sizeof(vec4_t));

	// Set up color
	curcolor = &mColors[mNextVert];
	*curcolor++ = *(unsigned long *)color;
	*curcolor++ = *(unsigned long *)color;
	*curcolor++ = *(unsigned long *)color;
	*curcolor++ = *(unsigned long *)color;

	if (fog)
	{
		curfogtexcoord = &mFogTextureCoords[mNextVert][0];
		*curfogtexcoord++ = fog[0];
		*curfogtexcoord++ = fog[1];

		*curfogtexcoord++ = fog[0];
		*curfogtexcoord++ = fog[1];

		*curfogtexcoord++ = fog[0];
		*curfogtexcoord++ = fog[1];

		*curfogtexcoord++ = fog[0];
		*curfogtexcoord++ = fog[1];

		mUseFog=qtrue;
	}
	else
	{
		mUseFog=qfalse;
	}

	mNextVert+=4;
}
